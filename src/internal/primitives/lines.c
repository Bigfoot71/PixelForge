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
#include "../helper.h"
#include <stdlib.h>

/* Enums for internal use */

enum PFclipcode {
    CLIP_INSIDE = 0x00, // 0000
    CLIP_LEFT   = 0x01, // 0001
    CLIP_RIGHT  = 0x02, // 0010
    CLIP_BOTTOM = 0x04, // 0100
    CLIP_TOP    = 0x08, // 1000
};

/* Internal helper function declarations */

static PFubyte Helper_EncodeClip2D(const PFMvec2 screen, PFint xMin, PFint yMin, PFint xMax, PFint yMax);
static PFboolean Helper_ClipCoord3D(PFfloat q, PFfloat p, PFfloat* t1, PFfloat* t2);

/* Internal line processing functions declarations */

static PFboolean Process_ClipLine2D(PFvertex* restrict v1, PFvertex* restrict v2);
static PFboolean Process_ClipLine3D(PFvertex* restrict v1, PFvertex* restrict v2);
static void Process_ProjectAndClipLine(PFvertex* line, int_fast8_t* vertexCounter);

/* Internal line rasterizer function declarations */

static void Rasterize_Line_NODEPTH(const PFvertex* v1, const PFvertex* v2);
static void Rasterize_Line_DEPTH(const PFvertex* v1, const PFvertex* v2);

static void Rasterize_Line_THICK_NODEPTH(const PFvertex* v1, const PFvertex* v2);
static void Rasterize_Line_THICK_DEPTH(const PFvertex* v1, const PFvertex* v2);


/* Line Process And Rasterize Function */

void pfiProcessRasterize_LINE(void)
{
    // Process vertices
    int_fast8_t processedCounter = 2;

    PFvertex processed[2] = {
        G_currentCtx->vertexBuffer[0],
        G_currentCtx->vertexBuffer[1]
    };

    Process_ProjectAndClipLine(processed, &processedCounter);
    if (processedCounter != 2) return;

    // Rasterize line (review condition)
    if (G_currentCtx->lineWidth > 1.5f) {
        (G_currentCtx->state & PF_DEPTH_TEST ? Rasterize_Line_THICK_DEPTH
            : Rasterize_Line_THICK_NODEPTH)(&processed[0], &processed[1]);
    } else {
        (G_currentCtx->state & PF_DEPTH_TEST ? Rasterize_Line_DEPTH
            : Rasterize_Line_NODEPTH)(&processed[0], &processed[1]);
    }
}

void pfiProcessRasterize_POLY_LINES(int_fast8_t vertexCount)
{
    for (int_fast8_t i = 0; i < vertexCount; i++) {
        // Process vertices
        int_fast8_t processedCounter = 2;

        PFvertex processed[2] = {
            G_currentCtx->vertexBuffer[i],
            G_currentCtx->vertexBuffer[(i + 1) % vertexCount]
        };

        Process_ProjectAndClipLine(processed, &processedCounter);
        if (processedCounter != 2) return;

        // Rasterize line
        if (G_currentCtx->lineWidth > 1.5f) {
            (G_currentCtx->state & PF_DEPTH_TEST ? Rasterize_Line_THICK_DEPTH
                : Rasterize_Line_THICK_NODEPTH)(&processed[0], &processed[1]);
        } else {
            (G_currentCtx->state & PF_DEPTH_TEST ? Rasterize_Line_DEPTH
                : Rasterize_Line_NODEPTH)(&processed[0], &processed[1]);
        }
    }
}


/* Internal helper function definitions */

PFubyte Helper_EncodeClip2D(const PFMvec2 screen, PFint xMin, PFint yMin, PFint xMax, PFint yMax)
{
    PFubyte code = CLIP_INSIDE;
    if (screen[0] < xMin) code |= CLIP_LEFT;
    if (screen[0] > xMax) code |= CLIP_RIGHT;
    if (screen[1] < yMin) code |= CLIP_TOP;
    if (screen[1] > yMax) code |= CLIP_BOTTOM;
    return code;
}

PFboolean Helper_ClipCoord3D(PFfloat q, PFfloat p, PFfloat* t1, PFfloat* t2)
{
    if (fabsf(p) < PF_CLIP_EPSILON) {
        // Check if the line is entirely outside the window
        if (q < -PF_CLIP_EPSILON) return 0; // Completely outside
        return 1;                           // Completely inside or on the edges
    }

    const float r = q / p;

    if (p < 0) {
        if (r > *t2) return 0;
        if (r > *t1) *t1 = r;
    } else {
        if (r < *t1) return 0;
        if (r < *t2) *t2 = r;
    }

    return 1;
}

/* Internal helper function definitions */

PFboolean Process_ClipLine2D(PFvertex* restrict v1, PFvertex* restrict v2)
{
    PFint xMin = G_currentCtx->vpMin[0];
    PFint yMin = G_currentCtx->vpMin[1];
    PFint xMax = G_currentCtx->vpMax[0];
    PFint yMax = G_currentCtx->vpMax[1];

    PFboolean accept = PF_FALSE;
    PFubyte code0, code1;
    PFfloat m = 0;

    if (v1->screen[0] != v2->screen[0]) {
        m = (v2->screen[1] - v1->screen[1]) / (v2->screen[0] - v1->screen[0]);
    }

    for (;;) {
        code0 = Helper_EncodeClip2D(v1->screen, xMin, yMin, xMax, yMax);
        code1 = Helper_EncodeClip2D(v2->screen, xMin, yMin, xMax, yMax);

        // Accepted if both endpoints lie within rectangle
        if ((code0 | code1) == 0) {
            accept = PF_TRUE;
            break;
        }

        // Rejected if both endpoints are outside rectangle, in same region
        if (code0 & code1) break;

        if (code0 == CLIP_INSIDE) {
            pfiSwapByte(&code0, &code1);
            pfiSwapVertex(v1, v2);
        }

        if (code0 & CLIP_LEFT) {
            v1->screen[1] += (G_currentCtx->vpMin[0] - v1->screen[0])*m;
            v1->screen[0] = (PFfloat)G_currentCtx->vpMin[0];
        } else if (code0 & CLIP_RIGHT) {
            v1->screen[1] += (G_currentCtx->vpMax[0] - v1->screen[0])*m;
            v1->screen[0] = (PFfloat)G_currentCtx->vpMax[0];
        } else if (code0 & CLIP_BOTTOM) {
            if (m) v1->screen[0] += (G_currentCtx->vpMin[1] - v1->screen[1]) / m;
            v1->screen[1] = (PFfloat)G_currentCtx->vpMin[1];
        } else if (code0 & CLIP_TOP) {
            if (m) v1->screen[0] += (G_currentCtx->vpMax[1] - v1->screen[1]) / m;
            v1->screen[1] = (PFfloat)G_currentCtx->vpMax[1];
        }
    }

    return accept;
}

PFboolean Process_ClipLine3D(PFvertex* restrict v1, PFvertex* restrict v2)
{
    PFfloat t1 = 0, t2 = 1;

    PFMvec4 delta;
    pfmVec4SubR(delta, v2->homogeneous, v1->homogeneous);

    if (!Helper_ClipCoord3D(v1->homogeneous[3] - v1->homogeneous[0], -delta[3] + delta[0], &t1, &t2)) return PF_FALSE;
    if (!Helper_ClipCoord3D(v1->homogeneous[3] + v1->homogeneous[0], -delta[3] - delta[0], &t1, &t2)) return PF_FALSE;

    if (!Helper_ClipCoord3D(v1->homogeneous[3] - v1->homogeneous[1], -delta[3] + delta[1], &t1, &t2)) return PF_FALSE;
    if (!Helper_ClipCoord3D(v1->homogeneous[3] + v1->homogeneous[1], -delta[3] - delta[1], &t1, &t2)) return PF_FALSE;

    if (!Helper_ClipCoord3D(v1->homogeneous[3] - v1->homogeneous[2], -delta[3] + delta[2], &t1, &t2)) return PF_FALSE;
    if (!Helper_ClipCoord3D(v1->homogeneous[3] + v1->homogeneous[2], -delta[3] - delta[2], &t1, &t2)) return PF_FALSE;

    if (t2 < 1) {
        PFMvec4 d;
        pfmVec4ScaleR(d, delta, t2);
        pfmVec4AddR(v2->homogeneous, v1->homogeneous, d);
    }

    if (t1 > 0) {
        PFMvec4 d;
        pfmVec4ScaleR(d, delta, t1);
        pfmVec4Add(v1->homogeneous, v1->homogeneous, d);
    }

    return PF_TRUE;
}

void Process_ProjectAndClipLine(PFvertex* line, int_fast8_t* vertexCounter)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        PFvertex *v = line + i;

        memcpy(v->homogeneous, v->position, sizeof(PFMvec4));
        pfmVec4Transform(v->homogeneous, v->homogeneous, G_currentCtx->matMVP);
    }

    if (line[0].homogeneous[3] == 1.0f && line[1].homogeneous[3] == 1.0f) {
        pfiHomogeneousToScreen(&line[0]);
        pfiHomogeneousToScreen(&line[1]);

        if (!Process_ClipLine2D(&line[0], &line[1]))
        {
            *vertexCounter = 0;
            return;
        }
    } else {
        if (!Process_ClipLine3D(&line[0], &line[1])) {
            *vertexCounter = 0;
            return;
        }

        for (int_fast8_t i = 0; i < 2; i++) {
            // Division of XY coordinates by weight (perspective correct)
            PFfloat invW = 1.0f / line[i].homogeneous[3];
            line[i].homogeneous[0] *= invW;
            line[i].homogeneous[1] *= invW;
        }

        pfiHomogeneousToScreen(&line[0]);
        pfiHomogeneousToScreen(&line[1]);
    }
}

/* Internal line rasterizer function definitions */

void Rasterize_Line_NODEPTH(const PFvertex* v1, const PFvertex* v2)
{
    /* Get Some Values*/

    PFframebuffer *fbDst = G_currentCtx->currentFramebuffer;
    struct PFtex *texDst = G_currentCtx->currentFramebuffer->texture;

    PFpixelsetter setter = texDst->setter;
    PFpixelgetter getter = texDst->getter;

    PFblendfunc blendFunc = G_currentCtx->state & PF_BLEND ?
        G_currentCtx->blendFunction : NULL;

    PFfloat *zbDst = fbDst->zbuffer;
    void *bufDst = texDst->pixels;
    PFsizei wDst = texDst->w;

    PFint x1 = (PFint)v1->screen[0];
    PFint y1 = (PFint)v1->screen[1];
    PFint x2 = (PFint)v2->screen[0];
    PFint y2 = (PFint)v2->screen[1];
    PFfloat z1 = v1->homogeneous[2];
    PFfloat z2 = v2->homogeneous[2];
    PFcolor c1 = v1->color;
    PFcolor c2 = v2->color;

    /* Draw Line */

    PFint shortLen = y2 - y1;
    PFint longLen = x2 - x1;
    PFboolean yLonger = 0;

    if (abs(shortLen) > abs(longLen)) {
        PFint tmp = shortLen;
        shortLen = longLen;
        longLen = tmp;
        yLonger = 1;
    }

    PFfloat invEndVal = 1.0f/longLen;
    PFint endVal = longLen;
    PFint sgnInc = 1;

    if (longLen < 0) {
        longLen = -longLen;
        sgnInc = -1;
    }

    PFint decInc = (longLen == 0) ? 0
        : (shortLen << 16) / longLen;

    PFint j = 0;
    if (yLonger) {	
        for (PFint i = 0; i != endVal; i += sgnInc, j += decInc) {
            PFfloat t = (PFfloat)i*invEndVal;

            PFint x = x1 + (j >> 16), y = y1 + i;
            PFfloat z = z1 + t*(z2 - z1);

            PFsizei pOffset = (PFint)y*wDst + (PFint)x;

            PFcolor finalColor = pfiColorLerpSmooth(c1, c2, t);

            if (blendFunc) {
                finalColor = blendFunc(finalColor,
                    getter(bufDst, pOffset));
            }

            setter(bufDst, pOffset, finalColor);
            zbDst[pOffset] = z;
        }
    } else {
        for (PFint i = 0; i != endVal; i += sgnInc, j += decInc) {
            PFfloat t = (PFfloat)i*invEndVal;

            PFint x = x1 + i, y = y1 + (j >> 16);
            PFfloat z = z1 + t*(z2 - z1);

            PFsizei pOffset = (PFint)y*wDst + (PFint)x;

            PFcolor finalColor = pfiColorLerpSmooth(c1, c2, t);

            if (blendFunc) {
                finalColor = blendFunc(finalColor,
                    getter(bufDst, pOffset));
            }

            setter(bufDst, pOffset, finalColor);
            zbDst[pOffset] = z;
        }
    }
}

void Rasterize_Line_DEPTH(const PFvertex* v1, const PFvertex* v2)
{
    /* Get Some Values*/

    PFframebuffer *fbDst = G_currentCtx->currentFramebuffer;
    struct PFtex *texDst = G_currentCtx->currentFramebuffer->texture;

    PFpixelsetter setter = texDst->setter;
    PFpixelgetter getter = texDst->getter;

    PFblendfunc blendFunc = G_currentCtx->state & PF_BLEND ?
        G_currentCtx->blendFunction : NULL;

    PFfloat *zbDst = fbDst->zbuffer;
    void *bufDst = texDst->pixels;
    PFsizei wDst = texDst->w;

    PFint x1 = (PFint)v1->screen[0];
    PFint y1 = (PFint)v1->screen[1];
    PFint x2 = (PFint)v2->screen[0];
    PFint y2 = (PFint)v2->screen[1];
    PFfloat z1 = v1->homogeneous[2];
    PFfloat z2 = v2->homogeneous[2];
    PFcolor c1 = v1->color;
    PFcolor c2 = v2->color;

    /* Draw Line */

    PFint shortLen = y2 - y1;
    PFint longLen = x2 - x1;
    PFboolean yLonger = 0;

    if (abs(shortLen) > abs(longLen)) {
        PFint tmp = shortLen;
        shortLen = longLen;
        longLen = tmp;
        yLonger = 1;
    }

    PFfloat invEndVal = 1.0f/longLen;
    PFint endVal = longLen;
    PFint sgnInc = 1;

    if (longLen < 0) {
        longLen = -longLen;
        sgnInc = -1;
    }

    PFint decInc = (longLen == 0) ? 0
        : (shortLen << 16) / longLen;

    PFint j = 0;
    if (yLonger) {	
        for (PFint i = 0; i != endVal; i += sgnInc, j += decInc) {
            PFfloat t = (PFfloat)i*invEndVal;

            PFint x = x1 + (j >> 16), y = y1 + i;
            PFfloat z = z1 + t*(z2 - z1);

            PFsizei pOffset = (PFint)y*wDst + (PFint)x;
            if (G_currentCtx->depthFunction(z, zbDst[pOffset])) {
                PFcolor finalColor = pfiColorLerpSmooth(c1, c2, t);
                if (blendFunc) {
                    finalColor = blendFunc(finalColor,
                        getter(bufDst, pOffset));
                }
                setter(bufDst, pOffset, finalColor);
                zbDst[pOffset] = z;
            }
        }
    }
    else
    {
        for (PFint i = 0; i != endVal; i += sgnInc, j += decInc) {
            PFfloat t = (PFfloat)i*invEndVal;

            PFint x = x1 + i, y = y1 + (j >> 16);
            PFfloat z = z1 + t*(z2 - z1);

            PFsizei pOffset = (PFint)y*wDst + (PFint)x;
            if (G_currentCtx->depthFunction(z, zbDst[pOffset])) {
                PFcolor finalColor = pfiColorLerpSmooth(c1, c2, t);
                if (blendFunc) {
                    finalColor = blendFunc(finalColor,
                        getter(bufDst, pOffset));
                }
                setter(bufDst, pOffset, finalColor);
                zbDst[pOffset] = z;
            }
        }
    }
}

// TODO REVIEW: Can be highly optimized
void Rasterize_Line_THICK_NODEPTH(const PFvertex* v1, const PFvertex* v2)
{
    PFvertex tv1, tv2;

    PFint x1 = (PFint)v1->screen[0];
    PFint y1 = (PFint)v1->screen[1];
    PFint x2 = (PFint)v2->screen[0];
    PFint y2 = (PFint)v2->screen[1];

    PFint dx = x2 - x1, dy = y2 - y1;

    Rasterize_Line_DEPTH(v1, v2);

    if (dx != 0 && abs(dy / dx) < 1) {
        PFint wy = (PFint)((G_currentCtx->lineWidth - 1.0f) * rsqrtf(dx * dx + dy * dy) * (PFfloat)abs(dx));
        wy >>= 1; // Division by 2 via bit shift

        for (PFint i = 1; i <= wy; i++) {
            tv1 = *v1, tv2 = *v2;
            tv1.screen[1] -= i;
            tv2.screen[1] -= i;
            Rasterize_Line_NODEPTH(&tv1, &tv2);

            tv1 = *v1, tv2 = *v2;
            tv1.screen[1] += i;
            tv2.screen[1] += i;
            Rasterize_Line_NODEPTH(&tv1, &tv2);
        }
    } else if (dy != 0) {
        PFint wx = (PFint)((G_currentCtx->lineWidth - 1.0f) * rsqrtf(dx * dx + dy * dy) * (PFfloat)abs(dy));
        wx >>= 1; // Division by 2 via bit shift

        for (PFint i = 1; i <= wx; i++) {
            tv1 = *v1, tv2 = *v2;
            tv1.screen[0] -= i, tv2.screen[0] -= i;
            Rasterize_Line_NODEPTH(&tv1, &tv2);

            tv1 = *v1, tv2 = *v2;
            tv1.screen[0] += i, tv2.screen[0] += i;
            Rasterize_Line_NODEPTH(&tv1, &tv2);
        }
    }
}

// TODO REVIEW: Can be highly optimized
void Rasterize_Line_THICK_DEPTH(const PFvertex* v1, const PFvertex* v2)
{
    PFvertex tv1, tv2;

    PFint x1 = (PFint)v1->screen[0];
    PFint y1 = (PFint)v1->screen[1];
    PFint x2 = (PFint)v2->screen[0];
    PFint y2 = (PFint)v2->screen[1];

    PFint dx = x2 - x1, dy = y2 - y1;

    Rasterize_Line_DEPTH(v1, v2);

    if (dx != 0 && abs(dy / dx) < 1) {
        PFint wy = (PFint)((G_currentCtx->lineWidth - 1.0f) * rsqrtf(dx * dx + dy * dy) * (PFfloat)abs(dx));
        wy >>= 1; // Division by 2 via bit shift

        for (PFint i = 1; i <= wy; i++) {
            tv1 = *v1, tv2 = *v2;
            tv1.screen[1] -= i, tv2.screen[1] -= i;
            Rasterize_Line_DEPTH(&tv1, &tv2);

            tv1 = *v1, tv2 = *v2;
            tv1.screen[1] += i, tv2.screen[1] += i;
            Rasterize_Line_DEPTH(&tv1, &tv2);
        }
    } else if (dy != 0) {
        PFint wx = (PFint)((G_currentCtx->lineWidth - 1.0f) * rsqrtf(dx * dx + dy * dy) * (PFfloat)abs(dy));
        wx >>= 1; // Division by 2 via bit shift

        for (PFint i = 1; i <= wx; i++) {
            tv1 = *v1, tv2 = *v2;
            tv1.screen[0] -= i, tv2.screen[0] -= i;
            Rasterize_Line_DEPTH(&tv1, &tv2);

            tv1 = *v1, tv2 = *v2;
            tv1.screen[0] += i, tv2.screen[0] += i;
            Rasterize_Line_DEPTH(&tv1, &tv2);
        }
    }
}
