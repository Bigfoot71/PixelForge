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

#include "./context.h"
#include "../primitives/primitives.h"

/* Internal vertex processing function definitions */

void pfInternal_HomogeneousToScreen(PFvertex* v)
{
    // NOTE: We add 0.5 to the screen coordinates to round them to the nearest integer
    // when they are converted to integer coordinates. This adjustment was added because
    // during clipping, some triangle vertices from the positive plane were found to be
    // offset by -1 pixel in X or Y (or both in some cases), which could even cause
    // triangle "tearing". While it's unclear if this is the best or correct solution,
    // it effectively resolves the issue without observed problems so far. There may be
    // an error in the polygon clipping functions. Nonetheless, this solution has been
    // functioning without issue up to this point.

    v->screen[0] = (currentCtx->vpPos[0] + (v->homogeneous[0] + 1.0f) * 0.5f * currentCtx->vpDim[0]) + 0.5f;
    v->screen[1] = (currentCtx->vpPos[1] + (1.0f - v->homogeneous[1]) * 0.5f * currentCtx->vpDim[1]) + 0.5f;
}

/* Internal processing and rasterization function definitions */

void pfInternal_ProcessAndRasterize(void)
{
    switch (currentCtx->currentDrawMode)
    {
        case PF_POINTS:
            pfInternal_ProcessRasterize_POINT();
            break;

        case PF_LINES:
            pfInternal_ProcessRasterize_LINE();
            break;

        case PF_TRIANGLES:
        {
            // Get faces to render
            // NOTE: Here we invert cullFace, because PF_FRONT = 0,
            //       !PF_FRONT = PF_BACK, and vice versa.
            PFface faceToRender = (currentCtx->state & PF_CULL_FACE)
                ? (!currentCtx->cullFace) : PF_FRONT_AND_BACK;

            if (faceToRender == PF_FRONT_AND_BACK)
            {
                for (PFint iFace = 0; iFace < 2; iFace++)
                {
                    switch (currentCtx->polygonMode[iFace])
                    {
                        case PF_POINT:
                            pfInternal_ProcessRasterize_POLY_POINTS(3);
                            break;

                        case PF_LINE:
                            pfInternal_ProcessRasterize_POLY_LINES(3);
                            break;

                        case PF_FILL:
                            pfInternal_ProcessRasterize_TRIANGLE(iFace);
                            break;
                    }
                }
            }
            else
            {
                switch (currentCtx->polygonMode[faceToRender])
                {
                    case PF_POINT:
                        pfInternal_ProcessRasterize_POLY_POINTS(3);
                        break;

                    case PF_LINE:
                        pfInternal_ProcessRasterize_POLY_LINES(3);
                        break;

                    case PF_FILL:
                        pfInternal_ProcessRasterize_TRIANGLE(faceToRender);
                        break;
                }
            }
        }
        break;

        case PF_TRIANGLE_FAN:
        {
            // Get faces to render
            // NOTE: Here we invert cullFace, because PF_FRONT = 0,
            //       !PF_FRONT = PF_BACK, and vice versa.
            PFface faceToRender = (currentCtx->state & PF_CULL_FACE)
                ? (!currentCtx->cullFace) : PF_FRONT_AND_BACK;

            if (faceToRender == PF_FRONT_AND_BACK)
            {
                pfInternal_ProcessRasterize_TRIANGLE_FAN(PF_FRONT, 2);
                pfInternal_ProcessRasterize_TRIANGLE_FAN(PF_BACK, 2);
            }
            else
            {
                pfInternal_ProcessRasterize_TRIANGLE_FAN(faceToRender, 2);
            }
        }
        break;

        case PF_TRIANGLE_STRIP:
        {
            // Get faces to render
            // NOTE: Here we invert cullFace, because PF_FRONT = 0,
            //       !PF_FRONT = PF_BACK, and vice versa.
            PFface faceToRender = (currentCtx->state & PF_CULL_FACE)
                ? (!currentCtx->cullFace) : PF_FRONT_AND_BACK;

            if (faceToRender == PF_FRONT_AND_BACK)
            {
                pfInternal_ProcessRasterize_TRIANGLE_STRIP(PF_FRONT, 2);
                pfInternal_ProcessRasterize_TRIANGLE_STRIP(PF_BACK, 2);
            }
            else
            {
                pfInternal_ProcessRasterize_TRIANGLE_STRIP(faceToRender, 2);
            }
        }
        break;

        case PF_QUADS:
        {
            // Get faces to render
            // NOTE: Here we invert cullFace, because PF_FRONT = 0,
            //       !PF_FRONT = PF_BACK, and vice versa.
            PFface faceToRender = (currentCtx->state & PF_CULL_FACE)
                ? (!currentCtx->cullFace) : PF_FRONT_AND_BACK;

            if (faceToRender == PF_FRONT_AND_BACK)
            {
                for (PFint iFace = 0; iFace < 2; iFace++)
                {
                    switch (currentCtx->polygonMode[iFace])
                    {
                        case PF_POINT:
                            pfInternal_ProcessRasterize_POLY_POINTS(4);
                            break;

                        case PF_LINE:
                            pfInternal_ProcessRasterize_POLY_LINES(4);
                            break;

                        case PF_FILL:
                            pfInternal_ProcessRasterize_TRIANGLE_FAN(iFace, 2);
                            break;
                    }
                }
            }
            else
            {
                switch (currentCtx->polygonMode[faceToRender])
                {
                    case PF_POINT:
                        pfInternal_ProcessRasterize_POLY_POINTS(4);
                        break;

                    case PF_LINE:
                        pfInternal_ProcessRasterize_POLY_LINES(4);
                        break;

                    case PF_FILL:
                        pfInternal_ProcessRasterize_TRIANGLE_FAN(faceToRender, 2);
                        break;
                }
            }
        }
        break;

        case PF_QUAD_FAN:
        {
            // Get faces to render
            // NOTE: Here we invert cullFace, because PF_FRONT = 0,
            //       !PF_FRONT = PF_BACK, and vice versa.
            PFface faceToRender = (currentCtx->state & PF_CULL_FACE)
                ? (!currentCtx->cullFace) : PF_FRONT_AND_BACK;

            if (faceToRender == PF_FRONT_AND_BACK)
            {
                pfInternal_ProcessRasterize_TRIANGLE_FAN(PF_FRONT, 4);
                pfInternal_ProcessRasterize_TRIANGLE_FAN(PF_BACK, 4);
            }
            else
            {
                pfInternal_ProcessRasterize_TRIANGLE_FAN(faceToRender, 4);
            }
        }
        break;

        case PF_QUAD_STRIP:
        {
            // Get faces to render
            // NOTE: Here we invert cullFace, because PF_FRONT = 0,
            //       !PF_FRONT = PF_BACK, and vice versa.
            PFface faceToRender = (currentCtx->state & PF_CULL_FACE)
                ? (!currentCtx->cullFace) : PF_FRONT_AND_BACK;

            if (faceToRender == PF_FRONT_AND_BACK)
            {
                pfInternal_ProcessRasterize_TRIANGLE_STRIP(PF_FRONT, 4);
                pfInternal_ProcessRasterize_TRIANGLE_STRIP(PF_BACK, 4);
            }
            else
            {
                pfInternal_ProcessRasterize_TRIANGLE_STRIP(faceToRender, 4);
            }
        }
        break;
    }
}
