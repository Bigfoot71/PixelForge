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

void Process_ProjectAndClipLine(PFvertex* restrict line, int_fast8_t* restrict vertexCounter, const PFMmat4 mvp);

void Rasterize_LineFlat(const PFvertex* v1, const PFvertex* v2);
void Rasterize_LineDepth(const PFvertex* v1, const PFvertex* v2);

/* Internal helper function declarations */

static void Helper_SwapVertex(PFvertex* a, PFvertex* b);
static void Helper_SwapByte(PFubyte* a, PFubyte* b);

static PFubyte Helper_EncodeClip2D(const PFMvec2 screen, PFint xMin, PFint yMin, PFint xMax, PFint yMax);
static PFboolean Helper_ClipCoord3D(PFfloat q, PFfloat p, PFfloat* t1, PFfloat* t2);

static PFcolor Helper_LerpColor(PFcolor a, PFcolor b, PFfloat t);

/* Enums for internal use */

typedef enum {
    CLIP_INSIDE = 0x00, // 0000
    CLIP_LEFT   = 0x01, // 0001
    CLIP_RIGHT  = 0x02, // 0010
    CLIP_BOTTOM = 0x04, // 0100
    CLIP_TOP    = 0x08, // 1000
} PFclipcode;


/* Line processing functions */

static PFboolean Process_ClipLine2D(PFvertex* restrict v1, PFvertex* restrict v2)
{
    const PFctx *ctx = pfGetCurrentContext();

    PFint xMin = ctx->viewportX;
    PFint yMin = ctx->viewportY;
    PFint xMax = ctx->viewportX + ctx->viewportW;
    PFint yMax = ctx->viewportY + ctx->viewportH;

    PFboolean accept = PF_FALSE;
    PFubyte code0, code1;
    PFfloat m = 0;

    if (v1->screen[0] != v2->screen[0])
    {
        m = (v2->screen[1] - v1->screen[1]) / (v2->screen[0] - v1->screen[0]);
    }

    for (;;)
    {
        code0 = Helper_EncodeClip2D(v1->screen, xMin, yMin, xMax, yMax);
        code1 = Helper_EncodeClip2D(v2->screen, xMin, yMin, xMax, yMax);

        // Accepted if both endpoints lie within rectangle
        if ((code0 | code1) == 0)
        {
            accept = PF_TRUE;
            break;
        }

        // Rejected if both endpoints are outside rectangle, in same region
        if (code0 & code1) break;

        if (code0 == CLIP_INSIDE)
        {
            Helper_SwapByte(&code0, &code1);
            Helper_SwapVertex(v1, v2);
        }

        if (code0 & CLIP_LEFT)
        {
            v1->screen[1] += (ctx->viewportX - v1->screen[0])*m;
            v1->screen[0] = ctx->viewportX;
        }
        else if (code0 & CLIP_RIGHT)
        {
            v1->screen[1] += (ctx->viewportW - v1->screen[0])*m;
            v1->screen[0] = ctx->viewportW;
        }
        else if (code0 & CLIP_BOTTOM)
        {
            if (m) v1->screen[0] += (ctx->viewportY - v1->screen[1]) / m;
            v1->screen[1] = ctx->viewportY;
        }
        else if (code0 & CLIP_TOP)
        {
            if (m) v1->screen[0] += (ctx->viewportH - v1->screen[1]) / m;
            v1->screen[1] = ctx->viewportH;
        }
    }

    return accept;
}

static PFboolean Process_ClipLine3D(PFvertex* restrict v1, PFvertex* restrict v2)
{
    PFfloat t1 = 0, t2 = 1;

    PFMvec4 delta;
    pfmVec4Sub(delta, v2->homogeneous, v1->homogeneous);

    if (!Helper_ClipCoord3D(v1->homogeneous[3] - v1->homogeneous[0], -delta[3] + delta[0], &t1, &t2)) return PF_FALSE;
    if (!Helper_ClipCoord3D(v1->homogeneous[3] + v1->homogeneous[0], -delta[3] - delta[0], &t1, &t2)) return PF_FALSE;

    if (!Helper_ClipCoord3D(v1->homogeneous[3] - v1->homogeneous[1], -delta[3] + delta[1], &t1, &t2)) return PF_FALSE;
    if (!Helper_ClipCoord3D(v1->homogeneous[3] + v1->homogeneous[1], -delta[3] - delta[1], &t1, &t2)) return PF_FALSE;

    if (!Helper_ClipCoord3D(v1->homogeneous[3] - v1->homogeneous[2], -delta[3] + delta[2], &t1, &t2)) return PF_FALSE;
    if (!Helper_ClipCoord3D(v1->homogeneous[3] + v1->homogeneous[2], -delta[3] - delta[2], &t1, &t2)) return PF_FALSE;

    if (t2 < 1)
    {
        PFMvec4 d;
        pfmVec4Scale(d, delta, t2);
        pfmVec4Add(v2->homogeneous, v1->homogeneous, d);
    }

    if (t1 > 0)
    {
        PFMvec4 d;
        pfmVec4Scale(d, delta, t1);
        pfmVec4Add(v1->homogeneous, v1->homogeneous, d);
    }

    return PF_TRUE;
}

void Process_ProjectAndClipLine(PFvertex* restrict line, int_fast8_t* restrict vertexCounter, const PFMmat4 mvp)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        PFvertex *v = line + i;

        memcpy(v->homogeneous, v->position, sizeof(PFMvec4));
        pfmVec4Transform(v->homogeneous, v->homogeneous, mvp);
    }

    if (line[0].homogeneous[3] == 1.0f && line[1].homogeneous[3] == 1.0f)
    {
        pfInternal_HomogeneousToScreen(&line[0]);
        pfInternal_HomogeneousToScreen(&line[1]);

        if (!Process_ClipLine2D(&line[0], &line[1]))
        {
            *vertexCounter = 0;
            return;
        }
    }
    else
    {
        if (!Process_ClipLine3D(&line[0], &line[1]))
        {
            *vertexCounter = 0;
            return;
        }

        for (int_fast8_t i = 0; i < 2; i++)
        {
            // Division of XY coordinates by weight (perspective correct)
            PFfloat invW = 1.0f / line[i].homogeneous[3];
            line[i].homogeneous[0] *= invW;
            line[i].homogeneous[1] *= invW;
        }

        pfInternal_HomogeneousToScreen(&line[0]);
        pfInternal_HomogeneousToScreen(&line[1]);
    }
}


/* Internal line rasterizer function definitions */

void Rasterize_LineFlat(const PFvertex* v1, const PFvertex* v2)
{
    const PFctx *ctx = pfGetCurrentContext();

    const PFfloat dx = v2->screen[0] - v1->screen[0];
    const PFfloat dy = v2->screen[1] - v1->screen[1];

    if (dx == 0 && dy == 0)
    {
        pfSetFramebufferPixel(ctx->currentFramebuffer, v1->screen[0], v1->screen[1], v1->color);
        return;
    }

    const PFfloat adx = fabsf(dx);
    const PFfloat ady = fabsf(dy);

    if (adx > ady)
    {
        const PFfloat invAdx = 1.0f / adx;
        const PFfloat slope = dy / dx;

        PFint xMin, xMax;
        if (v1->screen[0] < v2->screen[0])
        {
            xMin = v1->screen[0], xMax = v2->screen[0];
        }
        else
        {
            xMin = v2->screen[0], xMax = v1->screen[0];
        }

        for (PFint x = xMin; x <= xMax; x++)
        {
            const PFfloat t = (x - xMin)*invAdx;
            const PFint y = v1->screen[1] + (x - v1->screen[0])*slope;
            pfSetFramebufferPixel(ctx->currentFramebuffer, x, y, Helper_LerpColor(v1->color, v2->color, t));
        }
    }
    else
    {
        const PFfloat invAdy = 1.0f / ady;
        const PFfloat slope = dx / dy;

        PFint yMin, yMax;
        if (v1->screen[1] < v2->screen[1])
        {
            yMin = v1->screen[1], yMax = v2->screen[1];
        }
        else
        {
            yMin = v2->screen[1], yMax = v1->screen[1];
        }

        for (PFint y = yMin; y <= yMax; y++)
        {
            const PFfloat t = (y - yMin)*invAdy;
            const PFint x = v1->screen[0] + (y - v1->screen[1])*slope;
            pfSetFramebufferPixel(ctx->currentFramebuffer, x, y, Helper_LerpColor(v1->color, v2->color, t));
        }
    }
}

void Rasterize_LineDepth(const PFvertex* v1, const PFvertex* v2)
{
    const PFctx *ctx = pfGetCurrentContext();

    const PFfloat dx = v2->screen[0] - v1->screen[0];
    const PFfloat dy = v2->screen[1] - v1->screen[1];

    if (dx == 0 && dy == 0)
    {
        pfSetFramebufferPixelDepth(ctx->currentFramebuffer, v1->screen[0], v1->screen[1], v1->homogeneous[2], v1->color);
        return;
    }

    const PFfloat adx = fabsf(dx);
    const PFfloat ady = fabsf(dy);

    if (adx > ady)
    {
        const PFfloat invAdx = 1.0f / adx;
        const PFfloat slope = dy / dx;

        PFint xMin, xMax;
        PFfloat zMin, zMax;
        if (v1->screen[0] < v2->screen[0])
        {
            xMin = v1->screen[0], xMax = v2->screen[0];
            zMin = v1->homogeneous[2], zMax = v2->homogeneous[2];
        }
        else
        {
            xMin = v2->screen[0], xMax = v1->screen[0];
            zMin = v2->homogeneous[2], zMax = v1->homogeneous[2];
        }

        for (PFint x = xMin; x <= xMax; x++)
        {
            const PFfloat t = (x - xMin)*invAdx;
            const PFfloat z = zMin + t*(zMax - zMin);
            const PFint y = v1->screen[1] + (x - v1->screen[0])*slope;
            pfSetFramebufferPixelDepth(ctx->currentFramebuffer, x, y, z, Helper_LerpColor(v1->color, v2->color, t));
        }
    }
    else
    {
        const PFfloat invAdy = 1.0f / ady;
        const PFfloat slope = dx / dy;

        PFint yMin, yMax;
        PFfloat zMin, zMax;
        if (v1->screen[1] < v2->screen[1])
        {
            yMin = v1->screen[1], yMax = v2->screen[1];
            zMin = v1->homogeneous[2], zMax = v2->homogeneous[2];
        }
        else
        {
            yMin = v2->screen[1], yMax = v1->screen[1];
            zMin = v2->homogeneous[2], zMax = v1->homogeneous[2];
        }

        for (PFint y = yMin; y <= yMax; y++)
        {
            const PFfloat t = (y - yMin)*invAdy;
            const PFfloat z = zMin + t*(zMax - zMin);
            const PFint x = v1->screen[0] + (y - v1->screen[1])*slope;
            pfSetFramebufferPixelDepth(ctx->currentFramebuffer, x, y, z, Helper_LerpColor(v1->color, v2->color, t));
        }
    }
}


/* Internal helper function definitions */

void Helper_SwapVertex(PFvertex* a, PFvertex* b)
{
    PFvertex tmp = *a;
    *a = *b; *b = tmp;
}

void Helper_SwapByte(PFubyte* a, PFubyte* b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

static PFubyte Helper_EncodeClip2D(const PFMvec2 screen, PFint xMin, PFint yMin, PFint xMax, PFint yMax)
{
    PFubyte code = CLIP_INSIDE;
    if (screen[0] < xMin) code |= CLIP_LEFT;
    if (screen[0] > yMin) code |= CLIP_RIGHT;
    if (screen[1] < xMax) code |= CLIP_BOTTOM;
    if (screen[1] > yMax) code |= CLIP_TOP;
    return code;
}

static PFboolean Helper_ClipCoord3D(PFfloat q, PFfloat p, PFfloat* t1, PFfloat* t2)
{
    if (fabsf(p) < PF_CLIP_EPSILON && q < 0)
    {
        return PF_FALSE;
    }

    const PFfloat r = q / p;

    if (p < 0)
    {
        if (r > *t2) return PF_FALSE;
        if (r > *t1) *t1 = r;
    }
    else
    {
        if (r < *t1) return PF_FALSE;
        if (r < *t2) *t2 = r;
    }

    return PF_TRUE;
}

PFcolor Helper_LerpColor(PFcolor a, PFcolor b, PFfloat t)
{
    return (PFcolor) {
        a.r + t*(b.r - a.r),
        a.g + t*(b.g - a.g),
        a.b + t*(b.b - a.b),
        a.a + t*(b.a - a.a)
    };
}

