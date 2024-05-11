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

#include "./points.h"

/* Including internal function prototypes */

extern void pfInternal_HomogeneousToScreen(PFvertex* restrict v);

/* Main functions declaration used by 'context.c' */

PFboolean Process_ProjectPoint(PFvertex* restrict v)
{
    memcpy(v->homogeneous, v->position, sizeof(PFMvec4));
    pfmVec4Transform(v->homogeneous, v->homogeneous, currentCtx->matMVP);

    if (v->homogeneous[3] != 1.0f)
    {
        for (int_fast8_t i = 0; i < 3; i++)
        {
            if (v->homogeneous[i] < -v->homogeneous[3] || v->homogeneous[i] > v->homogeneous[3])
            {
                return PF_FALSE;
            }
        }

        PFfloat invW = 1.0f / v->homogeneous[3];
        v->homogeneous[0] *= invW;
        v->homogeneous[1] *= invW;
    }

    pfInternal_HomogeneousToScreen(v);

    return v->screen[0] >= currentCtx->vpMin[0]
        && v->screen[1] >= currentCtx->vpMin[1]
        && v->screen[0] <= currentCtx->vpMax[0]
        && v->screen[1] <= currentCtx->vpMax[1];
}

void Rasterize_Point_NODEPTH(const PFvertex* point)
{
    PFframebuffer *fbDst = currentCtx->currentFramebuffer;

    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;

    PFblendfunc blendFunc = currentCtx->state & PF_BLEND ?
        currentCtx->blendFunction : NULL;

    void *pbDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    PFsizei wDst = fbDst->texture.width;
    PFsizei hDst = fbDst->texture.height;

    PFint cx = point->screen[0];
    PFint cy = point->screen[1];
    PFfloat z = point->homogeneous[2];
    PFcolor color = point->color;

    if (currentCtx->pointSize <= 1.0f)
    {
        PFsizei pOffset = cy*wDst + cx;
        pixelSetter(pbDst, pOffset, blendFunc
            ? blendFunc(color, pixelGetter(pbDst, pOffset)) : color);
        zbDst[pOffset] = z;
        return;
    }

    PFfloat r = currentCtx->pointSize*0.5f;
    PFfloat rSq = r*r;

    for (PFint y = -r; y <= r; y++)
    {
        for (PFint x = -r; x <= r; x++)
        {
            if (y*y + x*x <= rSq)
            {
                PFsizei px = cx + x, py = cy + y;
                if (px < wDst && py < hDst)
                {
                    PFsizei pOffset = py*wDst + px;
                    pixelSetter(pbDst, pOffset, blendFunc
                        ? blendFunc(color, pixelGetter(pbDst, pOffset)) : color);
                    zbDst[pOffset] = z;
                }
            }
        }
    }
}

void Rasterize_Point_DEPTH(const PFvertex* point)
{
    PFframebuffer *fbDst = currentCtx->currentFramebuffer;

    PFpixelsetter pixelSetter = fbDst->texture.pixelSetter;
    PFpixelgetter pixelGetter = fbDst->texture.pixelGetter;

    PFblendfunc blendFunc = currentCtx->state & PF_BLEND ?
        currentCtx->blendFunction : NULL;

    void *pbDst = fbDst->texture.pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    PFsizei wDst = fbDst->texture.width;
    PFsizei hDst = fbDst->texture.height;

    PFint cx = point->screen[0];
    PFint cy = point->screen[1];
    PFfloat z = point->homogeneous[2];
    PFcolor color = point->color;

    if (currentCtx->pointSize <= 1.0f)
    {
        PFsizei pOffset = cy*wDst + cx;
        if (currentCtx->depthFunction(z, zbDst[pOffset]))
        {
            pixelSetter(pbDst, pOffset, blendFunc
                ? blendFunc(color, pixelGetter(pbDst, pOffset)) : color);
            zbDst[pOffset] = z;
        }

        return;
    }

    PFfloat r = currentCtx->pointSize*0.5f;
    PFfloat rSq = r*r;

    for (PFint y = -r; y <= r; y++)
    {
        for (PFint x = -r; x <= r; x++)
        {
            if (y*y + x*x <= rSq)
            {
                PFsizei px = cx + x, py = cy + y;
                if (px < wDst && py < hDst)
                {
                    PFsizei pOffset = py*wDst + px;
                    if (currentCtx->depthFunction(z, zbDst[pOffset]))
                    {
                        pixelSetter(pbDst, pOffset, blendFunc
                            ? blendFunc(color, pixelGetter(pbDst, pOffset)) : color);
                        zbDst[pOffset] = z;
                    }
                }
            }
        }
    }
}
