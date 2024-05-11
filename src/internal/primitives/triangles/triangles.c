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

#include "./triangles.h"
#include <stdint.h>

/* Internal typedefs */

#ifdef PF_SCANLINES_RASTER_METHOD
typedef PFcolor (*InterpolateColorFunc)(PFcolor, PFcolor, PFfloat);
#else //PF_BARYCENTRIC_RASTER_METHOD
typedef PFcolor (*InterpolateColorFunc)(PFcolor, PFcolor, PFcolor, PFfloat, PFfloat, PFfloat);
#endif //PF_RASTER_METHOD


/* Including internal function prototypes */

extern void pfInternal_HomogeneousToScreen(PFvertex* restrict v);


/* Internal helper function declarations */

static PFvertex Helper_LerpVertex(const PFvertex* start, const PFvertex* end, PFfloat t);

#ifdef PF_SCANLINES_RASTER_METHOD

static PFboolean Helper_FaceCanBeRendered(PFface faceToRender, PFfloat* area, const PFMvec2 p1, const PFMvec2 p2, const PFMvec2 p3);
static void Helper_SortVertices(const PFvertex** v1, const PFvertex** v2, const PFvertex** v3);

static PFcolor Helper_InterpolateColor_SMOOTH(PFcolor v1, PFcolor v2, PFfloat t);
static PFcolor Helper_InterpolateColor_FLAT(PFcolor v1, PFcolor v2, PFfloat t);

#else //PF_BARYCENTRIC_RASTER_METHOD

static PFcolor Helper_InterpolateColor_SMOOTH(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3);
static PFcolor Helper_InterpolateColor_FLAT(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3);

#endif //PF_RASTER_METHOD


/* Polygon processing functions */

static PFboolean Process_ClipPolygonW(PFvertex* restrict polygon, int_fast8_t* restrict vertexCounter);
static PFboolean Process_ClipPolygonXYZ(PFvertex* restrict polygon, int_fast8_t* restrict vertexCounter);

PFboolean Process_ProjectAndClipTriangle(PFvertex* restrict polygon, int_fast8_t* restrict vertexCounter)
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

PFboolean Process_ClipPolygonW(PFvertex* restrict polygon, int_fast8_t* restrict vertexCounter)
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
            polygon[(*vertexCounter)++] = Helper_LerpVertex(prevVt, &input[i], 
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

PFboolean Process_ClipPolygonXYZ(PFvertex* restrict polygon, int_fast8_t* restrict vertexCounter)
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
                polygon[(*vertexCounter)++] = Helper_LerpVertex(prevVt, &input[i], (prevVt->homogeneous[3] - prevVt->homogeneous[iAxis]) /
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
                polygon[(*vertexCounter)++] = Helper_LerpVertex(prevVt, &input[i], (prevVt->homogeneous[3] + prevVt->homogeneous[iAxis]) /
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


/* Triangle rasterization functions */

static PFcolor Process_Lights(const PFlight* activeLights, const PFmaterial* material,
    PFcolor diffuse, const PFMvec3 viewPos, const PFMvec3 fragPos, const PFMvec3 fragNormal);

#ifdef PF_SCANLINES_RASTER_METHOD

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

    PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];
    PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;

    /* Precompute inverse heights for interpolation */

    PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    /* Choose color interpolation method based on shading mode */

    InterpolateColorFunc interpolateColor = (currentCtx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    /* Extract framebuffer information */

    PFblendfunc blendFunction = currentCtx->state & PF_BLEND ? currentCtx->blendFunction : NULL;
    PFpixelsetter pixelSetter = currentCtx->currentFramebuffer->texture.pixelSetter;
    PFpixelgetter pixelGetter = currentCtx->currentFramebuffer->texture.pixelGetter;
    PFsizei widthDst = currentCtx->currentFramebuffer->texture.width;
    void *pbDst = currentCtx->currentFramebuffer->texture.pixels;
    PFfloat *zbDst = currentCtx->currentFramebuffer->zbuffer;
    PFtexture *texture = currentCtx->currentTexture;

    /*  */

    PFint yMin = y1;
    PFint yMax = y3;

    if (!is3D)
    {
        yMin = CLAMP(yMin, currentCtx->vpMin[1], currentCtx->vpMax[1]);
        yMax = CLAMP(yMax, currentCtx->vpMin[1], currentCtx->vpMax[1]);
    }

    /* Travel the triangle from top to bottom */

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for schedule(dynamic) \
            if(area >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif
    for (PFint y = yMin; y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1)*invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;

        PFMvec2 uvA, uvB;
        PFMvec3 pA, pB;
        PFMvec3 nA, nB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1)*invSegmentHeight21;

            xA = x1 + (x3 - x1)*alpha;
            zA = z1 + (z3 - z1)*alpha;

            xB = x1 + (x2 - x1)*beta;
            zB = z1 + (z2 - z1)*beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);

            if (texturing)
            {
                pfmVec2Lerp(uvA, v1->texcoord, v3->texcoord, alpha);
                pfmVec2Lerp(uvB, v1->texcoord, v2->texcoord, beta);
            }

            if (lighting)
            {
                pfmVec3Lerp(pA, v1->position, v3->position, alpha);
                pfmVec3Lerp(pB, v1->position, v2->position, beta);
                pfmVec3Lerp(nA, v1->normal, v3->normal, alpha);
                pfmVec3Lerp(nB, v1->normal, v2->normal, beta);
            }
        }
        else // Second half
        {
            beta = (y - y2 + 1)*invSegmentHeight32;

            xA = x1 + (x3 - x1)*alpha;
            zA = z1 + (z3 - z1)*alpha;

            xB = x2 + (x3 - x2)*beta;
            zB = z2 + (z3 - z2)*beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);

            if (texturing)
            {
                pfmVec2Lerp(uvA, v1->texcoord, v3->texcoord, alpha);
                pfmVec2Lerp(uvB, v2->texcoord, v3->texcoord, beta);
            }

            if (lighting)
            {
                pfmVec3Lerp(pA, v1->position, v3->position, alpha);
                pfmVec3Lerp(pB, v2->position, v3->position, beta);
                pfmVec3Lerp(nA, v1->normal, v3->normal, alpha);
                pfmVec3Lerp(nB, v2->normal, v3->normal, beta);
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

        /* Draw Horizontal Line */

        PFint xMin = xA;
        PFint xMax = xB;

        if (!is3D)
        {
            xMin = CLAMP(xMin, currentCtx->vpMin[0], currentCtx->vpMax[0]);
            xMax = CLAMP(xMax, currentCtx->vpMin[0], currentCtx->vpMax[0]);
        }

        PFsizei xyOffset = y*widthDst + xMin;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xMin; x <= xMax; x++, xyOffset++)
        {
            /* Calculate interpolation factor and Z */

            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            /* Perform depth test */

            if (noDepth || currentCtx->depthFunction(z, zbDst[xyOffset]))
            {
                /* Obtain fragment color */

                PFcolor fragment = interpolateColor(cA, cB, t);

                /* Blend with corresponding texture sample */

                if (texturing)
                {
                    PFMvec2 uv;
                    pfmVec2Lerp(uv, uvA, uvB, t);

                    if (is3D)
                    {
                        // NOTE 1: Divided by 'z', correct perspective
                        // NOTE 2: 'z' is actually the reciprocal
                        pfmVec2Scale(uv, uv, z);
                    }

                    PFcolor tex = pfGetTextureSample(texture, uv[0], uv[1]);
                    fragment = pfBlendMultiplicative(tex, fragment);
                }

                /* Compute lighting */

                if (lighting)
                {
                    PFMvec3 position;
                    pfmVec3Lerp(position, pA, pB, t);

                    PFMvec3 normal;
                    pfmVec3Lerp(normal, nA, nB, t);

                    fragment = Process_Lights(currentCtx->activeLights,
                        &currentCtx->faceMaterial[faceToRender], fragment,
                        viewPos, position, normal);
                }

                /* Apply final color and depth */

                PFcolor finalColor = blendFunction ? blendFunction(fragment, pixelGetter(pbDst, xyOffset)) : fragment;
                pixelSetter(pbDst, xyOffset, finalColor);
                zbDst[xyOffset] = z;
            }
        }
    }
}

#else //PF_BARYCENTRIC_RASTER_METHOD

void Rasterize_Triangle(PFface faceToRender, PFboolean is3D, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
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

    PFsizei xMin = (PFsizei)MIN(x1, MIN(x2, x3));
    PFsizei yMin = (PFsizei)MIN(y1, MIN(y2, y3));
    PFsizei xMax = (PFsizei)MAX(x1, MAX(x2, x3));
    PFsizei yMax = (PFsizei)MAX(y1, MAX(y2, y3));

    if (!is3D)
    {
        xMin = (PFsizei)CLAMP((PFint)xMin, currentCtx->vpMin[0], currentCtx->vpMax[0]);
        yMin = (PFsizei)CLAMP((PFint)yMin, currentCtx->vpMin[1], currentCtx->vpMax[1]);
        xMax = (PFsizei)CLAMP((PFint)xMax, currentCtx->vpMin[0], currentCtx->vpMax[0]);
        yMax = (PFsizei)CLAMP((PFint)yMax, currentCtx->vpMin[1], currentCtx->vpMax[1]);

        if (xMin == xMax && yMin == yMax) return;
    }

    /* Barycentric interpolation */

    PFint w1XStep = y3 - y2, w1YStep = x2 - x3;
    PFint w2XStep = y1 - y3, w2YStep = x3 - x1;
    PFint w3XStep = y2 - y1, w3YStep = x1 - x2;

    if (faceToRender == PF_BACK)
    {
        w1XStep = -w1XStep, w1YStep = -w1YStep;
        w2XStep = -w2XStep, w2YStep = -w2YStep;
        w3XStep = -w3XStep, w3YStep = -w3YStep;
    }

    PFint w1Row = (xMin - x2)*w1XStep + w1YStep*(yMin - y2);
    PFint w2Row = (xMin - x3)*w2XStep + w2YStep*(yMin - y3);
    PFint w3Row = (xMin - x1)*w3XStep + w3YStep*(yMin - y1);

    /*
        Finally, we calculate the inverse of the sum of
        the barycentric coordinates for the top-left point; this
        sum always remains the same, regardless of the coordinate
        within the triangle.
    */

    PFfloat wInvSum = 1.0f/(w1Row + w2Row + w3Row);

    /* Get some contextual values */

    InterpolateColorFunc interpolateColor = (currentCtx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    PFblendfunc blendFunction = currentCtx->state & PF_BLEND ? currentCtx->blendFunction : NULL;
    PFpixelgetter pixelGetter = currentCtx->currentFramebuffer->texture.pixelGetter;
    PFpixelsetter pixelSetter = currentCtx->currentFramebuffer->texture.pixelSetter;
    PFsizei widthDst = currentCtx->currentFramebuffer->texture.width;
    void *pbDst = currentCtx->currentFramebuffer->texture.pixels;
    PFfloat *zbDst = currentCtx->currentFramebuffer->zbuffer;
    PFtexture *texture = currentCtx->currentTexture;

    PFfloat z1 = v1->homogeneous[2];
    PFfloat z2 = v2->homogeneous[2];
    PFfloat z3 = v3->homogeneous[2];

    const PFboolean noDepth = !(currentCtx->state & PF_DEPTH_TEST);
    const PFboolean lighting = (currentCtx->state & PF_LIGHTING) && currentCtx->activeLights;
    const PFboolean texturing = (currentCtx->state & PF_TEXTURE_2D) && currentCtx->currentTexture;

    /* Loop macro definition */

#ifdef PF_SUPPORT_OPENMP
#   define BEGIN_LOOP() \
    _Pragma("omp parallel for schedule(dynamic) \
        if((yMax - yMin)*(xMax - xMin) >= PF_OPENMP_RASTER_THRESHOLD_AREA)") \
    for (PFsizei y = yMin; y <= yMax; y++) \
    { \
        PFint i = y - yMin; \
        PFint w1 = w1Row + i*w1YStep; \
        PFint w2 = w2Row + i*w2YStep; \
        PFint w3 = w3Row + i*w3YStep; \
        const PFsizei yOffset = y*widthDst; \
        \
        for (PFsizei x = xMin; x <= xMax; x++) \
        { \
            if ((w1 | w2 | w3) >= 0) \
            { \
                PFfloat aW1 = w1*wInvSum, aW2 = w2*wInvSum, aW3 = w3*wInvSum; \
                PFfloat z = 1.0f/(aW1*z1 + aW2*z2 + aW3*z3); \
                PFsizei xyOffset = yOffset + x; \
                \
                if (noDepth || currentCtx->depthFunction(z, zbDst[xyOffset])) \
                {

#   define END_LOOP() \
                } \
            } \
            w1 += w1XStep, w2 += w2XStep, w3 += w3XStep; \
        } \
    }
#else
#   define BEGIN_LOOP() \
    for (PFsizei y = yMin, yOffset = yMin*widthDst; y <= yMax; y++, yOffset += widthDst) \
    { \
        PFint w1 = w1Row, w2 = w2Row, w3 = w3Row; \
        \
        for (PFsizei x = xMin; x <= xMax; x++) \
        { \
            if ((w1 | w2 | w3) >= 0) \
            { \
                PFfloat aW1 = w1*wInvSum, aW2 = w2*wInvSum, aW3 = w3*wInvSum; \
                PFfloat z = 1.0f/(aW1*z1 + aW2*z2 + aW3*z3); \
                PFsizei xyOffset = yOffset + x; \
                \
                if (noDepth || currentCtx->depthFunction(z, zbDst[xyOffset])) \
                {

#   define END_LOOP() \
                } \
            } \
            w1 += w1XStep, w2 += w2XStep, w3 += w3XStep; \
        } \
        w1Row += w1YStep, w2Row += w2YStep, w3Row += w3YStep; \
    }
#endif

    /* Processing macro definitions */

#   define GET_FRAG() \
    PFcolor fragment = interpolateColor( \
        v1->color, v2->color, v3->color, \
        aW1, aW2, aW3);

#   define TEXTURING() \
        PFMvec2 texcoord; \
        pfmVec2BaryInterp(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3); \
        if (is3D) texcoord[0] *= z, texcoord[1] *= z; /* Perspective correct */ \
        PFcolor texel = pfGetTextureSample(texture, texcoord[0], texcoord[1]); \
        fragment = pfBlendMultiplicative(texel, fragment);

#   define LIGHTING() \
        PFMvec3 normal, position; \
        pfmVec3BaryInterp(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3); \
        pfmVec3BaryInterp(position, v1->position, v2->position, v3->position, aW1, aW2, aW3); \
        fragment = Process_Lights(currentCtx->activeLights, &currentCtx->faceMaterial[faceToRender], fragment, viewPos, position, normal);

#   define SET_FRAG() \
        PFcolor finalColor = blendFunction ? blendFunction(fragment, pixelGetter(pbDst, xyOffset)) : fragment; \
        pixelSetter(pbDst, xyOffset, finalColor); \
        zbDst[xyOffset] = z;

    /* Loop rasterization */

    if (texturing && lighting)
    {
        BEGIN_LOOP();
        GET_FRAG();
        TEXTURING();
        LIGHTING();
        SET_FRAG();
        END_LOOP();
    }
    else if (texturing)
    {
        BEGIN_LOOP();
        GET_FRAG();
        TEXTURING();
        SET_FRAG();
        END_LOOP();
    }
    else if (lighting)
    {
        BEGIN_LOOP();
        GET_FRAG();
        LIGHTING();
        SET_FRAG();
        END_LOOP();
    }
    else
    {
        BEGIN_LOOP();
        GET_FRAG();
        SET_FRAG();
        END_LOOP();
    }
}

#endif //PF_RASTER_METHOD

static PFcolor Process_Lights(const PFlight* activeLights, const PFmaterial* material, PFcolor diffuse, const PFMvec3 viewPos, const PFMvec3 fragPos, const PFMvec3 fragNormal)
{
    // Final color
    // Calculate the emission component of the final color
    PFubyte R = material->emission.r;
    PFubyte G = material->emission.g;
    PFubyte B = material->emission.b;

    // Ambient component
    // Calculate the ambient lighting contribution
    PFubyte aR = (material->ambient.r*diffuse.r)/255;
    PFubyte aG = (material->ambient.g*diffuse.g)/255;
    PFubyte aB = (material->ambient.b*diffuse.b)/255;

    // Compute the view direction from fragment position
    PFMvec3 viewDir;
    pfmVec3Direction(viewDir, viewPos, fragPos);

    // Specular properties
    // Retrieve material's shininess and specular color
    PFfloat shininess = material->shininess;
    PFcolor specular = material->specular;

    // Loop through active lights
    for (const PFlight *light = activeLights; light != NULL; light = light->next)
    {
        // Declare the light contribution and initialize it to zero for now.
        PFubyte lR = 0, lG = 0, lB = 0;

        // Compute light direction
        PFMvec3 lightDir;
        pfmVec3Sub(lightDir, light->position, fragPos);

        // Compute distance from light to fragment position
        // And also normalize the light direction if necessary.
        PFfloat lightToFragPosDistSq =
            lightDir[0]*lightDir[0] +
            lightDir[1]*lightDir[1] +
            lightDir[2]*lightDir[2];

        PFfloat lightToFragPosDist = 0;
        if (lightToFragPosDistSq != 0.0f)
        {
            lightToFragPosDist = sqrtf(lightToFragPosDistSq);
            PFfloat invMag = 1.0f/lightToFragPosDist;
            for (int_fast8_t i = 0; i < 3; i++)
            {
                lightDir[i] *= invMag;
            }
        }

        // Spotlight (soft edges)
        PFubyte intensity = 255;
        if (light->innerCutOff < M_PI)
        {
            PFMvec3 negLightDir;
            pfmVec3Neg(negLightDir, light->direction);

            PFfloat theta = pfmVec3Dot(lightDir, negLightDir);
            PFfloat epsilon = light->innerCutOff - light->outerCutOff;
            intensity = CLAMP((PFint)(255*(theta - light->outerCutOff)/epsilon), 0, 255);

            if (intensity == 0)
                goto apply_light_contribution;
        }

        // Attenuation
        PFubyte attenuation = 255;
        if (light->attLinear || light->attQuadratic)
        {
            attenuation = 255/(light->attConstant +
                light->attLinear*lightToFragPosDist +
                light->attQuadratic*lightToFragPosDistSq);

            if (attenuation == 0)
                goto apply_light_contribution;
        }

        // Factor used to scale the final color
        PFubyte factor = (intensity*attenuation)/255;

        // Diffuse component
        // Calculate the diffuse reflection of the light
        PFubyte diff = MAX((PFint)(255*pfmVec3Dot(fragNormal, lightDir)), 0);
        lR = MIN_255(lR + (diffuse.r*light->diffuse.r*diff)/(255*255));
        lG = MIN_255(lG + (diffuse.g*light->diffuse.g*diff)/(255*255));
        lB = MIN_255(lB + (diffuse.b*light->diffuse.b*diff)/(255*255));

        // Specular component
#       ifndef PF_PHONG_REFLECTION
            // Blinn-Phong
            PFMvec3 halfWayDir;
            pfmVec3Add(halfWayDir, lightDir, viewDir);
            pfmVec3Normalize(halfWayDir, halfWayDir);
            PFubyte spec = 255*powf(fmaxf(pfmVec3Dot(fragNormal, halfWayDir), 0.0f), shininess);
#       else
            // Phong
            PFMvec3 reflectDir, negLightDir;
            pfmVec3Neg(negLightDir, lightDir);
            pfmVec3Reflect(reflectDir, negLightDir, fragNormal);
            PFubyte spec = 255*powf(fmaxf(pfmVec3Dot(reflectDir, viewDir), 0.0f), shininess);
#       endif

        lR = MIN_255(lR + (specular.r*light->specular.r*spec)/(255*255));
        lG = MIN_255(lG + (specular.g*light->specular.g*spec)/(255*255));
        lB = MIN_255(lB + (specular.b*light->specular.b*spec)/(255*255));

        // Apply spotlight soft edges and distance attenuation
        lR = (lR*factor)/255;
        lG = (lG*factor)/255;
        lB = (lB*factor)/255;

        // Add ambient contribution of the light
        // Then add the light's contribution to the final color
        apply_light_contribution:
        R = MIN_255(R + lR + (aR*light->ambient.r)/255);
        G = MIN_255(G + lG + (aG*light->ambient.g)/255);
        B = MIN_255(B + lB + (aB*light->ambient.b)/255);
    }

    // Return the final computed color
    return (PFcolor) { R, G, B, diffuse.a };
}


/* Internal helper function definitions */

PFvertex Helper_LerpVertex(const PFvertex* start, const PFvertex* end, PFfloat t)
{
    PFvertex result = { 0 };

    const PFubyte *startCol = (const PFubyte*)(&start->color);
    const PFubyte *endCol = (const PFubyte*)(&end->color);
    PFubyte *resultCol = (PFubyte*)(&result.color);
    PFubyte uT = 255*t;

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        result.homogeneous[i] = start->homogeneous[i] + t*(end->homogeneous[i] - start->homogeneous[i]);
        result.position[i] = start->position[i] + t*(end->position[i] - start->position[i]);
        resultCol[i] = startCol[i] + (uT*((PFint)endCol[i] - startCol[i]))/255;

        if (i < 2) result.texcoord[i] = start->texcoord[i] + t*(end->texcoord[i] - start->texcoord[i]);
        if (i < 3) result.normal[i] = start->normal[i] + t*(end->normal[i] - start->normal[i]);
    }

    return result;
}

#ifdef PF_SCANLINES_RASTER_METHOD

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

PFcolor Helper_InterpolateColor_SMOOTH(PFcolor v1, PFcolor v2, PFfloat t)
{
    return (PFcolor) {
        v1.r + t*(v2.r - v1.r),
        v1.g + t*(v2.g - v1.g),
        v1.b + t*(v2.b - v1.b),
        v1.a + t*(v2.a - v1.a)
    };
}

PFcolor Helper_InterpolateColor_FLAT(PFcolor v1, PFcolor v2, PFfloat t)
{
    return (t < 0.5f) ? v1 : v2;
}

#else //PF_BARYCENTRIC_RASTER_METHOD

PFcolor Helper_InterpolateColor_SMOOTH(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    PFubyte uW1 = 255*w1;
    PFubyte uW2 = 255*w2;
    PFubyte uW3 = 255*w3;

    return (PFcolor) {
        ((uW1*v1.r) + (uW2*v2.r) + (uW3*v3.r))/255,
        ((uW1*v1.g) + (uW2*v2.g) + (uW3*v3.g))/255,
        ((uW1*v1.b) + (uW2*v2.b) + (uW3*v3.b))/255,
        ((uW1*v1.a) + (uW2*v2.a) + (uW3*v3.a))/255
    };
}

PFcolor Helper_InterpolateColor_FLAT(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    return ((w1 > w2) & (w1 > w3)) ? v1 : (w2 >= w3) ? v2 : v3;
}

#endif
