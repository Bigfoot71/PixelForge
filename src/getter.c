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
#include <stdint.h>
#include <string.h>

void pfGetBooleanv(PFenum pname, PFboolean* params)
{
    PFctx *ctx = pfGetCurrentContext();

    switch (pname)
    {
        /* State context */

        case PF_TEXTURE:
            *params = ctx->state & PF_TEXTURE;
            break;

        case PF_DEPTH_TEST:
            *params = ctx->state & PF_DEPTH_TEST;
            break;

        case PF_CULL_FACE:
            *params = ctx->state & PF_CULL_FACE;
            break;

        case PF_NORMALIZE:
            *params = ctx->state & PF_NORMALIZE;
            break;

        case PF_LIGHTING:
            *params = ctx->state & PF_LIGHTING;
            break;

        case PF_COLOR_MATERIAL:
            *params = ctx->state & PF_COLOR_MATERIAL;
            break;

        case PF_VERTEX_ARRAY:
            *params = ctx->state & PF_VERTEX_ARRAY;
            break;

        case PF_NORMAL_ARRAY:
            *params = ctx->state & PF_NORMAL_ARRAY;
            break;

        case PF_COLOR_ARRAY:
            *params = ctx->state & PF_COLOR_ARRAY;
            break;

        case PF_TEXTURE_COORD_ARRAY:
            *params = ctx->state & PF_TEXTURE_COORD_ARRAY;
            break;

        //case PF_INDEX_ARRAY:
        //    *params = ctx->state & PF_INDEX_ARRAY;
        //    break;

        /* Other values */

        case PF_BLEND:
            *params = ctx->blendFunction != pfBlendDisabled;
            break;

        //case PF_CURRENT_RASTER_POSITION_VALID:
        //  break;

        //case PF_POLYGON_OFFSET_FILL:
        //  break;

        //case PF_POLYGON_OFFSET_LINE:
        //  break;

        //case PF_POLYGON_OFFSET_POINT:
        //  break;

        //case PF_POLYGON_SMOOTH:
        //  break;

        //case PF_POINT_SMOOTH:
        //  break;

        //case PF_LIGHT_MODEL_TWO_SIDE:
        //  break;

        //case PF_LIGHT_MODEL_LOCAL_VIEWER:
        //  break;

        //case PF_LINE_SMOOTH:
        //  break;

        /* Default */

        default:
            ctx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfGetIntegerv(PFenum pname, PFint* params)
{
    PFctx *ctx = pfGetCurrentContext();

    switch (pname)
    {
        case PF_VIEWPORT:
            params[0] = ctx->vpPos[0];
            params[1] = ctx->vpPos[1];
            params[2] = ctx->vpDim[0] + 1;
            params[3] = ctx->vpDim[1] + 1;
            break;

        case PF_COLOR_CLEAR_VALUE:
            params[0] = ctx->clearColor.r;
            params[1] = ctx->clearColor.g;
            params[2] = ctx->clearColor.b;
            params[3] = ctx->clearColor.a;
            break;

        //case PF_INDEX_CLEAR_VALUE:
        //  break;

        case PF_CULL_FACE_MODE:
            *params = ctx->cullFace;
            break;

        case PF_CURRENT_COLOR:
            params[0] = ctx->currentColor.r;
            params[1] = ctx->currentColor.g;
            params[2] = ctx->currentColor.b;
            params[3] = ctx->currentColor.a;
            break;

        //case PF_CURRENT_INDEX:
        //  break;

        //case PF_CURRENT_RASTER_COLOR:
        //  break;

        //case PF_CURRENT_RASTER_INDEX:
        //  break;

        case PF_CURRENT_RASTER_POSITION:
            params[0] = ctx->rasterPos[0];
            params[1] = ctx->rasterPos[1];
            break;

        case PF_POLYGON_MODE:
            params[0] = ctx->polygonMode[0];
            params[1] = ctx->polygonMode[1];
            break;

        //case PF_POLYGON_SMOOTH_HINT:
        //  break;

        //case PF_POINT_SMOOTH_HINT:
        //  break;

        //case PF_LINE_SMOOTH_HINT:
        //  break;

        //case PF_LIGHT_MODEL_AMBIENT:
        //  break;

        case PF_MATRIX_MODE:
            *params = ctx->currentMatrixMode;
            break;

        case PF_MAX_PROJECTION_STACK_DEPTH:
            *params = PF_MAX_PROJECTION_STACK_SIZE;
            break;

        case PF_MAX_MODELVIEW_STACK_DEPTH:
            *params = PF_MAX_MODELVIEW_STACK_SIZE;
            break;

        //case PF_MAX_TEXTURE_STACK_DEPTH:
        //    *params = PF_MAX_MATRIX_TEXTURE_STACK_SIZE;
        //    break;

        case PF_SHADE_MODEL:
            *params = ctx->shadingMode;
            break;

        case PF_MAX_LIGHTS:
            *params = PF_MAX_LIGHT_STACK;
            break;

        case PF_VERTEX_ARRAY_SIZE:
            *params = ctx->vertexAttribs.positions.size;
            break;

        case PF_VERTEX_ARRAY_STRIDE:
            *params = ctx->vertexAttribs.positions.stride;
            break;

        case PF_VERTEX_ARRAY_TYPE:
            *params = ctx->vertexAttribs.positions.type;
            break;

        case PF_NORMAL_ARRAY_STRIDE:
            *params = ctx->vertexAttribs.normals.stride;
            break;

        case PF_NORMAL_ARRAY_TYPE:
            *params = ctx->vertexAttribs.normals.type;
            break;

        //case PF_TEXTURE_COORD_ARRAY_SIZE:
        //    *params = ctx->vertexAttribs.texcoords.size;
        //    break;

        case PF_TEXTURE_COORD_ARRAY_STRIDE:
            *params = ctx->vertexAttribs.texcoords.stride;
            break;

        case PF_TEXTURE_COORD_ARRAY_TYPE:
            *params = ctx->vertexAttribs.texcoords.type;
            break;

        case PF_COLOR_ARRAY_SIZE:
            *params = ctx->vertexAttribs.colors.size;
            break;

        case PF_COLOR_ARRAY_STRIDE:
            *params = ctx->vertexAttribs.colors.stride;
            break;

        case PF_COLOR_ARRAY_TYPE:
            *params = ctx->vertexAttribs.colors.type;
            break;

        //case PF_INDEX_ARRAY_STRIDE:
        //  break;

        //case PF_INDEX_ARRAY_TYPE:
        //  break;

        //case PF_INDEX_MODE:
        //  break;

        //case PF_INDEX_OFFSET:
        //  break;

        //case PF_INDEX_SHIFT:
        //  break;

        default:
            ctx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfGetFloatv(PFenum pname, PFfloat* params)
{
    PFctx *ctx = pfGetCurrentContext();

    switch (pname)
    {
        case PF_COLOR_CLEAR_VALUE:
            params[0] = ctx->clearColor.r * INV_255;
            params[1] = ctx->clearColor.g * INV_255;
            params[2] = ctx->clearColor.b * INV_255;
            params[3] = ctx->clearColor.a * INV_255;
            break;

        case PF_DEPTH_CLEAR_VALUE:
            *params = ctx->clearDepth;
            break;

        case PF_CURRENT_COLOR:
            params[0] = ctx->currentColor.r * INV_255;
            params[1] = ctx->currentColor.g * INV_255;
            params[2] = ctx->currentColor.b * INV_255;
            params[3] = ctx->currentColor.a * INV_255;
            break;

        case PF_CURRENT_NORMAL:
            params[0] = ctx->currentNormal[0];
            params[1] = ctx->currentNormal[1];
            params[2] = ctx->currentNormal[2];
            break;

        case PF_CURRENT_TEXTURE_COORDS:
            params[0] = ctx->currentTexcoord[0];
            params[1] = ctx->currentTexcoord[1];
            break;

        //case PF_CURRENT_RASTER_COLOR:
        //  break;

        //case PF_CURRENT_RASTER_DISTANCE:
        //  break;

        //case PF_CURRENT_RASTER_INDEX:
        //  break;

        case PF_CURRENT_RASTER_POSITION:
            params[0] = ctx->rasterPos[0];
            params[1] = ctx->rasterPos[1];
            break;

        //case PF_CURRENT_RASTER_TEXTURE_COORDS:
        //  break;

        //case PF_POLYGON_OFFSET_FACTOR:
        //  break;

        //case PF_POLYGON_OFFSET_UNITS:
        //  break;

        //case PF_POLYGON_STIPPLE:
        //  break;

        case PF_POINT_SIZE:
            *params = ctx->pointSize;
            break;

        //case PF_POINT_SIZE_GRANULARITY:
        //  break;

        //case PF_POINT_SIZE_RANGE:
        //  break;

        //case PF_LIGHT_MODEL_AMBIENT:
        //  break;

        //case PF_LINE_STIPPLE:
        //  break;

        //case PF_LINE_STIPPLE_PATTERN:
        //  break;

        //case PF_LINE_STIPPLE_REPEAT:
        //  break;

        case PF_LINE_WIDTH:
            *params = ctx->lineWidth;
            break;

        //case PF_LINE_WIDTH_GRANULARITY:
        //  break;

        //case PF_LINE_WIDTH_RANGE:
        //  break;

        case PF_PROJECTION_MATRIX:
            memcpy(params, ctx->projection, sizeof(PFMmat4));
            break;

        case PF_MODELVIEW_MATRIX:
        {
            PFMmat4 modelview;
            pfmMat4Mul(modelview, ctx->model, ctx->view);
            memcpy(params, modelview, sizeof(PFMmat4));
        }
        break;

        case PF_ZOOM_X:
            *params = ctx->pixelZoom[0];
            break;

        case PF_ZOOM_Y:
            *params = ctx->pixelZoom[1];
            break;

        default:
            ctx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfGetDoublev(PFenum pname, PFdouble* params)
{
    PFctx *ctx = pfGetCurrentContext();

    switch (pname)
    {
        case PF_COLOR_CLEAR_VALUE:
            params[0] = ctx->clearColor.r * INV_255;
            params[1] = ctx->clearColor.g * INV_255;
            params[2] = ctx->clearColor.b * INV_255;
            params[3] = ctx->clearColor.a * INV_255;
            break;

        case PF_DEPTH_CLEAR_VALUE:
            *params = ctx->clearDepth;
            break;

        case PF_CURRENT_COLOR:
            params[0] = ctx->currentColor.r * INV_255;
            params[1] = ctx->currentColor.g * INV_255;
            params[2] = ctx->currentColor.b * INV_255;
            params[3] = ctx->currentColor.a * INV_255;
            break;

        case PF_CURRENT_NORMAL:
            params[0] = ctx->currentNormal[0];
            params[1] = ctx->currentNormal[1];
            params[2] = ctx->currentNormal[2];
            break;

        case PF_CURRENT_TEXTURE_COORDS:
            params[0] = ctx->currentTexcoord[0];
            params[1] = ctx->currentTexcoord[1];
            break;

        //case PF_CURRENT_RASTER_COLOR:
        //  break;

        //case PF_CURRENT_RASTER_DISTANCE:
        //  break;

        //case PF_CURRENT_RASTER_INDEX:
        //  break;

        case PF_CURRENT_RASTER_POSITION:
            params[0] = ctx->rasterPos[0];
            params[1] = ctx->rasterPos[1];
            break;

        //case PF_CURRENT_RASTER_TEXTURE_COORDS:
        //  break;

        //case PF_POLYGON_OFFSET_FACTOR:
        //  break;

        //case PF_POLYGON_OFFSET_UNITS:
        //  break;

        //case PF_POLYGON_STIPPLE:
        //  break;

        case PF_POINT_SIZE:
            *params = ctx->pointSize;
            break;

        //case PF_POINT_SIZE_GRANULARITY:
        //  break;

        //case PF_POINT_SIZE_RANGE:
        //  break;

        //case PF_LIGHT_MODEL_AMBIENT:
        //  break;

        //case PF_LINE_STIPPLE:
        //  break;

        //case PF_LINE_STIPPLE_PATTERN:
        //  break;

        //case PF_LINE_STIPPLE_REPEAT:
        //  break;

        case PF_LINE_WIDTH:
            *params = ctx->lineWidth;
            break;

        //case PF_LINE_WIDTH_GRANULARITY:
        //  break;

        //case PF_LINE_WIDTH_RANGE:
        //  break;

        case PF_PROJECTION_MATRIX:
        {
            for (int_fast8_t i = 0; i < 16; i++)
            {
                params[i] = ctx->projection[i];
            }
        }
        break;

        case PF_MODELVIEW_MATRIX:
        {
            PFMmat4 modelview;
            pfmMat4Mul(modelview, ctx->model, ctx->view);

            for (int_fast8_t i = 0; i < 16; i++)
            {
                params[i] = modelview[i];
            }
        }
        break;

        case PF_ZOOM_X:
            *params = ctx->pixelZoom[0];
            break;

        case PF_ZOOM_Y:
            *params = ctx->pixelZoom[1];
            break;

        default:
            ctx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfGetPointerv(PFenum pname, const void** params)
{
    PFctx *ctx = pfGetCurrentContext();

    switch (pname)
    {
        case PF_TEXTURE:
            *params = ctx->currentTexture;
            break;

        case PF_BLEND_FUNC:
            // NOTE: Returns the address of the function pointer
            // internally and not the address of the function itself
            *params = (const void*)&(ctx->blendFunction);
            break;

        case PF_DEPTH_FUNC:
            // NOTE: Returns the address of the function pointer
            // internally and not the address of the function itself
            *params = (const void*)&(ctx->depthFunction);
            break;

        default:
            ctx->errCode = PF_INVALID_ENUM;
            break;
    }
}
