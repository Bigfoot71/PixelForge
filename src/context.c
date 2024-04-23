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
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <float.h>
#include <math.h>

// TODO: Review the API pfGet/pfSet functions

/* Current thread local-thread declaration */

#if defined(__GNUC__) || defined(__clang__)
#   if defined(PF_USE_OPENMP) && defined(__GNUC__)
        // TODO: Using a static global variable with __thread in GCC 11.4 seems to cause segmentation faults at runtime.
        //       I haven't been able to obtain more information through debugging and some research. To investigate further.
        static PFctx *currentCtx = NULL;
#   else
        static __thread PFctx *currentCtx = NULL;
#   endif
#elif defined(_MSC_VER)
    __declspec(thread) static PFctx *currentCtx = NULL;
#else
    static PFctx *currentCtx = NULL;
#endif

/* Internal types */

typedef void (*RasterizeTriangleFunc)(PFface, const PFvertex*, const PFvertex*, const PFvertex*);
typedef void (*RasterizeTriangleLightFunc)(PFface, const PFvertex*, const PFvertex*, const PFvertex*, const PFMvec3);

/* Including internal function prototypes */

extern void pfInternal_GetPixelGetterSetter(PFpixelgetter* getter, PFpixelsetter* setter, PFpixelformat format);
extern PFsizei pfInternal_GetPixelBytes(PFpixelformat format);

/* Internal processing and rasterization function declarations */

static void GetMVP(PFMmat4 outMVP, PFMmat4 outMatNormal, PFMmat4 outTransformedModelview);
static void ProcessRasterize(const PFMmat4 mvp, const PFMmat4 matNormal);

/* Context API functions */

PFctx* pfCreateContext(void* screenBuffer, PFsizei screenWidth, PFsizei screenHeight, PFpixelformat screenFormat)
{
    PFctx *ctx = (PFctx*)PF_MALLOC(sizeof(PFctx));
    if (!ctx) return NULL;

    ctx->screenBuffer = (PFframebuffer) { 0 };

    ctx->screenBuffer.texture = pfGenTexture(
        screenBuffer, screenWidth, screenHeight, screenFormat);

    const PFsizei bufferSize = screenWidth*screenHeight;
    ctx->screenBuffer.zbuffer = (PFfloat*)PF_MALLOC(bufferSize*sizeof(PFctx));

    if (!ctx->screenBuffer.zbuffer)
    {
        PF_FREE(ctx);
        return NULL;
    }

    for (PFsizei i = 0; i < bufferSize; i++)
    {
        ctx->screenBuffer.zbuffer[i] = FLT_MAX;
    }

    ctx->currentFramebuffer = &ctx->screenBuffer;

    ctx->viewportW = screenWidth - 1;
    ctx->viewportH = screenHeight - 1;
    ctx->viewportX = ctx->viewportY = 0;

    ctx->currentDrawMode = 0;
    ctx->blendFunction = pfBlendDisabled;
    ctx->clearColor = (PFcolor) { 0 };

    ctx->pointSize = 1.0f;

    ctx->polygonMode[0] = PF_FILL;
    ctx->polygonMode[1] = PF_FILL;

    memset(ctx->currentNormal, 0, sizeof(PFMvec3));
    memset(ctx->currentTexcoord, 0, sizeof(PFMvec2));
    ctx->currentColor = (PFcolor) { 255, 255, 255, 255 };

    ctx->vertexCount = 0;

    memset(ctx->rasterPos, 0, sizeof(PFMvec4));
    ctx->pixelZoom[0] = ctx->pixelZoom[1] = 1.0f;

    for (PFsizei i = 0; i < PF_MAX_LIGHTS; i++)
    {
        ctx->lights[i] = (PFlight) {
            .position = { 0 },
            .direction = { 0 },
            .ambient = (PFcolor) { 51, 51, 51, 255 },
            .diffuse = (PFcolor) { 255, 255, 255, 255 },
            .specular = (PFcolor) { 255, 255, 255, 255 },
            .active = PF_FALSE
        };
    }

    ctx->lastActiveLight = -1;

    ctx->frontMaterial = (PFmaterial) {
        .ambient = (PFcolor) { 255, 255, 255, 255 },
        .diffuse = (PFcolor) { 255, 255, 255, 255 },
        .specular = (PFcolor) { 255, 255, 255, 255 },
        .emission = (PFcolor) { 0, 0, 0, 255 },
#   ifdef PF_NO_BLINN_PHONG
        .shininess = 16.0f
#   else
        .shininess = 64.0f
#   endif
    };

    ctx->currentMatrixMode = PF_MODELVIEW;
    ctx->currentMatrix = &ctx->modelview;

    pfmMat4Ortho(ctx->projection, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    pfmMat4Identity(ctx->modelview);
    pfmMat4Identity(ctx->transform);

    ctx->transformRequired = PF_FALSE;
    ctx->stackCounter = 0;

    ctx->vertexAttribs = (PFvertexattribs) { 0 };
    ctx->currentTexture = NULL;

    ctx->vertexAttribState = 0x00;
    ctx->state = 0x00;

    ctx->state |= PF_CULL_FACE;
    ctx->cullFace = PF_BACK;

    ctx->errCode = PF_NO_ERROR;

    return ctx;
}

void pfDeleteContext(PFctx* ctx)
{
    if (ctx)
    {
        if (ctx->screenBuffer.zbuffer)
        {
            PF_FREE(ctx->screenBuffer.zbuffer);
            ctx->screenBuffer = (PFframebuffer) { 0 };
        }

        PF_FREE(ctx);
    }
}

PFctx* pfGetCurrentContext(void)
{
    return currentCtx;
}

void pfMakeCurrent(PFctx* ctx)
{
    currentCtx = ctx;
}

PFboolean pfIsEnabled(PFstate state)
{
    return currentCtx->state & state;
}

void pfEnable(PFstate state)
{
    currentCtx->state |= state;
}

void pfDisable(PFstate state)
{
    currentCtx->state &= ~state;
}


/* Error management API functions */

PFerrcode pfGetError(void)
{
    PFerrcode errCode = currentCtx->errCode;
    currentCtx->errCode = PF_NO_ERROR;
    return errCode;
}

PFerrcode* pfInternal_GetErrorPtr(void)
{
    return &currentCtx->errCode;
}


/* Matrix management API functions */

void pfMatrixMode(PFmatrixmode mode)
{
    switch (mode)
    {
        case PF_PROJECTION:
            currentCtx->currentMatrix = &currentCtx->projection;
            break;

        case PF_MODELVIEW:
            currentCtx->currentMatrix = &currentCtx->modelview;
            break;

        default:
            currentCtx->errCode = PF_INVALID_ENUM;
            return;
    }

    currentCtx->currentMatrixMode = mode;
}

void pfPushMatrix(void)
{
    if (currentCtx->stackCounter >= PF_MAX_MATRIX_STACK_SIZE)
    {
        currentCtx->errCode = PF_STACK_OVERFLOW;
    }

    if (currentCtx->currentMatrixMode == PF_MODELVIEW)
    {
        currentCtx->transformRequired = PF_TRUE;
        currentCtx->currentMatrix = &currentCtx->transform;
    }

    pfmMat4Copy(currentCtx->stack[currentCtx->stackCounter], *currentCtx->currentMatrix);
    currentCtx->stackCounter++;
}

void pfPopMatrix(void)
{
    if (currentCtx->stackCounter > 0)
    {
        pfmMat4Copy(*currentCtx->currentMatrix, currentCtx->stack[--currentCtx->stackCounter]);
    }

    if (currentCtx->stackCounter == 0 && currentCtx->currentMatrixMode == PF_MODELVIEW)
    {
        currentCtx->currentMatrix = &currentCtx->modelview;
        currentCtx->transformRequired = PF_FALSE;
    }
}

void pfLoadIdentity(void)
{
    pfmMat4Identity(*currentCtx->currentMatrix);
}

void pfTranslatef(PFfloat x, PFfloat y, PFfloat z)
{
    PFMmat4 translation;
    pfmMat4Translate(translation, x, y, z);

    // NOTE: We transpose matrix with multiplication order
    pfmMat4Mul(*currentCtx->currentMatrix, translation, *currentCtx->currentMatrix);
}

void pfRotatef(PFfloat angle, PFfloat x, PFfloat y, PFfloat z)
{
    PFMvec3 axis = { x, y, z }; // TODO: review

    PFMmat4 rotation;
    pfmMat4Rotate(rotation, axis, DEG2RAD(angle));

    // NOTE: We transpose matrix with multiplication order
    pfmMat4Mul(*currentCtx->currentMatrix, rotation, *currentCtx->currentMatrix);
}

void pfScalef(PFfloat x, PFfloat y, PFfloat z)
{
    PFMmat4 scale;
    pfmMat4Scale(scale, x, y, z);

    // NOTE: We transpose matrix with multiplication order
    pfmMat4Mul(*currentCtx->currentMatrix, scale, *currentCtx->currentMatrix);
}

void pfMultMatrixf(const PFfloat* mat)
{
    pfmMat4Mul(*currentCtx->currentMatrix, *currentCtx->currentMatrix, mat);
}

void pfFrustum(PFdouble left, PFdouble right, PFdouble bottom, PFdouble top, PFdouble znear, PFdouble zfar)
{
    PFMmat4 frustum;
    pfmMat4Frustum(frustum, left, right, bottom, top, znear, zfar);
    pfmMat4Mul(*currentCtx->currentMatrix, *currentCtx->currentMatrix, frustum);
}

void pfOrtho(PFdouble left, PFdouble right, PFdouble bottom, PFdouble top, PFdouble znear, PFdouble zfar)
{
    PFMmat4 ortho;
    pfmMat4Ortho(ortho, left, right, bottom, top, znear, zfar);
    pfmMat4Mul(*currentCtx->currentMatrix, *currentCtx->currentMatrix, ortho);
}


/* Render configuration API functions */

void pfGetViewport(PFint* x, PFint* y, PFsizei* width, PFsizei* height)
{
    *x = currentCtx->viewportX;
    *y = currentCtx->viewportY;
    *width = currentCtx->viewportW + 1;
    *height = currentCtx->viewportH + 1;
}

void pfViewport(PFint x, PFint y, PFsizei width, PFsizei height)
{
    currentCtx->viewportX = x;
    currentCtx->viewportY = y;
    currentCtx->viewportW = width - 1;
    currentCtx->viewportH = height - 1;
}

void pfSetDefaultPixelGetter(PFpixelgetter func)
{
    currentCtx->screenBuffer.texture.pixelGetter = func;
}

void pfSetDefaultPixelSetter(PFpixelsetter func)
{
    currentCtx->screenBuffer.texture.pixelSetter = func;
}

PFpolygonmode pfGetPolygonMode(PFface face)
{
    if (face < PF_BACK && face > PF_FRONT)
    {
        currentCtx->errCode = PF_INVALID_ENUM;
        return 0;
    }

    return currentCtx->polygonMode[face];
}

void pfPolygonMode(PFface face, PFpolygonmode mode)
{
    if (!(mode == PF_POINT || mode == PF_LINE || mode == PF_FILL))
    {
        currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    switch (face)
    {
        case PF_FRONT:
            currentCtx->polygonMode[0] = mode;
            break;

        case PF_BACK:
            currentCtx->polygonMode[1] = mode;
            break;

        case PF_FRONT_AND_BACK:
            currentCtx->polygonMode[0] = mode;
            currentCtx->polygonMode[1] = mode;
            break;

        default:
            currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

PFfloat pfGetPointSize(void)
{
    return currentCtx->pointSize;
}

void pfPointSize(PFfloat size)
{
    if (size <= 0.0f)
    {
        currentCtx->errCode = PF_INVALID_VALUE;
        return;
    }

    currentCtx->pointSize = size;
}

PFblendfunc pfGetBlendFunction(void)
{
    return currentCtx->blendFunction;
}

void pfSetBlendFunction(PFblendfunc func)
{
    currentCtx->blendFunction = func;
}

PFface pfGetCullFace(void)
{
    return currentCtx->cullFace;
}

void pfSetCullFace(PFface face)
{
    if (face < PF_FRONT || face > PF_BACK) return;
    currentCtx->cullFace = face;
}

PFframebuffer* pfGetActiveFramebuffer(void)
{
    return currentCtx->currentFramebuffer;
}

void pfEnableFramebuffer(PFframebuffer* framebuffer)
{
    if (!framebuffer) { pfDisableFramebuffer(); return; }
    currentCtx->currentFramebuffer = framebuffer;
}

void pfDisableFramebuffer(void)
{
    currentCtx->currentFramebuffer = &currentCtx->screenBuffer;
}

PFtexture* pfGetActiveTexture(void)
{
    return currentCtx->currentTexture;
}

void pfBindTexture(PFtexture* texture)
{
    currentCtx->currentTexture = texture;
}

void pfEnableLight(PFsizei light)
{
    if (light < PF_MAX_LIGHTS)
    {
        currentCtx->lights[light].active = PF_TRUE;
        currentCtx->lastActiveLight = -1;

        for (PFint i = PF_MAX_LIGHTS - 1; i > currentCtx->lastActiveLight; i--)
        {
            if (currentCtx->lights[i].active) currentCtx->lastActiveLight = i;
        }
    }
}

void pfDisableLight(PFsizei light)
{
    if (light < PF_MAX_LIGHTS)
    {
        currentCtx->lights[light].active = PF_FALSE;
        currentCtx->lastActiveLight = -1;

        for (PFint i = PF_MAX_LIGHTS - 1; i > currentCtx->lastActiveLight; i--)
        {
            if (currentCtx->lights[i].active) currentCtx->lastActiveLight = i;
        }
    }
}

void pfLightfv(PFsizei light, PFenum param, const void* value)
{
    if (light < PF_MAX_LIGHTS)
    {
        PFlight *l = &currentCtx->lights[light];

        switch (param)
        {
            case PF_POSITION:
                memcpy(l->position, value, sizeof(PFMvec3));
                break;

            case PF_SPOT_DIRECTION:
                memcpy(l->direction, value, sizeof(PFMvec3));
                break;

            case PF_AMBIENT:
                l->ambient = (PFcolor) {
                    (PFubyte)(((PFfloat*)value)[0]*255.0f),
                    (PFubyte)(((PFfloat*)value)[1]*255.0f),
                    (PFubyte)(((PFfloat*)value)[2]*255.0f),
                    255
                };
                break;

            case PF_DIFFUSE:
                l->diffuse = (PFcolor) {
                    (PFubyte)(((PFfloat*)value)[0]*255.0f),
                    (PFubyte)(((PFfloat*)value)[1]*255.0f),
                    (PFubyte)(((PFfloat*)value)[2]*255.0f),
                    255
                };
                break;

            case PF_SPECULAR:
                l->specular = (PFcolor) {
                    (PFubyte)(((PFfloat*)value)[0]*255.0f),
                    (PFubyte)(((PFfloat*)value)[1]*255.0f),
                    (PFubyte)(((PFfloat*)value)[2]*255.0f),
                    255
                };
                break;

            default:
                // NOTE: The definition 'PF_AMBIENT_AND_DIFFUSE' is reserved for 'pfMaterialfv'
                currentCtx->errCode = PF_INVALID_ENUM;
                break;
        }
    }
}

void pfMaterialf(PFface face, PFenum param, PFfloat value)
{
    PFmaterial *material0 = NULL;
    PFmaterial *material1 = NULL;

    switch (face)
    {
        case PF_FRONT:
            material0 = &currentCtx->frontMaterial;
            material1 = &currentCtx->frontMaterial;
            break;

        case PF_BACK:
            material0 = &currentCtx->backMaterial;
            material1 = &currentCtx->backMaterial;
            break;

        case PF_FRONT_AND_BACK:
            material0 = &currentCtx->frontMaterial;
            material1 = &currentCtx->backMaterial;
            break;

        default:
            currentCtx->errCode = PF_INVALID_ENUM;
            return;
    }

    switch (param)
    {
        case PF_AMBIENT:
            material0->ambient = material1->ambient = (PFcolor) {
                (PFubyte)(value*255.0f),
                (PFubyte)(value*255.0f),
                (PFubyte)(value*255.0f),
                255
            };
            break;

        case PF_DIFFUSE:
            material0->diffuse = material1->diffuse = (PFcolor) {
                (PFubyte)(value*255.0f),
                (PFubyte)(value*255.0f),
                (PFubyte)(value*255.0f),
                255
            };
            break;

        case PF_SPECULAR:
            material0->specular = material1->specular = (PFcolor) {
                (PFubyte)(value*255.0f),
                (PFubyte)(value*255.0f),
                (PFubyte)(value*255.0f),
                255
            };
            break;

        case PF_EMISSION:
            material0->emission = material1->emission = (PFcolor) {
                (PFubyte)(value*255.0f),
                (PFubyte)(value*255.0f),
                (PFubyte)(value*255.0f),
                255
            };
            break;

        case PF_SHININESS:
            material0->shininess = material1->shininess = value;
            break;

        case PF_AMBIENT_AND_DIFFUSE:
            material0->ambient = material1->ambient = (PFcolor) {
                (PFubyte)(value*255.0f),
                (PFubyte)(value*255.0f),
                (PFubyte)(value*255.0f),
                255
            };
            material0->diffuse = material1->diffuse = (PFcolor) {
                (PFubyte)(value*255.0f),
                (PFubyte)(value*255.0f),
                (PFubyte)(value*255.0f),
                255
            };
            break;

        default:
            currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfMaterialfv(PFface face, PFenum param, const void *value)
{
    PFmaterial *material0 = NULL;
    PFmaterial *material1 = NULL;

    switch (face)
    {
        case PF_FRONT:
            material0 = &currentCtx->frontMaterial;
            material1 = &currentCtx->frontMaterial;
            break;

        case PF_BACK:
            material0 = &currentCtx->backMaterial;
            material1 = &currentCtx->backMaterial;
            break;

        case PF_FRONT_AND_BACK:
            material0 = &currentCtx->frontMaterial;
            material1 = &currentCtx->backMaterial;
            break;

        default:
            currentCtx->errCode = PF_INVALID_ENUM;
            return;
    }

    switch (param)
    {
        case PF_AMBIENT:
            material0->ambient = material1->ambient = (PFcolor) {
                (PFubyte)(((PFfloat*)value)[0]*255.0f),
                (PFubyte)(((PFfloat*)value)[1]*255.0f),
                (PFubyte)(((PFfloat*)value)[2]*255.0f),
                255
            };
            break;

        case PF_DIFFUSE:
            material0->diffuse = material1->diffuse = (PFcolor) {
                (PFubyte)(((PFfloat*)value)[0]*255.0f),
                (PFubyte)(((PFfloat*)value)[1]*255.0f),
                (PFubyte)(((PFfloat*)value)[2]*255.0f),
                255
            };
            break;

        case PF_SPECULAR:
            material0->specular = material1->specular = (PFcolor) {
                (PFubyte)(((PFfloat*)value)[0]*255.0f),
                (PFubyte)(((PFfloat*)value)[1]*255.0f),
                (PFubyte)(((PFfloat*)value)[2]*255.0f),
                255
            };
            break;

        case PF_EMISSION:
            material0->emission = material1->emission = (PFcolor) {
                (PFubyte)(((PFfloat*)value)[0]*255.0f),
                (PFubyte)(((PFfloat*)value)[1]*255.0f),
                (PFubyte)(((PFfloat*)value)[2]*255.0f),
                255
            };
            break;

        case PF_SHININESS:
            material0->shininess = material1->shininess = *((PFfloat*)value);
            break;

        case PF_AMBIENT_AND_DIFFUSE:
            material0->ambient = material1->ambient = (PFcolor) {
                (PFubyte)(((PFfloat*)value)[0]*255.0f),
                (PFubyte)(((PFfloat*)value)[1]*255.0f),
                (PFubyte)(((PFfloat*)value)[2]*255.0f),
                255
            };
            material0->diffuse = material1->diffuse = (PFcolor) {
                (PFubyte)(((PFfloat*)value)[0]*255.0f),
                (PFubyte)(((PFfloat*)value)[1]*255.0f),
                (PFubyte)(((PFfloat*)value)[2]*255.0f),
                255
            };
            break;

        default:
            currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfClear(PFclearflag flag)
{
    if (!flag) return;

    PFframebuffer *framebuffer = currentCtx->currentFramebuffer;
    PFsizei size = framebuffer->texture.width*framebuffer->texture.height;

    if (flag & (PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT))
    {
        PFtexture *texture = &currentCtx->currentFramebuffer->texture;
        PFfloat *zbuffer = currentCtx->currentFramebuffer->zbuffer;
        PFcolor color = currentCtx->clearColor;

        for (PFsizei i = 0; i < size; i++)
        {
            texture->pixelSetter(texture->pixels, i, color);
            zbuffer[i] = FLT_MAX;
        }
    }
    else if (flag & PF_COLOR_BUFFER_BIT)
    {
        PFtexture *texture = &currentCtx->currentFramebuffer->texture;
        PFcolor color = currentCtx->clearColor;

        for (PFsizei i = 0; i < size; i++)
        {
            texture->pixelSetter(texture->pixels, i, color);
        }
    }
    else if (flag & PF_DEPTH_BUFFER_BIT)
    {
        PFfloat *zbuffer = currentCtx->currentFramebuffer->zbuffer;
        for (PFsizei i = 0; i < size; i++) zbuffer[i] = FLT_MAX;
    }
}

void pfClearColor(PFubyte r, PFubyte g, PFubyte b, PFubyte a)
{
    currentCtx->clearColor = (PFcolor) { r, g, b, a };
}

void pfEnableStatePointer(PFarraytype vertexAttribType, const void* buffer)
{
    if (!buffer)
    {
        pfDisableStatePointer(vertexAttribType);
        return;
    }

    switch (vertexAttribType)
    {
        case PF_VERTEX_ARRAY:
            currentCtx->vertexAttribs.positions = buffer;
            break;

        case PF_NORMAL_ARRAY:
            currentCtx->vertexAttribs.normals = buffer;
            break;

        case PF_COLOR_ARRAY:
            currentCtx->vertexAttribs.colors = buffer;
            break;

        case PF_TEXTURE_COORD_ARRAY:
            currentCtx->vertexAttribs.texcoords = buffer;
            break;

        default:
            currentCtx->errCode = PF_INVALID_ENUM;
            return;
    }

    currentCtx->vertexAttribState |= vertexAttribType;
}

void pfDisableStatePointer(PFarraytype vertexAttribType)
{
    currentCtx->vertexAttribState &= ~vertexAttribType;

    switch (vertexAttribType)
    {
        case PF_VERTEX_ARRAY:
            currentCtx->vertexAttribs.positions = NULL;
            break;

        case PF_NORMAL_ARRAY:
            currentCtx->vertexAttribs.normals = NULL;
            break;

        case PF_COLOR_ARRAY:
            currentCtx->vertexAttribs.colors = NULL;
            break;

        case PF_TEXTURE_COORD_ARRAY:
            currentCtx->vertexAttribs.texcoords = NULL;
            break;

        default:
            currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}


/* Vertex array drawing API functions */

void pfDrawVertexArrayElements(PFsizei offset, PFsizei count, const void *buffer)
{
    if (!(currentCtx->vertexAttribState & PF_VERTEX_ARRAY)) return;
    const PFMvec3 *positions = (const PFMvec3*)(currentCtx->vertexAttribs.positions) + offset;

    const PFMvec3 *normals = (currentCtx->vertexAttribState & PF_NORMAL_ARRAY)
        ? (const PFMvec3*)(currentCtx->vertexAttribs.normals) + offset : NULL;

    const PFcolor *colors = (currentCtx->vertexAttribState & PF_COLOR_ARRAY)
        ? (const PFcolor*)(currentCtx->vertexAttribs.colors) + offset : NULL;

    const PFMvec2 *texcoords = (currentCtx->vertexAttribState & PF_TEXTURE_COORD_ARRAY)
        ? (const PFMvec2*)(currentCtx->vertexAttribs.texcoords) + offset : NULL;

    PFMmat4 mvp, matNormal;
    GetMVP(mvp, matNormal, NULL);

    pfBegin(PF_TRIANGLES);

    for (PFsizei i = 0; i < count; i++)
    {
        const PFushort j = ((PFushort*)buffer)[i];

        // Get the pointer of the current vertex of the batch and pad it with zero

        PFvertex *vertex = currentCtx->vertexBuffer + (currentCtx->vertexCount++);

        // Fill the vertex with given vertices data

        memcpy(vertex->position, *(positions + j), sizeof(PFMvec3));
        vertex->position[3] = 1.0f;

        if (normals) memcpy(vertex->normal, *(normals + j), sizeof(PFMvec3));
        else memset(vertex->normal, 0, sizeof(PFMvec3));

        if (texcoords) memcpy(vertex->texcoord, *(texcoords + j), sizeof(PFMvec2));
        else memset(vertex->texcoord, 0, sizeof(PFMvec2));

        if (colors) memcpy(&vertex->color, colors + j, sizeof(PFcolor));
        else memcpy(&vertex->color, &currentCtx->currentColor, sizeof(PFcolor));

        // If the number of vertices has reached that necessary for, we process the shape

        if (currentCtx->vertexCount == currentCtx->currentDrawMode)
        {
            currentCtx->vertexCount = 0;
            ProcessRasterize(mvp, matNormal);
        }
    }

    pfEnd();
}

void pfDrawVertexArray(PFsizei offset, PFsizei count)
{
    if (!(currentCtx->vertexAttribState & PF_VERTEX_ARRAY)) return;
    const PFMvec3 *positions = (const PFMvec3*)(currentCtx->vertexAttribs.positions) + offset;

    const PFMvec3 *normals = (currentCtx->vertexAttribState & PF_NORMAL_ARRAY)
        ? (const PFMvec3*)(currentCtx->vertexAttribs.normals) + offset : NULL;

    const PFcolor *colors = (currentCtx->vertexAttribState & PF_COLOR_ARRAY)
        ? (const PFcolor*)(currentCtx->vertexAttribs.colors) + offset : NULL;

    const PFMvec2 *texcoords = (currentCtx->vertexAttribState & PF_TEXTURE_COORD_ARRAY)
        ? (const PFMvec2*)(currentCtx->vertexAttribs.texcoords) + offset : NULL;

    PFMmat4 mvp, matNormal;
    GetMVP(mvp, matNormal, NULL);

    pfBegin(PF_TRIANGLES);

    for (PFsizei i = 0; i < count; i++)
    {
        // Get the pointer of the current vertex of the batch and pad it with zero

        PFvertex *vertex = currentCtx->vertexBuffer + (currentCtx->vertexCount++);
        memset(vertex, 0, sizeof(PFvertex));

        // Fill the vertex with given vertices data

        memcpy(vertex->position, *(positions + i), sizeof(PFMvec3));
        vertex->position[3] = 1.0f;

        if (normals) memcpy(vertex->normal, *(normals + i), sizeof(PFMvec3));
        else memset(vertex->normal, 0, sizeof(PFMvec3));

        if (texcoords) memcpy(vertex->texcoord, *(texcoords + i), sizeof(PFMvec2));
        else memset(vertex->texcoord, 0, sizeof(PFMvec2));

        if (colors) memcpy(&vertex->color, colors + i, sizeof(PFcolor));
        else memcpy(&vertex->color, &currentCtx->currentColor, sizeof(PFcolor));

        // If the number of vertices has reached that necessary for, we process the shape

        if (currentCtx->vertexCount == currentCtx->currentDrawMode)
        {
            currentCtx->vertexCount = 0;
            ProcessRasterize(mvp, matNormal);
        }
    }

    pfEnd();
}


/* Primitives drawing API functions */

void pfBegin(PFdrawmode mode)
{
    if (mode >= PF_POINTS && mode <= PF_QUADS)
    {
        currentCtx->currentDrawMode = mode;
        currentCtx->vertexCount = 0;
    }
    else
    {
        currentCtx->errCode = PF_INVALID_ENUM;
    }
}

void pfEnd(void)
{
    currentCtx->vertexCount = 0;
}

void pfVertex2i(PFint x, PFint y)
{
    PFMvec4 v = { (PFfloat)x, (PFfloat)y, 0.0f, 1.0f };
    pfVertex4fv(v);
}

void pfVertex2f(PFfloat x, PFfloat y)
{
    PFMvec4 v = { x, y, 0.0f, 1.0f };
    pfVertex4fv(v);
}

void pfVertex2fv(const PFfloat* v)
{
    PFMvec4 v4 = { v[0], v[1], 0.0f, 1.0f };
    pfVertex4fv(v4);
}

void pfVertex3i(PFint x, PFint y, PFint z)
{
    PFMvec4 v = { (PFfloat)x, (PFfloat)y, (PFfloat)z, 1.0f };
    pfVertex4fv(v);
}

void pfVertex3f(PFfloat x, PFfloat y, PFfloat z)
{
    PFMvec4 v = { x, y, z, 1.0f };
    pfVertex4fv(v);
}

void pfVertex3fv(const PFfloat* v)
{
    PFMvec4 v4 = { v[0], v[1], v[2], 1.0f };
    pfVertex4fv(v4);
}

void pfVertex4i(PFint x, PFint y, PFint z, PFint w)
{
    PFMvec4 v = { (PFfloat)x, (PFfloat)y, (PFfloat)z, (PFfloat)w };
    pfVertex4fv(v);
}

void pfVertex4f(PFfloat x, PFfloat y, PFfloat z, PFfloat w)
{
    PFMvec4 v = { x, y, z, w };
    pfVertex4fv(v);
}

void pfVertex4fv(const PFfloat* v)
{
    // Get the pointer of the current vertex of the batch and pad it with zero

    PFvertex *vertex = currentCtx->vertexBuffer + (currentCtx->vertexCount++);

    // Fill the vertex with given vertices data

    memcpy(vertex->position, v, sizeof(PFMvec4));
    memcpy(vertex->normal, currentCtx->currentNormal, sizeof(PFMvec3));
    memcpy(vertex->texcoord, currentCtx->currentTexcoord, sizeof(PFMvec2));
    memcpy(&vertex->color, &currentCtx->currentColor, sizeof(PFcolor));

    // If the number of vertices has reached that necessary for, we process the shape

    if (currentCtx->vertexCount == currentCtx->currentDrawMode)
    {
        PFMmat4 mvp, matNormal;
        GetMVP(mvp, matNormal, NULL);

        currentCtx->vertexCount = 0;
        ProcessRasterize(mvp, matNormal);
    }
}

void pfColor3ub(PFubyte r, PFubyte g, PFubyte b)
{
    currentCtx->currentColor = (PFcolor) {
        r,
        g,
        b,
        255
    };
}

void pfColor3ubv(const PFubyte* v)
{
    currentCtx->currentColor = (PFcolor) {
        v[0],
        v[1],
        v[2],
        255
    };
}

void pfColor3us(PFushort r, PFushort g, PFushort b)
{
    currentCtx->currentColor = (PFcolor) {
        (PFubyte)(r >> 8),
        (PFubyte)(g >> 8),
        (PFubyte)(b >> 8),
        255
    };
}

void pfColor3usv(const PFushort* v)
{
    currentCtx->currentColor = (PFcolor) {
        (PFubyte)(v[0] >> 8),
        (PFubyte)(v[1] >> 8),
        (PFubyte)(v[2] >> 8),
        255
    };
}

void pfColor3ui(PFuint r, PFuint g, PFuint b)
{
    currentCtx->currentColor = (PFcolor) {
        (PFubyte)(r >> 24),
        (PFubyte)(g >> 24),
        (PFubyte)(b >> 24),
        255
    };
}

void pfColor3uiv(const PFuint* v)
{
    currentCtx->currentColor = (PFcolor) {
        (PFubyte)(v[0] >> 24),
        (PFubyte)(v[1] >> 24),
        (PFubyte)(v[2] >> 24),
        255
    };
}

void pfColor3f(PFfloat r, PFfloat g, PFfloat b)
{
    currentCtx->currentColor = (PFcolor) {
        (PFubyte)(r*255),
        (PFubyte)(g*255),
        (PFubyte)(b*255),
        255
    };
}

void pfColor3fv(const PFfloat* v)
{
    currentCtx->currentColor = (PFcolor) {
        (PFubyte)(v[0]*255),
        (PFubyte)(v[1]*255),
        (PFubyte)(v[2]*255),
        255
    };
}

void pfColor4ub(PFubyte r, PFubyte g, PFubyte b, PFubyte a)
{
    currentCtx->currentColor = (PFcolor) {
        r,
        g,
        b,
        a
    };
}

void pfColor4ubv(const PFubyte* v)
{
    currentCtx->currentColor = (PFcolor) {
        v[0],
        v[1],
        v[2],
        v[3]
    };
}

void pfColor4us(PFushort r, PFushort g, PFushort b, PFushort a)
{
    currentCtx->currentColor = (PFcolor) {
        (PFubyte)(r >> 8),
        (PFubyte)(g >> 8),
        (PFubyte)(b >> 8),
        (PFubyte)(a >> 8)
    };
}

void pfColor4usv(const PFushort* v)
{
    currentCtx->currentColor = (PFcolor) {
        (PFubyte)(v[0] >> 8),
        (PFubyte)(v[1] >> 8),
        (PFubyte)(v[2] >> 8),
        (PFubyte)(v[3] >> 8)
    };
}

void pfColor4ui(PFuint r, PFuint g, PFuint b, PFuint a)
{
    currentCtx->currentColor = (PFcolor) {
        (PFubyte)(r >> 24),
        (PFubyte)(g >> 24),
        (PFubyte)(b >> 24),
        (PFubyte)(a >> 24)
    };
}

void pfColor4uiv(const PFuint* v)
{
    currentCtx->currentColor = (PFcolor) {
        (PFubyte)(v[0] >> 24),
        (PFubyte)(v[1] >> 24),
        (PFubyte)(v[2] >> 24),
        (PFubyte)(v[3] >> 24)
    };
}

void pfColor4f(PFfloat r, PFfloat g, PFfloat b, PFfloat a)
{
    currentCtx->currentColor = (PFcolor) {
        (PFubyte)(r*255),
        (PFubyte)(g*255),
        (PFubyte)(b*255),
        (PFubyte)(a*255)
    };
}

void pfColor4fv(const PFfloat* v)
{
    currentCtx->currentColor = (PFcolor) {
        (PFubyte)(v[0]*255),
        (PFubyte)(v[1]*255),
        (PFubyte)(v[2]*255),
        (PFubyte)(v[3]*255)
    };
}

void pfTexCoord2f(PFfloat u, PFfloat v)
{
    currentCtx->currentTexcoord[0] = u;
    currentCtx->currentTexcoord[1] = v;
}

void pfTexCoordfv(const PFfloat* v)
{
    memcpy(currentCtx->currentTexcoord, v, sizeof(PFMvec2));
}

void pfNormal3f(PFfloat x, PFfloat y, PFfloat z)
{
    currentCtx->currentNormal[0] = x;
    currentCtx->currentNormal[1] = y;
    currentCtx->currentNormal[2] = z;
}

void pfNormal3fv(const PFfloat* v)
{
    memcpy(currentCtx->currentNormal, v, sizeof(PFMvec3));
}


/* Supplementary primitive drawing API functions */

void pfRects(PFshort x1, PFshort y1, PFshort x2, PFshort y2)
{
    pfRectf(x1, y1, x2, y2);
}

void pfRectsv(const PFshort* v1, const PFshort* v2)
{
    pfRectf(v1[0], v1[1], v2[0], v2[1]);
}

void pfRecti(PFint x1, PFint y1, PFint x2, PFint y2)
{
    pfRectf(x1, y1, x2, y2);
}

void pfRectiv(const PFint* v1, const PFint* v2)
{
    pfRectf(v1[0], v1[1], v2[0], v2[1]);
}

void pfRectf(PFfloat x1, PFfloat y1, PFfloat x2, PFfloat y2)
{
    // Get the transformation matrix from model to view (ModelView) and projection
    PFMmat4 mvp;
    GetMVP(mvp, NULL, NULL);

    // Project corner points
    PFMvec4 v1 = { x1, y1, 0.0f, 1.0f };
    PFMvec4 v2 = { x2, y2, 0.0f, 1.0f };
    pfmVec4Transform(v1, v1, mvp);
    pfmVec4Transform(v2, v2, mvp);

    // Calculate screen coordinates from projected coordinates
    PFint iX1 = currentCtx->viewportX + (v1[0] + 1.0f) * 0.5f * currentCtx->viewportW;
    PFint iY1 = currentCtx->viewportY + (1.0f - v1[1]) * 0.5f * currentCtx->viewportH;
    PFint iX2 = currentCtx->viewportX + (v2[0] + 1.0f) * 0.5f * currentCtx->viewportW;
    PFint iY2 = currentCtx->viewportY + (1.0f - v2[1]) * 0.5f * currentCtx->viewportH;

    // Ensure iX1 <= iX2 and iY1 <= iY2
    if (iX2 < iX1) iX1 ^= iX2, iX2 ^= iX1, iX1 ^= iX2;
    if (iY2 < iY1) iY1 ^= iY2, iY2 ^= iY1, iY1 ^= iY2;

    // Clamp screen coordinates to viewport boundaries
    iX1 = CLAMP(iX1, (PFint)currentCtx->viewportX, (PFint)currentCtx->viewportW);
    iY1 = CLAMP(iY1, (PFint)currentCtx->viewportY, (PFint)currentCtx->viewportH);
    iX2 = CLAMP(iX2, (PFint)currentCtx->viewportX, (PFint)currentCtx->viewportW);
    iY2 = CLAMP(iY2, (PFint)currentCtx->viewportY, (PFint)currentCtx->viewportH);

    // Retrieve framebuffer information
    PFint wDst = currentCtx->currentFramebuffer->texture.width;
    void *bufDst = currentCtx->currentFramebuffer->texture.pixels;
    PFpixelsetter pixelSetter = currentCtx->currentFramebuffer->texture.pixelSetter;

    // Retrieve current drawing color
    PFcolor color = currentCtx->currentColor;

    // Draw rectangle
    for (PFint y = iY1; y <= iY2; y++)
    {
        PFint yOffset = y * wDst;

        for (PFint x = iX1; x <= iX2; x++)
        {
            pixelSetter(bufDst, yOffset + x, color);
        }
    }
}

void pfRectfv(const PFfloat* v1, const PFfloat* v2)
{
    pfRectf(v1[0], v1[1], v2[0], v2[1]);
}


/* Drawing pixels API functions */

void pfDrawPixels(PFsizei width, PFsizei height, PFpixelformat format, const void* pixels)
{
    // Retrieve the appropriate pixel getter function for the given buffer format
    PFpixelgetter getPixelSrc = NULL;
    pfInternal_GetPixelGetterSetter(&getPixelSrc, NULL, format);

    // Checked if we were able to get the pixel getter
    if (!getPixelSrc)
    {
        currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    // Get the transformation matrix from model to view (ModelView) and projection
    PFMmat4 mvp;
    GetMVP(mvp, NULL, NULL);

    // Project raster point
    PFMvec4 rasterPos = { currentCtx->rasterPos[0], currentCtx->rasterPos[1], currentCtx->rasterPos[2], 1.0f };
    pfmVec4Transform(rasterPos, rasterPos, mvp);

    // Calculate screen coordinates from projected coordinates
    PFint xScreen = currentCtx->viewportX + (rasterPos[0] + 1.0f) * 0.5f * currentCtx->viewportW;
    PFint yScreen = currentCtx->viewportY + (1.0f - rasterPos[1]) * 0.5f * currentCtx->viewportH;

    // Draw pixels on current framebuffer
    PFtexture *texDst = &currentCtx->currentFramebuffer->texture;
    PFfloat *zBuffer = currentCtx->currentFramebuffer->zbuffer;

    PFsizei wDst = texDst->width;
    PFsizei hDst = texDst->height;

    PFfloat xPixelZoom = currentCtx->pixelZoom[0];
    PFfloat yPixelZoom = currentCtx->pixelZoom[1];

    PFfloat xSrcInc = (xPixelZoom < 1.0f) ? 1.0f / xPixelZoom : 1.0f;
    PFfloat ySrcInc = (yPixelZoom < 1.0f) ? 1.0f / yPixelZoom : 1.0f;

    if (currentCtx->state & PF_DEPTH_TEST)
    {
        for (PFfloat ySrc = 0; ySrc < height; ySrc += ySrcInc)
        {
            PFsizei ySrcOffset = (PFsizei)ySrc * width;
            PFfloat yDstMin = yScreen + ySrc * yPixelZoom;
            PFfloat yDstMax = yDstMin + yPixelZoom;

            for (PFfloat xSrc = 0; xSrc < width; xSrc += xSrcInc)
            {
                PFsizei yDstOffset = (PFsizei)(yDstMin + 0.5f) * wDst;
                PFfloat xDstMin = xScreen + xSrc * xPixelZoom;
                PFfloat xDstMax = xDstMin + xPixelZoom;

                for (PFfloat yDst = yDstMin; yDst < yDstMax; yDst++)
                {
                    for (PFfloat xDst = xDstMin; xDst < xDstMax; xDst++)
                    {
                        if (xDst >= 0 && xDst < wDst && yDst >= 0 && yDst < hDst)
                        {
                            PFsizei xyDstOffset = yDstOffset + (PFsizei)(xDst + 0.5f);

                            if (rasterPos[2] < zBuffer[xyDstOffset])
                            {
                                PFsizei xySrcOffset = ySrcOffset + (PFsizei)xSrc;

                                zBuffer[xyDstOffset] = rasterPos[2];
                                PFcolor colSrc = getPixelSrc(pixels, xySrcOffset);
                                PFcolor colDst = texDst->pixelGetter(texDst->pixels, xyDstOffset);
                                texDst->pixelSetter(texDst->pixels, xyDstOffset, currentCtx->blendFunction(colSrc, colDst));
                            }
                        }
                    }

                    yDstOffset++;
                }
            }
        }
    }
    else
    {
        for (PFfloat ySrc = 0; ySrc < height; ySrc += ySrcInc)
        {
            PFsizei ySrcOffset = (PFsizei)ySrc * width;
            PFfloat yDstMin = yScreen + ySrc * yPixelZoom;
            PFfloat yDstMax = yDstMin + yPixelZoom;

            for (PFfloat xSrc = 0; xSrc < width; xSrc += xSrcInc)
            {
                PFsizei yDstOffset = (PFsizei)(yDstMin + 0.5f) * wDst;
                PFfloat xDstMin = xScreen + xSrc * xPixelZoom;
                PFfloat xDstMax = xDstMin + xPixelZoom;

                for (PFfloat yDst = yDstMin; yDst < yDstMax; yDst++)
                {
                    for (PFfloat xDst = xDstMin; xDst < xDstMax; xDst++)
                    {
                        if (xDst >= 0 && xDst < wDst && yDst >= 0 && yDst < hDst)
                        {
                            PFsizei xySrcOffset = ySrcOffset + (PFsizei)xSrc;
                            PFsizei xyDstOffset = yDstOffset + (PFsizei)(xDst + 0.5f);

                            zBuffer[xyDstOffset] = rasterPos[2];
                            PFcolor colSrc = getPixelSrc(pixels, xySrcOffset);
                            PFcolor colDst = texDst->pixelGetter(texDst->pixels, xyDstOffset);
                            texDst->pixelSetter(texDst->pixels, xyDstOffset, currentCtx->blendFunction(colSrc, colDst));
                        }
                    }

                    yDstOffset++;
                }
            }
        }
    }
}

void pfPixelZoom(PFfloat xFactor, PFfloat yFactor)
{
    currentCtx->pixelZoom[0] = xFactor;
    currentCtx->pixelZoom[1] = yFactor;
}

void pfRasterPos2i(PFint x, PFint y)
{
    currentCtx->rasterPos[0] = (PFfloat)x;
    currentCtx->rasterPos[1] = (PFfloat)y;
    currentCtx->rasterPos[2] = 0.0f;
    currentCtx->rasterPos[3] = 1.0f;
}

void pfRasterPos2f(PFfloat x, PFfloat y)
{
    currentCtx->rasterPos[0] = x;
    currentCtx->rasterPos[1] = y;
    currentCtx->rasterPos[2] = 0.0f;
    currentCtx->rasterPos[3] = 1.0f;
}

void pfRasterPos2fv(const PFfloat* v)
{
    memcpy(currentCtx->rasterPos, v, sizeof(PFMvec2));
    currentCtx->rasterPos[2] = 0.0f;
    currentCtx->rasterPos[3] = 1.0f;
}

void pfRasterPos3i(PFint x, PFint y, PFint z)
{
    currentCtx->rasterPos[0] = (PFfloat)x;
    currentCtx->rasterPos[1] = (PFfloat)y;
    currentCtx->rasterPos[2] = (PFfloat)z;
    currentCtx->rasterPos[3] = 1.0f;
}

void pfRasterPos3f(PFfloat x, PFfloat y, PFfloat z)
{
    currentCtx->rasterPos[0] = x;
    currentCtx->rasterPos[1] = y;
    currentCtx->rasterPos[2] = z;
    currentCtx->rasterPos[3] = 1.0f;
}

void pfRasterPos3fv(const PFfloat* v)
{
    memcpy(currentCtx->rasterPos, v, sizeof(PFMvec3));
    currentCtx->rasterPos[3] = 1.0f;
}

void pfRasterPos4i(PFint x, PFint y, PFint z, PFint w)
{
    currentCtx->rasterPos[0] = (PFfloat)x;
    currentCtx->rasterPos[1] = (PFfloat)y;
    currentCtx->rasterPos[2] = (PFfloat)z;
    currentCtx->rasterPos[3] = (PFfloat)w;
}

void pfRasterPos4f(PFfloat x, PFfloat y, PFfloat z, PFfloat w)
{
    currentCtx->rasterPos[0] = x;
    currentCtx->rasterPos[1] = y;
    currentCtx->rasterPos[2] = z;
    currentCtx->rasterPos[3] = w;
}

void pfRasterPos4fv(const PFfloat* v)
{
    memcpy(currentCtx->rasterPos, v, sizeof(PFMvec4));
}

/* Misc API functions */

void pfReadPixels(PFint x, PFint y, PFsizei width, PFsizei height, PFpixelformat format, void* pixels)
{
    // Get the pixel setter function for the given buffer format
    PFpixelsetter pixelSetter = NULL;
    pfInternal_GetPixelGetterSetter(NULL, &pixelSetter, format);

    // Check if pixel setter function was successfully obtained
    if (!pixelSetter)
    {
        currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    // Get the number of bytes per pixel for the destination and source formats
    PFsizei dstPixelBytes = pfInternal_GetPixelBytes(format);
    PFsizei srcPixelBytes = pfInternal_GetPixelBytes(currentCtx->currentFramebuffer->texture.format);

    // Check if the source pixel format is unknown
    if (srcPixelBytes == 0) // TODO REVIEW: Possible issue if the current framebuffer has a PF_PIXELFORMAT_UNKNOWN format
    {
        currentCtx->errCode = PF_INVALID_OPERATION;
        return;
    }

    // Clamp the coordinates and dimensions to fit within the framebuffer boundaries
    const PFframebuffer *curFB = currentCtx->currentFramebuffer;

    x = CLAMP(x, 0, (PFint)curFB->texture.width - 1);
    y = CLAMP(y, 0, (PFint)curFB->texture.height - 1);

    width = MIN(width, curFB->texture.width - 1);
    height = MIN(height, curFB->texture.height - 1);

    // Move to the beginning of the specified region in the framebuffer
    PFint wSrc = curFB->texture.width;
    const char* src = (char*)curFB->texture.pixels + (y * wSrc + x) * srcPixelBytes;

    // Copy pixels from the specified region of the framebuffer to the location specified by 'pixels'
    for (PFsizei i = 0; i < height; i++)
    {
        memcpy((char*)pixels + i * width * dstPixelBytes, src, width * srcPixelBytes);
        src += wSrc * srcPixelBytes;
    }
}


/* Point processing and rasterization functions (points.c) */

extern PFboolean Process_ProjectPoint(PFvertex* restrict v, const PFMmat4 mvp);

extern void Rasterize_PointFlat(const PFvertex* point);
extern void Rasterize_PointDepth(const PFvertex* point);

/* Line processing and rasterization functions (lines.c) */

extern void Process_ProjectAndClipLine(PFvertex* restrict line, int_fast8_t* restrict vertexCounter, const PFMmat4 mvp);

extern void Rasterize_LineFlat(const PFvertex* v1, const PFvertex* v2);
extern void Rasterize_LineDepth(const PFvertex* v1, const PFvertex* v2);

/* Triangle processing and rasterization functions (triangles.c) */

extern PFboolean Process_ProjectAndClipTriangle(PFvertex* restrict polygon, int_fast8_t* restrict vertexCounter, const PFMmat4 mvp);

extern void Rasterize_TriangleColorFlat2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
extern void Rasterize_TriangleColorDepth2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
extern void Rasterize_TriangleTextureFlat2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
extern void Rasterize_TriangleTextureDepth2D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);

extern void Rasterize_TriangleColorFlat3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
extern void Rasterize_TriangleColorDepth3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
extern void Rasterize_TriangleTextureFlat3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
extern void Rasterize_TriangleTextureDepth3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);

extern void Rasterize_TriangleColorFlatLight3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos);
extern void Rasterize_TriangleColorDepthLight3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos);
extern void Rasterize_TriangleTextureFlatLight3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos);
extern void Rasterize_TriangleTextureDepthLight3D(PFface faceToRender, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos);

/* Internal helper function definitions */

void GetMVP(PFMmat4 outMVP, PFMmat4 outMatNormal, PFMmat4 outTransformedModelview)
{
    PFMmat4 modelview;
    pfmMat4Copy(modelview, currentCtx->modelview);

    if (currentCtx->transformRequired)
    {
        pfmMat4Mul(modelview, currentCtx->transform, modelview);
    }

    if (outMVP)
    {
        pfmMat4Mul(outMVP, modelview, currentCtx->projection);
    }

    if (outMatNormal) // TODO REVIEW: Only calculate it when PF_LIGHTING state is activated??
    {
        pfmMat4Invert(outMatNormal, currentCtx->transform);
        pfmMat4Transpose(outMatNormal, outMatNormal);
    }

    if (outTransformedModelview)
    {
        pfmMat4Copy(outTransformedModelview, modelview);
    }
}

/* Internal vertex processing function definitions */

void pfInternal_HomogeneousToScreen(PFvertex* restrict v)
{
    // NOTE: We add 0.5 to the screen coordinates to round them to the nearest integer
    // when they are converted to integer coordinates. This adjustment was added because
    // during clipping, some triangle vertices from the positive plane were found to be
    // offset by -1 pixel in X or Y (or both in some cases), which could even cause
    // triangle "tearing". While it's unclear if this is the best or correct solution,
    // it effectively resolves the issue without observed problems so far. There may be
    // an error in the polygon clipping functions. Nonetheless, this solution has been
    // functioning without issue up to this point.

    v->screen[0] = (currentCtx->viewportX + (v->homogeneous[0] + 1.0f) * 0.5f * currentCtx->viewportW) + 0.5f;
    v->screen[1] = (currentCtx->viewportY + (1.0f - v->homogeneous[1]) * 0.5f * currentCtx->viewportH) + 0.5f;
}

/* Internal processing and rasterization function definitions */

static void ProcessRasterize_Point(const PFMmat4 mvp)
{
    PFvertex *processed = currentCtx->vertexBuffer;

    if (!Process_ProjectPoint(processed, mvp)) return;

    (currentCtx->state & (PF_DEPTH_TEST)
        ? Rasterize_PointDepth
        : Rasterize_PointFlat)
        (processed);
}

static void ProcessRasterize_PolygonPoints(const PFMmat4 mvp, int_fast8_t vertexCount)
{
    for (int_fast8_t i = 0; i < vertexCount; i++)
    {
        PFvertex *processed = currentCtx->vertexBuffer + i;

        if (!Process_ProjectPoint(processed, mvp)) return;

        (currentCtx->state & (PF_DEPTH_TEST)
            ? Rasterize_PointDepth
            : Rasterize_PointFlat)
            (processed);
    }
}

static void ProcessRasterize_Line(const PFMmat4 mvp)
{
    // Process vertices

    int_fast8_t processedCounter = 2;

    PFvertex processed[2] = {
        currentCtx->vertexBuffer[0],
        currentCtx->vertexBuffer[1]
    };

    Process_ProjectAndClipLine(processed, &processedCounter, mvp);
    if (processedCounter != 2) return;

    // Multiply vertices color with material diffuse

    for (int_fast8_t i = 0; i < processedCounter; i++)
    {
        processed[i].color = pfBlendMultiplicative(
            processed[i].color, currentCtx->frontMaterial.diffuse);
    }

    // Rasterize line

    if (currentCtx->state & (PF_DEPTH_TEST))
    {
        Rasterize_LineDepth(&processed[0], &processed[1]);
    }
    else
    {
        Rasterize_LineFlat(&processed[0], &processed[1]);
    }
}

static void ProcessRasterize_PolygonLines(const PFMmat4 mvp, int_fast8_t vertexCount)
{
    for (int_fast8_t i = 0; i < vertexCount; i++)
    {
        // Process vertices

        int_fast8_t processedCounter = 2;

        PFvertex processed[2] = {
            currentCtx->vertexBuffer[i],
            currentCtx->vertexBuffer[(i + 1) % vertexCount]
        };

        Process_ProjectAndClipLine(processed, &processedCounter, mvp);
        if (processedCounter != 2) return;

        // Multiply vertices color with material diffuse

        for (int_fast8_t i = 0; i < processedCounter; i++)
        {
            processed[i].color = pfBlendMultiplicative(
                processed[i].color, currentCtx->frontMaterial.diffuse);
        }

        // Rasterize line

        if (currentCtx->state & (PF_DEPTH_TEST))
        {
            Rasterize_LineDepth(&processed[0], &processed[1]);
        }
        else
        {
            Rasterize_LineFlat(&processed[0], &processed[1]);
        }
    }
}

static void ProcessRasterize_TriangleFill(PFface faceToRender, const PFMmat4 mvp, const PFMmat4 matNormal)
{
    int_fast8_t processedCounter = 3;

    PFvertex processed[PF_MAX_CLIPPED_POLYGON_VERTICES] = {
        currentCtx->vertexBuffer[0],
        currentCtx->vertexBuffer[1],
        currentCtx->vertexBuffer[2]
    };

    // Transform normals if needed

    if (currentCtx->state & PF_LIGHTING)
    {
        for (int_fast8_t i = 0; i < processedCounter; i++)
        {
            pfmVec3Transform(processed[i].normal, processed[i].normal, matNormal);
            pfmVec3Normalize(processed[i].normal, processed[i].normal);
        }
    }

    // Multiply vertices color with material diffuse

    for (int_fast8_t i = 0; i < processedCounter; i++)
    {
        processed[i].color = pfBlendMultiplicative(
            processed[i].color, currentCtx->frontMaterial.diffuse);
    }

    // Process vertices

    PFboolean is2D = Process_ProjectAndClipTriangle(processed, &processedCounter, mvp);
    if (processedCounter < 3) return;

    // Rasterize filled triangles

    if (is2D)
    {
        RasterizeTriangleFunc rasterizer = Rasterize_TriangleColorFlat2D;

        // Selects the appropriate rasterization function

        if (currentCtx->currentTexture &&
            (currentCtx->state & (PF_TEXTURE_2D | PF_DEPTH_TEST)) == (PF_TEXTURE_2D | PF_DEPTH_TEST))
        {
            rasterizer = Rasterize_TriangleTextureDepth2D;
        }
        else if (currentCtx->currentTexture && currentCtx->state & (PF_TEXTURE_2D))
        {
            rasterizer = Rasterize_TriangleTextureFlat2D;
        }
        else if (currentCtx->state & (PF_DEPTH_TEST))
        {
            rasterizer = Rasterize_TriangleColorDepth2D;
        }

        // Performs rasterization of triangles

        for (int_fast8_t i = 0; i < processedCounter - 2; i++)
        {
            rasterizer(faceToRender, &processed[0], &processed[i + 1], &processed[i + 2]);
        }
    }
    else
    {
        if (currentCtx->state & PF_LIGHTING)
        {
            // Pre-calculation of specularity tints
            // by multiplying those of light and material

            PFcolor oldLightSpecTints[PF_MAX_LIGHTS];

            for (PFint i = 0; i <= currentCtx->lastActiveLight; i++)
            {
                PFlight *l = &currentCtx->lights[i];
                oldLightSpecTints[i] = l->specular;

                if (l->active) l->specular = pfBlendMultiplicative(
                    l->specular, currentCtx->frontMaterial.specular);
            }

            // Get camera/view position

            PFMmat4 invMV;
            pfmMat4Invert(invMV, currentCtx->modelview);
            PFMvec3 viewPos = { invMV[12], invMV[13], invMV[14] };

            // Selects the appropriate rasterization function

            RasterizeTriangleLightFunc rasterizer = Rasterize_TriangleColorFlatLight3D;

            if (currentCtx->currentTexture &&
                (currentCtx->state & (PF_TEXTURE_2D | PF_DEPTH_TEST)) == (PF_TEXTURE_2D | PF_DEPTH_TEST))
            {
                rasterizer = Rasterize_TriangleTextureDepthLight3D;
            }
            else if (currentCtx->currentTexture && currentCtx->state & (PF_TEXTURE_2D))
            {
                rasterizer = Rasterize_TriangleTextureFlatLight3D;
            }
            else if (currentCtx->state & (PF_DEPTH_TEST))
            {
                rasterizer = Rasterize_TriangleColorDepthLight3D;
            }

            // Performs rasterization of triangles

            for (int_fast8_t i = 0; i < processedCounter - 2; i++)
            {
                rasterizer(faceToRender, &processed[0], &processed[i + 1], &processed[i + 2], viewPos);
            }

            // Reset old light specular tints

            for (PFint i = 0; i <= currentCtx->lastActiveLight; i++)
            {
                if (currentCtx->lights[i].active)
                {
                    currentCtx->lights[i].specular = oldLightSpecTints[i];
                }
            }
        }
        else
        {
            // Selects the appropriate rasterization function

            RasterizeTriangleFunc rasterizer = Rasterize_TriangleColorFlat3D;

            if (currentCtx->currentTexture &&
                (currentCtx->state & (PF_TEXTURE_2D | PF_DEPTH_TEST)) == (PF_TEXTURE_2D | PF_DEPTH_TEST))
            {
                rasterizer = Rasterize_TriangleTextureDepth3D;
            }
            else if (currentCtx->currentTexture && currentCtx->state & (PF_TEXTURE_2D))
            {
                rasterizer = Rasterize_TriangleTextureFlat3D;
            }
            else if (currentCtx->state & (PF_DEPTH_TEST))
            {
                rasterizer = Rasterize_TriangleColorDepth3D;
            }

            // Performs rasterization of triangles

            for (int_fast8_t i = 0; i < processedCounter - 2; i++)
            {
                rasterizer(faceToRender, &processed[0], &processed[i + 1], &processed[i + 2]);
            }
        }
    }
}

static void ProcessRasterize_QuadFill(PFface faceToRender, const PFMmat4 mvp, const PFMmat4 matNormal)
{
    for (PFint i = 0; i < 2; i++)
    {
        int_fast8_t processedCounter = 3;

        PFvertex processed[PF_MAX_CLIPPED_POLYGON_VERTICES] = {
            currentCtx->vertexBuffer[0],
            currentCtx->vertexBuffer[i + 1],
            currentCtx->vertexBuffer[i + 2]
        };

        // Transform normals if needed

        if (currentCtx->state & PF_LIGHTING)
        {
            for (int_fast8_t j = 0; j < processedCounter; j++)
            {
                pfmVec3Transform(processed[j].normal, processed[j].normal, matNormal);
                pfmVec3Normalize(processed[j].normal, processed[j].normal);
            }
        }

        // Multiply vertices color with material diffuse

        for (int_fast8_t j = 0; j < processedCounter; j++)
        {
            processed[j].color = pfBlendMultiplicative(
                processed[j].color, currentCtx->frontMaterial.diffuse);
        }

        // Process vertices

        PFboolean is2D = Process_ProjectAndClipTriangle(processed, &processedCounter, mvp);
        if (processedCounter < 3) continue;

        // Rasterize triangles

        if (is2D)
        {
            // Selects the appropriate rasterization function

            RasterizeTriangleFunc rasterizer = Rasterize_TriangleColorFlat2D;

            if (currentCtx->currentTexture &&
                (currentCtx->state & (PF_TEXTURE_2D | PF_DEPTH_TEST)) == (PF_TEXTURE_2D | PF_DEPTH_TEST))
            {
                rasterizer = Rasterize_TriangleTextureDepth2D;
            }
            else if (currentCtx->currentTexture && currentCtx->state & (PF_TEXTURE_2D))
            {
                rasterizer = Rasterize_TriangleTextureFlat2D;
            }
            else if (currentCtx->state & (PF_DEPTH_TEST))
            {
                rasterizer = Rasterize_TriangleColorDepth2D;
            }

            // Performs rasterization of triangles

            for (int_fast8_t j = 0; j < processedCounter - 2; j++)
            {
                rasterizer(faceToRender, &processed[0], &processed[j + 1], &processed[j + 2]);
            }
        }
        else
        {
            if (currentCtx->state & PF_LIGHTING)
            {
                // Pre-calculation of specularity tints
                // by multiplying those of light and material

                PFcolor oldLightSpecTints[PF_MAX_LIGHTS];

                for (PFint j = 0; j <= currentCtx->lastActiveLight; j++)
                {
                    PFlight *l = &currentCtx->lights[j];
                    oldLightSpecTints[j] = l->specular;

                    if (l->active) l->specular = pfBlendMultiplicative(
                        l->specular, currentCtx->frontMaterial.specular);
                }

                // Get camera/view position

                PFMmat4 invMV;
                pfmMat4Invert(invMV, currentCtx->modelview);
                PFMvec3 viewPos = { invMV[12], invMV[13], invMV[14] };

                // Selects the appropriate rasterization function

                RasterizeTriangleLightFunc rasterizer = Rasterize_TriangleColorFlatLight3D;

                if (currentCtx->currentTexture &&
                    (currentCtx->state & (PF_TEXTURE_2D | PF_DEPTH_TEST)) == (PF_TEXTURE_2D | PF_DEPTH_TEST))
                {
                    rasterizer = Rasterize_TriangleTextureDepthLight3D;
                }
                else if (currentCtx->currentTexture && currentCtx->state & (PF_TEXTURE_2D))
                {
                    rasterizer = Rasterize_TriangleTextureFlatLight3D;
                }
                else if (currentCtx->state & (PF_DEPTH_TEST))
                {
                    rasterizer = Rasterize_TriangleColorDepthLight3D;
                }

                // Performs rasterization of triangles

                for (int_fast8_t j = 0; j < processedCounter - 2; j++)
                {
                    rasterizer(faceToRender, &processed[0], &processed[j + 1], &processed[j + 2], viewPos);
                }

                // Reset old light specular tints

                for (PFint j = 0; j <= currentCtx->lastActiveLight; j++)
                {
                    if (currentCtx->lights[j].active)
                    {
                        currentCtx->lights[j].specular = oldLightSpecTints[j];
                    }
                }
            }
            else
            {
                // Selects the appropriate rasterization function

                RasterizeTriangleFunc rasterizer = Rasterize_TriangleColorFlat3D;

                if (currentCtx->currentTexture &&
                    (currentCtx->state & (PF_TEXTURE_2D | PF_DEPTH_TEST)) == (PF_TEXTURE_2D | PF_DEPTH_TEST))
                {
                    rasterizer = Rasterize_TriangleTextureDepth3D;
                }
                else if (currentCtx->currentTexture && currentCtx->state & (PF_TEXTURE_2D))
                {
                    rasterizer = Rasterize_TriangleTextureFlat3D;
                }
                else if (currentCtx->state & (PF_DEPTH_TEST))
                {
                    rasterizer = Rasterize_TriangleColorDepth3D;
                }

                // Performs rasterization of triangles

                for (int_fast8_t j = 0; j < processedCounter - 2; j++)
                {
                    rasterizer(faceToRender, &processed[0], &processed[j + 1], &processed[j + 2]);
                }
            }
        }
    }
}

void ProcessRasterize(const PFMmat4 mvp, const PFMmat4 matNormal)
{
    switch (currentCtx->currentDrawMode)
    {
        case PF_POINTS:
            ProcessRasterize_Point(mvp);
            break;

        case PF_LINES:
            ProcessRasterize_Line(mvp);
            break;

        case PF_TRIANGLES:
        {
            // Get faces to render
            // NOTE: Here we invert cullFace, because PF_FRONT = 0,
            //       !PF_FRONT = PF_BACK, and vice versa.
            PFface faceToRender = (currentCtx->state & PF_CULL_FACE)
                ? (!currentCtx->cullFace) : PF_FRONT_AND_BACK;

            if (faceToRender == PF_FRONT_AND_BACK &&
                currentCtx->polygonMode[0] != currentCtx->polygonMode[1])
            {
                for (PFint iFace = 0; iFace < 2; iFace++)
                {
                    switch (currentCtx->polygonMode[iFace])
                    {
                        case PF_POINT:
                            ProcessRasterize_PolygonPoints(mvp, 3);
                            break;

                        case PF_LINE:
                            ProcessRasterize_PolygonLines(mvp, 3);
                            break;

                        case PF_FILL:
                            ProcessRasterize_TriangleFill(iFace, mvp, matNormal);
                            break;
                    }
                }
            }
            else
            {
                switch (currentCtx->polygonMode[faceToRender])
                {
                    case PF_POINT:
                        ProcessRasterize_PolygonPoints(mvp, 3);
                        break;

                    case PF_LINE:
                        ProcessRasterize_PolygonLines(mvp, 3);
                        break;

                    case PF_FILL:
                        ProcessRasterize_TriangleFill(faceToRender, mvp, matNormal);
                        break;
                }
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

            if (faceToRender == PF_FRONT_AND_BACK &&
                currentCtx->polygonMode[0] != currentCtx->polygonMode[1])
            {
                for (PFint iFace = 0; iFace < 2; iFace++)
                {
                    switch (currentCtx->polygonMode[iFace])
                    {
                        case PF_POINT:
                            ProcessRasterize_PolygonPoints(mvp, 4);
                            break;

                        case PF_LINE:
                            ProcessRasterize_PolygonLines(mvp, 4);
                            break;

                        case PF_FILL:
                            ProcessRasterize_QuadFill(iFace, mvp, matNormal);
                            break;
                    }
                }
            }
            else
            {
                switch (currentCtx->polygonMode[faceToRender])
                {
                    case PF_POINT:
                        ProcessRasterize_PolygonPoints(mvp, 4);
                        break;

                    case PF_LINE:
                        ProcessRasterize_PolygonLines(mvp, 4);
                        break;

                    case PF_FILL:
                        ProcessRasterize_TriangleFill(faceToRender, mvp, matNormal);
                        break;
                }
            }
        }
        break;
    }
}
