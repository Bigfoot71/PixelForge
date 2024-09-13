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

void pfiProcessRasterize_POINT(void)
{
    PFvertex *processed = G_currentCtx->vertexBuffer;

    if (Process_ProjectPoint(processed))
    {
        (G_currentCtx->state & PF_DEPTH_TEST ?
            Rasterize_Point_DEPTH : Rasterize_Point_NODEPTH)(processed);
    }
}

void pfiProcessRasterize_POLY_POINTS(int_fast8_t vertexCount)
{
    for (int_fast8_t i = 0; i < vertexCount; i++)
    {
        PFvertex *processed = G_currentCtx->vertexBuffer + i;

        if (Process_ProjectPoint(processed))
        {
            (G_currentCtx->state & PF_DEPTH_TEST ?
                Rasterize_Point_DEPTH : Rasterize_Point_NODEPTH)(processed);
        }
    }
}


/* Internal point processing functions definitions */

PFboolean Process_ProjectPoint(PFvertex* v)
{
    memcpy(v->homogeneous, v->position, sizeof(PFMvec4));
    pfmVec4Transform(v->homogeneous, v->homogeneous, G_currentCtx->matMVP);

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

    pfiHomogeneousToScreen(v);

    return v->screen[0] >= G_currentCtx->vpMin[0]
        && v->screen[1] >= G_currentCtx->vpMin[1]
        && v->screen[0] <= G_currentCtx->vpMax[0]
        && v->screen[1] <= G_currentCtx->vpMax[1];
}

/* Internal point rasterizer function definitions */

void Rasterize_Point_NODEPTH(const PFvertex* point)
{
    PFframebuffer *fbDst = G_currentCtx->currentFramebuffer;
    struct PFtex *texDst = G_currentCtx->currentFramebuffer->texture;

    PFpixelsetter setter = texDst->setter;
    PFpixelgetter getter = texDst->getter;

    PFblendfunc blendFunc = G_currentCtx->state & PF_BLEND ?
        G_currentCtx->blendFunction : NULL;

    void *pbDst = texDst->pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    PFsizei wDst = texDst->w;
    PFsizei hDst = texDst->h;

    PFint cx = point->screen[0];
    PFint cy = point->screen[1];
    PFfloat z = point->homogeneous[2];
    PFcolor color = point->color;

    if (G_currentCtx->pointSize <= 1.0f)
    {
        PFsizei pOffset = cy*wDst + cx;
        setter(pbDst, pOffset, blendFunc
            ? blendFunc(color, getter(pbDst, pOffset)) : color);
        zbDst[pOffset] = z;
        return;
    }

    PFfloat r = G_currentCtx->pointSize*0.5f;
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
    PFframebuffer *fbDst = G_currentCtx->currentFramebuffer;
    struct PFtex *texDst = G_currentCtx->currentFramebuffer->texture;

    PFpixelsetter setter = texDst->setter;
    PFpixelgetter getter = texDst->getter;

    PFblendfunc blendFunc = G_currentCtx->state & PF_BLEND ?
        G_currentCtx->blendFunction : NULL;

    void *pbDst = texDst->pixels;
    PFfloat *zbDst = fbDst->zbuffer;

    PFsizei wDst = texDst->w;
    PFsizei hDst = texDst->h;

    PFint cx = point->screen[0];
    PFint cy = point->screen[1];
    PFfloat z = point->homogeneous[2];
    PFcolor color = point->color;

    if (G_currentCtx->pointSize <= 1.0f)
    {
        PFsizei pOffset = cy*wDst + cx;
        if (G_currentCtx->depthFunction(z, zbDst[pOffset]))
        {
            setter(pbDst, pOffset, blendFunc
                ? blendFunc(color, getter(pbDst, pOffset)) : color);
            zbDst[pOffset] = z;
        }

        return;
    }

    PFfloat r = G_currentCtx->pointSize*0.5f;
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
                    if (G_currentCtx->depthFunction(z, zbDst[pOffset]))
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
