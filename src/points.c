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
#include "pixelforge.h"

/* Including internal function prototypes */

extern void pfInternal_HomogeneousToScreen(PFvertex* restrict v);

/* Main functions declaration used by 'context.c' */

PFboolean Process_ProjectPoint(PFvertex* restrict v, const PFMmat4 mvp)
{
    const PFctx *ctx = pfGetCurrentContext();

    memcpy(v->homogeneous, v->position, sizeof(PFMvec4));
    pfmVec4Transform(v->homogeneous, v->homogeneous, mvp);

    if (v->homogeneous[3] != 1.0f)
    {
        PFfloat invW = 1.0f / v->homogeneous[3];
        v->homogeneous[0] *= invW;
        v->homogeneous[1] *= invW;
    }

    pfInternal_HomogeneousToScreen(v);

    return v->screen[0] >= ctx->viewportX
        && v->screen[1] >= ctx->viewportY
        && v->screen[0] <= ctx->viewportX + ctx->viewportW
        && v->screen[1] <= ctx->viewportY + ctx->viewportH;
}

void Rasterize_PointFlat(const PFvertex* point)
{
    PFctx *ctx = pfGetCurrentContext();
    PFframebuffer *fbDst = ctx->currentFramebuffer;

    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;

    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    PFsizei wDst = fbDst->texture.width;
    PFsizei hDst = fbDst->texture.height;

    PFint cx = point->screen[0];
    PFint cy = point->screen[1];
    PFfloat z = point->homogeneous[2];
    PFcolor color = point->color;

    if (ctx->pointSize <= 1.0f)
    {
        PFsizei pOffset = cy*wDst + cx;
        pixelSetter(bufDst, pOffset, blendFunc(color, pixelGetter(bufDst, pOffset)));
        zbDst[pOffset] = z;
        return;
    }

    PFfloat r = ctx->pointSize*0.5f;
    PFfloat rSq = r*r;

    for (PFint y = -r; y <= r; y++)
    {
        for (PFint x = -r; x <= r; x++)
        {
            if (y*y + x*x <= rSq)
            {
                PFint px = cx + x, py = cy + y;
                if (px >= 0 && px < wDst && py >= 0 && py < hDst)
                {
                    PFsizei pOffset = py*wDst + px;
                    pixelSetter(bufDst, pOffset, blendFunc(color, pixelGetter(bufDst, pOffset)));
                    zbDst[pOffset] = z;
                }
            }
        }
    }
}

void Rasterize_PointDepth(const PFvertex* point)
{
    PFctx *ctx = pfGetCurrentContext();
    PFframebuffer *fbDst = ctx->currentFramebuffer;

    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;
    PFblendfunc blendFunc = ctx->blendFunction;

    void *bufDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    PFsizei wDst = fbDst->texture.width;
    PFsizei hDst = fbDst->texture.height;

    PFint cx = point->screen[0];
    PFint cy = point->screen[1];
    PFfloat z = point->homogeneous[2];
    PFcolor color = point->color;

    if (ctx->pointSize <= 1.0f)
    {
        PFsizei pOffset = cy*wDst + cx;
        PFfloat *zp = zbDst + pOffset;

        if (z < *zp)
        {
            pixelSetter(bufDst, pOffset, blendFunc(color, pixelGetter(bufDst, pOffset)));
            *zp = z;
        }

        return;
    }

    PFfloat r = ctx->pointSize*0.5f;
    PFfloat rSq = r*r;

    for (PFint y = -r; y <= r; y++)
    {
        for (PFint x = -r; x <= r; x++)
        {
            if (y*y + x*x <= rSq)
            {
                PFint px = cx + x, py = cy + y;
                if (px >= 0 && px < wDst && py >= 0 && py < hDst)
                {
                    PFsizei pOffset = py*wDst + px;
                    PFfloat *zp = zbDst + pOffset;

                    if (z < *zp)
                    {
                        pixelSetter(bufDst, pOffset, blendFunc(color, pixelGetter(bufDst, pOffset)));
                        *zp = z;
                    }
                }
            }
        }
    }
}
