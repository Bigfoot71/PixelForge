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

#include "internal/context/context.h"
#include <stdint.h>
#include <string.h>

void pfGetBooleanv(PFenum pname, PFboolean* params)
{
    switch (pname)
    {
        /* State context */

        case PF_TEXTURE_2D:
            *params = currentCtx->state & PF_TEXTURE_2D;
            break;

        case PF_FRAMEBUFFER:
            *params = currentCtx->state & PF_FRAMEBUFFER;
            break;

        case PF_BLEND:
            *params = currentCtx->state & PF_BLEND;
            break;

        case PF_DEPTH_TEST:
            *params = currentCtx->state & PF_DEPTH_TEST;
            break;

        case PF_CULL_FACE:
            *params = currentCtx->state & PF_CULL_FACE;
            break;

        case PF_NORMALIZE:
            *params = currentCtx->state & PF_NORMALIZE;
            break;

        case PF_LIGHTING:
            *params = currentCtx->state & PF_LIGHTING;
            break;

        case PF_COLOR_MATERIAL:
            *params = currentCtx->state & PF_COLOR_MATERIAL;
            break;

        case PF_VERTEX_ARRAY:
            *params = currentCtx->state & PF_VERTEX_ARRAY;
            break;

        case PF_NORMAL_ARRAY:
            *params = currentCtx->state & PF_NORMAL_ARRAY;
            break;

        case PF_COLOR_ARRAY:
            *params = currentCtx->state & PF_COLOR_ARRAY;
            break;

        case PF_TEXTURE_COORD_ARRAY:
            *params = currentCtx->state & PF_TEXTURE_COORD_ARRAY;
            break;

        /* Other values */

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
            currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfGetIntegerv(PFenum pname, PFint* params)
{
    switch (pname)
    {
        case PF_VIEWPORT:
            params[0] = currentCtx->vpPos[0];
            params[1] = currentCtx->vpPos[1];
            params[2] = currentCtx->vpDim[0] + 1;
            params[3] = currentCtx->vpDim[1] + 1;
            break;

        case PF_COLOR_CLEAR_VALUE:
            params[0] = currentCtx->clearColor.r;
            params[1] = currentCtx->clearColor.g;
            params[2] = currentCtx->clearColor.b;
            params[3] = currentCtx->clearColor.a;
            break;

        case PF_CULL_FACE_MODE:
            *params = currentCtx->cullFace;
            break;

        case PF_CURRENT_COLOR:
            params[0] = currentCtx->currentColor.r;
            params[1] = currentCtx->currentColor.g;
            params[2] = currentCtx->currentColor.b;
            params[3] = currentCtx->currentColor.a;
            break;

        //case PF_CURRENT_RASTER_COLOR:
        //  break;

        case PF_CURRENT_RASTER_POSITION:
            params[0] = currentCtx->rasterPos[0];
            params[1] = currentCtx->rasterPos[1];
            break;

        case PF_POLYGON_MODE:
            params[0] = currentCtx->polygonMode[0];
            params[1] = currentCtx->polygonMode[1];
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
            *params = currentCtx->currentMatrixMode;
            break;

        case PF_MAX_PROJECTION_STACK_DEPTH:
            *params = PF_MAX_PROJECTION_STACK_SIZE;
            break;

        case PF_MAX_MODELVIEW_STACK_DEPTH:
            *params = PF_MAX_MODELVIEW_STACK_SIZE;
            break;

        case PF_MAX_TEXTURE_STACK_DEPTH:
            *params = PF_MAX_TEXTURE_STACK_SIZE;
            break;

        case PF_SHADE_MODEL:
            *params = currentCtx->shadingMode;
            break;

        case PF_MAX_LIGHTS:
            *params = PF_MAX_LIGHT_STACK;
            break;

        case PF_VERTEX_ARRAY_SIZE:
            *params = currentCtx->vertexAttribs.positions.size;
            break;

        case PF_VERTEX_ARRAY_STRIDE:
            *params = currentCtx->vertexAttribs.positions.stride;
            break;

        case PF_VERTEX_ARRAY_TYPE:
            *params = currentCtx->vertexAttribs.positions.type;
            break;

        case PF_NORMAL_ARRAY_STRIDE:
            *params = currentCtx->vertexAttribs.normals.stride;
            break;

        case PF_NORMAL_ARRAY_TYPE:
            *params = currentCtx->vertexAttribs.normals.type;
            break;

        //case PF_TEXTURE_COORD_ARRAY_SIZE:
        //    *params = currentCtx->vertexAttribs.texcoords.size;
        //    break;

        case PF_TEXTURE_COORD_ARRAY_STRIDE:
            *params = currentCtx->vertexAttribs.texcoords.stride;
            break;

        case PF_TEXTURE_COORD_ARRAY_TYPE:
            *params = currentCtx->vertexAttribs.texcoords.type;
            break;

        case PF_COLOR_ARRAY_SIZE:
            *params = currentCtx->vertexAttribs.colors.size;
            break;

        case PF_COLOR_ARRAY_STRIDE:
            *params = currentCtx->vertexAttribs.colors.stride;
            break;

        case PF_COLOR_ARRAY_TYPE:
            *params = currentCtx->vertexAttribs.colors.type;
            break;

        default:
            currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfGetFloatv(PFenum pname, PFfloat* params)
{
    switch (pname)
    {
        case PF_COLOR_CLEAR_VALUE:
            params[0] = currentCtx->clearColor.r*INV_255;
            params[1] = currentCtx->clearColor.g*INV_255;
            params[2] = currentCtx->clearColor.b*INV_255;
            params[3] = currentCtx->clearColor.a*INV_255;
            break;

        case PF_DEPTH_CLEAR_VALUE:
            *params = currentCtx->clearDepth;
            break;

        case PF_CURRENT_COLOR:
            params[0] = currentCtx->currentColor.r*INV_255;
            params[1] = currentCtx->currentColor.g*INV_255;
            params[2] = currentCtx->currentColor.b*INV_255;
            params[3] = currentCtx->currentColor.a*INV_255;
            break;

        case PF_CURRENT_NORMAL:
            params[0] = currentCtx->currentNormal[0];
            params[1] = currentCtx->currentNormal[1];
            params[2] = currentCtx->currentNormal[2];
            break;

        case PF_CURRENT_TEXTURE_COORDS:
            params[0] = currentCtx->currentTexcoord[0];
            params[1] = currentCtx->currentTexcoord[1];
            break;

        //case PF_CURRENT_RASTER_COLOR:
        //  break;

        //case PF_CURRENT_RASTER_DISTANCE:
        //  break;

        case PF_CURRENT_RASTER_POSITION:
            params[0] = currentCtx->rasterPos[0];
            params[1] = currentCtx->rasterPos[1];
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
            *params = currentCtx->pointSize;
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
            *params = currentCtx->lineWidth;
            break;

        //case PF_LINE_WIDTH_GRANULARITY:
        //  break;

        //case PF_LINE_WIDTH_RANGE:
        //  break;

        case PF_PROJECTION_MATRIX:
            memcpy(params, currentCtx->matProjection, sizeof(PFMmat4));
            break;

        case PF_MODELVIEW_MATRIX:
        {
            PFMmat4 modelview;
            pfmMat4Mul(modelview, currentCtx->matModel, currentCtx->matView);
            memcpy(params, modelview, sizeof(PFMmat4));
        }
        break;

        case PF_TEXTURE_MATRIX:
            memcpy(params, currentCtx->matTexture, sizeof(PFMmat4));
            break;

        case PF_ZOOM_X:
            *params = currentCtx->pixelZoom[0];
            break;

        case PF_ZOOM_Y:
            *params = currentCtx->pixelZoom[1];
            break;

        default:
            currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfGetDoublev(PFenum pname, PFdouble* params)
{
    switch (pname)
    {
        case PF_COLOR_CLEAR_VALUE:
            params[0] = currentCtx->clearColor.r*INV_255;
            params[1] = currentCtx->clearColor.g*INV_255;
            params[2] = currentCtx->clearColor.b*INV_255;
            params[3] = currentCtx->clearColor.a*INV_255;
            break;

        case PF_DEPTH_CLEAR_VALUE:
            *params = currentCtx->clearDepth;
            break;

        case PF_CURRENT_COLOR:
            params[0] = currentCtx->currentColor.r*INV_255;
            params[1] = currentCtx->currentColor.g*INV_255;
            params[2] = currentCtx->currentColor.b*INV_255;
            params[3] = currentCtx->currentColor.a*INV_255;
            break;

        case PF_CURRENT_NORMAL:
            params[0] = currentCtx->currentNormal[0];
            params[1] = currentCtx->currentNormal[1];
            params[2] = currentCtx->currentNormal[2];
            break;

        case PF_CURRENT_TEXTURE_COORDS:
            params[0] = currentCtx->currentTexcoord[0];
            params[1] = currentCtx->currentTexcoord[1];
            break;

        //case PF_CURRENT_RASTER_COLOR:
        //  break;

        //case PF_CURRENT_RASTER_DISTANCE:
        //  break;

        case PF_CURRENT_RASTER_POSITION:
            params[0] = currentCtx->rasterPos[0];
            params[1] = currentCtx->rasterPos[1];
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
            *params = currentCtx->pointSize;
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
            *params = currentCtx->lineWidth;
            break;

        //case PF_LINE_WIDTH_GRANULARITY:
        //  break;

        //case PF_LINE_WIDTH_RANGE:
        //  break;

        case PF_PROJECTION_MATRIX:
        {
            for (int_fast8_t i = 0; i < 16; i++)
            {
                params[i] = currentCtx->matProjection[i];
            }
        }
        break;

        case PF_MODELVIEW_MATRIX:
        {
            PFMmat4 modelview;
            pfmMat4Mul(modelview, currentCtx->matModel, currentCtx->matView);

            for (int_fast8_t i = 0; i < 16; i++)
            {
                params[i] = modelview[i];
            }
        }
        break;

        case PF_TEXTURE_MATRIX:
        {
            for (int_fast8_t i = 0; i < 16; i++)
            {
                params[i] = currentCtx->matTexture[i];
            }
        }
        break;

        case PF_ZOOM_X:
            *params = currentCtx->pixelZoom[0];
            break;

        case PF_ZOOM_Y:
            *params = currentCtx->pixelZoom[1];
            break;

        default:
            currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfGetPointerv(PFenum pname, const void** params)
{
    switch (pname)
    {
        case PF_TEXTURE_2D:
            *params = currentCtx->currentTexture;
            break;

        case PF_FRAMEBUFFER:
            *params = currentCtx->bindedFramebuffer;
            break;

        case PF_BLEND_FUNC:
            // NOTE: Returns the address of the function pointer
            // internally and not the address of the function itself
            *params = (const void*)&(currentCtx->blendFunction);
            break;

        case PF_DEPTH_FUNC:
            // NOTE: Returns the address of the function pointer
            // internally and not the address of the function itself
            *params = (const void*)&(currentCtx->depthFunction);
            break;

        default:
            currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}
