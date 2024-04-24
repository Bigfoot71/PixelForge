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

#include "internal/context.h"

/* Including internal function prototypes */

extern void pfInternal_HomogeneousToScreen(PFvertex* restrict v);

/* Main functions declaration used by 'context.c' */

PFboolean Process_ProjectAndClipTriangle(PFvertex* restrict polygon, int_fast8_t* restrict vertexCounter, const PFMmat4 mvp);

void Rasterize_TriangleColorFlat2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
void Rasterize_TriangleColorDepth2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
void Rasterize_TriangleTextureFlat2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
void Rasterize_TriangleTextureDepth2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);

void Rasterize_TriangleColorFlat3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
void Rasterize_TriangleColorDepth3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
void Rasterize_TriangleTextureFlat3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
void Rasterize_TriangleTextureDepth3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);

void Rasterize_TriangleColorFlatLight3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos);
void Rasterize_TriangleColorDepthLight3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos);
void Rasterize_TriangleTextureFlatLight3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos);
void Rasterize_TriangleTextureDepthLight3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos);

/* Internal helper function declarations */

static PFvertex Helper_LerpVertex(const PFvertex* start, const PFvertex* end, PFfloat t);

static void Helper_InterpolateVec2(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2, const PFMvec2 v3, PFfloat w1, PFfloat w2, PFfloat w3);
static void Helper_InterpolateVec3f(PFMvec2 dst, const PFMvec3 v1, const PFMvec3 v2, const PFMvec3 v3, PFfloat w1, PFfloat w2, PFfloat w3);
static PFcolor Helper_InterpolateColor(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3);


/* Polygon processing functions */

static PFboolean Process_ClipPolygonW(PFvertex* restrict polygon, int_fast8_t* restrict vertexCounter)
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

static PFboolean Process_ClipPolygonXYZ(PFvertex* restrict polygon, int_fast8_t* restrict vertexCounter)
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

PFboolean Process_ProjectAndClipTriangle(PFvertex* restrict polygon, int_fast8_t* restrict vertexCounter, const PFMmat4 mvp)
{
    for (int_fast8_t i = 0; i < *vertexCounter; i++)
    {
        PFvertex *v = polygon + i;

        memcpy(v->homogeneous, v->position, sizeof(PFMvec4));
        pfmVec4Transform(v->homogeneous, v->homogeneous, mvp);
    }

    PFboolean is2D = (
        polygon[0].homogeneous[3] == 1.0f &&
        polygon[1].homogeneous[3] == 1.0f &&
        polygon[2].homogeneous[3] == 1.0f);

    if (is2D)
    {
        for (int_fast8_t i = 0; i < *vertexCounter; i++)
        {
            pfInternal_HomogeneousToScreen(&polygon[i]);
        }
    }
    else
    {
        if (Process_ClipPolygonW(polygon, vertexCounter) && Process_ClipPolygonXYZ(polygon, vertexCounter))
        {
            for (int_fast8_t i = 0; i < *vertexCounter; i++)
            {
                // Calculation of the reciprocal of Z for the perspective correct
                polygon[i].homogeneous[2] = 1.0f / polygon[i].homogeneous[2];

                // Division of texture coordinates by the Z axis (perspective correct)
                pfmVec2Scale(polygon[i].texcoord, polygon[i].texcoord, polygon[i].homogeneous[2]);

                // Division of XY coordinates by weight (perspective correct)
                PFfloat invW = 1.0f / polygon[i].homogeneous[3];
                polygon[i].homogeneous[0] *= invW;
                polygon[i].homogeneous[1] *= invW;

                pfInternal_HomogeneousToScreen(&polygon[i]);
            }
        }
    }

    return is2D;
}


/*
    Macros for preparing rendering areas as well as
    barycentric coordinates and their incrementation steps
*/

#define PF_PREPARE_TRIANGLE_FRONT_2D() \
    /*
        Get integer 2D position coordinates
    */ \
    const PFint x1 = (PFint)v1->screen[0], y1 = (PFint)v1->screen[1]; \
    const PFint x2 = (PFint)v2->screen[0], y2 = (PFint)v2->screen[1]; \
    const PFint x3 = (PFint)v3->screen[0], y3 = (PFint)v3->screen[1]; \
    /*
        Check if vertices are in anti-clockwise order or degenerate,
        in which case the triangle cannot be rendered
    */ \
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) >= 0) goto skip_front_face; \
    /*
        Calculate the 2D bounding box of the triangle clamped to the viewport dimensions
    */ \
    const PFsizei xMin = (PFsizei)CLAMP(MIN(x1, MIN(x2, x3)), MAX(ctx->viewportX, 0), ctx->viewportX + (PFint)ctx->viewportW); \
    const PFsizei yMin = (PFsizei)CLAMP(MIN(y1, MIN(y2, y3)), MAX(ctx->viewportY, 0), ctx->viewportY + (PFint)ctx->viewportH); \
    const PFsizei xMax = (PFsizei)CLAMP(MAX(x1, MAX(x2, x3)), MAX(ctx->viewportX, 0), ctx->viewportX + (PFint)ctx->viewportW); \
    const PFsizei yMax = (PFsizei)CLAMP(MAX(y1, MAX(y2, y3)), MAX(ctx->viewportY, 0), ctx->viewportY + (PFint)ctx->viewportH); \
    /*
        If triangle is entirely outside the viewport we can stop now
    */ \
    if (xMin == xMax && yMin == yMax) return; \
    /*
        Calculate weight increment steps for each edge
    */ \
    const PFint stepWX1 = y3 - y2, stepWY1 = x2 - x3; \
    const PFint stepWX2 = y1 - y3, stepWY2 = x3 - x1; \
    const PFint stepWX3 = y2 - y1, stepWY3 = x1 - x2; \
    /*
        Calculate original edge weights relative to bounds.min
        Will be used to obtain barycentric coordinates by incrementing then
        NOTE: The type of face rendered and the order of the vertices (FRONT/BACK) is dependent on these calculations.
    */ \
    PFint w1Row = (xMin - x2)*stepWX1 + stepWY1*(yMin - y2); \
    PFint w2Row = (xMin - x3)*stepWX2 + stepWY2*(yMin - y3); \
    PFint w3Row = (xMin - x1)*stepWX3 + stepWY3*(yMin - y1); \
    /*
        Finally, we calculate the inverse of the sum of
        the barycentric coordinates for the top-left point; this
        sum always remains the same, regardless of the coordinate
        within the triangle.
    */ \
    const PFfloat invWSum = 1.0f/(w1Row + w2Row + w3Row);


#define PF_PREPARE_TRIANGLE_BACK_2D() \
    /*
        Get integer 2D position coordinates
    */ \
    const PFint x1 = (PFint)v1->screen[0], y1 = (PFint)v1->screen[1]; \
    const PFint x2 = (PFint)v2->screen[0], y2 = (PFint)v2->screen[1]; \
    const PFint x3 = (PFint)v3->screen[0], y3 = (PFint)v3->screen[1]; \
    /*
        Check if vertices are in clockwise order or degenerate,
        in which case the triangle cannot be rendered
    */ \
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) <= 0) return; \
    /*
        Calculate the 2D bounding box of the triangle clamped to the viewport dimensions
    */ \
    const PFsizei xMin = (PFsizei)CLAMP(MIN(x1, MIN(x2, x3)), MAX(ctx->viewportX, 0), ctx->viewportX + (PFint)ctx->viewportW); \
    const PFsizei yMin = (PFsizei)CLAMP(MIN(y1, MIN(y2, y3)), MAX(ctx->viewportY, 0), ctx->viewportY + (PFint)ctx->viewportH); \
    const PFsizei xMax = (PFsizei)CLAMP(MAX(x1, MAX(x2, x3)), MAX(ctx->viewportX, 0), ctx->viewportX + (PFint)ctx->viewportW); \
    const PFsizei yMax = (PFsizei)CLAMP(MAX(y1, MAX(y2, y3)), MAX(ctx->viewportY, 0), ctx->viewportY + (PFint)ctx->viewportH); \
    /*
        If triangle is entirely outside the viewport we can stop now
    */ \
    if (xMin == xMax && yMin == yMax) return; \
    /*
        Calculate weight increment steps for each edge
        NOTE: The steps are reversed when we render the back faces
    */ \
    const PFint stepWX1 = y2 - y3, stepWY1 = x3 - x2; \
    const PFint stepWX2 = y3 - y1, stepWY2 = x1 - x3; \
    const PFint stepWX3 = y1 - y2, stepWY3 = x2 - x1; \
    /*
        Calculate original edge weights relative to bounds.min
        Will be used to obtain barycentric coordinates by incrementing then
        NOTE: The type of face rendered and the order of the vertices (FRONT/BACK) is dependent on these calculations.
    */ \
    PFint w1Row = (yMin - y2)*stepWY1 + stepWX1*(xMin - x2); \
    PFint w2Row = (yMin - y3)*stepWY2 + stepWX2*(xMin - x3); \
    PFint w3Row = (yMin - y1)*stepWY3 + stepWX3*(xMin - x1); \
    /*
        Finally, we calculate the inverse of the sum of
        the barycentric coordinates for the top-left point; this
        sum always remains the same, regardless of the coordinate
        within the triangle.
    */ \
    const PFfloat invWSum = 1.0f/(w1Row + w2Row + w3Row);


#define PF_PREPARE_TRIANGLE_FRONT_3D() \
    /*
        Get integer 2D position coordinates
    */ \
    const PFint x1 = (PFint)v1->screen[0], y1 = (PFint)v1->screen[1]; \
    const PFint x2 = (PFint)v2->screen[0], y2 = (PFint)v2->screen[1]; \
    const PFint x3 = (PFint)v3->screen[0], y3 = (PFint)v3->screen[1]; \
    /*
        Check if vertices are in anti-clockwise order or degenerate,
        in which case the triangle cannot be rendered
    */ \
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) >= 0) goto skip_front_face; \
    /*
        Calculate the 2D bounding box of the triangle
    */ \
    const PFuint xMin = MIN(x1, MIN(x2, x3)); \
    const PFuint yMin = MIN(y1, MIN(y2, y3)); \
    const PFuint xMax = MAX(x1, MAX(x2, x3)); \
    const PFuint yMax = MAX(y1, MAX(y2, y3)); \
    /*
        Calculate weight increment steps for each edge
    */ \
    const PFint stepWX1 = y3 - y2, stepWY1 = x2 - x3; \
    const PFint stepWX2 = y1 - y3, stepWY2 = x3 - x1; \
    const PFint stepWX3 = y2 - y1, stepWY3 = x1 - x2; \
    /*
        Calculate original edge weights relative to bounds.min
        Will be used to obtain barycentric coordinates by incrementing then
        NOTE: The type of face rendered and the order of the vertices (FRONT/BACK) is dependent on these calculations.
    */ \
    PFint w1Row = (xMin - x2)*stepWX1 + stepWY1*(yMin - y2); \
    PFint w2Row = (xMin - x3)*stepWX2 + stepWY2*(yMin - y3); \
    PFint w3Row = (xMin - x1)*stepWX3 + stepWY3*(yMin - y1); \
    /*
        Finally, we calculate the inverse of the sum of
        the barycentric coordinates for the top-left point; this
        sum always remains the same, regardless of the coordinate
        within the triangle.
    */ \
    const PFfloat invWSum = 1.0f/(w1Row + w2Row + w3Row);


#define PF_PREPARE_TRIANGLE_BACK_3D() \
    /*
        Get integer 2D position coordinates
    */ \
    const PFint x1 = (PFint)v1->screen[0], y1 = (PFint)v1->screen[1]; \
    const PFint x2 = (PFint)v2->screen[0], y2 = (PFint)v2->screen[1]; \
    const PFint x3 = (PFint)v3->screen[0], y3 = (PFint)v3->screen[1]; \
    /*
        Check if vertices are in clockwise order or degenerate,
        in which case the triangle cannot be rendered
    */ \
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) <= 0) return; \
    /*
        Calculate the 2D bounding box of the triangle
    */ \
    const PFuint xMin = MIN(x1, MIN(x2, x3)); \
    const PFuint yMin = MIN(y1, MIN(y2, y3)); \
    const PFuint xMax = MAX(x1, MAX(x2, x3)); \
    const PFuint yMax = MAX(y1, MAX(y2, y3)); \
    /*
        Calculate weight increment steps for each edge
        NOTE: The steps are reversed when we render the back faces
    */ \
    const PFint stepWX1 = y2 - y3, stepWY1 = x3 - x2; \
    const PFint stepWX2 = y3 - y1, stepWY2 = x1 - x3; \
    const PFint stepWX3 = y1 - y2, stepWY3 = x2 - x1; \
    /*
        Calculate original edge weights relative to bounds.min
        Will be used to obtain barycentric coordinates by incrementing then
        NOTE: The type of face rendered and the order of the vertices (FRONT/BACK) is dependent on these calculations.
    */ \
    PFint w1Row = (yMin - y2)*stepWY1 + stepWX1*(xMin - x2); \
    PFint w2Row = (yMin - y3)*stepWY2 + stepWX2*(xMin - x3); \
    PFint w3Row = (yMin - y1)*stepWY3 + stepWX3*(xMin - x1); \
    /*
        Finally, we calculate the inverse of the sum of
        the barycentric coordinates for the top-left point; this
        sum always remains the same, regardless of the coordinate
        within the triangle.
    */ \
    const PFfloat invWSum = 1.0f/(w1Row + w2Row + w3Row);


// Rasterization loop implementation

#ifndef PF_USE_OPENMP

// Begin/End flat triangle rasterizer macros

#define PF_BEGIN_TRIANGLE_FLAT_LOOP() \
    PFframebuffer *fbTarget = pfGetActiveFramebuffer(); \
    PFpixelgetter pixelGetter = fbTarget->texture.pixelGetter; \
    PFpixelsetter pixelSetter = fbTarget->texture.pixelSetter; \
    const PFsizei wDst = fbTarget->texture.width; \
    void *bufTarget = fbTarget->texture.pixels; \
    PFfloat *zbTarget = fbTarget->zbuffer; \
    const PFfloat z1 = v1->homogeneous[2]; \
    const PFfloat z2 = v2->homogeneous[2]; \
    const PFfloat z3 = v3->homogeneous[2]; \
    \
    for (PFuint y = yMin; y <= yMax; y++) \
    { \
        const PFuint yOffset = y*wDst; \
        PFint w1 = w1Row, w2 = w2Row, w3 = w3Row; \
        for (PFuint x = xMin; x <= xMax; x++) \
        { \
            if ((w1 | w2 | w3) >= 0) \
            { \
                PFcolor finalColor; \
                const PFuint xyOffset = yOffset + x; \
                const PFfloat aW1 = w1*invWSum, aW2 = w2*invWSum, aW3 = w3*invWSum; \
                const PFfloat z = 1.0f/(aW1*z1 + aW2*z2 + aW3*z3);

#define PF_END_TRIANGLE_FLAT_LOOP() \
                pixelSetter(bufTarget, xyOffset, finalColor); \
                zbTarget[xyOffset] = z; \
            } \
            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3; \
        } \
        w1Row += stepWY1, w2Row += stepWY2, w3Row += stepWY3; \
    }


// Begin/End depth triangle rasterizer macros

#define PF_BEGIN_TRIANGLE_DEPTH_LOOP() \
    PFframebuffer *fbTarget = pfGetActiveFramebuffer(); \
    PFpixelgetter pixelGetter = fbTarget->texture.pixelGetter; \
    PFpixelsetter pixelSetter = fbTarget->texture.pixelSetter; \
    const PFsizei wDst = fbTarget->texture.width; \
    void *bufTarget = fbTarget->texture.pixels; \
    PFfloat *zbTarget = fbTarget->zbuffer; \
    const PFfloat z1 = v1->homogeneous[2]; \
    const PFfloat z2 = v2->homogeneous[2]; \
    const PFfloat z3 = v3->homogeneous[2]; \
    \
    for (PFuint y = yMin; y <= yMax; y++) \
    { \
        const PFuint yOffset = y*wDst; \
        PFint w1 = w1Row, w2 = w2Row, w3 = w3Row; \
        for (PFuint x = xMin; x <= xMax; x++) \
        { \
            if ((w1 | w2 | w3) >= 0) \
            { \
                PFcolor finalColor; \
                const PFuint xyOffset = yOffset + x; \
                const PFfloat aW1 = w1*invWSum, aW2 = w2*invWSum, aW3 = w3*invWSum; \
                const PFfloat z = 1.0f/(aW1*z1 + aW2*z2 + aW3*z3); \
                if (ctx->depthFunction(z, zbTarget[xyOffset])) \
                {

#define PF_END_TRIANGLE_DEPTH_LOOP() \
                    pixelSetter(bufTarget, xyOffset, finalColor); \
                    zbTarget[xyOffset] = z; \
                } \
            } \
            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3; \
        } \
        w1Row += stepWY1, w2Row += stepWY2, w3Row += stepWY3; \
    }

// Begin/End flat triangle light rasterizer macros

#define PF_BEGIN_TRIANGLE_FLAT_LIGHT_LOOP(material) \
    for (int i = 0; i <= ctx->lastActiveLight; i++) \
    { \
        const PFlight *light = &ctx->lights[i]; \
        if (!light->active) continue; \
        const PFcolor ambient = pfBlendMultiplicative(light->ambient, (material).ambient); \
        PF_BEGIN_TRIANGLE_FLAT_LOOP(); \

#define PF_END_TRIANGLE_FLAT_LIGHT_LOOP() \
        PF_END_TRIANGLE_FLAT_LOOP(); \
    }


// Begin/End depth triangle light rasterizer macros

#define PF_BEGIN_TRIANGLE_DEPTH_LIGHT_LOOP(material) \
    for (int i = 0; i <= ctx->lastActiveLight; i++) \
    { \
        const PFlight *light = &ctx->lights[i]; \
        if (!light->active) continue; \
        \
        const PFcolor ambient = pfBlendMultiplicative(light->ambient, (material).ambient); \
        \
        PFframebuffer *fbTarget = pfGetActiveFramebuffer(); \
        PFpixelgetter pixelGetter = fbTarget->texture.pixelGetter; \
        PFpixelsetter pixelSetter = fbTarget->texture.pixelSetter; \
        const PFsizei wDst = fbTarget->texture.width; \
        void *bufTarget = fbTarget->texture.pixels; \
        PFfloat *zbTarget = fbTarget->zbuffer; \
        const PFfloat z1 = v1->homogeneous[2]; \
        const PFfloat z2 = v2->homogeneous[2]; \
        const PFfloat z3 = v3->homogeneous[2]; \
        \
        for (PFuint y = yMin; y <= yMax; y++) \
        { \
            const PFuint yOffset = y*wDst; \
            PFint w1 = w1Row, w2 = w2Row, w3 = w3Row; \
            for (PFuint x = xMin; x <= xMax; x++) \
            { \
                if ((w1 | w2 | w3) >= 0) \
                { \
                    PFcolor finalColor; \
                    const PFuint xyOffset = yOffset + x; \
                    const PFfloat aW1 = w1*invWSum, aW2 = w2*invWSum, aW3 = w3*invWSum; \
                    const PFfloat z = 1.0f/(aW1*z1 + aW2*z2 + aW3*z3); \
                    if (ctx->depthFunction(z, zbTarget[xyOffset])) \
                    {

#define PF_END_TRIANGLE_DEPTH_LIGHT_LOOP() \
                        pixelSetter(bufTarget, xyOffset, finalColor); \
                        zbTarget[xyOffset] = z; \
                    } \
                } \
                w1 += stepWX1, w2 += stepWX2, w3 += stepWX3; \
            } \
            w1Row += stepWY1, w2Row += stepWY2, w3Row += stepWY3; \
        } \
    }


#else //PF_USE_OPENMP

// Begin/End flat triangle rasterizer macros

#define PF_BEGIN_TRIANGLE_FLAT_LOOP() \
    PFframebuffer *fbTarget = pfGetActiveFramebuffer(); \
    PFpixelgetter pixelGetter = fbTarget->texture.pixelGetter; \
    PFpixelsetter pixelSetter = fbTarget->texture.pixelSetter; \
    const PFsizei wDst = fbTarget->texture.width; \
    void *bufTarget = fbTarget->texture.pixels; \
    PFfloat *zbTarget = fbTarget->zbuffer; \
    const PFfloat z1 = v1->homogeneous[2]; \
    const PFfloat z2 = v2->homogeneous[2]; \
    const PFfloat z3 = v3->homogeneous[2]; \
    \
    _Pragma("omp parallel for if((yMax - yMin)*(xMax - xMin) >= PF_OPENMP_PIXEL_RASTER_THRESHOLD)") \
    for (PFuint y = yMin; y <= yMax; y++) \
    { \
        const PFuint yOffset = y*wDst; \
        PFint i = y - yMin; \
        PFint w1 = w1Row + i*stepWY1; \
        PFint w2 = w2Row + i*stepWY2; \
        PFint w3 = w3Row + i*stepWY3; \
        for (PFuint x = xMin; x <= xMax; x++) \
        { \
            if ((w1 | w2 | w3) >= 0) \
            { \
                PFcolor finalColor; \
                const PFuint xyOffset = yOffset + x; \
                const PFfloat aW1 = w1*invWSum, aW2 = w2*invWSum, aW3 = w3*invWSum; \
                const PFfloat z = 1.0f/(aW1*z1 + aW2*z2 + aW3*z3);

#define PF_END_TRIANGLE_FLAT_LOOP() \
                pixelSetter(bufTarget, xyOffset, finalColor); \
                zbTarget[xyOffset] = z; \
            } \
            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3; \
        } \
    }


// Begin/End depth triangle rasterizer macros

#define PF_BEGIN_TRIANGLE_DEPTH_LOOP() \
    PFframebuffer *fbTarget = pfGetActiveFramebuffer(); \
    PFpixelgetter pixelGetter = fbTarget->texture.pixelGetter; \
    PFpixelsetter pixelSetter = fbTarget->texture.pixelSetter; \
    const PFsizei wDst = fbTarget->texture.width; \
    void *bufTarget = fbTarget->texture.pixels; \
    PFfloat *zbTarget = fbTarget->zbuffer; \
    const PFfloat z1 = v1->homogeneous[2]; \
    const PFfloat z2 = v2->homogeneous[2]; \
    const PFfloat z3 = v3->homogeneous[2]; \
    \
    _Pragma("omp parallel for if((yMax - yMin)*(xMax - xMin) >= PF_OPENMP_PIXEL_RASTER_THRESHOLD)") \
    for (PFuint y = yMin; y <= yMax; y++) \
    { \
        const PFuint yOffset = y*wDst; \
        PFint i = y - yMin; \
        PFint w1 = w1Row + i*stepWY1; \
        PFint w2 = w2Row + i*stepWY2; \
        PFint w3 = w3Row + i*stepWY3; \
        for (PFuint x = xMin; x <= xMax; x++) \
        { \
            if ((w1 | w2 | w3) >= 0) \
            { \
                PFcolor finalColor; \
                const PFuint xyOffset = yOffset + x; \
                const PFfloat aW1 = w1*invWSum, aW2 = w2*invWSum, aW3 = w3*invWSum; \
                const PFfloat z = 1.0f/(aW1*z1 + aW2*z2 + aW3*z3); \
                if (ctx->depthFunction(z, zbTarget[xyOffset])) \
                {

#define PF_END_TRIANGLE_DEPTH_LOOP() \
                    pixelSetter(bufTarget, xyOffset, finalColor); \
                    zbTarget[xyOffset] = z; \
                } \
            } \
            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3; \
        } \
    }

// Begin/End flat triangle light rasterizer macros

#define PF_BEGIN_TRIANGLE_FLAT_LIGHT_LOOP(material) \
    for (int i = 0; i <= ctx->lastActiveLight; i++) \
    { \
        const PFlight* light = &ctx->lights[i]; \
        if (!light->active) continue; \
        const PFcolor ambient = pfBlendMultiplicative(light->ambient, (material).ambient); \
        PF_BEGIN_TRIANGLE_FLAT_LOOP(); \

#define PF_END_TRIANGLE_FLAT_LIGHT_LOOP() \
        PF_END_TRIANGLE_FLAT_LOOP(); \
    }


// Begin/End depth triangle light rasterizer macros

#define PF_BEGIN_TRIANGLE_DEPTH_LIGHT_LOOP(material) \
    for (int i = 0; i <= ctx->lastActiveLight; i++) \
    { \
        const PFlight* light = &ctx->lights[i]; \
        if (!light->active) continue; \
        \
        const PFcolor ambient = pfBlendMultiplicative(light->ambient, (material).ambient); \
        \
        PFframebuffer *fbTarget = pfGetActiveFramebuffer(); \
        PFpixelgetter pixelGetter = fbTarget->texture.pixelGetter; \
        PFpixelsetter pixelSetter = fbTarget->texture.pixelSetter; \
        const PFsizei wDst = fbTarget->texture.width; \
        void *bufTarget = fbTarget->texture.pixels; \
        PFfloat *zbTarget = fbTarget->zbuffer; \
        const PFfloat z1 = v1->homogeneous[2]; \
        const PFfloat z2 = v2->homogeneous[2]; \
        const PFfloat z3 = v3->homogeneous[2]; \
        \
        _Pragma("omp parallel for if((yMax - yMin)*(xMax - xMin) >= PF_OPENMP_PIXEL_RASTER_THRESHOLD)") \
        for (PFuint y = yMin; y <= yMax; y++) \
        { \
            const PFuint yOffset = y*wDst; \
            PFint i = y - yMin; \
            PFint w1 = w1Row + i*stepWY1; \
            PFint w2 = w2Row + i*stepWY2; \
            PFint w3 = w3Row + i*stepWY3; \
            for (PFuint x = xMin; x <= xMax; x++) \
            { \
                if ((w1 | w2 | w3) >= 0) \
                { \
                    PFcolor finalColor; \
                    const PFuint xyOffset = yOffset + x; \
                    const PFfloat aW1 = w1*invWSum, aW2 = w2*invWSum, aW3 = w3*invWSum; \
                    const PFfloat z = 1.0f/(aW1*z1 + aW2*z2 + aW3*z3); \
                    if (ctx->depthFunction(z, zbTarget[xyOffset])) \
                    {

#define PF_END_TRIANGLE_DEPTH_LIGHT_LOOP() \
                        pixelSetter(bufTarget, xyOffset, finalColor); \
                        zbTarget[xyOffset] = z; \
                    } \
                } \
                w1 += stepWX1, w2 += stepWX2, w3 += stepWX3; \
            } \
        } \
    }


#endif //PF_USE_OPENMP


/* Internal triangle 2D rasterizer function definitions */

void Rasterize_TriangleColorFlat2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    const PFctx *ctx = pfGetCurrentContext();

    if (faceToRender == PF_FRONT)
    {
        // Define the origin barycentric coordinates
        // as well as the barycentric increment steps
        // and also the rendering area xMin, xMax, ...

        PF_PREPARE_TRIANGLE_FRONT_2D();

        // The BEGIN_XXX_LOOP macro provides access to certain useful variables and constants, including:
        //      - `void *bufTarget`, which is the destination pixel buffer
        //      - `PFpixelgetter pixelGetter`, which allows you to obtain a pixel in the destination buffer
        //      - `PFcolor finalColor`, which stores the color that will be put
        //      - `const PFfloat aW1, aW2, aW3` allowing barycentric interpolation
        //      - `const PFfloat z`, which is the interpolated depth

        PF_BEGIN_TRIANGLE_FLAT_LOOP();
        {
            const PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);
            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_FLAT_LOOP();

        return; // If we have rendered the front face, there is no need to render the back face.
    }

    skip_front_face:
    if (faceToRender == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_BACK_2D();

        PF_BEGIN_TRIANGLE_FLAT_LOOP();
        {
            const PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);
            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_FLAT_LOOP();
    }
}

void Rasterize_TriangleColorDepth2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    const PFctx *ctx = pfGetCurrentContext();

    if (faceToRender == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_FRONT_2D();

        PF_BEGIN_TRIANGLE_DEPTH_LOOP();
        {
            const PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);
            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_DEPTH_LOOP();

        return; // If we have rendered the front face, there is no need to render the back face.
    }

    skip_front_face:
    if (faceToRender == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_BACK_2D();

        PF_BEGIN_TRIANGLE_DEPTH_LOOP();
        {
            const PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);
            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_DEPTH_LOOP();
    }
}

void Rasterize_TriangleTextureFlat2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    const PFctx *ctx = pfGetCurrentContext();

    if (faceToRender == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_FRONT_2D();

        PF_BEGIN_TRIANGLE_FLAT_LOOP();
        {
            PFMvec2 texcoord = { 0 };
            Helper_InterpolateVec2(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            PFcolor texel = pfGetTextureSample(ctx->currentTexture, texcoord[0], texcoord[1]);

            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);
            PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            colSrc = pfBlendMultiplicative(texel, colSrc);

            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_FLAT_LOOP();

        return; // If we have rendered the front face, there is no need to render the back face.
    }

    skip_front_face:
    if (faceToRender == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_BACK_2D();

        PF_BEGIN_TRIANGLE_FLAT_LOOP();
        {
            PFMvec2 texcoord = { 0 };
            Helper_InterpolateVec2(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            PFcolor texel = pfGetTextureSample(ctx->currentTexture, texcoord[0], texcoord[1]);

            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);
            PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            colSrc = pfBlendMultiplicative(texel, colSrc);

            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_FLAT_LOOP();
    }
}

void Rasterize_TriangleTextureDepth2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    const PFctx *ctx = pfGetCurrentContext();

    if (faceToRender == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_FRONT_2D();

        PF_BEGIN_TRIANGLE_DEPTH_LOOP();
        {
            PFMvec2 texcoord = { 0 };
            Helper_InterpolateVec2(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            PFcolor texel = pfGetTextureSample(ctx->currentTexture, texcoord[0], texcoord[1]);

            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);
            PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            colSrc = pfBlendMultiplicative(texel, colSrc);

            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_DEPTH_LOOP();

        return; // If we have rendered the front face, there is no need to render the back face.
    }

    skip_front_face:
    if (faceToRender == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_BACK_2D();

        PF_BEGIN_TRIANGLE_DEPTH_LOOP();
        {
            PFMvec2 texcoord = { 0 };
            Helper_InterpolateVec2(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            PFcolor texel = pfGetTextureSample(ctx->currentTexture, texcoord[0], texcoord[1]);

            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);
            PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            colSrc = pfBlendMultiplicative(texel, colSrc);

            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_DEPTH_LOOP();
    }
}


/* Internal front triangle 3D rasterizer function definitions */

void Rasterize_TriangleColorFlat3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    const PFctx *ctx = pfGetCurrentContext();

    if (faceToRender == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        PF_BEGIN_TRIANGLE_FLAT_LOOP();
        {
            const PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);

            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_FLAT_LOOP();

        return; // If we have rendered the front face, there is no need to render the back face.
    }

    skip_front_face:
    if (faceToRender == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        PF_BEGIN_TRIANGLE_FLAT_LOOP();
        {
            const PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);

            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_FLAT_LOOP();
    }
}

void Rasterize_TriangleColorDepth3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    const PFctx *ctx = pfGetCurrentContext();

    if (faceToRender == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        PF_BEGIN_TRIANGLE_DEPTH_LOOP();
        {
            const PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);
            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_DEPTH_LOOP();

        return; // If we have rendered the front face, there is no need to render the back face.
    }

    skip_front_face:
    if (faceToRender == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        PF_BEGIN_TRIANGLE_DEPTH_LOOP();
        {
            const PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);
            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_DEPTH_LOOP();
    }
}

void Rasterize_TriangleTextureFlat3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    const PFctx *ctx = pfGetCurrentContext();

    if (faceToRender == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        PF_BEGIN_TRIANGLE_FLAT_LOOP();
        {
            PFMvec2 texcoord;
            Helper_InterpolateVec2(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            const PFcolor texel = pfGetTextureSample(ctx->currentTexture, texcoord[0], texcoord[1]);
            PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            colSrc = pfBlendMultiplicative(texel, colSrc);

            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);
            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_FLAT_LOOP();

        return; // If we have rendered the front face, there is no need to render the back face.
    }

    skip_front_face:
    if (faceToRender == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        PF_BEGIN_TRIANGLE_FLAT_LOOP();
        {
            PFMvec2 texcoord;
            Helper_InterpolateVec2(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            const PFcolor texel = pfGetTextureSample(ctx->currentTexture, texcoord[0], texcoord[1]);
            PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            colSrc = pfBlendMultiplicative(texel, colSrc);

            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);
            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_FLAT_LOOP();
    }
}

void Rasterize_TriangleTextureDepth3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    const PFctx *ctx = pfGetCurrentContext();

    if (faceToRender == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        PF_BEGIN_TRIANGLE_DEPTH_LOOP();
        {
            PFMvec2 texcoord;
            Helper_InterpolateVec2(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            PFcolor texel = pfGetTextureSample(ctx->currentTexture, texcoord[0], texcoord[1]);
            colSrc = pfBlendMultiplicative(texel, colSrc);

            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);
            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_DEPTH_LOOP();

        return; // If we have rendered the front face, there is no need to render the back face.
    }

    skip_front_face:
    if (faceToRender == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        PF_BEGIN_TRIANGLE_DEPTH_LOOP();
        {
            PFMvec2 texcoord;
            Helper_InterpolateVec2(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            PFcolor texel = pfGetTextureSample(ctx->currentTexture, texcoord[0], texcoord[1]);
            colSrc = pfBlendMultiplicative(texel, colSrc);

            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);
            finalColor = ctx->blendFunction(colSrc, colDst);
        }
        PF_END_TRIANGLE_DEPTH_LOOP();
    }
}


/* Internal lighting process functions defintions */

static PFcolor Light_Compute(const PFlight* light, PFcolor ambient, PFcolor texel, const PFMvec3 viewPos, const PFMvec3 vertex, const PFMvec3 normal, PFfloat shininess)
{
    // Calculate view direction vector
    PFMvec3 viewDir;
    pfmVec3Sub(viewDir, viewPos, vertex);
    pfmVec3Normalize(viewDir, viewDir);

    // Compute ambient lighting contribution
    ambient = pfBlendMultiplicative(texel, ambient);

    // Compute diffuse lighting
    const PFfloat intensity = fmaxf(-pfmVec3Dot(light->direction, normal), 0.0f);

    const PFcolor diffuse = {
        (PFubyte)(light->diffuse.r * intensity),
        (PFubyte)(light->diffuse.g * intensity),
        (PFubyte)(light->diffuse.b * intensity),
        255
    };

#ifdef PF_NO_BLINN_PHONG
    PFMvec3 reflectDir;
    pfmVec3Reflect(reflectDir, light->direction, normal);
    const PFfloat spec = powf(fmaxf(pfmVec3Dot(reflectDir, viewDir), 0.0f), shininess);
#else
    // To work here we take the opposite direction
    PFMvec3 negLightDir;
    pfmVec3Neg(negLightDir, light->direction);

    // Compute half vector (Blinn vector)
    PFMvec3 halfVec;
    pfmVec3Add(halfVec, negLightDir, viewDir);
    pfmVec3Normalize(halfVec, halfVec);

    // Compute specular term using half vector
    const PFfloat spec = powf(fmaxf(pfmVec3Dot(normal, halfVec), 0.0f), shininess);
#endif

    const PFcolor specular = {
        (PFubyte)(light->specular.r * spec),
        (PFubyte)(light->specular.g * spec),
        (PFubyte)(light->specular.b * spec),
        255
    };

    // Combine ambient, diffuse, and specular components
    PFcolor finalColor = pfBlendMultiplicative(texel, diffuse);
    finalColor = pfBlendAdditive(finalColor, specular);
    return pfBlendAdditive(finalColor, ambient);
}


/* Internal enlightened triangle 3D rasterizer function definitions */

void Rasterize_TriangleColorFlatLight3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{
    const PFctx *ctx = pfGetCurrentContext();

    if (faceToRender == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        const PFcolor emission = ctx->faceMaterial[PF_FRONT].emission;
        const PFfloat shininess = ctx->faceMaterial[PF_FRONT].shininess;

        // The 'PF_BEGIN_XXX_LIGHT_LOOP' macro additionally provides access to:
        //  - `const PFlight *light` which is the active light for the currently rendered pixel
        //  - `const PFcolor ambient` which is the ambient color of the light multiplied by that of the active material

        PF_BEGIN_TRIANGLE_FLAT_LIGHT_LOOP(ctx->faceMaterial[PF_FRONT]);
        {
            const PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);

            PFMvec3 normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, ctx->blendFunction(colSrc, colDst), viewPos, vertex, normal, shininess);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_FLAT_LIGHT_LOOP();

        return; // If we have rendered the front face, there is no need to render the back face.
    }

    skip_front_face:
    if (faceToRender == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        const PFcolor emission = ctx->faceMaterial[PF_BACK].emission;
        const PFfloat shininess = ctx->faceMaterial[PF_BACK].shininess;

        // The 'PF_BEGIN_XXX_LIGHT_LOOP' macro additionally provides access to:
        //  - `const PFlight *light` which is the active light for the currently rendered pixel
        //  - `const PFcolor ambient` which is the ambient color of the light multiplied by that of the active material

        PF_BEGIN_TRIANGLE_FLAT_LIGHT_LOOP(ctx->faceMaterial[PF_BACK]);
        {
            const PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);

            PFMvec3 normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, ctx->blendFunction(colSrc, colDst), viewPos, vertex, normal, shininess);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_FLAT_LIGHT_LOOP();
    }
}

void Rasterize_TriangleColorDepthLight3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{
    const PFctx *ctx = pfGetCurrentContext();

    if (faceToRender == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        const PFcolor emission = ctx->faceMaterial[PF_FRONT].emission;
        const PFfloat shininess = ctx->faceMaterial[PF_FRONT].shininess;

        PF_BEGIN_TRIANGLE_DEPTH_LIGHT_LOOP(ctx->faceMaterial[PF_FRONT]);
        {
            const PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);

            PFMvec3 normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, ctx->blendFunction(colSrc, colDst), viewPos, vertex, normal, shininess);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_DEPTH_LIGHT_LOOP();

        return; // If we have rendered the front face, there is no need to render the back face.
    }

    skip_front_face:
    if (faceToRender == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        const PFcolor emission = ctx->faceMaterial[PF_BACK].emission;
        const PFfloat shininess = ctx->faceMaterial[PF_BACK].shininess;

        PF_BEGIN_TRIANGLE_DEPTH_LIGHT_LOOP(ctx->faceMaterial[PF_BACK]);
        {
            const PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);

            PFMvec3 normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, ctx->blendFunction(colSrc, colDst), viewPos, vertex, normal, shininess);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_DEPTH_LIGHT_LOOP();
    }
}

void Rasterize_TriangleTextureFlatLight3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{
    const PFctx *ctx = pfGetCurrentContext();

    if (faceToRender == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        const PFcolor emission = ctx->faceMaterial[PF_FRONT].emission;
        const PFfloat shininess = ctx->faceMaterial[PF_FRONT].shininess;

        PF_BEGIN_TRIANGLE_FLAT_LIGHT_LOOP(ctx->faceMaterial[PF_FRONT]);
        {
            PFMvec2 texcoord;
            Helper_InterpolateVec2(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            const PFcolor texel = pfGetTextureSample(ctx->currentTexture, texcoord[0], texcoord[1]);
            PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            colSrc = pfBlendMultiplicative(texel, colSrc);

            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);

            PFMvec3 normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, ctx->blendFunction(colSrc, colDst), viewPos, vertex, normal, shininess);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_FLAT_LIGHT_LOOP();

        return; // If we have rendered the front face, there is no need to render the back face.
    }

    skip_front_face:
    if (faceToRender == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        const PFcolor emission = ctx->faceMaterial[PF_BACK].emission;
        const PFfloat shininess = ctx->faceMaterial[PF_BACK].shininess;

        PF_BEGIN_TRIANGLE_FLAT_LIGHT_LOOP(ctx->faceMaterial[PF_BACK]);
        {
            PFMvec2 texcoord;
            Helper_InterpolateVec2(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            const PFcolor texel = pfGetTextureSample(ctx->currentTexture, texcoord[0], texcoord[1]);
            PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            colSrc = pfBlendMultiplicative(texel, colSrc);

            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);

            PFMvec3 normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, ctx->blendFunction(colSrc, colDst), viewPos, vertex, normal, shininess);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_FLAT_LIGHT_LOOP();
    }
}

void Rasterize_TriangleTextureDepthLight3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{
    const PFctx *ctx = pfGetCurrentContext();

    if (faceToRender == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        const PFcolor emission = ctx->faceMaterial[PF_FRONT].emission;
        const PFfloat shininess = ctx->faceMaterial[PF_FRONT].shininess;

        PF_BEGIN_TRIANGLE_DEPTH_LIGHT_LOOP(ctx->faceMaterial[PF_FRONT]);
        {
            PFMvec2 texcoord;
            Helper_InterpolateVec2(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            const PFcolor texel = pfGetTextureSample(ctx->currentTexture, texcoord[0], texcoord[1]);
            PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            colSrc = pfBlendMultiplicative(texel, colSrc);

            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);

            PFMvec3 normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, ctx->blendFunction(colSrc, colDst), viewPos, vertex, normal, shininess);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_DEPTH_LIGHT_LOOP();

        return; // If we have rendered the front face, there is no need to render the back face.
    }

    skip_front_face:
    if (faceToRender == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        const PFcolor emission = ctx->faceMaterial[PF_BACK].emission;
        const PFfloat shininess = ctx->faceMaterial[PF_BACK].shininess;

        PF_BEGIN_TRIANGLE_DEPTH_LIGHT_LOOP(ctx->faceMaterial[PF_BACK]);
        {
            PFMvec2 texcoord;
            Helper_InterpolateVec2(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            const PFcolor texel = pfGetTextureSample(ctx->currentTexture, texcoord[0], texcoord[1]);
            PFcolor colSrc = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            colSrc = pfBlendMultiplicative(texel, colSrc);

            const PFcolor colDst = pixelGetter(bufTarget, xyOffset);

            PFMvec3 normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, ctx->blendFunction(colSrc, colDst), viewPos, vertex, normal, shininess);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_DEPTH_LIGHT_LOOP();
    }
}


/* Internal helper function definitions */

PFvertex Helper_LerpVertex(const PFvertex* start, const PFvertex* end, PFfloat t)
{
    PFvertex result = { 0 };

    // Interpolate homogeneous position
    for (int_fast8_t i = 0; i < 4; i++)
    {
        result.homogeneous[i] = start->homogeneous[i] + t*(end->homogeneous[i] - start->homogeneous[i]);
    }

    // Interpolate positions and normals
    for (int_fast8_t i = 0; i < 3; i++)
    {
        result.position[i] = start->position[i] + t*(end->position[i] - start->position[i]);
        result.normal[i] = start->normal[i] + t*(end->normal[i] - start->normal[i]);
    }

    // Interpolate texcoord
    for (int_fast8_t i = 0; i < 2; i++)
    {
        result.texcoord[i] = start->texcoord[i] + t*(end->texcoord[i] - start->texcoord[i]);
    }

    // Interpolate color
    result.color.r = start->color.r + t*(end->color.r - start->color.r);
    result.color.g = start->color.g + t*(end->color.g - start->color.g);
    result.color.b = start->color.b + t*(end->color.b - start->color.b);
    result.color.a = start->color.a + t*(end->color.a - start->color.a);

    return result;
}

void Helper_InterpolateVec2(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2, const PFMvec2 v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

void Helper_InterpolateVec3f(PFMvec2 dst, const PFMvec3 v1, const PFMvec3 v2, const PFMvec3 v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

PFcolor Helper_InterpolateColor(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    // REVIEW: Normalization necessary here ?

    return (PFcolor) {
        (PFubyte)(w1*v1.r + w2*v2.r + w3*v3.r),
        (PFubyte)(w1*v1.g + w2*v2.g + w3*v3.g),
        (PFubyte)(w1*v1.b + w2*v2.b + w3*v3.b),
        (PFubyte)(w1*v1.a + w2*v2.a + w3*v3.a)
    };
}
