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
    // TODO: Implement a more complete point "rasterization"
    pfSetFramebufferPixel(pfGetCurrentContext()->currentFramebuffer,
        point->screen[0], point->screen[1],
        point->color);
}

void Rasterize_PointDepth(const PFvertex* point)
{
    // TODO: Implement a more complete point "rasterization"
    pfSetFramebufferPixelDepth(pfGetCurrentContext()->currentFramebuffer,
        point->screen[0], point->screen[1],
        point->homogeneous[2],
        point->color);
}
