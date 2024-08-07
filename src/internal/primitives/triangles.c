/**
 *  Copyright (c) 2024 Le Juez Victor
 *
 *  This software is provided "as-is", without any express or implied warranty. In no event 
 *  will the authors be held liable for any damages arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose, including commercial 
 *  applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not claim that you 
 *  wrote the original software. If you use this software in a product, an acknowledgment 
 *  in the product documentation would be appreciated but is not required.
 *
 *  2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *  as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "../lighting/lighting.h"
#include "../context/context.h"
#include "../../pfm.h"
#include "../helper.h"
#include "../color.h"
#include "../blend.h"

#define PF_TRIANGLE_RASTER_BRAYCENTRIC_SIMD     1       ///< Can also use OpenMP (if available) in addition to SIMD support
#define PF_TRIANGLE_RASTER_BARYCENTRIC_OMP      2       ///< Only uses OpenMP if SIMD support is not available
#define PF_TRIANGLE_RASTER_SCANLINES            3       ///< Uses neither OpenMP nor SIMD support.

#if PF_SIMD_SIZE > 1
#   define PF_TRIANGLE_RASTER_MODE \
        PF_TRIANGLE_RASTER_BRAYCENTRIC_SIMD
#elif defined(_OPENMP)
#   define PF_TRIANGLE_RASTER_MODE \
        PF_TRIANGLE_RASTER_BARYCENTRIC_OMP
#else
#   define PF_TRIANGLE_RASTER_MODE \
        PF_TRIANGLE_RASTER_SCANLINES
#endif

/* External Functions */

extern PFMsimd_i
pfTextureSampleNearestWrapSimd(const PFtexture texture, const PFMsimd_vec2 texcoords);

/* Internal typedefs */

#if PF_TRIANGLE_RASTER_MODE == PF_TRIANGLE_RASTER_SCANLINES
typedef PFcolor (*InterpolateColorFunc)(PFcolor, PFcolor, PFfloat);
#else //PF_TRIANGLE_RASTER_BARYCENTRIC
typedef PFcolor (*InterpolateColorFunc)(PFcolor, PFcolor, PFcolor, PFfloat, PFfloat, PFfloat);
typedef void (*InterpolateColorSimdFunc)(PFsimd_color, const PFsimd_color, const PFsimd_color, const PFsimd_color, PFMsimd_f, PFMsimd_f, PFMsimd_f);
#endif //PF_RASTER_MODE

/* Internal helper function declarations */

#if PF_TRIANGLE_RASTER_MODE == PF_TRIANGLE_RASTER_SCANLINES
static PFboolean Helper_FaceCanBeRendered(PFface faceToRender, PFfloat* area, const PFMvec2 p1, const PFMvec2 p2, const PFMvec2 p3);
static void Helper_SortVertices(const PFvertex** v1, const PFvertex** v2, const PFvertex** v3);
#endif //PF_TRIANGLE_RASTER_SCANLINES

/* Internal triangle processing functions declarations */

static PFboolean Process_ClipPolygonW(PFvertex* polygon, int_fast8_t* vertexCounter);
static PFboolean Process_ClipPolygonXYZ(PFvertex* polygon, int_fast8_t* vertexCounter);
static PFboolean Process_ProjectAndClipTriangle(PFvertex* polygon, int_fast8_t* vertexCounter);

/* Internal triangle rasterizer function declarations */

static void Rasterize_Triangle(PFface faceToRender, PFboolean is3D,
                               const PFvertex* v1, const PFvertex* v2, const PFvertex* v3,
                               const PFMvec3 viewPos);


/* Line Process And Rasterize Function */

// NOTE: An array of vertices with a total size equal to 'PF_MAX_CLIPPED_POLYGON_VERTICES' must be provided as a parameter
//       with only the first three vertices defined; the extra space is used in case the triangle needs to be clipped.
static void pfInternal_ProcessRasterize_TRIANGLE_IMPL(PFface faceToRender, PFvertex processed[PF_MAX_CLIPPED_POLYGON_VERTICES])
{
#ifndef NDEBUG
    if (faceToRender == PF_FRONT_AND_BACK)
    {
        // WARNING: This should not be called with PF_FRONT_AND_BACK
        currentCtx->errCode = PF_DEBUG_INVALID_OPERATION;
        return;
    }
#endif

    PFboolean lighting = (currentCtx->state & PF_LIGHTING) &&
                         (currentCtx->activeLights != NULL);

    int_fast8_t processedCounter = 3;

    // Performs certain operations that must be done before
    // processing the vertices in case of light management

    if (lighting)
    {
        // Transform normals
        // And multiply vertex color with diffuse color
        for (int_fast8_t i = 0; i < processedCounter; i++)
        {
            pfmVec3Transform(processed[i].normal, processed[i].normal, currentCtx->matNormal);
            pfmVec3Normalize(processed[i].normal, processed[i].normal); // REVIEW: Only with PF_NORMALIZE state??

            processed[i].color = pfInternal_BlendMultiplicative(processed[i].color,
                currentCtx->faceMaterial[faceToRender].diffuse);
        }
    }

    // Process vertices

    PFboolean is3D = Process_ProjectAndClipTriangle(processed, &processedCounter);
    if (processedCounter < 3) return;

    // Rasterize filled triangles

    PFMvec3 viewPos = { 0 };

    if (lighting)
    {
        PFMmat4 invMatView;
        pfmMat4Invert(invMatView, currentCtx->matView);
        pfmVec3Copy(viewPos, invMatView + 12);
    }

    for (int_fast8_t i = 0; i < processedCounter - 2; i++)
    {
        Rasterize_Triangle(faceToRender, is3D, &processed[0], &processed[i + 1], &processed[i + 2], viewPos);
    }
}

void pfInternal_ProcessRasterize_TRIANGLE(PFface faceToRender)
{
    PFvertex processed[PF_MAX_CLIPPED_POLYGON_VERTICES];
    memcpy(processed, currentCtx->vertexBuffer, 3 * sizeof(PFvertex));
    pfInternal_ProcessRasterize_TRIANGLE_IMPL(faceToRender, processed);
}

void pfInternal_ProcessRasterize_TRIANGLE_FAN(PFface faceToRender, int_fast8_t numTriangles)
{
    for (int_fast8_t i = 0; i < numTriangles; i++)
    {
        PFvertex processed[PF_MAX_CLIPPED_POLYGON_VERTICES] = {
            currentCtx->vertexBuffer[0],
            currentCtx->vertexBuffer[i + 1],
            currentCtx->vertexBuffer[i + 2]
        };

        pfInternal_ProcessRasterize_TRIANGLE_IMPL(faceToRender, processed);
    }
}

void pfInternal_ProcessRasterize_TRIANGLE_STRIP(PFface faceToRender, int_fast8_t numTriangles)
{
    for (int_fast8_t i = 0; i < numTriangles; i++)
    {
        PFvertex processed[PF_MAX_CLIPPED_POLYGON_VERTICES];

        if (i % 2 == 0)
        {
            processed[0] = currentCtx->vertexBuffer[i];
            processed[1] = currentCtx->vertexBuffer[i + 1];
            processed[2] = currentCtx->vertexBuffer[i + 2];
        }
        else
        {
            processed[0] = currentCtx->vertexBuffer[i + 2];
            processed[1] = currentCtx->vertexBuffer[i + 1];
            processed[2] = currentCtx->vertexBuffer[i];
        }

        pfInternal_ProcessRasterize_TRIANGLE_IMPL(faceToRender, processed);
    }
}


/* Internal helper function definitions */

#if PF_TRIANGLE_RASTER_MODE == PF_TRIANGLE_RASTER_SCANLINES

PFboolean Helper_FaceCanBeRendered(PFface faceToRender, PFfloat* area, const PFMvec2 p1, const PFMvec2 p2, const PFMvec2 p3)
{
    PFfloat signedArea = (p2[0] - p1[0])*(p3[1] - p1[1]) - (p3[0] - p1[0])*(p2[1] - p1[1]);

    if ((faceToRender == PF_FRONT && signedArea < 0) || (faceToRender == PF_BACK && signedArea > 0))
    {
        *area = fabsf(signedArea)*0.5f;
        return PF_TRUE;
    }

    return PF_FALSE;
}

void Helper_SortVertices(const PFvertex** v1, const PFvertex** v2, const PFvertex** v3)
{
    // Sort vertices in ascending order of y coordinates
    const PFvertex* vTmp;
    if ((*v2)->screen[1] < (*v1)->screen[1]) { vTmp = *v1; *v1 = *v2; *v2 = vTmp; }
    if ((*v3)->screen[1] < (*v1)->screen[1]) { vTmp = *v1; *v1 = *v3; *v3 = vTmp; }
    if ((*v3)->screen[1] < (*v2)->screen[1]) { vTmp = *v2; *v2 = *v3; *v3 = vTmp; }
}

#endif //PF_TRIANGLE_RASTER_SCANLINES

/* Internal triangle processing functions definitions */

PFboolean Process_ClipPolygonW(PFvertex* polygon, int_fast8_t* vertexCounter)
{
    PFvertex input[PF_MAX_CLIPPED_POLYGON_VERTICES];
    memcpy(input, polygon, (*vertexCounter)*sizeof(PFvertex));

    int_fast8_t inputCounter = *vertexCounter;
    *vertexCounter = 0;

    const PFvertex *prevVt = &input[inputCounter-1];
    PFbyte prevDot = (prevVt->homogeneous[3] < PF_CLIP_EPSILON) ? -1 : 1;

    for (int_fast8_t i = 0; i < inputCounter; i++)
    {
        PFbyte currDot = (input[i].homogeneous[3] < PF_CLIP_EPSILON) ? -1 : 1;

        if (prevDot*currDot < 0)
        {
            polygon[(*vertexCounter)++] = pfInternal_LerpVertex(prevVt, &input[i], 
                (PF_CLIP_EPSILON - prevVt->homogeneous[3]) / (input[i].homogeneous[3] - prevVt->homogeneous[3]));
        }

        if (currDot > 0)
        {
            polygon[(*vertexCounter)++] = input[i];
        }

        prevDot = currDot;
        prevVt = &input[i];
    }

    return *vertexCounter > 0;
}

PFboolean Process_ClipPolygonXYZ(PFvertex* polygon, int_fast8_t* vertexCounter)
{
    for (int_fast8_t iAxis = 0; iAxis < 3; iAxis++)
    {
        if (*vertexCounter == 0) return PF_FALSE;

        PFvertex input[PF_MAX_CLIPPED_POLYGON_VERTICES];
        int_fast8_t inputCounter;

        const PFvertex *prevVt;
        PFbyte prevDot;

        // Clip against first plane

        memcpy(input, polygon, (*vertexCounter)*sizeof(PFvertex));
        inputCounter = *vertexCounter;
        *vertexCounter = 0;

        prevVt = &input[inputCounter-1];
        prevDot = (prevVt->homogeneous[iAxis] <= prevVt->homogeneous[3]) ? 1 : -1;

        for (int_fast8_t i = 0; i < inputCounter; i++)
        {
            PFbyte currDot = (input[i].homogeneous[iAxis] <= input[i].homogeneous[3]) ? 1 : -1;

            if (prevDot*currDot <= 0)
            {
                polygon[(*vertexCounter)++] = pfInternal_LerpVertex(prevVt, &input[i], (prevVt->homogeneous[3] - prevVt->homogeneous[iAxis]) /
                    ((prevVt->homogeneous[3] - prevVt->homogeneous[iAxis]) - (input[i].homogeneous[3] - input[i].homogeneous[iAxis])));
            }

            if (currDot > 0)
            {
                polygon[(*vertexCounter)++] = input[i];
            }

            prevDot = currDot;
            prevVt = &input[i];
        }

        if (*vertexCounter == 0) return PF_FALSE;

        // Clip against opposite plane

        memcpy(input, polygon, (*vertexCounter)*sizeof(PFvertex));
        inputCounter = *vertexCounter;
        *vertexCounter = 0;

        prevVt = &input[inputCounter-1];
        prevDot = (-prevVt->homogeneous[iAxis] <= prevVt->homogeneous[3]) ? 1 : -1;

        for (int_fast8_t i = 0; i < inputCounter; i++)
        {
            PFbyte currDot = (-input[i].homogeneous[iAxis] <= input[i].homogeneous[3]) ? 1 : -1;

            if (prevDot*currDot <= 0)
            {
                polygon[(*vertexCounter)++] = pfInternal_LerpVertex(prevVt, &input[i], (prevVt->homogeneous[3] + prevVt->homogeneous[iAxis]) /
                    ((prevVt->homogeneous[3] + prevVt->homogeneous[iAxis]) - (input[i].homogeneous[3] + input[i].homogeneous[iAxis])));
            }

            if (currDot > 0)
            {
                polygon[(*vertexCounter)++] = input[i];
            }

            prevDot = currDot;
            prevVt = &input[i];
        }
    }

    return *vertexCounter > 0;
}

PFboolean Process_ProjectAndClipTriangle(PFvertex* polygon, int_fast8_t* vertexCounter)
{
    PFfloat weightSum = 0.0f;

    for (int_fast8_t i = 0; i < *vertexCounter; i++)
    {
        PFvertex *v = polygon + i;

        memcpy(v->homogeneous, v->position, sizeof(PFMvec4));
        pfmVec4Transform(v->homogeneous, v->homogeneous, currentCtx->matMVP);

        weightSum += v->homogeneous[3];
    }

    if (fabsf(weightSum - 3.0f) < PF_CLIP_EPSILON)
    {
        for (int_fast8_t i = 0; i < *vertexCounter; i++)
        {
            pfInternal_HomogeneousToScreen(&polygon[i]);
        }

        return PF_FALSE; // Is "2D"
    }

    if (Process_ClipPolygonW(polygon, vertexCounter) && Process_ClipPolygonXYZ(polygon, vertexCounter))
    {
        for (int_fast8_t i = 0; i < *vertexCounter; i++)
        {
            // Calculation of the reciprocal of Z for the perspective correct
            polygon[i].homogeneous[2] = 1.0f / polygon[i].homogeneous[2];

            // Division of texture coordinates by the Z axis (perspective correct)
            pfmVec2Scale(polygon[i].texcoord, polygon[i].texcoord, polygon[i].homogeneous[2]);

            // Division of XY coordinates by weight
            PFfloat invW = 1.0f / polygon[i].homogeneous[3];
            polygon[i].homogeneous[0] *= invW;
            polygon[i].homogeneous[1] *= invW;

            pfInternal_HomogeneousToScreen(&polygon[i]);
        }
    }

    return PF_TRUE; // Is 3D
}


/* Triangle rasterization functions */

#if PF_TRIANGLE_RASTER_MODE == PF_TRIANGLE_RASTER_BRAYCENTRIC_SIMD

void Rasterize_Triangle(PFface faceToRender, PFboolean is3D, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{
    PFsizei xMin, yMin, xMax, yMax;
    PFint w1Row, w2Row, w3Row;
    PFint w1XStep, w1YStep;
    PFint w2XStep, w2YStep;
    PFint w3XStep, w3YStep;
    {
        /* Get integer 2D position coordinates */

        PFint x1 = (PFint)v1->screen[0], y1 = (PFint)v1->screen[1];
        PFint x2 = (PFint)v2->screen[0], y2 = (PFint)v2->screen[1];
        PFint x3 = (PFint)v3->screen[0], y3 = (PFint)v3->screen[1];

        /* Check if the desired face can be rendered */

        PFfloat signedArea = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1);

        if ((faceToRender == PF_FRONT && signedArea >= 0)
         || (faceToRender == PF_BACK  && signedArea <= 0))
        {
            return;
        }

        /* Calculate the 2D bounding box of the triangle */

        xMin = (PFsizei)MIN(x1, MIN(x2, x3));
        yMin = (PFsizei)MIN(y1, MIN(y2, y3));
        xMax = (PFsizei)MAX(x1, MAX(x2, x3));
        yMax = (PFsizei)MAX(y1, MAX(y2, y3));

        if (!is3D)
        {
            xMin = (PFsizei)CLAMP((PFint)xMin, currentCtx->vpMin[0], currentCtx->vpMax[0]);
            yMin = (PFsizei)CLAMP((PFint)yMin, currentCtx->vpMin[1], currentCtx->vpMax[1]);
            xMax = (PFsizei)CLAMP((PFint)xMax, currentCtx->vpMin[0], currentCtx->vpMax[0]);
            yMax = (PFsizei)CLAMP((PFint)yMax, currentCtx->vpMin[1], currentCtx->vpMax[1]);
        }

        /* Barycentric interpolation */

        w1XStep = y3 - y2, w1YStep = x2 - x3;
        w2XStep = y1 - y3, w2YStep = x3 - x1;
        w3XStep = y2 - y1, w3YStep = x1 - x2;

        if (faceToRender == PF_BACK)
        {
            w1XStep = -w1XStep, w1YStep = -w1YStep;
            w2XStep = -w2XStep, w2YStep = -w2YStep;
            w3XStep = -w3XStep, w3YStep = -w3YStep;
        }

        w1Row = (xMin - x2)*w1XStep + w1YStep*(yMin - y2);
        w2Row = (xMin - x3)*w2XStep + w2YStep*(yMin - y3);
        w3Row = (xMin - x1)*w3XStep + w3YStep*(yMin - y1);
    }

    // Vector constants
    PFMsimd_i pixOffsetV = pfmSimdSetR_I32(0, 1, 2, 3, 4, 5, 6, 7);
    PFMsimd_i w1XStepV = pfmSimdMullo_I32(pfmSimdSet1_I32(w1XStep), pixOffsetV);
    PFMsimd_i w2XStepV = pfmSimdMullo_I32(pfmSimdSet1_I32(w2XStep), pixOffsetV);
    PFMsimd_i w3XStepV = pfmSimdMullo_I32(pfmSimdSet1_I32(w3XStep), pixOffsetV);

    // Calculate the reciprocal of the sum of the barycentric coordinates for normalization
    // NOTE: This sum remains constant throughout the triangle
    PFMsimd_f wInvSumV = pfmSimdSet1_F32(1.0f/(w1Row + w2Row + w3Row));

    // Load vertices data into SIMD registers
    PFsimd_color c1V, c2V, c3V;
    pfInternal_SimdColorLoadUnpacked(c1V, v1->color);
    pfInternal_SimdColorLoadUnpacked(c2V, v2->color);
    pfInternal_SimdColorLoadUnpacked(c3V, v3->color);

    PFMsimd_vec3 p1V, p2V, p3V;
    pfmSimdVec3Load(p1V, v1->position);
    pfmSimdVec3Load(p2V, v2->position);
    pfmSimdVec3Load(p3V, v3->position);

    PFMsimd_vec3 n1V, n2V, n3V;
    pfmSimdVec3Load(n1V, v1->normal);
    pfmSimdVec3Load(n2V, v2->normal);
    pfmSimdVec3Load(n3V, v3->normal);

    PFMsimd_vec2 tc1V, tc2V, tc3V;
    pfmSimdVec2Load(tc1V, v1->texcoord);
    pfmSimdVec2Load(tc2V, v2->texcoord);
    pfmSimdVec2Load(tc3V, v3->texcoord);

    /* Get some contextual values */

    InterpolateColorSimdFunc interpolateColor = (currentCtx->shadingMode == PF_SMOOTH)
        ? pfInternal_SimdColorBarySmooth : pfInternal_SimdColorBaryFlat;

    PFblendfunc_simd blendFunction = (currentCtx->state & PF_BLEND) ? currentCtx->blendSimdFunction : NULL;
    PFdepthfunc_simd depthFunction = ((currentCtx->state & PF_DEPTH_TEST)) ? currentCtx->depthSimdFunction : NULL;

    struct PFtex *texDst = currentCtx->currentFramebuffer->texture;
    struct PFtex *texSrc = currentCtx->currentTexture;

    PFfloat *zbDst = currentCtx->currentFramebuffer->zbuffer;

    PFpixelgetter_simd getter = texDst->getterSimd;
    PFpixelsetter_simd setter = texDst->setterSimd;
    PFsizei widthDst = texDst->w;
    void *pbDst = texDst->pixels;

    PFMsimd_f z1V = pfmSimdSet1_F32(v1->homogeneous[2]);
    PFMsimd_f z2V = pfmSimdSet1_F32(v2->homogeneous[2]);
    PFMsimd_f z3V = pfmSimdSet1_F32(v3->homogeneous[2]);

    const PFboolean texturing = (currentCtx->state & PF_TEXTURE_2D) && texSrc;
    const PFboolean lighting  = (currentCtx->state & PF_LIGHTING) && currentCtx->activeLights;

    /* Loop macro definition */

#ifdef _OPENMP
#define PF_TRIANGLE_TRAVEL_SIMD(PIXEL_CODE)                                                 \
    _Pragma("omp parallel for schedule(dynamic, PF_OPENMP_TRIANGLE_ROW_PER_THREAD)          \
        if((yMax - yMin)*(xMax - xMin) >= PF_OPENMP_RASTER_THRESHOLD_AREA)")                \
    for (PFsizei y = yMin; y <= yMax; ++y)                                                  \
    {                                                                                       \
        size_t yOffset = y * widthDst;                                                      \
        PFint w1 = w1Row + (y - yMin)*w1YStep;                                              \
        PFint w2 = w2Row + (y - yMin)*w2YStep;                                              \
        PFint w3 = w3Row + (y - yMin)*w3YStep;                                              \
        for (PFsizei x = xMin; x <= xMax; x += PF_SIMD_SIZE)                                \
        {                                                                                   \
            /* Load the current barycentric coordinates into SIMD registers */              \
            PFMsimd_i w1V = pfmSimdAdd_I32(pfmSimdSet1_I32(w1), w1XStepV);                  \
            PFMsimd_i w2V = pfmSimdAdd_I32(pfmSimdSet1_I32(w2), w2XStepV);                  \
            PFMsimd_i w3V = pfmSimdAdd_I32(pfmSimdSet1_I32(w3), w3XStepV);                  \
            /* Test if pixels are inside the triangle */                                    \
            PFMsimd_i mask = pfmSimdOr_I32(pfmSimdOr_I32(w1V, w2V), w3V);                   \
            mask = pfmSimdCmpGT_I32(mask, pfmSimdSetZero_I32());                            \
            /* Normalize weights */                                                         \
            PFMsimd_f w1NormV = pfmSimdMul_F32(pfmSimdConvert_I32_F32(w1V), wInvSumV);      \
            PFMsimd_f w2NormV = pfmSimdMul_F32(pfmSimdConvert_I32_F32(w2V), wInvSumV);      \
            PFMsimd_f w3NormV = pfmSimdMul_F32(pfmSimdConvert_I32_F32(w3V), wInvSumV);      \
            /* Compute Z-Depth values */                                                    \
            PFMsimd_f zV;                                                                   \
            {                                                                               \
                PFMsimd_f wZ1 = pfmSimdMul_F32(z1V, w1NormV);                               \
                PFMsimd_f wZ2 = pfmSimdMul_F32(z2V, w2NormV);                               \
                PFMsimd_f wZ3 = pfmSimdMul_F32(z3V, w3NormV);                               \
                zV = pfmSimdAdd_F32(pfmSimdAdd_F32(wZ1, wZ2), wZ3);                         \
                zV = pfmSimdRCP_F32(zV);                                                    \
            }                                                                               \
            /* Depth Testing */                                                             \
            PFMsimd_f depths = pfmSimdLoad_F32(zbDst + yOffset + x);                        \
            if (depthFunction)                                                              \
            {                                                                               \
                mask = pfmSimdAnd_I32(mask, pfmSimdCast_F32_I32(                            \
                    depthFunction(zV, depths)));                                            \
            }                                                                               \
            /* Run the pixel code! */                                                       \
            PIXEL_CODE                                                                      \
            /* Increment the barycentric coordinates for the next pixels */                 \
            w1 += PF_SIMD_SIZE*w1XStep;                                                     \
            w2 += PF_SIMD_SIZE*w2XStep;                                                     \
            w3 += PF_SIMD_SIZE*w3XStep;                                                     \
        }                                                                                   \
    }
#else
#define PF_TRIANGLE_TRAVEL_SIMD(PIXEL_CODE)                                                 \
    for (PFsizei y = yMin; y <= yMax; ++y)                                                  \
    {                                                                                       \
        size_t yOffset = y * widthDst;                                                      \
        int w1 = w1Row;                                                                     \
        int w2 = w2Row;                                                                     \
        int w3 = w3Row;                                                                     \
        for (PFsizei x = xMin; x <= xMax; x += PF_SIMD_SIZE)                                \
        {                                                                                   \
            /* Load the current barycentric coordinates into SIMD registers */              \
            PFMsimd_i w1V = pfmSimdAdd_I32(pfmSimdSet1_I32(w1), w1XStepV);                  \
            PFMsimd_i w2V = pfmSimdAdd_I32(pfmSimdSet1_I32(w2), w2XStepV);                  \
            PFMsimd_i w3V = pfmSimdAdd_I32(pfmSimdSet1_I32(w3), w3XStepV);                  \
            /* Test if pixels are inside the triangle */                                    \
            PFMsimd_i mask = pfmSimdOr_I32(pfmSimdOr_I32(w1V, w2V), w3V);                   \
            mask = pfmSimdCmpGT_I32(mask, pfmSimdSetZero_I32());                            \
            /* Normalize weights */                                                         \
            PFMsimd_f w1NormV = pfmSimdMul_F32(pfmSimdConvert_I32_F32(w1V), wInvSumV);      \
            PFMsimd_f w2NormV = pfmSimdMul_F32(pfmSimdConvert_I32_F32(w2V), wInvSumV);      \
            PFMsimd_f w3NormV = pfmSimdMul_F32(pfmSimdConvert_I32_F32(w3V), wInvSumV);      \
            /* Compute Z-Depth values */                                                    \
            PFMsimd_f zV;                                                                   \
            {                                                                               \
                PFMsimd_f wZ1 = pfmSimdMul_F32(z1V, w1NormV);                               \
                PFMsimd_f wZ2 = pfmSimdMul_F32(z2V, w2NormV);                               \
                PFMsimd_f wZ3 = pfmSimdMul_F32(z3V, w3NormV);                               \
                zV = pfmSimdAdd_F32(pfmSimdAdd_F32(wZ1, wZ2), wZ3);                         \
                zV = pfmSimdRCP_F32(zV);                                                    \
            }                                                                               \
            /* Depth Testing */                                                             \
            PFMsimd_f depths = pfmSimdLoad_F32(zbDst + yOffset + x);                        \
            if (depthFunction)                                                              \
            {                                                                               \
                mask = pfmSimdAnd_I32(mask, pfmSimdCast_F32_I32(                            \
                    depthFunction(zV, depths)));                                            \
            }                                                                               \
            /* Run the pixel code! */                                                       \
            PIXEL_CODE                                                                      \
            /* Increment the barycentric coordinates for the next pixels */                 \
            w1 += PF_SIMD_SIZE*w1XStep;                                                     \
            w2 += PF_SIMD_SIZE*w2XStep;                                                     \
            w3 += PF_SIMD_SIZE*w3XStep;                                                     \
        }                                                                                   \
        /* Move to the next row in the bounding box */                                      \
        w1Row += w1YStep;                                                                   \
        w2Row += w2YStep;                                                                   \
        w3Row += w3YStep;                                                                   \
    }
#endif

    /* Processing macro definitions */

#   define GET_FRAG() \
        PFsimd_color fragments; \
        interpolateColor(fragments, c1V, c2V, c3V, w1NormV, w2NormV, w3NormV);

#   define TEXTURING() \
        PFMsimd_vec2 texcoords; \
        pfmSimdVec2BaryInterpR(texcoords, tc1V, tc2V, tc3V, w1NormV, w2NormV, w3NormV); \
        if (is3D) pfmSimdVec2Scale(texcoords, texcoords, zV); /* Perspective correct */ \
        PFsimd_color texels; pfInternal_SimdColorUnpack(texels, texSrc->samplerSimd(texSrc, texcoords)); \
        pfInternal_SimdBlendMultiplicative(fragments, texels, fragments);

#   define LIGHTING() \
        PFMsimd_vec3 normals, positions; \
        pfmSimdVec3BaryInterpR(normals, n1V, n2V, n3V, w1NormV, w2NormV, w3NormV); \
        pfmSimdVec3BaryInterpR(positions, p1V, p2V, p3V, w1NormV, w2NormV, w3NormV); \
        //fragment = pfInternal_ProcessLights(currentCtx->activeLights, &currentCtx->faceMaterial[faceToRender], fragment, viewPos, position, normal);

#   define SET_FRAG() \
        if (blendFunction) { \
            PFsimd_color dstCol; \
            pfInternal_SimdColorUnpack(dstCol, getter(pbDst, \
                pfmSimdAdd_I32(pfmSimdSet1_I32(yOffset + x), pixOffsetV))); \
            blendFunction(fragments, fragments, dstCol); \
        } \
        setter(pbDst, yOffset + x, pfInternal_SimdColorPack(fragments), mask); \
        pfmSimdStore_F32(zbDst + yOffset + x, pfmSimdBlendV_F32(depths, zV, pfmSimdCast_I32_F32(mask)));

    /* Loop rasterization */

    if (texturing && lighting)
    {
        PF_TRIANGLE_TRAVEL_SIMD({
            GET_FRAG();
            TEXTURING();
            LIGHTING();
            SET_FRAG();
        })
    }
    else if (texturing)
    {
        PF_TRIANGLE_TRAVEL_SIMD({
            GET_FRAG();
            TEXTURING();
            SET_FRAG();
        })
    }
    else if (lighting)
    {
        PF_TRIANGLE_TRAVEL_SIMD({
            GET_FRAG();
            LIGHTING();
            SET_FRAG();
        })
    }
    else
    {
        PF_TRIANGLE_TRAVEL_SIMD({
            GET_FRAG();
            SET_FRAG();
        })
    }
}

#elif PF_TRIANGLE_RASTER_MODE == PF_TRIANGLE_RASTER_BARYCENTRIC_OMP

void Rasterize_Triangle(PFface faceToRender, PFboolean is3D, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{
    PFsizei xMin, yMin, xMax, yMax;
    PFint w1Row, w2Row, w3Row;
    PFint w1XStep, w1YStep;
    PFint w2XStep, w2YStep;
    PFint w3XStep, w3YStep;
    PFfloat wInvSum;
    {
        /* Get integer 2D position coordinates */

        PFint x1 = (PFint)v1->screen[0], y1 = (PFint)v1->screen[1];
        PFint x2 = (PFint)v2->screen[0], y2 = (PFint)v2->screen[1];
        PFint x3 = (PFint)v3->screen[0], y3 = (PFint)v3->screen[1];

        /* Check if the desired face can be rendered */

        PFfloat signedArea = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1);

        if ((faceToRender == PF_FRONT && signedArea >= 0)
         || (faceToRender == PF_BACK  && signedArea <= 0))
        {
            return;
        }

        /* Calculate the 2D bounding box of the triangle */

        xMin = (PFsizei)MIN(x1, MIN(x2, x3));
        yMin = (PFsizei)MIN(y1, MIN(y2, y3));
        xMax = (PFsizei)MAX(x1, MAX(x2, x3));
        yMax = (PFsizei)MAX(y1, MAX(y2, y3));

        if (!is3D)
        {
            xMin = (PFsizei)CLAMP((PFint)xMin, currentCtx->vpMin[0], currentCtx->vpMax[0]);
            yMin = (PFsizei)CLAMP((PFint)yMin, currentCtx->vpMin[1], currentCtx->vpMax[1]);
            xMax = (PFsizei)CLAMP((PFint)xMax, currentCtx->vpMin[0], currentCtx->vpMax[0]);
            yMax = (PFsizei)CLAMP((PFint)yMax, currentCtx->vpMin[1], currentCtx->vpMax[1]);
        }

        /* Barycentric interpolation */

        w1XStep = y3 - y2, w1YStep = x2 - x3;
        w2XStep = y1 - y3, w2YStep = x3 - x1;
        w3XStep = y2 - y1, w3YStep = x1 - x2;

        if (faceToRender == PF_BACK)
        {
            w1XStep = -w1XStep, w1YStep = -w1YStep;
            w2XStep = -w2XStep, w2YStep = -w2YStep;
            w3XStep = -w3XStep, w3YStep = -w3YStep;
        }

        w1Row = (xMin - x2)*w1XStep + w1YStep*(yMin - y2);
        w2Row = (xMin - x3)*w2XStep + w2YStep*(yMin - y3);
        w3Row = (xMin - x1)*w3XStep + w3YStep*(yMin - y1);

        /*
            Finally, we calculate the inverse of the sum of
            the barycentric coordinates for the top-left point; this
            sum always remains the same, regardless of the coordinate
            within the triangle.
        */

        wInvSum = 1.0f/(w1Row + w2Row + w3Row);
    }

    /* Get some contextual values */

    InterpolateColorFunc interpolateColor = (currentCtx->shadingMode == PF_SMOOTH)
        ? pfInternal_ColorBarySmooth : pfInternal_ColorBaryFlat;

    PFblendfunc blendFunction = (currentCtx->state & PF_BLEND) ? currentCtx->blendFunction : NULL;
    PFdepthfunc depthFunction = (currentCtx->state & PF_DEPTH_TEST) ? currentCtx->depthFunction : NULL;

    struct PFtex *texDst = currentCtx->currentFramebuffer->texture;
    struct PFtex *texSrc = currentCtx->currentTexture;

    PFfloat *zbDst = currentCtx->currentFramebuffer->zbuffer;

    PFpixelgetter getter = texDst->getter;
    PFpixelsetter setter = texDst->setter;
    PFsizei widthDst = texDst->w;
    void *pbDst = texDst->pixels;

    PFfloat z1 = v1->homogeneous[2];
    PFfloat z2 = v2->homogeneous[2];
    PFfloat z3 = v3->homogeneous[2];

    const PFboolean texturing = (currentCtx->state & PF_TEXTURE_2D) && texSrc;
    const PFboolean lighting  = (currentCtx->state & PF_LIGHTING) && currentCtx->activeLights;

    /* Loop macro definition */

#   define PF_TRIANGLE_TRAVEL(PIXEL_CODE)                                           \
    _Pragma("omp parallel for schedule(dynamic, PF_OPENMP_TRIANGLE_ROW_PER_THREAD)  \
        if((yMax - yMin)*(xMax - xMin) >= PF_OPENMP_RASTER_THRESHOLD_AREA)")        \
    for (PFsizei y = yMin; y <= yMax; y++)                                          \
    {                                                                               \
        const PFsizei yOffset = y*widthDst;                                         \
        PFint w1 = w1Row + (y - yMin)*w1YStep;                                      \
        PFint w2 = w2Row + (y - yMin)*w2YStep;                                      \
        PFint w3 = w3Row + (y - yMin)*w3YStep;                                      \
        for (PFsizei x = xMin; x <= xMax; x++)                                      \
        {                                                                           \
            if ((w1 | w2 | w3) >= 0)                                                \
            {                                                                       \
                PFsizei xyOffset = yOffset + x;                                     \
                PFfloat w1Norm = w1*wInvSum;                                        \
                PFfloat w2Norm = w2*wInvSum;                                        \
                PFfloat w3Norm = w3*wInvSum;                                        \
                PFfloat z = 1.0f/(w1Norm*z1 + w2Norm*z2 + w3Norm*z3);               \
                if (!depthFunction || depthFunction(z, zbDst[xyOffset]))            \
                {                                                                   \
                    PIXEL_CODE                                                      \
                }                                                                   \
            }                                                                       \
            w1 += w1XStep;                                                          \
            w2 += w2XStep;                                                          \
            w3 += w3XStep;                                                          \
        }                                                                           \
    }

    /* Processing macro definitions */

#   define GET_FRAG() \
    PFcolor fragment = interpolateColor( \
        v1->color, v2->color, v3->color, \
        w1Norm, w2Norm, w3Norm);

#   define TEXTURING() \
        PFMvec2 texcoord; \
        pfmVec2BaryInterpR(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, w1Norm, w2Norm, w3Norm); \
        if (is3D) texcoord[0] *= z, texcoord[1] *= z; /* Perspective correct */ \
        PFcolor texel = texSrc->sampler(texSrc, texcoord[0], texcoord[1]); \
        fragment = pfInternal_BlendMultiplicative(texel, fragment);

#   define LIGHTING() \
        PFMvec3 normal, position; \
        pfmVec3BaryInterpR(normal, v1->normal, v2->normal, v3->normal, w1Norm, w2Norm, w3Norm); \
        pfmVec3BaryInterpR(position, v1->position, v2->position, v3->position, w1Norm, w2Norm, w3Norm); \
        fragment = pfInternal_ProcessLights(currentCtx->activeLights, &currentCtx->faceMaterial[faceToRender], fragment, viewPos, position, normal);

#   define SET_FRAG() \
        PFcolor finalColor = blendFunction ? blendFunction(fragment, getter(pbDst, xyOffset)) : fragment; \
        setter(pbDst, xyOffset, finalColor); \
        zbDst[xyOffset] = z;

    /* Loop rasterization */

    if (texturing && lighting)
    {
        PF_TRIANGLE_TRAVEL({
            GET_FRAG();
            TEXTURING();
            LIGHTING();
            SET_FRAG();
        })
    }
    else if (texturing)
    {
        PF_TRIANGLE_TRAVEL({
            GET_FRAG();
            TEXTURING();
            SET_FRAG();
        })
    }
    else if (lighting)
    {
        PF_TRIANGLE_TRAVEL({
            GET_FRAG();
            LIGHTING();
            SET_FRAG();
        })
    }
    else
    {
        PF_TRIANGLE_TRAVEL({
            GET_FRAG();
            SET_FRAG();
        })
    }
}

#else // PF_TRIANGLE_RASTER_MODE == PR_TRIANGLE_RASTER_SCANLINES

// TODO: Performed the interpolations by increments
// TODO: Find a maintainable way to reduce conditionality in loops
void Rasterize_Triangle(PFface faceToRender, PFboolean is3D, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{
    const PFboolean noDepth = !(currentCtx->state & PF_DEPTH_TEST);
    const PFboolean lighting = (currentCtx->state & PF_LIGHTING) && currentCtx->activeLights;
    const PFboolean texturing = (currentCtx->state & PF_TEXTURE_2D) && currentCtx->currentTexture;

    /* Check if the face can be rendered, if not, skip */

    PFfloat area;
    if (!Helper_FaceCanBeRendered(faceToRender, &area, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    /* Sort vertices by their y-coordinates */

    Helper_SortVertices(&v1, &v2, &v3);

    /* Cache screen coordinates, depths and colors of vertices */

    PFint x1 = v1->screen[0], y1 = v1->screen[1];
    PFint x2 = v2->screen[0], y2 = v2->screen[1];
    PFint x3 = v3->screen[0], y3 = v3->screen[1];

    PFfloat z1 = v1->homogeneous[2];
    PFfloat z2 = v2->homogeneous[2];
    PFfloat z3 = v3->homogeneous[2];

    PFcolor c1 = v1->color;
    PFcolor c2 = v2->color;
    PFcolor c3 = v3->color;

    /* Precompute inverse heights for interpolation */

    PFfloat invTotalHeight = 1.0f/(y3 - y1 + 1);
    PFfloat invSegmentHeight21 = 1.0f/(y2 - y1 + 1);
    PFfloat invSegmentHeight32 = 1.0f/(y3 - y2 + 1);

    /* Choose color interpolation method based on shading mode */

    InterpolateColorFunc interpolateColor = (currentCtx->shadingMode == PF_SMOOTH)
        ? pfInternal_ColorLerpSmooth : pfInternal_ColorLerpFlat;

    /* Extract framebuffer information */

    struct PFtex *texDst = currentCtx->currentFramebuffer->texture;
    PFfloat *zbDst = currentCtx->currentFramebuffer->zbuffer;

    PFblendfunc blendFunction = (currentCtx->state & PF_BLEND) ? currentCtx->blendFunction : NULL;
    PFpixelsetter setter = texDst->setter;
    PFpixelgetter getter = texDst->getter;
    PFsizei widthDst = texDst->width;
    void *pbDst = texDst->pixels;

    /*  */

    PFint yMin = y1;
    PFint yMax = y3;

    if (!is3D)
    {
        yMin = CLAMP(yMin, currentCtx->vpMin[1], currentCtx->vpMax[1]);
        yMax = CLAMP(yMax, currentCtx->vpMin[1], currentCtx->vpMax[1]);
    }

    PFsizei yOffset = yMin*widthDst;

    /*  */

    PFfloat alpha = invTotalHeight*(yMin - y1);
    PFfloat beta1 = invSegmentHeight21*(yMin - y1);   // First half
    PFfloat beta2 = invSegmentHeight32*(yMin - y2);   // Second half

    /* Travel the triangle from top to bottom */

    for (PFint y = yMin; y <= yMax; y++, yOffset += widthDst)
    {
        alpha += invTotalHeight;
        beta1 += invSegmentHeight21;
        beta2 += invSegmentHeight32;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;

        PFMvec2 uvA, uvB;
        PFMvec3 pA, pB;
        PFMvec3 nA, nB;

        if (y < y2) // First half
        {
            xA = x1 + (x3 - x1)*alpha;
            xB = x1 + (x2 - x1)*beta1;

            zA = z1 + (z3 - z1)*alpha;
            zB = z1 + (z2 - z1)*beta1;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta1);

            if (texturing)
            {
                pfmVec2LerpR(uvA, v1->texcoord, v3->texcoord, alpha);
                pfmVec2LerpR(uvB, v1->texcoord, v2->texcoord, beta1);
            }

            if (lighting)
            {
                pfmVec3LerpR(pA, v1->position, v3->position, alpha);
                pfmVec3LerpR(pB, v1->position, v2->position, beta1);
                pfmVec3LerpR(nA, v1->normal, v3->normal, alpha);
                pfmVec3LerpR(nB, v1->normal, v2->normal, beta1);
            }

        }
        else // Second half
        {
            xA = x1 + (x3 - x1)*alpha;
            xB = x2 + (x3 - x2)*beta2;

            zA = z1 + (z3 - z1)*alpha;
            zB = z2 + (z3 - z2)*beta2;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta2);

            if (texturing)
            {
                pfmVec2LerpR(uvA, v1->texcoord, v3->texcoord, alpha);
                pfmVec2LerpR(uvB, v2->texcoord, v3->texcoord, beta2);
            }

            if (lighting)
            {
                pfmVec3LerpR(pA, v1->position, v3->position, alpha);
                pfmVec3LerpR(pB, v2->position, v3->position, beta2);
                pfmVec3LerpR(nA, v1->normal, v3->normal, alpha);
                pfmVec3LerpR(nB, v2->normal, v3->normal, beta2);
            }
        }

        /* Swap endpoints if necessary to ensure xA <= xB */

        if (xA > xB)
        {
            PFint iTmp = xA; xA = xB; xB = iTmp;
            PFfloat fTmp = zA; zA = zB; zB = fTmp;
            PFcolor cTmp = cA; cA = cB; cB = cTmp;

            pfmVec2Swap(uvA, uvB);
            pfmVec3Swap(pA, pB);
            pfmVec3Swap(nA, nB);
        }

        /*  */

        PFint xMin = xA;
        PFint xMax = xB;

        if (!is3D)
        {
            xMin = CLAMP(xMin, currentCtx->vpMin[0], currentCtx->vpMax[0]);
            xMax = CLAMP(xMax, currentCtx->vpMin[0], currentCtx->vpMax[0]);
        }

        PFsizei xyOffset = yOffset + xMin;

        /*  */

        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);
        PFfloat gamma = xInvLen*(xMin - xA);

        PFfloat zStep = xInvLen*(zB - zA);

        // REIVEW: Very weird issue with UV increment
        /*
            PFMvec2 uvStep;
            pfmVec2SubR(uvStep, uvB, uvA);
            pfmVec2Scale(uvStep, uvStep, xInvLen);
        */

        PFMvec3 pStep;
        pfmVec3SubR(pStep, pB, pA);
        pfmVec3Scale(pStep, pStep, xInvLen);

        PFMvec3 nStep;
        pfmVec3SubR(nStep, nB, nA);
        pfmVec3Scale(nStep, nStep, xInvLen);

        /* Draw Horizontal Line */

        for (PFint x = xMin; x <= xMax; x++, xyOffset++, gamma += xInvLen)
        {
            /* Calculate interpolation factor and Z */

            PFfloat z = 1.0f/zA; zA += zStep;

            /* Perform depth test */

            if (noDepth || currentCtx->depthFunction(z, zbDst[xyOffset]))
            {
                /* Obtain fragment color */

                PFcolor fragment = interpolateColor(cA, cB, gamma);

                /* Blend with corresponding texture sample */

                if (texturing)
                {
                    PFMvec2 uv;
                    //pfmVec2Copy(uv, uvA);
                    //pfmVec2Add(uvA, uvA, uvStep);
                    pfmVec2LerpR(uv, uvA, uvB, gamma);

                    if (is3D)
                    {
                        // NOTE 1: Divided by 'z', correct perspective
                        // NOTE 2: 'z' is actually the reciprocal
                        pfmVec2Scale(uv, uv, z);
                    }

                    PFcolor texel = pfTextureSampleNearestWrap(texDst, uv[0], uv[1]);
                    fragment = pfBlendMultiplicative(texel, fragment);
                }

                /* Compute lighting */

                if (lighting)
                {
                    PFMvec3 position;
                    pfmVec3Copy(position, pA);
                    pfmVec3Add(pA, pA, pStep);

                    PFMvec3 normal;
                    pfmVec3Copy(normal, nA);
                    pfmVec3Add(nA, nA, nStep);

                    fragment = Process_Lights(currentCtx->activeLights,
                        &currentCtx->faceMaterial[faceToRender], fragment,
                        viewPos, position, normal);
                }

                /* Apply final color and depth */

                PFcolor finalColor = blendFunction ? blendFunction(fragment, getter(pbDst, xyOffset)) : fragment;
                setter(pbDst, xyOffset, finalColor);
                zbDst[xyOffset] = z;
            }
        }
    }
}

#endif //PF_TRIANGLE_RASTER_MODE
