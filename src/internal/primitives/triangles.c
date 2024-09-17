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
#include "../color.h"
#include "../blend.h"

#define PF_TRIANGLE_RASTER_BARYCENTRIC  1   ///< Can also use OpenMP (if available) in addition to SIMD support
#define PF_TRIANGLE_RASTER_SCANLINES    2   ///< Can use OpenMP but not SIMD.

#if PF_SIMD_SUPPORT
#   define PF_TRIANGLE_RASTER_MODE \
        PF_TRIANGLE_RASTER_BARYCENTRIC
#else
#   define PF_TRIANGLE_RASTER_MODE \
        PF_TRIANGLE_RASTER_SCANLINES
#endif

/* Internal typedefs */

#if PF_TRIANGLE_RASTER_MODE == PF_TRIANGLE_RASTER_BARYCENTRIC
typedef void (*InterpolateColorSimdFunc)(PFcolor_simd, const PFcolor_simd, const PFcolor_simd, const PFcolor_simd, PFIsimdvf, PFIsimdvf, PFIsimdvf);
#elif PF_TRIANGLE_RASTER_MODE == PF_TRIANGLE_RASTER_SCANLINES
typedef PFcolor (*InterpolateColorFunc)(PFcolor, PFcolor, PFfloat);
#endif //PF_RASTER_MODE

/* Internal triangle processing functions declarations */

static PFboolean Process_ClipPolygonW(PFIvertex* polygon, int_fast8_t* vertexCounter);
static PFboolean Process_ClipPolygonXYZ(PFIvertex* polygon, int_fast8_t* vertexCounter);
static PFboolean Process_ProjectAndClipTriangle(PFIvertex* polygon, int_fast8_t* vertexCounter);

/* Internal triangle rasterizer function declarations */

static void Rasterize_Triangle(PFface faceToRender, PFboolean is3D,
                               const PFIvertex* v1, const PFIvertex* v2, const PFIvertex* v3,
                               const PFMvec3 viewPos);


/* Line Process And Rasterize Function */

// NOTE: An array of vertices with a total size equal to 'PF_MAX_CLIPPED_POLYGON_VERTICES' must be provided as a parameter
//       with only the first three vertices defined; the extra space is used in case the triangle needs to be clipped.
static void pfiProcessRasterize_TRIANGLE_IMPL(PFface faceToRender, PFIvertex processed[PF_MAX_CLIPPED_POLYGON_VERTICES])
{
#ifndef NDEBUG
    if (faceToRender == PF_FRONT_AND_BACK) {
        // WARNING: This should not be called with PF_FRONT_AND_BACK
        G_currentCtx->errCode = PF_DEBUG_INVALID_OPERATION;
        return;
    }
#endif

    PFboolean lighting = (G_currentCtx->state & PF_LIGHTING) &&
                         (G_currentCtx->activeLights != NULL);

    int_fast8_t processedCounter = 3;

    // Performs certain operations that must be done before
    // processing the vertices in case of light management
    if (lighting) {
        // Transform normals
        // And multiply vertex color with diffuse color
        for (int_fast8_t i = 0; i < processedCounter; i++) {
            pfmVec3Transform(processed[i].normal, processed[i].normal, G_currentCtx->matNormal);
            pfmVec3Normalize(processed[i].normal, processed[i].normal); // REVIEW: Only with PF_NORMALIZE state??
            processed[i].color = pfiBlendMultiplicative(processed[i].color,
                G_currentCtx->faceMaterial[faceToRender].diffuse);
        }
    }

    // Process vertices
    PFboolean is3D = Process_ProjectAndClipTriangle(processed, &processedCounter);
    if (processedCounter < 3) return;

    // Rasterize filled triangles
    PFMvec3 viewPos = { 0 };

    if (lighting) {
        PFMmat4 invMatView;
        pfmMat4Invert(invMatView, G_currentCtx->matView);
        pfmVec3Copy(viewPos, invMatView + 12);
    }

    for (int_fast8_t i = 0; i < processedCounter - 2; i++) {
        Rasterize_Triangle(faceToRender, is3D, &processed[0], &processed[i + 1], &processed[i + 2], viewPos);
    }
}

void pfiProcessRasterize_TRIANGLE(PFface faceToRender)
{
    PFIvertex processed[PF_MAX_CLIPPED_POLYGON_VERTICES];
    memcpy(processed, G_currentCtx->vertexBuffer, 3 * sizeof(PFIvertex));
    pfiProcessRasterize_TRIANGLE_IMPL(faceToRender, processed);
}

void pfiProcessRasterize_TRIANGLE_FAN(PFface faceToRender, int_fast8_t numTriangles)
{
    for (int_fast8_t i = 0; i < numTriangles; i++) {
        PFIvertex processed[PF_MAX_CLIPPED_POLYGON_VERTICES] = {
            G_currentCtx->vertexBuffer[0],
            G_currentCtx->vertexBuffer[i + 1],
            G_currentCtx->vertexBuffer[i + 2]
        };
        pfiProcessRasterize_TRIANGLE_IMPL(faceToRender, processed);
    }
}

void pfiProcessRasterize_TRIANGLE_STRIP(PFface faceToRender, int_fast8_t numTriangles)
{
    for (int_fast8_t i = 0; i < numTriangles; i++) {
        PFIvertex processed[PF_MAX_CLIPPED_POLYGON_VERTICES];
        if (i % 2 == 0) {
            processed[0] = G_currentCtx->vertexBuffer[i];
            processed[1] = G_currentCtx->vertexBuffer[i + 1];
            processed[2] = G_currentCtx->vertexBuffer[i + 2];
        } else {
            processed[0] = G_currentCtx->vertexBuffer[i + 2];
            processed[1] = G_currentCtx->vertexBuffer[i + 1];
            processed[2] = G_currentCtx->vertexBuffer[i];
        }
        pfiProcessRasterize_TRIANGLE_IMPL(faceToRender, processed);
    }
}


/* Internal triangle processing functions definitions */

PFboolean Process_ClipPolygonW(PFIvertex* polygon, int_fast8_t* vertexCounter)
{
    PFIvertex input[PF_MAX_CLIPPED_POLYGON_VERTICES];
    memcpy(input, polygon, (*vertexCounter)*sizeof(PFIvertex));

    int_fast8_t inputCounter = *vertexCounter;
    *vertexCounter = 0;

    const PFIvertex *prevVt = &input[inputCounter-1];
    PFbyte prevDot = (prevVt->homogeneous[3] < PF_CLIP_EPSILON) ? -1 : 1;

    for (int_fast8_t i = 0; i < inputCounter; i++) {
        PFbyte currDot = (input[i].homogeneous[3] < PF_CLIP_EPSILON) ? -1 : 1;
        if (prevDot*currDot < 0) {
            polygon[(*vertexCounter)++] = pfiLerpVertex(prevVt, &input[i], 
                (PF_CLIP_EPSILON - prevVt->homogeneous[3]) / (input[i].homogeneous[3] - prevVt->homogeneous[3]));
        }
        if (currDot > 0) {
            polygon[(*vertexCounter)++] = input[i];
        }
        prevDot = currDot;
        prevVt = &input[i];
    }

    return *vertexCounter > 0;
}

PFboolean Process_ClipPolygonXYZ(PFIvertex* polygon, int_fast8_t* vertexCounter)
{
    for (int_fast8_t iAxis = 0; iAxis < 3; iAxis++)
    {
        if (*vertexCounter == 0) return PF_FALSE;

        PFIvertex input[PF_MAX_CLIPPED_POLYGON_VERTICES];
        int_fast8_t inputCounter;

        const PFIvertex *prevVt;
        PFbyte prevDot;

        // Clip against first plane

        memcpy(input, polygon, (*vertexCounter)*sizeof(PFIvertex));
        inputCounter = *vertexCounter;
        *vertexCounter = 0;

        prevVt = &input[inputCounter-1];
        prevDot = (prevVt->homogeneous[iAxis] <= prevVt->homogeneous[3]) ? 1 : -1;

        for (int_fast8_t i = 0; i < inputCounter; i++) {
            PFbyte currDot = (input[i].homogeneous[iAxis] <= input[i].homogeneous[3]) ? 1 : -1;
            if (prevDot*currDot <= 0) {
                polygon[(*vertexCounter)++] = pfiLerpVertex(prevVt, &input[i], (prevVt->homogeneous[3] - prevVt->homogeneous[iAxis]) /
                    ((prevVt->homogeneous[3] - prevVt->homogeneous[iAxis]) - (input[i].homogeneous[3] - input[i].homogeneous[iAxis])));
            }
            if (currDot > 0) {
                polygon[(*vertexCounter)++] = input[i];
            }
            prevDot = currDot;
            prevVt = &input[i];
        }

        if (*vertexCounter == 0) return PF_FALSE;

        // Clip against opposite plane

        memcpy(input, polygon, (*vertexCounter)*sizeof(PFIvertex));
        inputCounter = *vertexCounter;
        *vertexCounter = 0;

        prevVt = &input[inputCounter-1];
        prevDot = (-prevVt->homogeneous[iAxis] <= prevVt->homogeneous[3]) ? 1 : -1;

        for (int_fast8_t i = 0; i < inputCounter; i++) {
            PFbyte currDot = (-input[i].homogeneous[iAxis] <= input[i].homogeneous[3]) ? 1 : -1;
            if (prevDot*currDot <= 0) {
                polygon[(*vertexCounter)++] = pfiLerpVertex(prevVt, &input[i], (prevVt->homogeneous[3] + prevVt->homogeneous[iAxis]) /
                    ((prevVt->homogeneous[3] + prevVt->homogeneous[iAxis]) - (input[i].homogeneous[3] + input[i].homogeneous[iAxis])));
            }
            if (currDot > 0) {
                polygon[(*vertexCounter)++] = input[i];
            }
            prevDot = currDot;
            prevVt = &input[i];
        }
    }

    return *vertexCounter > 0;
}

PFboolean Process_ProjectAndClipTriangle(PFIvertex* polygon, int_fast8_t* vertexCounter)
{
    PFfloat weightSum = 0.0f;

    for (int_fast8_t i = 0; i < *vertexCounter; i++) {
        PFIvertex *v = polygon + i;
        memcpy(v->homogeneous, v->position, sizeof(PFMvec4));
        pfmVec4Transform(v->homogeneous, v->homogeneous, G_currentCtx->matMVP);
        weightSum += v->homogeneous[3];
    }

    if (fabsf(weightSum - 3.0f) < PF_CLIP_EPSILON) {
        for (int_fast8_t i = 0; i < *vertexCounter; i++) {
            pfiHomogeneousToScreen(&polygon[i]);
        }
        return PF_FALSE; // Is "2D"
    }

    if (Process_ClipPolygonW(polygon, vertexCounter) && Process_ClipPolygonXYZ(polygon, vertexCounter)) {
        for (int_fast8_t i = 0; i < *vertexCounter; i++) {
            // Calculation of the reciprocal of Z for the perspective correct
            polygon[i].homogeneous[2] = 1.0f / polygon[i].homogeneous[2];
            // Division of texture coordinates by the Z axis (perspective correct)
            pfmVec2Scale(polygon[i].texcoord, polygon[i].texcoord, polygon[i].homogeneous[2]);
            // Division of XY coordinates by weight
            PFfloat invW = 1.0f / polygon[i].homogeneous[3];
            polygon[i].homogeneous[0] *= invW;
            polygon[i].homogeneous[1] *= invW;
            // Transform to screen space
            pfiHomogeneousToScreen(&polygon[i]);
        }
    }

    return PF_TRUE; // Is 3D
}


/* Triangle rasterization functions */

#if PF_TRIANGLE_RASTER_MODE == PF_TRIANGLE_RASTER_BARYCENTRIC

void Rasterize_Triangle(PFface faceToRender, PFboolean is3D, const PFIvertex* v1, const PFIvertex* v2, const PFIvertex* v3, const PFMvec3 viewPos)
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
         || (faceToRender == PF_BACK  && signedArea <= 0)) {
            return;
        }

        /* Calculate the 2D bounding box of the triangle */

        xMin = (PFsizei)PF_MIN(x1, PF_MIN(x2, x3));
        yMin = (PFsizei)PF_MIN(y1, PF_MIN(y2, y3));
        xMax = (PFsizei)PF_MAX(x1, PF_MAX(x2, x3));
        yMax = (PFsizei)PF_MAX(y1, PF_MAX(y2, y3));

        if (!is3D) {
            xMin = (PFsizei)PF_CLAMP((PFint)xMin, G_currentCtx->vpMin[0], G_currentCtx->vpMax[0]);
            yMin = (PFsizei)PF_CLAMP((PFint)yMin, G_currentCtx->vpMin[1], G_currentCtx->vpMax[1]);
            xMax = (PFsizei)PF_CLAMP((PFint)xMax, G_currentCtx->vpMin[0], G_currentCtx->vpMax[0]);
            yMax = (PFsizei)PF_CLAMP((PFint)yMax, G_currentCtx->vpMin[1], G_currentCtx->vpMax[1]);
        }

        /* Barycentric interpolation */

        w1XStep = y3 - y2, w1YStep = x2 - x3;
        w2XStep = y1 - y3, w2YStep = x3 - x1;
        w3XStep = y2 - y1, w3YStep = x1 - x2;

        if (faceToRender == PF_BACK) {
            w1XStep = -w1XStep, w1YStep = -w1YStep;
            w2XStep = -w2XStep, w2YStep = -w2YStep;
            w3XStep = -w3XStep, w3YStep = -w3YStep;
        }

        w1Row = (xMin - x2)*w1XStep + w1YStep*(yMin - y2);
        w2Row = (xMin - x3)*w2XStep + w2YStep*(yMin - y3);
        w3Row = (xMin - x1)*w3XStep + w3YStep*(yMin - y1);
    }

    // Vector constants
    PFIsimdvi pixOffsetV = pfiSimdSetR_I32(0, 1, 2, 3, 4, 5, 6, 7);
    PFIsimdvi w1XStepV = pfiSimdMullo_I32(pfiSimdSet1_I32(w1XStep), pixOffsetV);
    PFIsimdvi w2XStepV = pfiSimdMullo_I32(pfiSimdSet1_I32(w2XStep), pixOffsetV);
    PFIsimdvi w3XStepV = pfiSimdMullo_I32(pfiSimdSet1_I32(w3XStep), pixOffsetV);

    // Calculate the reciprocal of the sum of the barycentric coordinates for normalization
    // NOTE: This sum remains constant throughout the triangle
    PFIsimdvf wInvSumV = pfiSimdSet1_F32(1.0f/(w1Row + w2Row + w3Row));

    // Load vertices data into SIMD registers
    PFcolor_simd c1V, c2V, c3V;
    pfiColorLoadUnpacked_simd(c1V, v1->color);
    pfiColorLoadUnpacked_simd(c2V, v2->color);
    pfiColorLoadUnpacked_simd(c3V, v3->color);

    PFsimdv3f p1V, p2V, p3V;
    pfiVec3Load_simd(p1V, v1->position);
    pfiVec3Load_simd(p2V, v2->position);
    pfiVec3Load_simd(p3V, v3->position);

    PFsimdv3f n1V, n2V, n3V;
    pfiVec3Load_simd(n1V, v1->normal);
    pfiVec3Load_simd(n2V, v2->normal);
    pfiVec3Load_simd(n3V, v3->normal);

    PFsimdv2f tc1V, tc2V, tc3V;
    pfiVec2Load_simd(tc1V, v1->texcoord);
    pfiVec2Load_simd(tc2V, v2->texcoord);
    pfiVec2Load_simd(tc3V, v3->texcoord);

    /* Get some contextual values */

    struct PFItex *texDst = G_currentCtx->currentFramebuffer->texture;
    struct PFItex *texSrc = G_currentCtx->currentTexture;

    PFfloat *zbDst = G_currentCtx->currentFramebuffer->zbuffer;

    PFIpixelgetter_simd fbGetter = texDst->getterSimd;
    PFIpixelsetter_simd fbSetter = texDst->setterSimd;
    PFsizei widthDst = texDst->w;
    void *pbDst = texDst->pixels;

    PFIsimdvf z1V = pfiSimdSet1_F32(v1->homogeneous[2]);
    PFIsimdvf z2V = pfiSimdSet1_F32(v2->homogeneous[2]);
    PFIsimdvf z3V = pfiSimdSet1_F32(v3->homogeneous[2]);

    PFsimdv3f viewPosV;
    pfiVec3Load_simd(viewPosV, viewPos);

    InterpolateColorSimdFunc interpolateColor = (G_currentCtx->shadingMode == PF_SMOOTH)
        ? pfiColorBarySmooth_simd : pfiColorBaryFlat_simd;

    PFIlight *lights = (G_currentCtx->state & PF_LIGHTING) ? G_currentCtx->activeLights : NULL;
    PFIblendfunc_simd blendFunction = (G_currentCtx->state & PF_BLEND) ? G_currentCtx->blendSimdFunction : NULL;
    PFIdepthfunc_simd depthFunction = (G_currentCtx->state & PF_DEPTH_TEST) ? G_currentCtx->depthSimdFunction : NULL;
    PFItexturesampler_simd texSampler = ((G_currentCtx->state & PF_TEXTURE_2D) && texSrc) ? texSrc->samplerSimd : NULL;

    /* Loop macro definition */

#ifdef _OPENMP
#define PF_TRIANGLE_TRAVEL_SIMD(PIXEL_CODE)                                                 \
    _Pragma("omp parallel for schedule(dynamic, PF_OPENMP_TRIANGLE_ROW_PER_THREAD)          \
        if((yMax - yMin)*(xMax - xMin) >= PF_OPENMP_RASTER_THRESHOLD_AREA)")                \
    for (PFsizei y = yMin; y <= yMax; ++y)  {                                               \
        size_t yOffset = y * widthDst;                                                      \
        PFint w1 = w1Row + (y - yMin)*w1YStep;                                              \
        PFint w2 = w2Row + (y - yMin)*w2YStep;                                              \
        PFint w3 = w3Row + (y - yMin)*w3YStep;                                              \
        for (PFsizei x = xMin; x <= xMax; x += PF_SIMD_SIZE) {                              \
            /* Load the current barycentric coordinates into SIMD registers */              \
            PFIsimdvi w1V = pfiSimdAdd_I32(pfiSimdSet1_I32(w1), w1XStepV);                   \
            PFIsimdvi w2V = pfiSimdAdd_I32(pfiSimdSet1_I32(w2), w2XStepV);                   \
            PFIsimdvi w3V = pfiSimdAdd_I32(pfiSimdSet1_I32(w3), w3XStepV);                   \
            /* Test if pixels are inside the triangle */                                    \
            PFIsimdvi mask = pfiSimdOr_I32(pfiSimdOr_I32(w1V, w2V), w3V);                    \
            mask = pfiSimdCmpGT_I32(mask, pfiSimdSetZero_I32());                            \
            /* Bounds check to ensure pixels' x-coords are within the xMax limit */         \
            /* Used for "2D" rendering; 3D clipping removes these cases. */                 \
            mask = pfiSimdAnd_I32(mask, pfiSimdCmpLT_I32(                                   \
                pfiSimdAdd_I32(pfiSimdSet1_I32(x), pixOffsetV), pfiSimdSet1_I32(xMax)));    \
            /* Normalize weights */                                                         \
            PFIsimdvf w1NormV = pfiSimdMul_F32(pfiSimdConvert_I32_F32(w1V), wInvSumV);       \
            PFIsimdvf w2NormV = pfiSimdMul_F32(pfiSimdConvert_I32_F32(w2V), wInvSumV);       \
            PFIsimdvf w3NormV = pfiSimdMul_F32(pfiSimdConvert_I32_F32(w3V), wInvSumV);       \
            /* Compute Z-Depth values */                                                    \
            PFIsimdvf zV; {                                                                  \
                PFIsimdvf wZ1 = pfiSimdMul_F32(z1V, w1NormV);                                \
                PFIsimdvf wZ2 = pfiSimdMul_F32(z2V, w2NormV);                                \
                PFIsimdvf wZ3 = pfiSimdMul_F32(z3V, w3NormV);                                \
                zV = pfiSimdAdd_F32(pfiSimdAdd_F32(wZ1, wZ2), wZ3);                         \
                zV = pfiSimdRCP_F32(zV);                                                    \
            }                                                                               \
            /* Depth Testing */                                                             \
            PFIsimdvf depths = pfiSimdLoad_F32(zbDst + yOffset + x);                         \
            if (depthFunction)  {                                                           \
                mask = pfiSimdAnd_I32(mask, pfiSimdCast_F32_I32(                            \
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
    for (PFsizei y = yMin; y <= yMax; ++y) {                                                \
        size_t yOffset = y * widthDst;                                                      \
        int w1 = w1Row;                                                                     \
        int w2 = w2Row;                                                                     \
        int w3 = w3Row;                                                                     \
        for (PFsizei x = xMin; x <= xMax; x += PF_SIMD_SIZE) {                              \
            /* Load the current barycentric coordinates into SIMD registers */              \
            PFIsimdvi w1V = pfiSimdAdd_I32(pfiSimdSet1_I32(w1), w1XStepV);                   \
            PFIsimdvi w2V = pfiSimdAdd_I32(pfiSimdSet1_I32(w2), w2XStepV);                   \
            PFIsimdvi w3V = pfiSimdAdd_I32(pfiSimdSet1_I32(w3), w3XStepV);                   \
            /* Test if pixels are inside the triangle */                                    \
            PFIsimdvi mask = pfiSimdOr_I32(pfiSimdOr_I32(w1V, w2V), w3V);                    \
            mask = pfiSimdCmpGT_I32(mask, pfiSimdSetZero_I32());                            \
            /* Bounds check to ensure pixels' x-coords are within the xMax limit */         \
            /* Used for "2D" rendering; 3D clipping removes these cases. */                 \
            mask = pfiSimdAnd_I32(mask, pfiSimdCmpLT_I32(                                   \
                pfiSimdAdd_I32(pfiSimdSet1_I32(x), pixOffsetV), pfiSimdSet1_I32(xMax)));    \
            /* Normalize weights */                                                         \
            PFIsimdvf w1NormV = pfiSimdMul_F32(pfiSimdConvert_I32_F32(w1V), wInvSumV);       \
            PFIsimdvf w2NormV = pfiSimdMul_F32(pfiSimdConvert_I32_F32(w2V), wInvSumV);       \
            PFIsimdvf w3NormV = pfiSimdMul_F32(pfiSimdConvert_I32_F32(w3V), wInvSumV);       \
            /* Compute Z-Depth values */                                                    \
            PFIsimdvf zV; {                                                                  \
                PFIsimdvf wZ1 = pfiSimdMul_F32(z1V, w1NormV);                                \
                PFIsimdvf wZ2 = pfiSimdMul_F32(z2V, w2NormV);                                \
                PFIsimdvf wZ3 = pfiSimdMul_F32(z3V, w3NormV);                                \
                zV = pfiSimdAdd_F32(pfiSimdAdd_F32(wZ1, wZ2), wZ3);                         \
                zV = pfiSimdRCP_F32(zV);                                                    \
            }                                                                               \
            /* Depth Testing */                                                             \
            PFIsimdvf depths = pfiSimdLoad_F32(zbDst + yOffset + x);                         \
            if (depthFunction) {                                                            \
                mask = pfiSimdAnd_I32(mask, pfiSimdCast_F32_I32(                            \
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
        PFcolor_simd fragments; \
        interpolateColor(fragments, c1V, c2V, c3V, w1NormV, w2NormV, w3NormV);

#   define TEXTURING() \
        PFsimdv2f texcoords; \
        { \
            PFsimdv2f zeroV2; pfiVec2Zero_simd(zeroV2); \
            pfiVec2BaryInterpR_simd(texcoords, tc1V, tc2V, tc3V, w1NormV, w2NormV, w3NormV); \
            if (is3D) pfiVec2Scale_simd(texcoords, texcoords, zV); /* Perspective correct */ \
            pfiVec2Blend_simd(texcoords, zeroV2, texcoords, pfiSimdCast_I32_F32(mask)); \
            PFcolor_simd texels; pfiColorUnpack_simd(texels, texSampler(texSrc, texcoords)); \
            pfiBlendMultiplicative_simd(fragments, texels, fragments); \
        }

#   define LIGHTING() \
    { \
        PFsimdv3f normals, positions; \
        pfiVec3BaryInterpR_simd(normals, n1V, n2V, n3V, w1NormV, w2NormV, w3NormV); \
        pfiVec3BaryInterpR_simd(positions, p1V, p2V, p3V, w1NormV, w2NormV, w3NormV); \
        pfiSimdLightingProcess(fragments, lights, &G_currentCtx->faceMaterial[faceToRender], viewPosV, positions, normals); \
    }

#   define SET_FRAG() \
        if (blendFunction) { \
            PFcolor_simd dstCol; \
            pfiColorUnpack_simd(dstCol, fbGetter(pbDst, \
                pfiSimdAdd_I32(pfiSimdSet1_I32(yOffset + x), pixOffsetV))); \
            blendFunction(fragments, fragments, dstCol); \
        } \
        fbSetter(pbDst, yOffset + x, pfiColorPack_simd(fragments), mask); \
        pfiSimdStore_F32(zbDst + yOffset + x, pfiSimdBlendV_F32(depths, zV, pfiSimdCast_I32_F32(mask)));

    /* Loop rasterization */

    if (texSampler && lights) {
        PF_TRIANGLE_TRAVEL_SIMD({
            GET_FRAG();
            TEXTURING();
            LIGHTING();
            SET_FRAG();
        })
    } else if (texSampler) {
        PF_TRIANGLE_TRAVEL_SIMD({
            GET_FRAG();
            TEXTURING();
            SET_FRAG();
        })
    } else if (lights) {
        PF_TRIANGLE_TRAVEL_SIMD({
            GET_FRAG();
            LIGHTING();
            SET_FRAG();
        })
    } else {
        PF_TRIANGLE_TRAVEL_SIMD({
            GET_FRAG();
            SET_FRAG();
        })
    }
}

#else // PF_TRIANGLE_RASTER_MODE == PR_TRIANGLE_RASTER_SCANLINES

void Rasterize_Triangle(PFface faceToRender, PFboolean is3D, const PFIvertex* v1, const PFIvertex* v2, const PFIvertex* v3, const PFMvec3 viewPos)
{
    /* Check if the face can be rendered, if not, skip */
    {
        PFfloat signedArea = pfmGeo2DSignedTriangleArea(v1->screen, v2->screen, v3->screen);
        if ((faceToRender == PF_FRONT && signedArea > 0) || (faceToRender == PF_BACK && signedArea < 0)) {
            return;
        }
    }

    /* Sort vertices in ascending order of y coordinates */
    {
        const PFIvertex* vTmp;
        if (v2->screen[1] < v1->screen[1]) { vTmp = v1; v1 = v2; v2 = vTmp; }
        if (v3->screen[1] < v1->screen[1]) { vTmp = v1; v1 = v3; v3 = vTmp; }
        if (v3->screen[1] < v2->screen[1]) { vTmp = v2; v2 = v3; v3 = vTmp; }
    }

    /* Cache vertex data */

    PFint x1 = (PFint)v1->screen[0], y1 = (PFint)v1->screen[1];
    PFint x2 = (PFint)v2->screen[0], y2 = (PFint)v2->screen[1];
    PFint x3 = (PFint)v3->screen[0], y3 = (PFint)v3->screen[1];

    const PFMvec2_cptr uv1 = v1->texcoord;
    const PFMvec2_cptr uv2 = v2->texcoord;
    const PFMvec2_cptr uv3 = v3->texcoord;

    const PFMvec3_cptr p1 = v1->position;
    const PFMvec3_cptr p2 = v2->position;
    const PFMvec3_cptr p3 = v3->position;
    
    const PFMvec3_cptr n1 = v1->normal;
    const PFMvec3_cptr n2 = v2->normal;
    const PFMvec3_cptr n3 = v3->normal;

    PFfloat z1 = v1->homogeneous[2];
    PFfloat z2 = v2->homogeneous[2];
    PFfloat z3 = v3->homogeneous[2];

    PFcolor c1 = v1->color;
    PFcolor c2 = v2->color;
    PFcolor c3 = v3->color;

    /* Get some contextual values */

    struct PFItex *texSrc = G_currentCtx->currentTexture;
    PFfloat *zbDst = G_currentCtx->currentFramebuffer->zbuffer;
    struct PFItex *texDst = G_currentCtx->currentFramebuffer->texture;
    PFIlight *lights = (G_currentCtx->state & PF_LIGHTING) ? G_currentCtx->activeLights : NULL;
    PFIblendfunc blendFunction = (G_currentCtx->state & PF_BLEND) ? G_currentCtx->blendFunction : NULL;
    PFIdepthfunc depthFunction = (G_currentCtx->state & PF_DEPTH_TEST) ? G_currentCtx->depthFunction : NULL;
    PFItexturesampler texSampler = ((G_currentCtx->state & PF_TEXTURE_2D) && texSrc) ? texSrc->sampler : NULL;
    InterpolateColorFunc interpolateColor = (G_currentCtx->shadingMode == PF_SMOOTH) ? pfiColorLerpSmooth : pfiColorLerpFlat;

    /*  */

    PFint yMin = y1;
    PFint yMax = y3;

    if (!is3D) {
        yMin = PF_CLAMP(yMin, G_currentCtx->vpMin[1], G_currentCtx->vpMax[1]);
        yMax = PF_CLAMP(yMax, G_currentCtx->vpMin[1], G_currentCtx->vpMax[1]);
    }

    PFsizei yOffset = yMin * texDst->w;

    /*  */

    PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    PFfloat alpha = invTotalHeight * (yMin - y1);
    PFfloat beta1 = invSegmentHeight21 * (yMin - y1);   // First half
    PFfloat beta2 = invSegmentHeight32 * (yMin - y2);   // Second half

    /* Travel the triangle from top to bottom */

    for (PFint y = yMin; y <= yMax; y++, yOffset += texDst->w) {
        alpha += invTotalHeight;
        beta1 += invSegmentHeight21;
        beta2 += invSegmentHeight32;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;
        PFMvec2 uvA, uvB;
        PFMvec3 pA, pB, nA, nB;

        if (y < y2) { // First half
            xA = (PFint)(x1 + (x3 - x1) * alpha);
            xB = (PFint)(x1 + (x2 - x1) * beta1);
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta1;
            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta1);
            if (texSampler) {
                pfmVec2LerpR(uvA, uv1, uv3, alpha);
                pfmVec2LerpR(uvB, uv1, uv2, beta1);
            }
            if (lights) {
                pfmVec3LerpR(pA, p1, p3, alpha);
                pfmVec3LerpR(pB, p1, p2, beta1);
                pfmVec3LerpR(nA, n1, n3, alpha);
                pfmVec3LerpR(nB, n1, n2, beta1);
            }
        } else { // Second half
            xA = (PFint)(x1 + (x3 - x1) * alpha);
            xB = (PFint)(x2 + (x3 - x2) * beta2);
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta2;
            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta2);
            if (texSampler) {
                pfmVec2LerpR(uvA, uv1, uv3, alpha);
                pfmVec2LerpR(uvB, uv2, uv3, beta2);
            }
            if (lights) {
                pfmVec3LerpR(pA, p1, p3, alpha);
                pfmVec3LerpR(pB, p2, p3, beta2);
                pfmVec3LerpR(nA, n1, n3, alpha);
                pfmVec3LerpR(nB, n2, n3, beta2);
            }
        }

        /* Swap endpoints if necessary to ensure xA <= xB */

        if (xA > xB) {
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

        if (!is3D) {
            xMin = PF_CLAMP(xMin, G_currentCtx->vpMin[0], G_currentCtx->vpMax[0]);
            xMax = PF_CLAMP(xMax, G_currentCtx->vpMin[0], G_currentCtx->vpMax[0]);
        }

        PFsizei xyOffset = yOffset + xMin;

        /*  */

        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f / (xB - xA);
        PFfloat gamma = xInvLen * (xMin - xA);

        /* Draw Horizontal Line */

        for (PFint x = xMin; x <= xMax; x++, xyOffset++, gamma += xInvLen) {
            PFfloat z = 1.0f / (zA + (zB - zA) * gamma);
            if (!depthFunction || depthFunction(z, zbDst[xyOffset])) {
                PFcolor fragment = interpolateColor(cA, cB, gamma);
                if (texSampler) {
                    PFMvec2 uv; pfmVec2LerpR(uv, uvA, uvB, gamma);
                    if (is3D) pfmVec2Scale(uv, uv, z); // Perspective correct
                    PFcolor texel = texSampler(texSrc, uv[0], uv[1]);
                    fragment = pfiBlendMultiplicative(texel, fragment);
                }
                if (lights) {
                    PFMvec3 position; pfmVec3LerpR(position, pA, pB, gamma);
                    PFMvec3 normal; pfmVec3LerpR(normal, nA, nB, gamma);
                    fragment = pfiLightingProcess(G_currentCtx->activeLights,
                        &G_currentCtx->faceMaterial[faceToRender], fragment,
                        viewPos, position, normal);
                }
                if (blendFunction) fragment = blendFunction(fragment, texDst->getter(texDst->pixels, xyOffset));
                texDst->setter(texDst->pixels, xyOffset, fragment);
                zbDst[xyOffset] = z;
            }
        }
    }
}

#endif //PF_TRIANGLE_RASTER_MODE
