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


#include "../lighting/lighting.h"
#include "../context.h"
#include "../../pfm.h"
#include <stdint.h>
#include <stdlib.h>

/* Internal typedefs */

typedef PFcolor (*InterpolateColorFunc)(PFcolor, PFcolor, PFfloat);

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

// NOTE: Used to interpolate texture coordinates
static void Helper_InterpolateVec2(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2, const PFMvec2 v3, PFfloat w1, PFfloat w2, PFfloat w3);

#ifndef PF_GOURAUD_SHADING
// NOTE: Used for interpolating vertices and normals when rendering light by fragment
static void Helper_InterpolateVec3f(PFMvec2 dst, const PFMvec3 v1, const PFMvec3 v2, const PFMvec3 v3, PFfloat w1, PFfloat w2, PFfloat w3);
#endif //PF_GOURAUD_SHADING

static PFcolor Helper_InterpolateColor_SMOOTH(PFcolor v1, PFcolor v2, PFfloat t);
static PFcolor Helper_InterpolateColor_FLAT(PFcolor v1, PFcolor v2, PFfloat t);

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
    /* Prepare for rasterization */

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

    /* Get Some Contextual Values */

    PFctx *ctx = pfGetCurrentContext();

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    // Rasterization of the first part of the triangle (y1 to y2)

    PFint yMax = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y1, ctx->vpMin[1], ctx->vpMax[1]); y < yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c1, c2, beta);

        if (xA > xB)
        {
            PFint iTmp = xA; xA = xB; xB = iTmp;
            PFfloat fTmp = zA; zA = zB; zB = fTmp;
            PFcolor cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFint xMin = CLAMP(xA, ctx->vpMin[0], ctx->vpMax[0]);
        PFint xMax = CLAMP(xB, ctx->vpMin[0], ctx->vpMax[0]);
        PFsizei xyOffset = y*fbDst->texture.width + xMin;

        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xMin; x != xMax; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFcolor src = interpolateColor(cA, cB, t);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            zbDst[xyOffset] = z;
        }
    }

    // Rasterization of the second part of the triangle (y2 to y3)

    yMax = CLAMP(y3, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]); y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c2, c3, beta);

        if (xA > xB)
        {
            PFint iTmp = xA; xA = xB; xB = iTmp;
            PFfloat fTmp = zA; zA = zB; zB = fTmp;
            PFcolor cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFint xMin = CLAMP(xA, ctx->vpMin[0], ctx->vpMax[0]);
        PFint xMax = CLAMP(xB, ctx->vpMin[0], ctx->vpMax[0]);
        PFsizei xyOffset = y*fbDst->texture.width + xMin;

        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xMin; x != xMax; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFcolor src = interpolateColor(cA, cB, t);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            zbDst[xyOffset] = z;
        }
    }
}

void Rasterize_Triangle_COLOR_DEPTH_2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    /* Prepare for rasterization */

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

    /* Get Some Contextual Values */

    PFctx *ctx = pfGetCurrentContext();

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    // Rasterization of the first part of the triangle (y1 to y2)

    PFint yMax = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y1, ctx->vpMin[1], ctx->vpMax[1]); y < yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c1, c2, beta);

        if (xA > xB)
        {
            PFint iTmp = xA; xA = xB; xB = iTmp;
            PFfloat fTmp = zA; zA = zB; zB = fTmp;
            PFcolor cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFint xMin = CLAMP(xA, ctx->vpMin[0], ctx->vpMax[0]);
        PFint xMax = CLAMP(xB, ctx->vpMin[0], ctx->vpMax[0]);
        PFsizei xyOffset = y*fbDst->texture.width + xMin;

        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xMin; x != xMax; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFfloat *zp = zbDst + xyOffset;
            if (ctx->depthFunction(z, *zp))
            {
                PFcolor src = interpolateColor(cA, cB, t);
                PFcolor dst = pixelGetter(bufDst, xyOffset);

                PFcolor finalColor = blendFunc(src, dst);
                pixelSetter(bufDst, xyOffset, finalColor);

                *zp = z;
            }
        }
    }

    // Rasterization of the second part of the triangle (y2 to y3)

    yMax = CLAMP(y3, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]); y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c2, c3, beta);

        if (xA > xB)
        {
            PFint iTmp = xA; xA = xB; xB = iTmp;
            PFfloat fTmp = zA; zA = zB; zB = fTmp;
            PFcolor cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFint xMin = CLAMP(xA, ctx->vpMin[0], ctx->vpMax[0]);
        PFint xMax = CLAMP(xB, ctx->vpMin[0], ctx->vpMax[0]);
        PFsizei xyOffset = y*fbDst->texture.width + xMin;

        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xMin; x != xMax; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFfloat *zp = zbDst + xyOffset;
            if (ctx->depthFunction(z, *zp))
            {
                PFcolor src = interpolateColor(cA, cB, t);
                PFcolor dst = pixelGetter(bufDst, xyOffset);

                PFcolor finalColor = blendFunc(src, dst);
                pixelSetter(bufDst, xyOffset, finalColor);

                *zp = z;
            }
        }
    }
}

void Rasterize_Triangle_TEXTURE_NODEPTH_2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    /* Prepare for rasterization */

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

    /* Get Some Contextual Values */

    PFctx *ctx = pfGetCurrentContext();

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    PFtexture *texture = ctx->currentTexture;
    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    // Rasterization of the first part of the triangle (y1 to y2)

    PFint yMax = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y1, ctx->vpMin[1], ctx->vpMax[1]); y < yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s1 + (s2 - s1) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t1 + (t2 - t1) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c1, c2, beta);

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = uA; uA = uB; uB = fTmp;
            fTmp = vA; vA = vB; vB = fTmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFint xMin = CLAMP(xA, ctx->vpMin[0], ctx->vpMax[0]);
        PFint xMax = CLAMP(xB, ctx->vpMin[0], ctx->vpMax[0]);
        PFsizei xyOffset = y*fbDst->texture.width + xMin;

        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xMin; x != xMax; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFfloat u = uA + t*(uB - uA);
            PFfloat v = vA + t*(vB - vA);

            PFcolor tex = pfGetTextureSample(texture, u, v);
            PFcolor src = interpolateColor(cA, cB, t);
            src = pfBlendMultiplicative(tex, src);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            zbDst[xyOffset] = z;
        }
    }

    // Rasterization of the second part of the triangle (y2 to y3)

    yMax = CLAMP(y3, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]); y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s2 + (s3 - s2) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t2 + (t3 - t2) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c2, c3, beta);

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = uA; uA = uB; uB = fTmp;
            fTmp = vA; vA = vB; vB = fTmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFint xMin = CLAMP(xA, ctx->vpMin[0], ctx->vpMax[0]);
        PFint xMax = CLAMP(xB, ctx->vpMin[0], ctx->vpMax[0]);
        PFsizei xyOffset = y*fbDst->texture.width + xMin;

        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xMin; x != xMax; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFfloat u = uA + t*(uB - uA);
            PFfloat v = vA + t*(vB - vA);

            PFcolor tex = pfGetTextureSample(texture, u, v);
            PFcolor src = interpolateColor(cA, cB, t);
            src = pfBlendMultiplicative(tex, src);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            zbDst[xyOffset] = z;
        }
    }
}

void Rasterize_Triangle_TEXTURE_DEPTH_2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    /* Prepare for rasterization */

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

    /* Get Some Contextual Values */

    PFctx *ctx = pfGetCurrentContext();

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    PFtexture *texture = ctx->currentTexture;
    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    // Rasterization of the first part of the triangle (y1 to y2)

    PFint yMax = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y1, ctx->vpMin[1], ctx->vpMax[1]); y < yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s1 + (s2 - s1) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t1 + (t2 - t1) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c1, c2, beta);

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = uA; uA = uB; uB = fTmp;
            fTmp = vA; vA = vB; vB = fTmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFint xMin = CLAMP(xA, ctx->vpMin[0], ctx->vpMax[0]);
        PFint xMax = CLAMP(xB, ctx->vpMin[0], ctx->vpMax[0]);
        PFsizei xyOffset = y*fbDst->texture.width + xMin;

        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xMin; x != xMax; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFfloat u = uA + t*(uB - uA);
            PFfloat v = vA + t*(vB - vA);

            PFfloat *zp = zbDst + xyOffset;
            if (ctx->depthFunction(z, *zp))
            {
                PFcolor tex = pfGetTextureSample(texture, u, v);
                PFcolor src = interpolateColor(cA, cB, t);
                src = pfBlendMultiplicative(tex, src);
                PFcolor dst = pixelGetter(bufDst, xyOffset);

                PFcolor finalColor = blendFunc(src, dst);
                pixelSetter(bufDst, xyOffset, finalColor);

                *zp = z;
            }
        }
    }

    // Rasterization of the second part of the triangle (y2 to y3)
    yMax = CLAMP(y3, ctx->vpMin[1], ctx->vpMax[1]);
    for (PFint y = CLAMP(y2, ctx->vpMin[1], ctx->vpMax[1]); y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s2 + (s3 - s2) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t2 + (t3 - t2) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c2, c3, beta);

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = uA; uA = uB; uB = fTmp;
            fTmp = vA; vA = vB; vB = fTmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFint xMin = CLAMP(xA, ctx->vpMin[0], ctx->vpMax[0]);
        PFint xMax = CLAMP(xB, ctx->vpMin[0], ctx->vpMax[0]);
        PFsizei xyOffset = y*fbDst->texture.width + xMin;

        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xMin; x != xMax; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFfloat u = uA + t*(uB - uA);
            PFfloat v = vA + t*(vB - vA);

            PFfloat *zp = zbDst + xyOffset;
            if (ctx->depthFunction(z, *zp))
            {
                PFcolor tex = pfGetTextureSample(texture, u, v);
                PFcolor src = interpolateColor(cA, cB, t);
                src = pfBlendMultiplicative(tex, src);
                PFcolor dst = pixelGetter(bufDst, xyOffset);

                PFcolor finalColor = blendFunc(src, dst);
                pixelSetter(bufDst, xyOffset, finalColor);

                *zp = z;
            }
        }
    }
}


/* Internal front triangle 3D rasterizer function definitions */

void Rasterize_Triangle_COLOR_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    /* Prepare for rasterization */

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

    /* Get Some Contextual Values */

    PFctx *ctx = pfGetCurrentContext();

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    // Rasterization of the first part of the triangle (y1 to y2)

    for (PFint y = y1; y < y2; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c1, c2, beta);

        if (xA > xB)
        {
            PFint iTmp = xA; xA = xB; xB = iTmp;
            PFfloat fTmp = zA; zA = zB; zB = fTmp;
            PFcolor cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x != xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFcolor src = interpolateColor(cA, cB, t);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            zbDst[xyOffset] = z;
        }
    }

    // Rasterization of the second part of the triangle (y2 to y3)

    for (PFint y = y2; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x2 + (x3 - x2) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c2, c3, beta);

        if (xA > xB)
        {
            PFint iTmp = xA; xA = xB; xB = iTmp;
            PFfloat fTmp = zA; zA = zB; zB = fTmp;
            PFcolor cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x != xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFcolor src = interpolateColor(cA, cB, t);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            zbDst[xyOffset] = z;
        }
    }
}

void Rasterize_Triangle_COLOR_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    /* Prepare for rasterization */

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

    /* Get Some Contextual Values */

    PFctx *ctx = pfGetCurrentContext();

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    // Rasterization of the first part of the triangle (y1 to y2)

    for (PFint y = y1; y < y2; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c1, c2, beta);

        if (xA > xB)
        {
            PFint iTmp = xA; xA = xB; xB = iTmp;
            PFfloat fTmp = zA; zA = zB; zB = fTmp;
            PFcolor cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x != xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFfloat *zp = zbDst + xyOffset;
            if (ctx->depthFunction(z, *zp))
            {
                PFcolor src = interpolateColor(cA, cB, t);
                PFcolor dst = pixelGetter(bufDst, xyOffset);

                PFcolor finalColor = blendFunc(src, dst);
                pixelSetter(bufDst, xyOffset, finalColor);

                *zp = z;
            }
        }
    }

    // Rasterization of the second part of the triangle (y2 to y3)

    for (PFint y = y2; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x2 + (x3 - x2) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c2, c3, beta);

        if (xA > xB)
        {
            PFint iTmp = xA; xA = xB; xB = iTmp;
            PFfloat fTmp = zA; zA = zB; zB = fTmp;
            PFcolor cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x != xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFfloat *zp = zbDst + xyOffset;
            if (ctx->depthFunction(z, *zp))
            {
                PFcolor src = interpolateColor(cA, cB, t);
                PFcolor dst = pixelGetter(bufDst, xyOffset);

                PFcolor finalColor = blendFunc(src, dst);
                pixelSetter(bufDst, xyOffset, finalColor);

                *zp = z;
            }
        }
    }
}

void Rasterize_Triangle_TEXTURE_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    /* Prepare for rasterization */

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

    /* Get Some Contextual Values */

    PFctx *ctx = pfGetCurrentContext();

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    PFtexture *texture = ctx->currentTexture;
    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    // Rasterization of the first part of the triangle (y1 to y2)

    for (PFint y = y1; y < y2; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s1 + (s2 - s1) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t1 + (t2 - t1) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c1, c2, beta);

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = uA; uA = uB; uB = fTmp;
            fTmp = vA; vA = vB; vB = fTmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x != xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            // NOTE 1: Divided by 'z', correct perspective
            // NOTE 2: 'z' is actually the reciprocal
            PFfloat u = z*(uA + t*(uB - uA));
            PFfloat v = z*(vA + t*(vB - vA));

            PFcolor tex = pfGetTextureSample(texture, u, v);
            PFcolor src = interpolateColor(cA, cB, t);
            src = pfBlendMultiplicative(tex, src);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            zbDst[xyOffset] = z;
        }
    }

    // Rasterization of the second part of the triangle (y2 to y3)

    for (PFint y = y2; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x2 + (x3 - x2) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s2 + (s3 - s2) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t2 + (t3 - t2) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c2, c3, beta);

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = uA; uA = uB; uB = fTmp;
            fTmp = vA; vA = vB; vB = fTmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x != xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            // NOTE 1: Divided by 'z', correct perspective
            // NOTE 2: 'z' is actually the reciprocal
            PFfloat u = z*(uA + t*(uB - uA));
            PFfloat v = z*(vA + t*(vB - vA));

            PFcolor tex = pfGetTextureSample(texture, u, v);
            PFcolor src = interpolateColor(cA, cB, t);
            src = pfBlendMultiplicative(tex, src);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            zbDst[xyOffset] = z;
        }
    }
}

void Rasterize_Triangle_TEXTURE_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    /* Prepare for rasterization */

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

    /* Get Some Contextual Values */

    PFctx *ctx = pfGetCurrentContext();

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    PFtexture *texture = ctx->currentTexture;
    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    // Rasterization of the first part of the triangle (y1 to y2)

    for (PFint y = y1; y < y2; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s1 + (s2 - s1) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t1 + (t2 - t1) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c1, c2, beta);

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = uA; uA = uB; uB = fTmp;
            fTmp = vA; vA = vB; vB = fTmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x <= xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            // NOTE 1: Divided by 'z', correct perspective
            // NOTE 2: 'z' is actually the reciprocal
            PFfloat u = z*(uA + t*(uB - uA));
            PFfloat v = z*(vA + t*(vB - vA));

            PFfloat *zp = zbDst + xyOffset;
            if (ctx->depthFunction(z, *zp))
            {
                PFcolor tex = pfGetTextureSample(texture, u, v);
                PFcolor src = interpolateColor(cA, cB, t);
                src = pfBlendMultiplicative(tex, src);
                PFcolor dst = pixelGetter(bufDst, xyOffset);

                PFcolor finalColor = blendFunc(src, dst);
                pixelSetter(bufDst, xyOffset, finalColor);

                *zp = z;
            }
        }
    }

    // Rasterization of the second part of the triangle (y2 to y3)

    for (PFint y = y2; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x2 + (x3 - x2) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s2 + (s3 - s2) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t2 + (t3 - t2) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c2, c3, beta);

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = uA; uA = uB; uB = fTmp;
            fTmp = vA; vA = vB; vB = fTmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x <= xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            // NOTE 1: Divided by 'z', correct perspective
            // NOTE 2: 'z' is actually the reciprocal
            PFfloat u = z*(uA + t*(uB - uA));
            PFfloat v = z*(vA + t*(vB - vA));

            PFfloat *zp = zbDst + xyOffset;
            if (ctx->depthFunction(z, *zp))
            {
                PFcolor tex = pfGetTextureSample(texture, u, v);
                PFcolor src = interpolateColor(cA, cB, t);
                src = pfBlendMultiplicative(tex, src);
                PFcolor dst = pixelGetter(bufDst, xyOffset);

                PFcolor finalColor = blendFunc(src, dst);
                pixelSetter(bufDst, xyOffset, finalColor);

                *zp = z;
            }
        }
    }
}


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
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    if (!Helper_FaceCanBeRendered(faceToRender, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFfloat x1 = v1->screen[0], y1 = v1->screen[1], z1 = v1->homogeneous[2];
    const PFfloat x2 = v2->screen[0], y2 = v2->screen[1], z2 = v2->homogeneous[2];
    const PFfloat x3 = v3->screen[0], y3 = v3->screen[1], z3 = v3->homogeneous[2];

    const PFcolor c1 = Process_Gouraud(ctx, v1, viewPos, &ctx->faceMaterial[faceToRender]);
    const PFcolor c2 = Process_Gouraud(ctx, v2, viewPos, &ctx->faceMaterial[faceToRender]);
    const PFcolor c3 = Process_Gouraud(ctx, v3, viewPos, &ctx->faceMaterial[faceToRender]);

    const PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    const PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    const PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    /* Get Some Contextual Values */

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    // Rasterization of the first part of the triangle (y1 to y2)

    for (PFint y = y1; y < y2; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c1, c2, beta);

        if (xA > xB)
        {
            PFint iTmp = xA; xA = xB; xB = iTmp;
            PFfloat fTmp = zA; zA = zB; zB = fTmp;
            PFcolor cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x != xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFcolor src = interpolateColor(cA, cB, t);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            zbDst[xyOffset] = z;
        }
    }

    // Rasterization of the second part of the triangle (y2 to y3)

    for (PFint y = y2; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x2 + (x3 - x2) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c2, c3, beta);

        if (xA > xB)
        {
            PFint iTmp = xA; xA = xB; xB = iTmp;
            PFfloat fTmp = zA; zA = zB; zB = fTmp;
            PFcolor cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x != xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFcolor src = interpolateColor(cA, cB, t);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            zbDst[xyOffset] = z;
        }
    }
}

void Rasterize_Triangle_COLOR_LIGHT_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    if (!Helper_FaceCanBeRendered(faceToRender, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFfloat x1 = v1->screen[0], y1 = v1->screen[1], z1 = v1->homogeneous[2];
    const PFfloat x2 = v2->screen[0], y2 = v2->screen[1], z2 = v2->homogeneous[2];
    const PFfloat x3 = v3->screen[0], y3 = v3->screen[1], z3 = v3->homogeneous[2];

    const PFcolor c1 = Process_Gouraud(ctx, v1, viewPos, &ctx->faceMaterial[faceToRender]);
    const PFcolor c2 = Process_Gouraud(ctx, v2, viewPos, &ctx->faceMaterial[faceToRender]);
    const PFcolor c3 = Process_Gouraud(ctx, v3, viewPos, &ctx->faceMaterial[faceToRender]);

    const PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    const PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    const PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    /* Get Some Contextual Values */

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    // Rasterization of the first part of the triangle (y1 to y2)

    for (PFint y = y1; y < y2; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c1, c2, beta);

        if (xA > xB)
        {
            PFint iTmp = xA; xA = xB; xB = iTmp;
            PFfloat fTmp = zA; zA = zB; zB = fTmp;
            PFcolor cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x != xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFfloat *zp = zbDst + xyOffset;
            if (ctx->depthFunction(z, *zp))
            {
                PFcolor src = interpolateColor(cA, cB, t);
                PFcolor dst = pixelGetter(bufDst, xyOffset);

                PFcolor finalColor = blendFunc(src, dst);
                pixelSetter(bufDst, xyOffset, finalColor);

                *zp = z;
            }
        }
    }

    // Rasterization of the second part of the triangle (y2 to y3)

    for (PFint y = y2; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x2 + (x3 - x2) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c2, c3, beta);

        if (xA > xB)
        {
            PFint iTmp = xA; xA = xB; xB = iTmp;
            PFfloat fTmp = zA; zA = zB; zB = fTmp;
            PFcolor cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x != xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            PFfloat *zp = zbDst + xyOffset;
            if (ctx->depthFunction(z, *zp))
            {
                PFcolor src = interpolateColor(cA, cB, t);
                PFcolor dst = pixelGetter(bufDst, xyOffset);

                PFcolor finalColor = blendFunc(src, dst);
                pixelSetter(bufDst, xyOffset, finalColor);

                *zp = z;
            }
        }
    }
}

void Rasterize_Triangle_TEXTURE_LIGHT_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    if (!Helper_FaceCanBeRendered(faceToRender, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFfloat x1 = v1->screen[0], y1 = v1->screen[1], z1 = v1->homogeneous[2];
    const PFfloat x2 = v2->screen[0], y2 = v2->screen[1], z2 = v2->homogeneous[2];
    const PFfloat x3 = v3->screen[0], y3 = v3->screen[1], z3 = v3->homogeneous[2];

    const PFcolor c1 = Process_Gouraud(ctx, v1, viewPos, &ctx->faceMaterial[faceToRender]);
    const PFcolor c2 = Process_Gouraud(ctx, v2, viewPos, &ctx->faceMaterial[faceToRender]);
    const PFcolor c3 = Process_Gouraud(ctx, v3, viewPos, &ctx->faceMaterial[faceToRender]);

    const PFfloat s1 = v1->texcoord[0], t1 = v1->texcoord[1];
    const PFfloat s2 = v2->texcoord[0], t2 = v2->texcoord[1];
    const PFfloat s3 = v3->texcoord[0], t3 = v3->texcoord[1];

    const PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    const PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    const PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    /* Get Some Contextual Values */

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    PFtexture *texture = ctx->currentTexture;
    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    // Rasterization of the first part of the triangle (y1 to y2)

    for (PFint y = y1; y < y2; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s1 + (s2 - s1) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t1 + (t2 - t1) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c1, c2, beta);

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = uA; uA = uB; uB = fTmp;
            fTmp = vA; vA = vB; vB = fTmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x != xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            // NOTE 1: Divided by 'z', correct perspective
            // NOTE 2: 'z' is actually the reciprocal
            PFfloat u = z*(uA + t*(uB - uA));
            PFfloat v = z*(vA + t*(vB - vA));

            PFcolor tex = pfGetTextureSample(texture, u, v);
            PFcolor src = interpolateColor(cA, cB, t);
            src = pfBlendMultiplicative(tex, src);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            zbDst[xyOffset] = z;
        }
    }

    // Rasterization of the second part of the triangle (y2 to y3)

    for (PFint y = y2; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x2 + (x3 - x2) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s2 + (s3 - s2) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t2 + (t3 - t2) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c2, c3, beta);

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = uA; uA = uB; uB = fTmp;
            fTmp = vA; vA = vB; vB = fTmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x != xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            // NOTE 1: Divided by 'z', correct perspective
            // NOTE 2: 'z' is actually the reciprocal
            PFfloat u = z*(uA + t*(uB - uA));
            PFfloat v = z*(vA + t*(vB - vA));

            PFcolor tex = pfGetTextureSample(texture, u, v);
            PFcolor src = interpolateColor(cA, cB, t);
            src = pfBlendMultiplicative(tex, src);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalColor = blendFunc(src, dst);
            pixelSetter(bufDst, xyOffset, finalColor);

            zbDst[xyOffset] = z;
        }
    }
}

void Rasterize_Triangle_TEXTURE_LIGHT_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    if (!Helper_FaceCanBeRendered(faceToRender, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFfloat x1 = v1->screen[0], y1 = v1->screen[1], z1 = v1->homogeneous[2];
    const PFfloat x2 = v2->screen[0], y2 = v2->screen[1], z2 = v2->homogeneous[2];
    const PFfloat x3 = v3->screen[0], y3 = v3->screen[1], z3 = v3->homogeneous[2];

    const PFcolor c1 = Process_Gouraud(ctx, v1, viewPos, &ctx->faceMaterial[faceToRender]);
    const PFcolor c2 = Process_Gouraud(ctx, v2, viewPos, &ctx->faceMaterial[faceToRender]);
    const PFcolor c3 = Process_Gouraud(ctx, v3, viewPos, &ctx->faceMaterial[faceToRender]);

    const PFfloat s1 = v1->texcoord[0], t1 = v1->texcoord[1];
    const PFfloat s2 = v2->texcoord[0], t2 = v2->texcoord[1];
    const PFfloat s3 = v3->texcoord[0], t3 = v3->texcoord[1];

    const PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    const PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    const PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    /* Get Some Contextual Values */

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    PFtexture *texture = ctx->currentTexture;
    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    // Rasterization of the first part of the triangle (y1 to y2)

    for (PFint y = y1; y < y2; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y1 + 1) * invSegmentHeight21;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x1 + (x2 - x1) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z1 + (z2 - z1) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s1 + (s2 - s1) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t1 + (t2 - t1) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c1, c2, beta);

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = uA; uA = uB; uB = fTmp;
            fTmp = vA; vA = vB; vB = fTmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x <= xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            // NOTE 1: Divided by 'z', correct perspective
            // NOTE 2: 'z' is actually the reciprocal
            PFfloat u = z*(uA + t*(uB - uA));
            PFfloat v = z*(vA + t*(vB - vA));

            PFfloat *zp = zbDst + xyOffset;
            if (ctx->depthFunction(z, *zp))
            {
                PFcolor tex = pfGetTextureSample(texture, u, v);
                PFcolor src = interpolateColor(cA, cB, t);
                src = pfBlendMultiplicative(tex, src);
                PFcolor dst = pixelGetter(bufDst, xyOffset);

                PFcolor finalColor = blendFunc(src, dst);
                pixelSetter(bufDst, xyOffset, finalColor);

                *zp = z;
            }
        }
    }

    // Rasterization of the second part of the triangle (y2 to y3)

    for (PFint y = y2; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta = (y - y2 + 1) * invSegmentHeight32;

        PFint xA = x1 + (x3 - x1) * alpha;
        PFint xB = x2 + (x3 - x2) * beta;
        PFfloat zA = z1 + (z3 - z1) * alpha;
        PFfloat zB = z2 + (z3 - z2) * beta;

        PFfloat uA = s1 + (s3 - s1) * alpha;
        PFfloat uB = s2 + (s3 - s2) * beta;
        PFfloat vA = t1 + (t3 - t1) * alpha;
        PFfloat vB = t2 + (t3 - t2) * beta;

        PFcolor cA = interpolateColor(c1, c3, alpha);
        PFcolor cB = interpolateColor(c2, c3, beta);

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = uA; uA = uB; uB = fTmp;
            fTmp = vA; vA = vB; vB = fTmp;

            PFcolor cTmp;
            cTmp = cA; cA = cB; cB = cTmp;
        }

        /* Draw Horizontal Line */

        PFsizei xyOffset = y*fbDst->texture.width + xA;
        PFfloat xInvLen = (xA == xB) ? 0.0f : 1.0f/(xB - xA);

        for (PFint x = xA; x <= xB; x++, xyOffset++)
        {
            PFfloat t = (PFfloat)(x-xA)*xInvLen;
            PFfloat z = 1.0f/(zA + t*(zB - zA));

            // NOTE 1: Divided by 'z', correct perspective
            // NOTE 2: 'z' is actually the reciprocal
            PFfloat u = z*(uA + t*(uB - uA));
            PFfloat v = z*(vA + t*(vB - vA));

            PFfloat *zp = zbDst + xyOffset;
            if (ctx->depthFunction(z, *zp))
            {
                PFcolor tex = pfGetTextureSample(texture, u, v);
                PFcolor src = interpolateColor(cA, cB, t);
                src = pfBlendMultiplicative(tex, src);
                PFcolor dst = pixelGetter(bufDst, xyOffset);

                PFcolor finalColor = blendFunc(src, dst);
                pixelSetter(bufDst, xyOffset, finalColor);

                *zp = z;
            }
        }
    }
}

#endif //PF_GOURAUD_SHADING


/* Internal helper function definitions */

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
