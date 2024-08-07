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

#include "../context/context.h"
#include <stdlib.h>

/* Internal point processing functions declarations */

static PFboolean Process_ProjectPoint(PFvertex* v);

/* Internal point rasterizer function declarations */

static void Rasterize_Point_NODEPTH(const PFvertex* point);
static void Rasterize_Point_DEPTH(const PFvertex* point);


/* Point Process And Rasterize Functions */

void pfInternal_ProcessRasterize_POINT(void)
{
    PFvertex *processed = currentCtx->vertexBuffer;

    if (Process_ProjectPoint(processed))
    {
        (currentCtx->state & PF_DEPTH_TEST ?
            Rasterize_Point_DEPTH : Rasterize_Point_NODEPTH)(processed);
    }
}

void pfInternal_ProcessRasterize_POLY_POINTS(int_fast8_t vertexCount)
{
    for (int_fast8_t i = 0; i < vertexCount; i++)
    {
        PFvertex *processed = currentCtx->vertexBuffer + i;

        if (Process_ProjectPoint(processed))
        {
            (currentCtx->state & PF_DEPTH_TEST ?
                Rasterize_Point_DEPTH : Rasterize_Point_NODEPTH)(processed);
        }
    }
}


/* Internal point processing functions definitions */

PFboolean Process_ProjectPoint(PFvertex* v)
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

/* Internal point rasterizer function definitions */

void Rasterize_Point_NODEPTH(const PFvertex* point)
{
    PFframebuffer *fbDst = currentCtx->currentFramebuffer;
    struct PFtex *texDst = currentCtx->currentFramebuffer->texture;

    PFpixelsetter setter = texDst->setter;
    PFpixelgetter getter = texDst->getter;

    PFblendfunc blendFunc = currentCtx->state & PF_BLEND ?
        currentCtx->blendFunction : NULL;

    void *pbDst = texDst->pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    PFsizei wDst = texDst->width;
    PFsizei hDst = texDst->height;

    PFint cx = point->screen[0];
    PFint cy = point->screen[1];
    PFfloat z = point->homogeneous[2];
    PFcolor color = point->color;

    if (currentCtx->pointSize <= 1.0f)
    {
        PFsizei pOffset = cy*wDst + cx;
        setter(pbDst, pOffset, blendFunc
            ? blendFunc(color, getter(pbDst, pOffset)) : color);
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
                    setter(pbDst, pOffset, blendFunc
                        ? blendFunc(color, getter(pbDst, pOffset)) : color);
                    zbDst[pOffset] = z;
                }
            }
        }
    }
}

void Rasterize_Point_DEPTH(const PFvertex* point)
{
    PFframebuffer *fbDst = currentCtx->currentFramebuffer;
    struct PFtex *texDst = currentCtx->currentFramebuffer->texture;

    PFpixelsetter setter = texDst->setter;
    PFpixelgetter getter = texDst->getter;

    PFblendfunc blendFunc = currentCtx->state & PF_BLEND ?
        currentCtx->blendFunction : NULL;

    void *pbDst = texDst->pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    PFsizei wDst = texDst->width;
    PFsizei hDst = texDst->height;

    PFint cx = point->screen[0];
    PFint cy = point->screen[1];
    PFfloat z = point->homogeneous[2];
    PFcolor color = point->color;

    if (currentCtx->pointSize <= 1.0f)
    {
        PFsizei pOffset = cy*wDst + cx;
        if (currentCtx->depthFunction(z, zbDst[pOffset]))
        {
            setter(pbDst, pOffset, blendFunc
                ? blendFunc(color, getter(pbDst, pOffset)) : color);
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
                        setter(pbDst, pOffset, blendFunc
                            ? blendFunc(color, getter(pbDst, pOffset)) : color);
                        zbDst[pOffset] = z;
                    }
                }
            }
        }
    }
}
