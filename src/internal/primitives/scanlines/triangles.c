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

#include "../../lighting/lighting.h"
#include "../../context.h"
#include "../../../pfm.h"

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

static PFboolean Helper_FaceCanBeRendered(PFface faceToRender, PFfloat* area, const PFMvec2 p1, const PFMvec2 p2, const PFMvec2 p3);
static void Helper_SortVertices(const PFvertex** v1, const PFvertex** v2, const PFvertex** v3);

static PFcolor Helper_InterpolateColor_SMOOTH(PFcolor v1, PFcolor v2, PFfloat t);
static PFcolor Helper_InterpolateColor_FLAT(PFcolor v1, PFcolor v2, PFfloat t);


/* Internal triangle 2D rasterizer function definitions */

void Rasterize_Triangle_COLOR_NODEPTH_2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;

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

    /* Travel the triangle from top to bottom */

    PFint yMax = CLAMP(y3, ctx->vpMin[1], ctx->vpMax[1]);

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = CLAMP(y1, ctx->vpMin[1], ctx->vpMax[1]); y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);
        }

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
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;

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

    /* Travel the triangle from top to bottom */

    PFint yMax = CLAMP(y3, ctx->vpMin[1], ctx->vpMax[1]);

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = CLAMP(y1, ctx->vpMin[1], ctx->vpMax[1]); y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);
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
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;
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

    /* Travel the triangle from top to bottom */

    PFint yMax = CLAMP(y3, ctx->vpMin[1], ctx->vpMax[1]);

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = CLAMP(y1, ctx->vpMin[1], ctx->vpMax[1]); y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFfloat uA, uB;
        PFfloat vA, vB;
        PFcolor cA, cB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            uA = s1 + (s3 - s1) * alpha;
            uB = s1 + (s2 - s1) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t1 + (t2 - t1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            uA = s1 + (s3 - s1) * alpha;
            uB = s2 + (s3 - s2) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t2 + (t3 - t2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);
        }

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
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;
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

    /* Travel the triangle from top to bottom */

    PFint yMax = CLAMP(y3, ctx->vpMin[1], ctx->vpMax[1]);

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = CLAMP(y1, ctx->vpMin[1], ctx->vpMax[1]); y <= yMax; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFfloat uA, uB;
        PFfloat vA, vB;
        PFcolor cA, cB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            uA = s1 + (s3 - s1) * alpha;
            uB = s1 + (s2 - s1) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t1 + (t2 - t1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            uA = s1 + (s3 - s1) * alpha;
            uB = s2 + (s3 - s2) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t2 + (t3 - t2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);
        }

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
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;

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

    /* Travel the triangle from top to bottom */

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = y1; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);
        }

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
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;

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

    /* Travel the triangle from top to bottom */

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = y1; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);
        }

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
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;
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

    /* Travel the triangle from top to bottom */

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = y1; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;
        PFfloat uA, uB;
        PFfloat vA, vB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);

            uA = s1 + (s3 - s1) * alpha;
            uB = s1 + (s2 - s1) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t1 + (t2 - t1) * beta;
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);

            uA = s1 + (s3 - s1) * alpha;
            uB = s2 + (s3 - s2) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t2 + (t3 - t2) * beta;
        }

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
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;
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

    /* Travel the triangle from top to bottom */

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = y1; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;
        PFfloat uA, uB;
        PFfloat vA, vB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);

            uA = s1 + (s3 - s1) * alpha;
            uB = s1 + (s2 - s1) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t1 + (t2 - t1) * beta;
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);

            uA = s1 + (s3 - s1) * alpha;
            uB = s2 + (s3 - s2) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t2 + (t3 - t2) * beta;
        }

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
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;

    const PFfloat px1 = v1->position[0], py1 = v1->position[1], pz1 = v1->position[2];
    const PFfloat px2 = v2->position[0], py2 = v2->position[1], pz2 = v2->position[2];
    const PFfloat px3 = v3->position[0], py3 = v3->position[1], pz3 = v3->position[2];

    const PFfloat nx1 = v1->normal[0], ny1 = v1->normal[1], nz1 = v1->normal[2];
    const PFfloat nx2 = v2->normal[0], ny2 = v2->normal[1], nz2 = v2->normal[2];
    const PFfloat nx3 = v3->normal[0], ny3 = v3->normal[1], nz3 = v3->normal[2];

    const PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    const PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    const PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    /* Get Some Contextual Values */

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    const PFlight *lights = ctx->lights;
    PFfloat shininess = ctx->faceMaterial[faceToRender].shininess;

    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    /* Travel the triangle from top to bottom */

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = y1; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;
        PFfloat pxA, pyA, pzA;
        PFfloat pxB, pyB, pzB;
        PFfloat nxA, nyA, nzA;
        PFfloat nxB, nyB, nzB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);

            pxA = px1 + (px3 - px1) * alpha;
            pxB = px1 + (px2 - px1) * beta;
            pyA = py1 + (py3 - py1) * alpha;
            pyB = py1 + (py2 - py1) * beta;
            pzA = pz1 + (pz3 - pz1) * alpha;
            pzB = pz1 + (pz2 - pz1) * beta;

            nxA = nx1 + (nx3 - nx1) * alpha;
            nxB = nx1 + (nx2 - nx1) * beta;
            nyA = ny1 + (ny3 - ny1) * alpha;
            nyB = ny1 + (ny2 - ny1) * beta;
            nzA = nz1 + (nz3 - nz1) * alpha;
            nzB = nz1 + (nz2 - nz1) * beta;
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);

            pxA = px1 + (px3 - px1) * alpha;
            pxB = px2 + (px3 - px2) * beta;
            pyA = py1 + (py3 - py1) * alpha;
            pyB = py2 + (py3 - py2) * beta;
            pzA = pz1 + (pz3 - pz1) * alpha;
            pzB = pz2 + (pz3 - pz2) * beta;

            nxA = nx1 + (nx3 - nx1) * alpha;
            nxB = nx2 + (nx3 - nx2) * beta;
            nyA = ny1 + (ny3 - ny1) * alpha;
            nyB = ny2 + (ny3 - ny2) * beta;
            nzA = nz1 + (nz3 - nz1) * alpha;
            nzB = nz2 + (nz3 - nz2) * beta;
        }

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = pxA; pxA = pxB; pxB = fTmp;
            fTmp = pyA; pyA = pyB; pyB = fTmp;
            fTmp = pzA; pzA = pzB; pzB = fTmp;
            fTmp = nxA; nxA = nxB; nxB = fTmp;
            fTmp = nyA; nyA = nyB; nyB = fTmp;
            fTmp = nzA; nzA = nzB; nzB = fTmp;

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

            PFMvec3 position = {
                pxA + t*(pxB - pxA),
                pyA + t*(pyB - pyA),
                pzA + t*(pzB - pzA)
            };

            PFMvec3 normal = {
                nxA + t*(nxB - nxA),
                nyA + t*(nyB - nyA),
                nzA + t*(nzB - nzA)
            };

            PFcolor src = interpolateColor(cA, cB, t);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalLightColor = { 0 };
            PFcolor finalColor = blendFunc(src, dst);

            for (int_fast8_t i = 0; i <= ctx->lastActiveLight; i++)
            {
                const PFlight *light = lights + i;
                if (!light->active) continue;

                PFcolor lightColor = Process_Light(light, light->ambient,
                    finalColor, viewPos, position, normal, shininess);

                finalLightColor = pfBlendAdditive(
                    finalLightColor, lightColor);
            }

            pixelSetter(bufDst, xyOffset, finalLightColor);
            zbDst[xyOffset] = z;
        }
    }
}

void Rasterize_Triangle_COLOR_LIGHT_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;

    const PFfloat px1 = v1->position[0], py1 = v1->position[1], pz1 = v1->position[2];
    const PFfloat px2 = v2->position[0], py2 = v2->position[1], pz2 = v2->position[2];
    const PFfloat px3 = v3->position[0], py3 = v3->position[1], pz3 = v3->position[2];

    const PFfloat nx1 = v1->normal[0], ny1 = v1->normal[1], nz1 = v1->normal[2];
    const PFfloat nx2 = v2->normal[0], ny2 = v2->normal[1], nz2 = v2->normal[2];
    const PFfloat nx3 = v3->normal[0], ny3 = v3->normal[1], nz3 = v3->normal[2];

    const PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    const PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    const PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    /* Get Some Contextual Values */

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    const PFlight *lights = ctx->lights;
    PFfloat shininess = ctx->faceMaterial[faceToRender].shininess;

    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    /* Travel the triangle from top to bottom */

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = y1; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;
        PFfloat pxA, pyA, pzA;
        PFfloat pxB, pyB, pzB;
        PFfloat nxA, nyA, nzA;
        PFfloat nxB, nyB, nzB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);

            pxA = px1 + (px3 - px1) * alpha;
            pxB = px1 + (px2 - px1) * beta;
            pyA = py1 + (py3 - py1) * alpha;
            pyB = py1 + (py2 - py1) * beta;
            pzA = pz1 + (pz3 - pz1) * alpha;
            pzB = pz1 + (pz2 - pz1) * beta;

            nxA = nx1 + (nx3 - nx1) * alpha;
            nxB = nx1 + (nx2 - nx1) * beta;
            nyA = ny1 + (ny3 - ny1) * alpha;
            nyB = ny1 + (ny2 - ny1) * beta;
            nzA = nz1 + (nz3 - nz1) * alpha;
            nzB = nz1 + (nz2 - nz1) * beta;
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);

            pxA = px1 + (px3 - px1) * alpha;
            pxB = px2 + (px3 - px2) * beta;
            pyA = py1 + (py3 - py1) * alpha;
            pyB = py2 + (py3 - py2) * beta;
            pzA = pz1 + (pz3 - pz1) * alpha;
            pzB = pz2 + (pz3 - pz2) * beta;

            nxA = nx1 + (nx3 - nx1) * alpha;
            nxB = nx2 + (nx3 - nx2) * beta;
            nyA = ny1 + (ny3 - ny1) * alpha;
            nyB = ny2 + (ny3 - ny2) * beta;
            nzA = nz1 + (nz3 - nz1) * alpha;
            nzB = nz2 + (nz3 - nz2) * beta;
        }

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = pxA; pxA = pxB; pxB = fTmp;
            fTmp = pyA; pyA = pyB; pyB = fTmp;
            fTmp = pzA; pzA = pzB; pzB = fTmp;
            fTmp = nxA; nxA = nxB; nxB = fTmp;
            fTmp = nyA; nyA = nyB; nyB = fTmp;
            fTmp = nzA; nzA = nzB; nzB = fTmp;

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

            PFfloat *zp = zbDst + xyOffset;
            if (ctx->depthFunction(z, *zp))
            {
                PFMvec3 position = {
                    pxA + t*(pxB - pxA),
                    pyA + t*(pyB - pyA),
                    pzA + t*(pzB - pzA)
                };

                PFMvec3 normal = {
                    nxA + t*(nxB - nxA),
                    nyA + t*(nyB - nyA),
                    nzA + t*(nzB - nzA)
                };

                PFcolor src = interpolateColor(cA, cB, t);
                PFcolor dst = pixelGetter(bufDst, xyOffset);

                PFcolor finalLightColor = { 0 };
                PFcolor finalColor = blendFunc(src, dst);

                for (int_fast8_t i = 0; i <= ctx->lastActiveLight; i++)
                {
                    const PFlight *light = lights + i;
                    if (!light->active) continue;

                    PFcolor lightColor = Process_Light(light, light->ambient,
                        finalColor, viewPos, position, normal, shininess);

                    finalLightColor = pfBlendAdditive(
                        finalLightColor, lightColor);
                }

                pixelSetter(bufDst, xyOffset, finalLightColor);
                *zp = z;
            }
        }
    }
}

void Rasterize_Triangle_TEXTURE_LIGHT_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;

    const PFfloat px1 = v1->position[0], py1 = v1->position[1], pz1 = v1->position[2];
    const PFfloat px2 = v2->position[0], py2 = v2->position[1], pz2 = v2->position[2];
    const PFfloat px3 = v3->position[0], py3 = v3->position[1], pz3 = v3->position[2];

    const PFfloat nx1 = v1->normal[0], ny1 = v1->normal[1], nz1 = v1->normal[2];
    const PFfloat nx2 = v2->normal[0], ny2 = v2->normal[1], nz2 = v2->normal[2];
    const PFfloat nx3 = v3->normal[0], ny3 = v3->normal[1], nz3 = v3->normal[2];

    const PFfloat s1 = v1->texcoord[0], t1 = v1->texcoord[1];
    const PFfloat s2 = v2->texcoord[0], t2 = v2->texcoord[1];
    const PFfloat s3 = v3->texcoord[0], t3 = v3->texcoord[1];

    const PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    const PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    const PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    /* Get Some Contextual Values */

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    const PFlight *lights = ctx->lights;
    PFfloat shininess = ctx->faceMaterial[faceToRender].shininess;

    PFtexture *texture = ctx->currentTexture;
    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    /* Travel the triangle from top to bottom */

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = y1; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;
        PFfloat uA, uB;
        PFfloat vA, vB;
        PFfloat pxA, pyA, pzA;
        PFfloat pxB, pyB, pzB;
        PFfloat nxA, nyA, nzA;
        PFfloat nxB, nyB, nzB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);

            uA = s1 + (s3 - s1) * alpha;
            uB = s1 + (s2 - s1) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t1 + (t2 - t1) * beta;

            pxA = px1 + (px3 - px1) * alpha;
            pxB = px1 + (px2 - px1) * beta;
            pyA = py1 + (py3 - py1) * alpha;
            pyB = py1 + (py2 - py1) * beta;
            pzA = pz1 + (pz3 - pz1) * alpha;
            pzB = pz1 + (pz2 - pz1) * beta;

            nxA = nx1 + (nx3 - nx1) * alpha;
            nxB = nx1 + (nx2 - nx1) * beta;
            nyA = ny1 + (ny3 - ny1) * alpha;
            nyB = ny1 + (ny2 - ny1) * beta;
            nzA = nz1 + (nz3 - nz1) * alpha;
            nzB = nz1 + (nz2 - nz1) * beta;
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);

            uA = s1 + (s3 - s1) * alpha;
            uB = s2 + (s3 - s2) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t2 + (t3 - t2) * beta;

            pxA = px1 + (px3 - px1) * alpha;
            pxB = px2 + (px3 - px2) * beta;
            pyA = py1 + (py3 - py1) * alpha;
            pyB = py2 + (py3 - py2) * beta;
            pzA = pz1 + (pz3 - pz1) * alpha;
            pzB = pz2 + (pz3 - pz2) * beta;

            nxA = nx1 + (nx3 - nx1) * alpha;
            nxB = nx2 + (nx3 - nx2) * beta;
            nyA = ny1 + (ny3 - ny1) * alpha;
            nyB = ny2 + (ny3 - ny2) * beta;
            nzA = nz1 + (nz3 - nz1) * alpha;
            nzB = nz2 + (nz3 - nz2) * beta;
        }

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = uA; uA = uB; uB = fTmp;
            fTmp = vA; vA = vB; vB = fTmp;
            fTmp = pxA; pxA = pxB; pxB = fTmp;
            fTmp = pyA; pyA = pyB; pyB = fTmp;
            fTmp = pzA; pzA = pzB; pzB = fTmp;
            fTmp = nxA; nxA = nxB; nxB = fTmp;
            fTmp = nyA; nyA = nyB; nyB = fTmp;
            fTmp = nzA; nzA = nzB; nzB = fTmp;

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

            PFMvec3 position = {
                pxA + t*(pxB - pxA),
                pyA + t*(pyB - pyA),
                pzA + t*(pzB - pzA)
            };

            PFMvec3 normal = {
                nxA + t*(nxB - nxA),
                nyA + t*(nyB - nyA),
                nzA + t*(nzB - nzA)
            };

            PFcolor tex = pfGetTextureSample(texture, u, v);
            PFcolor src = interpolateColor(cA, cB, t);
            src = pfBlendMultiplicative(tex, src);
            PFcolor dst = pixelGetter(bufDst, xyOffset);

            PFcolor finalLightColor = { 0 };
            PFcolor finalColor = blendFunc(src, dst);

            for (int_fast8_t i = 0; i <= ctx->lastActiveLight; i++)
            {
                const PFlight *light = lights + i;
                if (!light->active) continue;

                PFcolor lightColor = Process_Light(light, light->ambient,
                    finalColor, viewPos, position, normal, shininess);

                finalLightColor = pfBlendAdditive(
                    finalLightColor, lightColor);
            }

            pixelSetter(bufDst, xyOffset, finalLightColor);
            zbDst[xyOffset] = z;
        }
    }
}

void Rasterize_Triangle_TEXTURE_LIGHT_DEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];
    const PFcolor c1 = v1->color, c2 = v2->color, c3 = v3->color;

    const PFfloat px1 = v1->position[0], py1 = v1->position[1], pz1 = v1->position[2];
    const PFfloat px2 = v2->position[0], py2 = v2->position[1], pz2 = v2->position[2];
    const PFfloat px3 = v3->position[0], py3 = v3->position[1], pz3 = v3->position[2];

    const PFfloat nx1 = v1->normal[0], ny1 = v1->normal[1], nz1 = v1->normal[2];
    const PFfloat nx2 = v2->normal[0], ny2 = v2->normal[1], nz2 = v2->normal[2];
    const PFfloat nx3 = v3->normal[0], ny3 = v3->normal[1], nz3 = v3->normal[2];

    const PFfloat s1 = v1->texcoord[0], t1 = v1->texcoord[1];
    const PFfloat s2 = v2->texcoord[0], t2 = v2->texcoord[1];
    const PFfloat s3 = v3->texcoord[0], t3 = v3->texcoord[1];

    const PFfloat invTotalHeight = 1.0f / (y3 - y1 + 1);
    const PFfloat invSegmentHeight21 = 1.0f / (y2 - y1 + 1);
    const PFfloat invSegmentHeight32 = 1.0f / (y3 - y2 + 1);

    /* Get Some Contextual Values */

    InterpolateColorFunc interpolateColor = (ctx->shadingMode == PF_SMOOTH)
        ? Helper_InterpolateColor_SMOOTH : Helper_InterpolateColor_FLAT;

    const PFlight *lights = ctx->lights;
    PFfloat shininess = ctx->faceMaterial[faceToRender].shininess;

    PFtexture *texture = ctx->currentTexture;
    PFframebuffer *fbDst = ctx->currentFramebuffer;
    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;
    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    /* Travel the triangle from top to bottom */

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = y1; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;
        PFfloat uA, uB;
        PFfloat vA, vB;
        PFfloat pxA, pyA, pzA;
        PFfloat pxB, pyB, pzB;
        PFfloat nxA, nyA, nzA;
        PFfloat nxB, nyB, nzB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);

            uA = s1 + (s3 - s1) * alpha;
            uB = s1 + (s2 - s1) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t1 + (t2 - t1) * beta;

            pxA = px1 + (px3 - px1) * alpha;
            pxB = px1 + (px2 - px1) * beta;
            pyA = py1 + (py3 - py1) * alpha;
            pyB = py1 + (py2 - py1) * beta;
            pzA = pz1 + (pz3 - pz1) * alpha;
            pzB = pz1 + (pz2 - pz1) * beta;

            nxA = nx1 + (nx3 - nx1) * alpha;
            nxB = nx1 + (nx2 - nx1) * beta;
            nyA = ny1 + (ny3 - ny1) * alpha;
            nyB = ny1 + (ny2 - ny1) * beta;
            nzA = nz1 + (nz3 - nz1) * alpha;
            nzB = nz1 + (nz2 - nz1) * beta;
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);

            uA = s1 + (s3 - s1) * alpha;
            uB = s2 + (s3 - s2) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t2 + (t3 - t2) * beta;

            pxA = px1 + (px3 - px1) * alpha;
            pxB = px2 + (px3 - px2) * beta;
            pyA = py1 + (py3 - py1) * alpha;
            pyB = py2 + (py3 - py2) * beta;
            pzA = pz1 + (pz3 - pz1) * alpha;
            pzB = pz2 + (pz3 - pz2) * beta;

            nxA = nx1 + (nx3 - nx1) * alpha;
            nxB = nx2 + (nx3 - nx2) * beta;
            nyA = ny1 + (ny3 - ny1) * alpha;
            nyB = ny2 + (ny3 - ny2) * beta;
            nzA = nz1 + (nz3 - nz1) * alpha;
            nzB = nz2 + (nz3 - nz2) * beta;
        }

        if (xA > xB)
        {
            PFint iTmp;
            iTmp = xA; xA = xB; xB = iTmp;

            PFfloat fTmp;
            fTmp = zA; zA = zB; zB = fTmp;
            fTmp = uA; uA = uB; uB = fTmp;
            fTmp = vA; vA = vB; vB = fTmp;
            fTmp = pxA; pxA = pxB; pxB = fTmp;
            fTmp = pyA; pyA = pyB; pyB = fTmp;
            fTmp = pzA; pzA = pzB; pzB = fTmp;
            fTmp = nxA; nxA = nxB; nxB = fTmp;
            fTmp = nyA; nyA = nyB; nyB = fTmp;
            fTmp = nzA; nzA = nzB; nzB = fTmp;

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

            PFfloat *zp = zbDst + xyOffset;
            if (ctx->depthFunction(z, *zp))
            {
                // NOTE 1: Divided by 'z', correct perspective
                // NOTE 2: 'z' is actually the reciprocal
                PFfloat u = z*(uA + t*(uB - uA));
                PFfloat v = z*(vA + t*(vB - vA));

                PFMvec3 position = {
                    pxA + t*(pxB - pxA),
                    pyA + t*(pyB - pyA),
                    pzA + t*(pzB - pzA)
                };

                PFMvec3 normal = {
                    nxA + t*(nxB - nxA),
                    nyA + t*(nyB - nyA),
                    nzA + t*(nzB - nzA)
                };

                PFcolor tex = pfGetTextureSample(texture, u, v);
                PFcolor src = interpolateColor(cA, cB, t);
                src = pfBlendMultiplicative(tex, src);
                PFcolor dst = pixelGetter(bufDst, xyOffset);

                PFcolor finalLightColor = { 0 };
                PFcolor finalColor = blendFunc(src, dst);

                for (int_fast8_t i = 0; i <= ctx->lastActiveLight; i++)
                {
                    const PFlight *light = lights + i;
                    if (!light->active) continue;

                    PFcolor lightColor = Process_Light(light, light->ambient,
                        finalColor, viewPos, position, normal, shininess);

                    finalLightColor = pfBlendAdditive(
                        finalLightColor, lightColor);
                }

                pixelSetter(bufDst, xyOffset, finalLightColor);
                *zp = z;
            }
        }
    }
}

#else

void Rasterize_Triangle_COLOR_LIGHT_NODEPTH_3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos)
{
    PFctx *ctx = pfGetCurrentContext();

    /* Prepare for rasterization */

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];

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

    /* Travel the triangle from top to bottom */

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = y1; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);
        }

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

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];

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

    /* Travel the triangle from top to bottom */

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = y1; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFcolor cA, cB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);
        }

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

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];

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

    /* Travel the triangle from top to bottom */

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = y1; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFfloat uA, uB;
        PFfloat vA, vB;
        PFcolor cA, cB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            uA = s1 + (s3 - s1) * alpha;
            uB = s1 + (s2 - s1) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t1 + (t2 - t1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            uA = s1 + (s3 - s1) * alpha;
            uB = s2 + (s3 - s2) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t2 + (t3 - t2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);
        }

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

    PFfloat triangleArea = 0;

    if (!Helper_FaceCanBeRendered(faceToRender, &triangleArea, v1->screen, v2->screen, v3->screen))
    {
        return;
    }

    Helper_SortVertices(&v1, &v2, &v3);

    const PFint x1 = v1->screen[0], y1 = v1->screen[1];
    const PFint x2 = v2->screen[0], y2 = v2->screen[1];
    const PFint x3 = v3->screen[0], y3 = v3->screen[1];

    const PFfloat z1 = v1->homogeneous[2], z2 = v2->homogeneous[2], z3 = v3->homogeneous[2];

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

    /* Travel the triangle from top to bottom */

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp parallel for if(triangleArea >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif //PF_SUPPORT_OPENMP
    for (PFint y = y1; y <= y3; y++)
    {
        PFfloat alpha = (y - y1 + 1) * invTotalHeight;
        PFfloat beta;

        PFint xA, xB;
        PFfloat zA, zB;
        PFfloat uA, uB;
        PFfloat vA, vB;
        PFcolor cA, cB;

        if (y < y2) // First half
        {
            beta = (y - y1 + 1) * invSegmentHeight21;

            xA = x1 + (x3 - x1) * alpha;
            xB = x1 + (x2 - x1) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z1 + (z2 - z1) * beta;

            uA = s1 + (s3 - s1) * alpha;
            uB = s1 + (s2 - s1) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t1 + (t2 - t1) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c1, c2, beta);
        }
        else // Second half
        {
            beta = (y - y2 + 1) * invSegmentHeight32;

            xA = x1 + (x3 - x1) * alpha;
            xB = x2 + (x3 - x2) * beta;
            zA = z1 + (z3 - z1) * alpha;
            zB = z2 + (z3 - z2) * beta;

            uA = s1 + (s3 - s1) * alpha;
            uB = s2 + (s3 - s2) * beta;
            vA = t1 + (t3 - t1) * alpha;
            vB = t2 + (t3 - t2) * beta;

            cA = interpolateColor(c1, c3, alpha);
            cB = interpolateColor(c2, c3, beta);
        }

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
