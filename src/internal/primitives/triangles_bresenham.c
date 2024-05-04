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


// WARNING: This rendering mode is WIP


#include "../context.h"
#include "../../pfm.h"
#include <stdlib.h>

/* Internal typedefs */

typedef PFcolor (*InterpolateColorFunc)(PFcolor, PFcolor, PFcolor, PFfloat, PFfloat, PFfloat);

/* Including internal function prototypes */

extern void pfInternal_HomogeneousToScreen(PFvertex* restrict v);

/* Main functions declaration used by 'context.c' */

PFboolean Process_ProjectAndClipTriangle(PFvertex* restrict polygon, int_fast8_t* restrict vertexCounter, const PFMmat4 mvp);

void Rasterize_Triangle_COLOR_NODEPTH_2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
void Rasterize_Triangle_COLOR_DEPTH_2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
void Rasterize_Triangle_TEXTURE_NODEPTH_2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
void Rasterize_Triangle_TEXTURE_DEPTH_2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);

void Rasterize_Triangle_COLOR_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
void Rasterize_Triangle_COLOR_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
void Rasterize_Triangle_TEXTURE_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
void Rasterize_Triangle_TEXTURE_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);

void Rasterize_Triangle_COLOR_LIGHT_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos);
void Rasterize_Triangle_COLOR_LIGHT_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos);
void Rasterize_Triangle_TEXTURE_LIGHT_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos);
void Rasterize_Triangle_TEXTURE_LIGHT_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos);

/* Internal helper function declarations */

// NOTE: Used to get vertices when clipping
static PFvertex Helper_LerpVertex(const PFvertex* start, const PFvertex* end, PFfloat t);

// NOTE: Used to interpolate texture coordinates
static void Helper_InterpolateVec2(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2, const PFMvec2 v3, PFfloat w1, PFfloat w2, PFfloat w3);

#ifndef PF_GOURAUD_SHADING
// NOTE: Used for interpolating vertices and normals when rendering light by fragment
static void Helper_InterpolateVec3f(PFMvec2 dst, const PFMvec3 v1, const PFMvec3 v2, const PFMvec3 v3, PFfloat w1, PFfloat w2, PFfloat w3);
#endif //PF_GOURAUD_SHADING

static PFcolor Helper_InterpolateColor_SMOOTH(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3);
static PFcolor Helper_InterpolateColor_FLAT(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3);


/* Polygon processing functions */

PFboolean Process_ProjectAndClipTriangle(PFvertex* restrict polygon, int_fast8_t* restrict vertexCounter, const PFMmat4 mvp)
{
    for (int_fast8_t i = 0; i < *vertexCounter; i++)
    {
        PFvertex *v = polygon + i;

        memcpy(v->homogeneous, v->position, sizeof(PFMvec4));
        pfmVec4Transform(v->homogeneous, v->homogeneous, mvp);

        // Si le sommet est dans le viewport, le triangle n'est pas entièrement en dehors du viewport
        if ((v->homogeneous[0] < -v->homogeneous[3] || v->homogeneous[0] > v->homogeneous[3]) &&
            (v->homogeneous[1] < -v->homogeneous[3] || v->homogeneous[1] > v->homogeneous[3]) &&
            (v->homogeneous[2] < 0.0f || v->homogeneous[2] > v->homogeneous[3]))
        {
            *vertexCounter = 0;
            return 0;
        }
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

    return is2D;
}

/* ----------------------------------------------- */

static PFcolor Helper_LerpColor(PFcolor a, PFcolor b, PFfloat t)
{
    return (PFcolor) {
        a.r + t*(b.r - a.r),
        a.g + t*(b.g - a.g),
        a.b + t*(b.b - a.b),
        a.a + t*(b.a - a.a)
    };
}

static void Rasterize_Line_COLOR_NODEPTH(PFctx* ctx, int y,
    int x1, float z1, int x2, float z2,
    PFcolor c1, PFcolor c2)
{
    /* Get Some Values */

    PFframebuffer *fbDst = ctx->currentFramebuffer;

    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;

    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    /* Min/Max coord (clipping) */

    PFint xMin = CLAMP(x1, ctx->vpMin[0], ctx->vpMax[0]);
    PFint xMax = CLAMP(x2, ctx->vpMin[0], ctx->vpMax[0]);

    /* Draw Horizontal Line */

    PFsizei xyOffset = y*fbDst->texture.width+xMin;
    PFfloat xInvLen = 1.0f/(x2 - x1);               ///< TODO REVIEW: One pixel, div by 0 (?)

    for (PFint x = xMin, i = xMin-x1; x <= xMax; x++, i++, xyOffset++)
    {
        PFfloat t = (PFfloat)i*xInvLen;
        PFfloat z = 1.0f/(z1 + t*(z2 - z1));

        PFcolor src = Helper_LerpColor(c1, c2, t);
        PFcolor dst = pixelGetter(bufDst, xyOffset);

        PFcolor finalColor = blendFunc(src, dst);
        pixelSetter(bufDst, xyOffset, finalColor);

        zbDst[xyOffset] = z;
    }
}

static void Rasterize_Line_COLOR_DEPTH(PFctx* ctx, int y,
    int x1, float z1, int x2, float z2,
    PFcolor c1, PFcolor c2)
{
    /* Get Some Values*/

    PFframebuffer *fbDst = ctx->currentFramebuffer;

    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;

    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    /* Min/Max coord (clipping) */

    PFint xMin = CLAMP(x1, ctx->vpMin[0], ctx->vpMax[0]);
    PFint xMax = CLAMP(x2, ctx->vpMin[0], ctx->vpMax[0]);

    /* Draw Horizontal Line */

    PFsizei xyOffset = y*fbDst->texture.width+xMin;
    PFfloat xInvLen = 1.0f/(x2 - x1);

    for (PFint x = xMin, i = xMin-x1; x <= xMax; x++, i++, xyOffset++)
    {
        PFfloat t = (PFfloat)i*xInvLen;
        PFfloat z = 1.0f/(z1 + t*(z2 - z1));

        PFfloat *zp = zbDst + xyOffset;
        if (ctx->depthFunction(z, *zp))
        {
            PFcolor src = Helper_LerpColor(c1, c2, t);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            *zp = z;
        }
    }
}

static void Rasterize_Line_TEXTURE_NODEPTH(PFctx* ctx, int y,
    int x1, float z1, float u1, float v1,
    int x2, float z2, float u2, float v2,
    PFcolor c1, PFcolor c2)
{
    /* Get Some Values*/

    PFframebuffer *fbDst = ctx->currentFramebuffer;

    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;

    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    PFtexture *texture = ctx->currentTexture;

    /* Min/Max coord (clipping) */

    PFint xMin = CLAMP(x1, ctx->vpMin[0], ctx->vpMax[0]);
    PFint xMax = CLAMP(x2, ctx->vpMin[0], ctx->vpMax[0]);

    /* Draw Horizontal Line */

    PFsizei xyOffset = y*fbDst->texture.width+xMin;
    PFfloat xInvLen = 1.0f/(x2 - x1);

    for (PFint x = xMin, i = xMin-x1; x <= xMax; x++, i++, xyOffset++)
    {
        PFfloat t = (PFfloat)i*xInvLen;
        PFfloat z = 1.0f/(z1 + t*(z2 - z1));

        PFfloat u = u1 + t*(u2 - u1);
        PFfloat v = v1 + t*(v2 - v1);

        PFcolor tex = pfGetTextureSample(texture, u, v);
        PFcolor src = Helper_LerpColor(c1, c2, t);
        src = pfBlendMultiplicative(tex, src);
        PFcolor dst = pixelGetter(bufDst, xyOffset);

        PFcolor finalColor = blendFunc(src, dst);
        pixelSetter(bufDst, xyOffset, finalColor);

        zbDst[xyOffset] = z;
    }
}

static void Rasterize_Line_TEXTURE_DEPTH(PFctx* ctx, int y,
    int x1, float z1, float u1, float v1,
    int x2, float z2, float u2, float v2,
    PFcolor c1, PFcolor c2)
{
    /* Get Some Values*/

    PFframebuffer *fbDst = ctx->currentFramebuffer;

    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;

    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    PFtexture *texture = ctx->currentTexture;

    /* Min/Max coord (clipping) */

    PFint xMin = CLAMP(x1, ctx->vpMin[0], ctx->vpMax[0]);
    PFint xMax = CLAMP(x2, ctx->vpMin[0], ctx->vpMax[0]);

    /* Draw Horizontal Line */

    PFsizei xyOffset = y*fbDst->texture.width+xMin;
    PFfloat xInvLen = 1.0f/(x2 - x1);

    for (PFint x = xMin, i = xMin-x1; x <= xMax; x++, i++, xyOffset++)
    {
        PFfloat t = (PFfloat)i*xInvLen;
        PFfloat z = 1.0f/(z1 + t*(z2 - z1));

        PFfloat u = u1 + t*(u2 - u1);
        PFfloat v = v1 + t*(v2 - v1);

        PFfloat *zp = zbDst + xyOffset;

        if (ctx->depthFunction(z, *zp))
        {
            PFcolor tex = pfGetTextureSample(texture, u, v);
            PFcolor src = Helper_LerpColor(c1, c2, t);
            src = pfBlendMultiplicative(tex, src);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            *zp = z;
        }
    }
}

static void Rasterize_Line_TEXTURE_PERSPECTIVE_NODEPTH(PFctx* ctx, int y,
    int x1, float z1, float u1, float v1,
    int x2, float z2, float u2, float v2,
    PFcolor c1, PFcolor c2)
{
    /* Get Some Values*/

    PFframebuffer *fbDst = ctx->currentFramebuffer;

    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;

    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    PFtexture *texture = ctx->currentTexture;

    /* Min/Max coord (clipping) */

    PFint xMin = CLAMP(x1, ctx->vpMin[0], ctx->vpMax[0]);
    PFint xMax = CLAMP(x2, ctx->vpMin[0], ctx->vpMax[0]);

    /* Draw Horizontal Line */

    PFsizei xyOffset = y*fbDst->texture.width+xMin;
    PFfloat xInvLen = 1.0f/(x2 - x1);

    for (PFint x = xMin, i = xMin-x1; x <= xMax; x++, i++, xyOffset++)
    {
        PFfloat t = (PFfloat)x*xInvLen;
        PFfloat z = 1.0f/(z1 + t*(z2 - z1));

        // NOTE 1: Divided by 'z', correct perspective
        // NOTE 2: 'z' is actually the reciprocal
        PFfloat u = z*(u1 + t*(u2 - u1));
        PFfloat v = z*(v1 + t*(v2 - v1));

        PFcolor tex = pfGetTextureSample(texture, u, v);
        PFcolor src = Helper_LerpColor(c1, c2, t);
        src = pfBlendMultiplicative(tex, src);
        PFcolor dst = pixelGetter(bufDst, xyOffset);

        PFcolor finalColor = blendFunc(src, dst);
        pixelSetter(bufDst, xyOffset, finalColor);

        zbDst[xyOffset] = z;
    }
}

static void Rasterize_Line_TEXTURE_PERSPECTIVE_DEPTH(PFctx* ctx, int y,
    int x1, float z1, float u1, float v1,
    int x2, float z2, float u2, float v2,
    PFcolor c1, PFcolor c2)
{
    /* Get Some Values*/

    PFframebuffer *fbDst = ctx->currentFramebuffer;

    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;

    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    PFtexture *texture = ctx->currentTexture;

    /* Min/Max coord (clipping) */

    PFint xMin = CLAMP(x1, ctx->vpMin[0], ctx->vpMax[0]);
    PFint xMax = CLAMP(x2, ctx->vpMin[0], ctx->vpMax[0]);

    /* Draw Horizontal Line */

    PFsizei xyOffset = y*fbDst->texture.width+xMin;
    PFfloat xInvLen = 1.0f/(x2 - x1);

    for (PFint x = xMin, i = xMin-x1; x <= xMax; x++, i++, xyOffset++)
    {
        PFfloat t = (PFfloat)i*xInvLen;
        PFfloat z = 1.0f/(z1 + t*(z2 - z1));

        // NOTE 1: Divided by 'z', correct perspective
        // NOTE 2: 'z' is actually the reciprocal
        PFfloat u = z*(u1 + t*(u2 - u1));
        PFfloat v = z*(v1 + t*(v2 - v1));

        PFfloat *zp = zbDst + xyOffset;

        if (ctx->depthFunction(z, *zp))
        {
            PFcolor tex = pfGetTextureSample(texture, u, v);
            PFcolor src = Helper_LerpColor(c1, c2, t);
            src = pfBlendMultiplicative(tex, src);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            *zp = z;
        }
    }
}

/*------------------------------------------------------*/


static PFboolean Helper_FaceCanBeRendered(PFface faceToRender, const PFMvec2 p1, const PFMvec2 p2, const PFMvec2 p3)
{
    switch (faceToRender)
    {
        case PF_FRONT:  return (p2[0] - p1[0])*(p3[1] - p1[1]) - (p3[0] - p1[0])*(p2[1] - p1[1]) < 0;
        case PF_BACK:   return (p2[0] - p1[0])*(p3[1] - p1[1]) - (p3[0] - p1[0])*(p2[1] - p1[1]) > 0;
        default:        break;
    }

    return PF_TRUE;
}

static void Helper_SortVertices(const PFvertex** v1, const PFvertex** v2, const PFvertex** v3)
{
    // Sort vertices in ascending order of y coordinates
    const PFvertex* vTmp;
    if ((*v2)->screen[1] < (*v1)->screen[1]) { vTmp = *v1; *v1 = *v2; *v2 = vTmp; }
    if ((*v3)->screen[1] < (*v1)->screen[1]) { vTmp = *v1; *v1 = *v3; *v3 = vTmp; }
    if ((*v3)->screen[1] < (*v2)->screen[1]) { vTmp = *v2; *v2 = *v3; *v3 = vTmp; }
}


/* Internal triangle 2D rasterizer function definitions */

void Rasterize_Triangle_COLOR_NODEPTH_2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    PFctx *ctx = pfGetCurrentContext();

    // TODO: Implement PF_FLAT interpolation
    //InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
    //    ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    if (!Helper_FaceCanBeRendered(faceToRender, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFfloat x1 = v1->screen[0], y1 = v1->screen[1], z1 = v1->homogeneous[2];
    const PFfloat x2 = v2->screen[0], y2 = v2->screen[1], z2 = v2->homogeneous[2];
    const PFfloat x3 = v3->screen[0], y3 = v3->screen[1], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;

    const PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    const PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    const PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    // Rasterization of the first part of the triangle (y1 to y2)
    PFint yMax = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y1, ctx->vpMin[1], ctx->vpMax[1]); y < yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFfloat xA = x1 + (x3 - x1) * alpha;
        PFfloat xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFcolor cA = Helper_LerpColor(c1, c3, alpha);
        PFcolor cB = Helper_LerpColor(c1, c2, beta);

        if (xA > xB)
        {
            PFfloat tmp;
            tmp = xA; xA = xB; xB = tmp;
            tmp = zA; zA = zB; zB = tmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        Rasterize_Line_COLOR_NODEPTH(ctx, y, xA, zA, xB, zB, cA, cB);
    }

    // Rasterization of the second part of the triangle (y2 to y3)
    yMax = CLAMP(y3, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]); y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFfloat xA = x1 + (x3 - x1) * alpha;
        PFfloat xB = x2 + (x3 - x2) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFcolor cA = Helper_LerpColor(c1, c3, alpha);
        PFcolor cB = Helper_LerpColor(c2, c3, beta);

        if (xA > xB)
        {
            PFfloat tmp;
            tmp = xA; xA = xB; xB = tmp;
            tmp = zA; zA = zB; zB = tmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        Rasterize_Line_COLOR_NODEPTH(ctx, y, xA, zA, xB, zB, cA, cB);
    }
}

void Rasterize_Triangle_COLOR_DEPTH_2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    PFctx *ctx = pfGetCurrentContext();

    // TODO: Implement PF_FLAT interpolation
    //InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
    //    ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    if (!Helper_FaceCanBeRendered(faceToRender, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFfloat x1 = v1->screen[0], y1 = v1->screen[1], z1 = v1->homogeneous[2];
    const PFfloat x2 = v2->screen[0], y2 = v2->screen[1], z2 = v2->homogeneous[2];
    const PFfloat x3 = v3->screen[0], y3 = v3->screen[1], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;

    const PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    const PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    const PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    // Rasterization of the first part of the triangle (y1 to y2)
    PFint yMax = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y1, ctx->vpMin[1], ctx->vpMax[1]); y < yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFfloat xA = x1 + (x3 - x1) * alpha;
        PFfloat xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFcolor cA = Helper_LerpColor(c1, c3, alpha);
        PFcolor cB = Helper_LerpColor(c1, c2, beta);

        if (xA > xB)
        {
            PFfloat tmp;
            tmp = xA; xA = xB; xB = tmp;
            tmp = zA; zA = zB; zB = tmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        Rasterize_Line_COLOR_DEPTH(ctx, y, xA, zA, xB, zB, cA, cB);
    }

    // Rasterization of the second part of the triangle (y2 to y3)
    yMax = CLAMP(y3, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]); y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFfloat xA = x1 + (x3 - x1) * alpha;
        PFfloat xB = x2 + (x3 - x2) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFcolor cA = Helper_LerpColor(c1, c3, alpha);
        PFcolor cB = Helper_LerpColor(c2, c3, beta);

        if (xA > xB)
        {
            PFfloat tmp;
            tmp = xA; xA = xB; xB = tmp;
            tmp = zA; zA = zB; zB = tmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        Rasterize_Line_COLOR_DEPTH(ctx, y, xA, zA, xB, zB, cA, cB);
    }
}

void Rasterize_Triangle_TEXTURE_NODEPTH_2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    PFctx *ctx = pfGetCurrentContext();

    // TODO: Implement PF_FLAT interpolation
    //InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
    //    ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    if (!Helper_FaceCanBeRendered(faceToRender, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFfloat x1 = v1->screen[0], y1 = v1->screen[1], z1 = v1->homogeneous[2];
    const PFfloat x2 = v2->screen[0], y2 = v2->screen[1], z2 = v2->homogeneous[2];
    const PFfloat x3 = v3->screen[0], y3 = v3->screen[1], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;
    const PFfloat s1 = v1->texcoord[0], t1 = v1->texcoord[1];
    const PFfloat s2 = v2->texcoord[0], t2 = v2->texcoord[1];
    const PFfloat s3 = v3->texcoord[0], t3 = v3->texcoord[1];

    const PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    const PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    const PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    // Rasterization of the first part of the triangle (y1 to y2)
    PFint yMax = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y1, ctx->vpMin[1], ctx->vpMax[1]); y < yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFfloat xA = x1 + (x3 - x1) * alpha;
        PFfloat xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s1 + (s2 - s1) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t1 + (t2 - t1) * beta;

        PFcolor cA = Helper_LerpColor(c1, c3, alpha);
        PFcolor cB = Helper_LerpColor(c1, c2, beta);

        if (xA > xB)
        {
            PFfloat tmp;
            tmp = xA; xA = xB; xB = tmp;
            tmp = zA; zA = zB; zB = tmp;
            tmp = uA; uA = uB; uB = tmp;
            tmp = vA; vA = vB; vB = tmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        Rasterize_Line_TEXTURE_NODEPTH(ctx, y, xA, zA, uA, vA, xB, zB, uB, vB, cA, cB);
    }

    // Rasterization of the second part of the triangle (y2 to y3)
    yMax = CLAMP(y3, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]); y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFfloat xA = x1 + (x3 - x1) * alpha;
        PFfloat xB = x2 + (x3 - x2) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s2 + (s3 - s2) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t2 + (t3 - t2) * beta;

        PFcolor cA = Helper_LerpColor(c1, c3, alpha);
        PFcolor cB = Helper_LerpColor(c2, c3, beta);

        if (xA > xB)
        {
            PFfloat tmp;
            tmp = xA; xA = xB; xB = tmp;
            tmp = zA; zA = zB; zB = tmp;
            tmp = uA; uA = uB; uB = tmp;
            tmp = vA; vA = vB; vB = tmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        Rasterize_Line_TEXTURE_NODEPTH(ctx, y, xA, zA, uA, vA, xB, zB, uB, vB, cA, cB);
    }
}

void Rasterize_Triangle_TEXTURE_DEPTH_2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    PFctx *ctx = pfGetCurrentContext();

    // TODO: Implement PF_FLAT interpolation
    //InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
    //    ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    if (!Helper_FaceCanBeRendered(faceToRender, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFfloat x1 = v1->screen[0], y1 = v1->screen[1], z1 = v1->homogeneous[2];
    const PFfloat x2 = v2->screen[0], y2 = v2->screen[1], z2 = v2->homogeneous[2];
    const PFfloat x3 = v3->screen[0], y3 = v3->screen[1], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;
    const PFfloat s1 = v1->texcoord[0], t1 = v1->texcoord[1];
    const PFfloat s2 = v2->texcoord[0], t2 = v2->texcoord[1];
    const PFfloat s3 = v3->texcoord[0], t3 = v3->texcoord[1];

    const PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    const PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    const PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    // Rasterization of the first part of the triangle (y1 to y2)
    PFint yMax = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y1, ctx->vpMin[1], ctx->vpMax[1]); y < yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFfloat xA = x1 + (x3 - x1) * alpha;
        PFfloat xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s1 + (s2 - s1) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t1 + (t2 - t1) * beta;

        PFcolor cA = Helper_LerpColor(c1, c3, alpha);
        PFcolor cB = Helper_LerpColor(c1, c2, beta);

        if (xA > xB)
        {
            PFfloat tmp;
            tmp = xA; xA = xB; xB = tmp;
            tmp = zA; zA = zB; zB = tmp;
            tmp = uA; uA = uB; uB = tmp;
            tmp = vA; vA = vB; vB = tmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        Rasterize_Line_TEXTURE_DEPTH(ctx, y, xA, zA, uA, vA, xB, zB, uB, vB, cA, cB);
    }

    // Rasterization of the second part of the triangle (y2 to y3)
    yMax = CLAMP(y3, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]); y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFfloat xA = x1 + (x3 - x1) * alpha;
        PFfloat xB = x2 + (x3 - x2) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s2 + (s3 - s2) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t2 + (t3 - t2) * beta;

        PFcolor cA = Helper_LerpColor(c1, c3, alpha);
        PFcolor cB = Helper_LerpColor(c2, c3, beta);

        if (xA > xB)
        {
            PFfloat tmp;
            tmp = xA; xA = xB; xB = tmp;
            tmp = zA; zA = zB; zB = tmp;
            tmp = uA; uA = uB; uB = tmp;
            tmp = vA; vA = vB; vB = tmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        Rasterize_Line_TEXTURE_DEPTH(ctx, y, xA, zA, uA, vA, xB, zB, uB, vB, cA, cB);
    }
}


/* Internal front triangle 3D rasterizer function definitions */

void Rasterize_Triangle_COLOR_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    Rasterize_Triangle_COLOR_NODEPTH_2D(faceToRender, v1, v2, v3);
}

void Rasterize_Triangle_COLOR_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    Rasterize_Triangle_COLOR_DEPTH_2D(faceToRender, v1, v2, v3);
}

void Rasterize_Triangle_TEXTURE_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    PFctx *ctx = pfGetCurrentContext();

    // TODO: Implement PF_FLAT interpolation
    //InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
    //    ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    if (!Helper_FaceCanBeRendered(faceToRender, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFfloat x1 = v1->screen[0], y1 = v1->screen[1], z1 = v1->homogeneous[2];
    const PFfloat x2 = v2->screen[0], y2 = v2->screen[1], z2 = v2->homogeneous[2];
    const PFfloat x3 = v3->screen[0], y3 = v3->screen[1], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;
    const PFfloat s1 = v1->texcoord[0], t1 = v1->texcoord[1];
    const PFfloat s2 = v2->texcoord[0], t2 = v2->texcoord[1];
    const PFfloat s3 = v3->texcoord[0], t3 = v3->texcoord[1];

    const PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    const PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    const PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    // Rasterization of the first part of the triangle (y1 to y2)
    PFint yMax = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y1, ctx->vpMin[1], ctx->vpMax[1]); y < yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFfloat xA = x1 + (x3 - x1) * alpha;
        PFfloat xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s1 + (s2 - s1) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t1 + (t2 - t1) * beta;

        PFcolor cA = Helper_LerpColor(c1, c3, alpha);
        PFcolor cB = Helper_LerpColor(c1, c2, beta);

        if (xA > xB)
        {
            PFfloat tmp;
            tmp = xA; xA = xB; xB = tmp;
            tmp = zA; zA = zB; zB = tmp;
            tmp = uA; uA = uB; uB = tmp;
            tmp = vA; vA = vB; vB = tmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        Rasterize_Line_TEXTURE_PERSPECTIVE_NODEPTH(ctx, y, xA, zA, uA, vA, xB, zB, uB, vB, cA, cB);
    }

    // Rasterization of the second part of the triangle (y2 to y3)
    yMax = CLAMP(y3, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]); y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFfloat xA = x1 + (x3 - x1) * alpha;
        PFfloat xB = x2 + (x3 - x2) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s2 + (s3 - s2) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t2 + (t3 - t2) * beta;

        PFcolor cA = Helper_LerpColor(c1, c3, alpha);
        PFcolor cB = Helper_LerpColor(c2, c3, beta);

        if (xA > xB)
        {
            PFfloat tmp;
            tmp = xA; xA = xB; xB = tmp;
            tmp = zA; zA = zB; zB = tmp;
            tmp = uA; uA = uB; uB = tmp;
            tmp = vA; vA = vB; vB = tmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        Rasterize_Line_TEXTURE_PERSPECTIVE_NODEPTH(ctx, y, xA, zA, uA, vA, xB, zB, uB, vB, cA, cB);
    }
}

void Rasterize_Triangle_TEXTURE_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    PFctx *ctx = pfGetCurrentContext();

    // TODO: Implement PF_FLAT interpolation
    //InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
    //    ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    if (!Helper_FaceCanBeRendered(faceToRender, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFfloat x1 = v1->screen[0], y1 = v1->screen[1], z1 = v1->homogeneous[2];
    const PFfloat x2 = v2->screen[0], y2 = v2->screen[1], z2 = v2->homogeneous[2];
    const PFfloat x3 = v3->screen[0], y3 = v3->screen[1], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;
    const PFfloat s1 = v1->texcoord[0], t1 = v1->texcoord[1];
    const PFfloat s2 = v2->texcoord[0], t2 = v2->texcoord[1];
    const PFfloat s3 = v3->texcoord[0], t3 = v3->texcoord[1];

    const PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    const PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    const PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    // Rasterization of the first part of the triangle (y1 to y2)
    PFint yMax = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y1, ctx->vpMin[1], ctx->vpMax[1]); y < yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFfloat xA = x1 + (x3 - x1) * alpha;
        PFfloat xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s1 + (s2 - s1) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t1 + (t2 - t1) * beta;

        PFcolor cA = Helper_LerpColor(c1, c3, alpha);
        PFcolor cB = Helper_LerpColor(c1, c2, beta);

        if (xA > xB)
        {
            PFfloat tmp;
            tmp = xA; xA = xB; xB = tmp;
            tmp = zA; zA = zB; zB = tmp;
            tmp = uA; uA = uB; uB = tmp;
            tmp = vA; vA = vB; vB = tmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        Rasterize_Line_TEXTURE_PERSPECTIVE_DEPTH(ctx, y, xA, zA, uA, vA, xB, zB, uB, vB, cA, cB);
    }

    // Rasterization of the second part of the triangle (y2 to y3)
    yMax = CLAMP(y3, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]); y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFfloat xA = x1 + (x3 - x1) * alpha;
        PFfloat xB = x2 + (x3 - x2) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s2 + (s3 - s2) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t2 + (t3 - t2) * beta;

        PFcolor cA = Helper_LerpColor(c1, c3, alpha);
        PFcolor cB = Helper_LerpColor(c2, c3, beta);

        if (xA > xB)
        {
            PFfloat tmp;
            tmp = xA; xA = xB; xB = tmp;
            tmp = zA; zA = zB; zB = tmp;
            tmp = uA; uA = uB; uB = tmp;
            tmp = vA; vA = vB; vB = tmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        Rasterize_Line_TEXTURE_PERSPECTIVE_DEPTH(ctx, y, xA, zA, uA, vA, xB, zB, uB, vB, cA, cB);
    }
}


/* Internal lighting process functions defintions */

static PFcolor Process_Light(const PFlight* light, PFcolor ambient, PFcolor texel, const PFMvec3 viewPos, const PFMvec3 position, const PFMvec3 normal, PFfloat shininess)
{
    // get view direction for this fragment position **(can be optimized)**
    PFMvec3 viewDir;
    pfmVec3Sub(viewDir, viewPos, position);
    pfmVec3Normalize(viewDir, viewDir);

    // Compute ambient lighting contribution
    ambient = pfBlendMultiplicative(texel, ambient);

    // diffuse
    PFMvec3 lightFragPosDt;
    pfmVec3Sub(lightFragPosDt, light->position, position);

    PFMvec3 lightDir;
    pfmVec3Normalize(lightDir, lightFragPosDt);

    PFfloat diff = fmaxf(pfmVec3Dot(normal, lightDir), 0.0f);

    PFcolor diffuse = pfBlendMultiplicative(light->diffuse, texel);
    diffuse.r = (PFubyte)((PFfloat)diffuse.r*diff);
    diffuse.g = (PFubyte)((PFfloat)diffuse.g*diff);
    diffuse.b = (PFubyte)((PFfloat)diffuse.b*diff);

    // specular
#ifndef PF_PHONG_REFLECTION
    // Blinn-Phong
    PFMvec3 halfWayDir;
    pfmVec3Add(halfWayDir, lightDir, viewDir);
    pfmVec3Normalize(halfWayDir, halfWayDir);
    PFfloat spec = powf(fmaxf(pfmVec3Dot(normal, halfWayDir), 0.0f), shininess);
#else
    // Phong
    PFMvec3 reflectionDir, negLightDir;
    pfmVec3Neg(negLightDir, lightDir);
    pfmVec3Reflect(reflectionDir, negLightDir, normal);
    PFfloat spec = powf(fmaxf(pfmVec3Dot(reflectionDir, viewDir), 0.0f), shininess);
#endif

    const PFcolor specular = {
        (PFubyte)((PFfloat)light->specular.r*spec),
        (PFubyte)((PFfloat)light->specular.g*spec),
        (PFubyte)((PFfloat)light->specular.b*spec),
        255
    };

    // spotlight (soft edges)
    PFfloat intensity = 1.0f;
    if (light->cutoff < 180)
    {
        PFMvec3 negLightDir;
        pfmVec3Neg(negLightDir, light->direction);

        PFfloat theta = pfmVec3Dot(lightDir, negLightDir);
        PFfloat epsilon = light->cutoff - light->outerCutoff;
        intensity = 1.0f - CLAMP((theta - light->outerCutoff) / epsilon, 0.0f, 1.0f);
    }

    // attenuation
    PFfloat attenuation = 1.0f;
    if (light->attLinear || light->attQuadratic)
    {
        PFfloat distanceSq = lightFragPosDt[0]*lightFragPosDt[0] +
                             lightFragPosDt[1]*lightFragPosDt[1] +
                             lightFragPosDt[2]*lightFragPosDt[2];

        PFfloat distance = sqrtf(distanceSq);

        attenuation = 1.0f/(light->attConstant + light->attLinear*distance + light->attQuadratic*distanceSq);
    }

    // add final light color
    PFcolor finalColor = pfBlendAdditive(diffuse, specular);
    PFfloat factor = intensity*attenuation;

    finalColor.r = (PFubyte)((PFfloat)finalColor.r*factor);
    finalColor.g = (PFubyte)((PFfloat)finalColor.g*factor);
    finalColor.b = (PFubyte)((PFfloat)finalColor.b*factor);

    return pfBlendAdditive(ambient, finalColor);
}

#ifdef PF_GOURAUD_SHADING
static PFcolor Process_Gouraud(const PFctx* ctx, const PFvertex* v, const PFMvec3 viewPos, const PFmaterial* material)
{
    PFcolor finalColor = { 0 };

    for (PFint i = 0; i <= ctx->lastActiveLight; i++)
    {
        const PFlight *light = &ctx->lights[i];
        if (!light->active) continue;

        const PFcolor ambient = pfBlendMultiplicative(light->ambient, material->ambient);
        PFcolor color = Process_Light(light, ambient, v->color, viewPos, v->position, v->normal, material->shininess);
        color = pfBlendAdditive(color, material->emission);

        finalColor = pfBlendAdditive(finalColor, color);
    }

    return finalColor;
}
#endif //PF_GOURAUD_SHADING


/* Internal enlightened triangle 3D rasterizer function definitions */

#ifndef PF_GOURAUD_SHADING

void Rasterize_Triangle_COLOR_LIGHT_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{

}

void Rasterize_Triangle_COLOR_LIGHT_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{

}

void Rasterize_Triangle_TEXTURE_LIGHT_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{

}

void Rasterize_Triangle_TEXTURE_LIGHT_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{

}

#else

void Rasterize_Triangle_COLOR_LIGHT_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{

}

void Rasterize_Triangle_COLOR_LIGHT_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{

}

void Rasterize_Triangle_TEXTURE_LIGHT_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{

}

void Rasterize_Triangle_TEXTURE_LIGHT_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{

}

#endif //PF_GOURAUD_SHADING


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

#ifndef PF_GOURAUD_SHADING
// NOTE: Used for interpolating vertices and normals when rendering light by fragment
void Helper_InterpolateVec3f(PFMvec2 dst, const PFMvec3 v1, const PFMvec3 v2, const PFMvec3 v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}
#endif //PF_GOURAUD_SHADING

PFcolor Helper_InterpolateColor_SMOOTH(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    // REVIEW: Normalization necessary here ?

    return (PFcolor) {
        (PFubyte)(w1*v1.r + w2*v2.r + w3*v3.r),
        (PFubyte)(w1*v1.g + w2*v2.g + w3*v3.g),
        (PFubyte)(w1*v1.b + w2*v2.b + w3*v3.b),
        (PFubyte)(w1*v1.a + w2*v2.a + w3*v3.a)
    };
}

PFcolor Helper_InterpolateColor_FLAT(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    return ((w1 > w2) & (w1 > w3)) ? v1 : (w2 >= w3) ? v2 : v3;
}
