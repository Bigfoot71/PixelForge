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

#include "pixelforge.h"
#include "trirast.c"

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <float.h>
#include <math.h>

// TODO: Review the API get/set and enable/disable functions

/* Internal data */

typedef struct {
    const void *positions;
    const void *normals;
    const void *colors;
    const void *texcoords;
} PFvertexattribs;

typedef struct {
    PFvec4f homogeneous;
    PFvec2f screen;
    PFvec3f position;
    PFvec3f normal;
    PFvec2f texcoord;
    PFcolor color;
} PFvertex;

typedef struct {
    PFvec3f position;
    PFvec3f direction;
    PFcolor ambient;
    PFcolor diffuse;
    PFcolor specular;
    PFboolean active;
} PFlight;

typedef struct {
    PFcolor ambient;
    PFcolor diffuse;
    PFcolor specular;
    PFcolor emission;
    PFfloat shininess;
} PFmaterial;

struct PFctx {

    PFframebuffer screenBuffer;                 // Screen buffer for rendering
    PFframebuffer *currentFramebuffer;          // Pointer to the current framebuffer

    PFuint viewportX, viewportY;                // X and Y coordinates of the viewport
    PFuint viewportW, viewportH;                // Width and height of the viewport

    PFdrawmode currentDrawMode;                 // Current drawing mode (e.g., lines, triangles)
    PFblendfunc blendFunction;                  // Blend function for alpha blending
    PFcolor clearColor;                         // Color used to clear the screen

    PFvec3f currentNormal;                      // Current normal vector for lighting calculations
    PFvec2f currentTexcoord;                    // Current texture coordinates
    PFcolor currentColor;                       // Current color for vertex rendering

    PFvertex vertexBuffer[6];                   // Vertex buffer for geometry
    PFuint vertexCount;                         // Number of vertices in the buffer

    PFlight lights[PF_MAX_LIGHTS];
    PFint lastActiveLight;

    PFmaterial frontMaterial;
    PFmaterial backMaterial;

    PFmatrixmode currentMatrixMode;             // Current matrix mode (e.g., PF_MODELVIEW, PF_PROJECTION)
    PFmat4f *currentMatrix;                     // Pointer to the current matrix
    PFmat4f modelview;                          // Default modelview matrix
    PFmat4f projection;                         // Default projection matrix
    PFmat4f transform;                          // Transformation matrix for translation, rotation, and scaling
    PFboolean transformRequired;                // Flag indicating whether transformation is required for vertices
    PFmat4f stack[PF_MAX_MATRIX_STACK_SIZE];    // Matrix stack for push/pop operations
    PFint stackCounter;                         // Counter for matrix stack operations

    PFvertexattribs vertexAttribs;              // Vertex attributes (e.g., normal, texture coordinates)
    PFtexture *currentTexture;                  // Pointer to the current texture

    PFushort vertexAttribState;                 // State of vertex attributes
    PFushort state;                             // Current rendering state

    PFface cullFace;                           // Faces to cull

};

typedef enum {
    CLIP_INSIDE = 0x00, // 0000
    CLIP_LEFT   = 0x01, // 0001
    CLIP_RIGHT  = 0x02, // 0010
    CLIP_BOTTOM = 0x04, // 0100
    CLIP_TOP    = 0x08, // 1000
} PFclipcode;

/* Current thread local-thread declaration */

#if defined(__GNUC__) || defined(__clang__)
    static __thread PFctx *currentCtx = NULL;
#elif defined(_MSC_VER)
    __declspec(thread) static PFctx *currentCtx = NULL;
#else
    static PFctx *currentCtx = NULL;
#endif

/* Internal types */

typedef void (*RasterizeTriangleFunc)(const PFvertex*, const PFvertex*, const PFvertex*);
typedef void (*RasterizeTriangleLightFunc)(const PFvertex*, const PFvertex*, const PFvertex*, const PFvec3f);

/* Internal processing and rasterization function declarations */

static void ProcessRasterize(const PFmat4f* mvp, const PFmat4f* matNormal);

/* Internal helper function declarations */

static PFmat4f Helper_GetModelView(PFmat4f* outMatNormal, PFmat4f* outMVP)
{
    PFmat4f modelview = currentCtx->modelview;

    if (currentCtx->transformRequired)
    {
        modelview = pfMat4fMul(&currentCtx->transform, &modelview);
    }

    if (outMatNormal)   // TODO REVIEW: Only calculate it when PF_LIGHTING state is activated??
    {
        *outMatNormal = pfMat4fInvert(&currentCtx->transform);  // TODO REVIEW: modelview strange behaviour, i need to recheck this
        *outMatNormal = pfMat4fTranspose(outMatNormal);
    }

    if (outMVP)
    {
        *outMVP = pfMat4fMul(&modelview, &currentCtx->projection);
    }

    return modelview;
}

/* Context API functions */

PFctx* pfContextCreate(void* screenBuffer, PFuint screenWidth, PFuint screenHeight, PFpixelformat screenFormat)
{
    PFctx *ctx = (PFctx*)PF_MALLOC(sizeof(PFctx));

    ctx->screenBuffer = (PFframebuffer) { 0 };

    ctx->screenBuffer.texture = pfTextureCreate(
        screenBuffer, screenWidth, screenHeight, screenFormat);

    const PFsizei bufferSize = screenWidth*screenHeight;
    ctx->screenBuffer.zbuffer = (PFfloat*)PF_MALLOC(bufferSize*sizeof(PFctx));
    for (PFsizei i = 0; i < bufferSize; i++) ctx->screenBuffer.zbuffer[i] = FLT_MAX;

    ctx->currentFramebuffer = &ctx->screenBuffer;

    ctx->viewportX = 0;
    ctx->viewportY = 0;
    ctx->viewportW = screenWidth - 1;
    ctx->viewportH = screenHeight - 1;

    ctx->currentDrawMode = 0;
    ctx->blendFunction = pfBlendDisabled;
    ctx->clearColor = (PFcolor) { 0 };

    memset(ctx->currentNormal, 0, sizeof(PFvec3f));
    memset(ctx->currentTexcoord, 0, sizeof(PFvec2f));
    ctx->currentColor = (PFcolor) { 255, 255, 255, 255 };

    ctx->vertexCount = 0;

    for (PFuint i = 0; i < PF_MAX_LIGHTS; i++)
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
    ctx->currentMatrix = NULL;
    ctx->modelview = pfMat4fIdentity();
    ctx->projection = pfMat4fIdentity();
    ctx->transform = pfMat4fIdentity();
    ctx->transformRequired = PF_FALSE;
    ctx->stackCounter = 0;

    ctx->vertexAttribs = (PFvertexattribs) { 0 };
    ctx->currentTexture = NULL;

    ctx->vertexAttribState = 0x00;
    ctx->state = 0x00;

    ctx->state |= PF_CULL_FACE;
    ctx->cullFace = PF_BACK;

    return ctx;
}

void pfContextDestroy(PFctx* ctx)
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

PFctx* pfGetCurrent(void)
{
    return currentCtx;
}

void pfMakeCurrent(PFctx* ctx)
{
    currentCtx = ctx;
}

PFboolean pfIsCurrent(PFctx* ctx)
{
    return (PFboolean)(currentCtx == ctx);
}

void pfEnable(PFstate state)
{
    currentCtx->state |= state;
}

void pfDisable(PFstate state)
{
    currentCtx->state &= ~state;
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
    }

    currentCtx->currentMatrixMode = mode;
}

void pfPushMatrix(void)
{
    if (currentCtx->stackCounter >= PF_MAX_MATRIX_STACK_SIZE)
    {
        PF_LOG(PF_LOG_ERROR, "[pfPushMatrix] Matrix stack overflow (PF_MAX_MATRIX_STACK_SIZE=%i)", PF_MAX_MATRIX_STACK_SIZE);
    }

    if (currentCtx->currentMatrixMode == PF_MODELVIEW)
    {
        currentCtx->transformRequired = PF_TRUE;
        currentCtx->currentMatrix = &currentCtx->transform;
    }

    currentCtx->stack[currentCtx->stackCounter] = *currentCtx->currentMatrix;
    currentCtx->stackCounter++;
}

void pfPopMatrix(void)
{
    if (currentCtx->stackCounter > 0)
    {
        *currentCtx->currentMatrix = currentCtx->stack[--currentCtx->stackCounter];
    }

    if (currentCtx->stackCounter == 0 && currentCtx->currentMatrixMode == PF_MODELVIEW)
    {
        currentCtx->currentMatrix = &currentCtx->modelview;
        currentCtx->transformRequired = PF_FALSE;
    }
}

void pfLoadIdentity(void)
{
    *currentCtx->currentMatrix = pfMat4fIdentity();
}

void pfTranslatef(PFfloat x, PFfloat y, PFfloat z)
{
    PFmat4f translation = pfMat4fTranslate(x, y, z);

    // NOTE: We transpose matrix with multiplication order
    *currentCtx->currentMatrix = pfMat4fMul(&translation, currentCtx->currentMatrix);
}

void pfRotatef(PFfloat angle, PFfloat x, PFfloat y, PFfloat z)
{
    PFvec3f axis = { x, y, z }; // TODO: review
    PFmat4f rotation = pfMat4fRotate(axis, DEG2RAD(angle));

    // NOTE: We transpose matrix with multiplication order
    *currentCtx->currentMatrix = pfMat4fMul(&rotation, currentCtx->currentMatrix);
}

void pfScalef(PFfloat x, PFfloat y, PFfloat z)
{
    PFmat4f scale = pfMat4fScale(x, y, z);

    // NOTE: We transpose matrix with multiplication order
    *currentCtx->currentMatrix = pfMat4fMul(&scale, currentCtx->currentMatrix);
}

void pfMultMatrixf(const PFfloat* mat)
{
    *currentCtx->currentMatrix = pfMat4fMul(currentCtx->currentMatrix, (PFmat4f*)&mat);
}

void pfMultMatrixMat4f(const PFmat4f* mat)
{
    *currentCtx->currentMatrix = pfMat4fMul(currentCtx->currentMatrix, mat);
}

void pfFrustum(PFdouble left, PFdouble right, PFdouble bottom, PFdouble top, PFdouble znear, PFdouble zfar)
{
    PFmat4f frustum = pfMat4fFrustum(left, right, bottom, top, znear, zfar);
    *currentCtx->currentMatrix = pfMat4fMul(currentCtx->currentMatrix, &frustum);
}

void pfOrtho(PFdouble left, PFdouble right, PFdouble bottom, PFdouble top, PFdouble znear, PFdouble zfar)
{
    PFmat4f ortho = pfMat4fOrtho(left, right, bottom, top, znear, zfar);
    *currentCtx->currentMatrix = pfMat4fMul(currentCtx->currentMatrix, &ortho);
}


/* Render API functions */

void pfGetViewport(PFuint* x, PFuint* y, PFuint* width, PFuint* height)
{
    *x = currentCtx->viewportX;
    *y = currentCtx->viewportY;
    *width = currentCtx->viewportW + 1;
    *height = currentCtx->viewportH + 1;
}

void pfViewport(PFuint x, PFuint y, PFuint width, PFuint height)
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

void pfEnableStatePointer(PFarraytype vertexAttribType, const void* buffer)
{
    if (!buffer)
    {
        pfDisableStatePointer(vertexAttribType);
        return;
    }

    currentCtx->vertexAttribState |= vertexAttribType;

    switch (vertexAttribType)
    {
        case PF_VERTEX_ARRAY: currentCtx->vertexAttribs.positions = buffer; break;
        case PF_NORMAL_ARRAY: currentCtx->vertexAttribs.normals = buffer; break;
        case PF_COLOR_ARRAY: currentCtx->vertexAttribs.colors = buffer; break;
        case PF_TEXTURE_COORD_ARRAY: currentCtx->vertexAttribs.texcoords = buffer; break;
    }
}

void pfDisableStatePointer(PFarraytype vertexAttribType)
{
    currentCtx->vertexAttribState &= ~vertexAttribType;

    switch (vertexAttribType)
    {
        case PF_VERTEX_ARRAY: currentCtx->vertexAttribs.positions = NULL; break;
        case PF_NORMAL_ARRAY: currentCtx->vertexAttribs.normals = NULL; break;
        case PF_COLOR_ARRAY: currentCtx->vertexAttribs.colors = NULL; break;
        case PF_TEXTURE_COORD_ARRAY: currentCtx->vertexAttribs.texcoords = NULL; break;
    }
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

void pfEnableLight(PFuint light)
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

void pfDisableLight(PFuint light)
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

void pfLightfv(PFuint light, PFuint param, const void* value)
{
    if (light < PF_MAX_LIGHTS)
    {
        PFlight *l = &currentCtx->lights[light];

        switch (param)
        {
            case PF_POSITION:
                memcpy(l->position, value, sizeof(PFvec3f));
                break;

            case PF_SPOT_DIRECTION:
                memcpy(l->direction, value, sizeof(PFvec3f));
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

            case PF_AMBIENT_AND_DIFFUSE:
                PF_LOG(PF_LOG_WARNING, "[pfLightfv] The definition 'PF_AMBIENT_AND_DIFFUSE' is reserved for 'pfMaterialfv'");
                break;

            default: break;
        }
    }
}

void pfMaterialf(PFface face, PFuint param, PFfloat value)
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

        default: return;
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

        default: break;
    }
}

void pfMaterialfv(PFface face, PFuint param, const void *value)
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

        default: return;
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

        default: break;
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

void pfDrawVertexArrayElements(PFsizei offset, PFsizei count, const void *buffer)
{
    if (!(currentCtx->vertexAttribState & PF_VERTEX_ARRAY)) return;
    const PFvec3f *positions = (const PFvec3f*)(currentCtx->vertexAttribs.positions) + offset;

    const PFvec3f *normals = (currentCtx->vertexAttribState & PF_NORMAL_ARRAY)
        ? (const PFvec3f*)(currentCtx->vertexAttribs.normals) + offset : NULL;

    const PFcolor *colors = (currentCtx->vertexAttribState & PF_COLOR_ARRAY)
        ? (const PFcolor*)(currentCtx->vertexAttribs.colors) + offset : NULL;

    const PFvec2f *texcoords = (currentCtx->vertexAttribState & PF_TEXTURE_COORD_ARRAY)
        ? (const PFvec2f*)(currentCtx->vertexAttribs.texcoords) + offset : NULL;

    PFmat4f matNormal, mvp;
    (void)Helper_GetModelView(&matNormal, &mvp);

    pfBegin((currentCtx->state & PF_WIRE_MODE) ? PF_LINES : PF_TRIANGLES);

    for (PFsizei i = 0; i < count; i++)
    {
        const PFushort j = ((PFushort*)buffer)[i];

        // Get the pointer of the current vertex of the batch and pad it with zero

        PFvertex *vertex = currentCtx->vertexBuffer + (currentCtx->vertexCount++);

        // Fill the vertex with given vertices data

        memcpy(vertex->position, *(positions + j), sizeof(PFvec3f));

        if (normals) memcpy(vertex->normal, *(normals + j), sizeof(PFvec3f));
        else memset(vertex->normal, 0, sizeof(PFvec3f));

        if (texcoords) memcpy(vertex->texcoord, *(texcoords + j), sizeof(PFvec2f));
        else memset(vertex->texcoord, 0, sizeof(PFvec2f));

        if (colors) memcpy(&vertex->color, colors + j, sizeof(PFcolor));
        else memcpy(&vertex->color, &currentCtx->currentColor, sizeof(PFcolor));

        // If the number of vertices has reached that necessary for, we process the shape

        if (currentCtx->vertexCount == currentCtx->currentDrawMode)
        {
            currentCtx->vertexCount = 0;
            ProcessRasterize(&mvp, &matNormal);
        }
    }

    pfEnd();
}

void pfDrawVertexArray(PFsizei offset, PFsizei count)
{
    if (!(currentCtx->vertexAttribState & PF_VERTEX_ARRAY)) return;
    const PFvec3f *positions = (const PFvec3f*)(currentCtx->vertexAttribs.positions) + offset;

    const PFvec3f *normals = (currentCtx->vertexAttribState & PF_NORMAL_ARRAY)
        ? (const PFvec3f*)(currentCtx->vertexAttribs.normals) + offset : NULL;

    const PFcolor *colors = (currentCtx->vertexAttribState & PF_COLOR_ARRAY)
        ? (const PFcolor*)(currentCtx->vertexAttribs.colors) + offset : NULL;

    const PFvec2f *texcoords = (currentCtx->vertexAttribState & PF_TEXTURE_COORD_ARRAY)
        ? (const PFvec2f*)(currentCtx->vertexAttribs.texcoords) + offset : NULL;

    PFmat4f matNormal, mvp;
    (void)Helper_GetModelView(&matNormal, &mvp);

    pfBegin((currentCtx->state & PF_WIRE_MODE) ? PF_LINES : PF_TRIANGLES);

    for (PFsizei i = 0; i < count; i++)
    {
        // Get the pointer of the current vertex of the batch and pad it with zero

        PFvertex *vertex = currentCtx->vertexBuffer + (currentCtx->vertexCount++);
        memset(vertex, 0, sizeof(PFvertex));

        // Fill the vertex with given vertices data

        memcpy(vertex->position, *(positions + i), sizeof(PFvec3f));

        if (normals) memcpy(vertex->normal, *(normals + i), sizeof(PFvec3f));
        else memset(vertex->normal, 0, sizeof(PFvec3f));

        if (texcoords) memcpy(vertex->texcoord, *(texcoords + i), sizeof(PFvec2f));
        else memset(vertex->texcoord, 0, sizeof(PFvec2f));

        if (colors) memcpy(&vertex->color, colors + i, sizeof(PFcolor));
        else memcpy(&vertex->color, &currentCtx->currentColor, sizeof(PFcolor));

        // If the number of vertices has reached that necessary for, we process the shape

        if (currentCtx->vertexCount == currentCtx->currentDrawMode)
        {
            currentCtx->vertexCount = 0;
            ProcessRasterize(&mvp, &matNormal);
        }
    }

    pfEnd();
}

void pfBegin(PFdrawmode mode)
{
    currentCtx->currentDrawMode = mode;
    currentCtx->vertexCount = 0;
}

void pfEnd(void)
{
    currentCtx->vertexCount = 0;
}

void pfVertex2i(PFint x, PFint y)
{
    PFvec3f v = { (PFfloat)x, (PFfloat)y, 0.0f };
    pfVertexfv(v);
}

void pfVertex2f(PFfloat x, PFfloat y)
{
    PFvec3f v = { x, y, 0.0f };
    pfVertexfv(v);
}

void pfVertex2fv(const PFfloat* v)
{
    PFvec3f v3 = { v[0], v[1], 0.0f };
    pfVertexfv(v3);
}

void pfVertex3f(PFfloat x, PFfloat y, PFfloat z)
{
    PFvec3f v = { x, y, z };
    pfVertexfv(v);
}

void pfVertexfv(const PFfloat* v)
{
    // Get the pointer of the current vertex of the batch and pad it with zero

    PFvertex *vertex = currentCtx->vertexBuffer + (currentCtx->vertexCount++);

    // Fill the vertex with given vertices data

    memcpy(vertex->position, v, sizeof(PFvec3f));
    memcpy(vertex->normal, currentCtx->currentNormal, sizeof(PFvec3f));
    memcpy(vertex->texcoord, currentCtx->currentTexcoord, sizeof(PFvec2f));
    memcpy(&vertex->color, &currentCtx->currentColor, sizeof(PFcolor));

    // If the number of vertices has reached that necessary for, we process the shape

    if (currentCtx->vertexCount == currentCtx->currentDrawMode)
    {
        PFmat4f matNormal, mvp;
        (void)Helper_GetModelView(&matNormal, &mvp);

        currentCtx->vertexCount = 0;
        ProcessRasterize(&mvp, &matNormal);
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
    memcpy(currentCtx->currentTexcoord, v, sizeof(PFvec2f));
}

void pfNormal3f(PFfloat x, PFfloat y, PFfloat z)
{
    currentCtx->currentNormal[0] = x;
    currentCtx->currentNormal[1] = y;
    currentCtx->currentNormal[2] = z;
}

void pfNormal3fv(const PFfloat* v)
{
    memcpy(currentCtx->currentNormal, v, sizeof(PFvec3f));
}


/* Internal helper function definitions */

static PFvertex Helper_LerpVertex(const PFvertex* start, const PFvertex* end, PFfloat t)
{
    PFvertex result = { 0 };

    // Interpolate homogeneous position
    for (int_fast8_t i = 0; i < 4; i++)
    {
        result.homogeneous[i] = start->homogeneous[i] + t*(end->homogeneous[i] - start->homogeneous[i]);
    }

    // Interpolate position
    for (int_fast8_t i = 0; i < 3; i++)
    {
        result.position[i] = start->position[i] + t*(end->position[i] - start->position[i]);
    }

    // Interpolate normal
    for (int_fast8_t i = 0; i < 3; i++)
    {
        result.normal[i] = start->normal[i] + t*(end->normal[i] - start->normal[i]);
    }

    // Interpolate texcoord
    for (int_fast8_t i = 0; i < 2; i++)
    {
        result.texcoord[i] = start->texcoord[i] + t*(end->texcoord[i] - start->texcoord[i]);
    }

    // Interpolate color
    result.color.r = start->color.r + t*(end->color.r - start->color.r);
    result.color.g = start->color.g + t*(end->color.g - start->color.g);
    result.color.b = start->color.b + t*(end->color.b - start->color.b);
    result.color.a = start->color.a + t*(end->color.a - start->color.a);

    return result;
}

static PFcolor Helper_LerpColor(PFcolor a, PFcolor b, PFfloat t)
{
    return (PFcolor) {
        a.r + t*(b.r - a.r),
        a.g + t*(b.g - a.g),
        a.b + t*(b.b - a.b),
        a.a + t*(b.a - a.a)
    };
}

static void Helper_InterpolateVec2f(PFvec2f dst, const PFvec2f v1, const PFvec2f v2, const PFvec2f v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

static void Helper_InterpolateVec3f(PFvec2f dst, const PFvec3f v1, const PFvec3f v2, const PFvec3f v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

static PFcolor Helper_InterpolateColor(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    // REVIEW: Normalization necessary here ?

    return (PFcolor) {
        (PFubyte)(w1*v1.r + w2*v2.r + w3*v3.r),
        (PFubyte)(w1*v1.g + w2*v2.g + w3*v3.g),
        (PFubyte)(w1*v1.b + w2*v2.b + w3*v3.b),
        (PFubyte)(w1*v1.a + w2*v2.a + w3*v3.a)
    };
}

static void Helper_SwapVertex(PFvertex* a, PFvertex* b)
{
    PFvertex tmp = *a;
    *a = *b; *b = tmp;
}

static void Helper_SwapByte(PFubyte* a, PFubyte* b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

// Used by 'Process_ClipLine2D'
static PFubyte Helper_EncodeClip2D(const PFvec2f screen)
{
    PFubyte code = CLIP_INSIDE;
    if (screen[0] < currentCtx->viewportX) code |= CLIP_LEFT;
    if (screen[0] > currentCtx->viewportW) code |= CLIP_RIGHT;
    if (screen[1] < currentCtx->viewportY) code |= CLIP_BOTTOM;
    if (screen[1] > currentCtx->viewportH) code |= CLIP_TOP;
    return code;
}

// Used by 'Process_ClipLine3D'
// 'q' represents a homogeneous coordinate weight from which a homogeneous coordinate 'x', 'y', or 'z' has been subtracted
// 'p' represents the delta between two homogeneous coordinate weights from which the corresponding homogeneous delta 'x', 'y', or 'z' has been subtracted
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


/* Internal vertex processing function definitions */

static void Process_HomogeneousToScreen(PFvertex* restrict v)
{
    v->screen[0] = currentCtx->viewportX + (v->homogeneous[0] + 1.0f)*0.5f*currentCtx->viewportW;
    v->screen[1] = currentCtx->viewportY + (1.0f - v->homogeneous[1])*0.5f*currentCtx->viewportH;
}

static PFboolean Process_ClipLine2D(PFvertex* restrict v1, PFvertex* restrict v2)
{
    PFboolean accept = PF_FALSE;
    PFubyte code0, code1;
    PFfloat m = 0;

    if (v1->screen[0] != v2->screen[0])
    {
        m = (v2->screen[1] - v1->screen[1]) / (v2->screen[0] - v1->screen[0]);
    }

    for (;;)
    {
        code0 = Helper_EncodeClip2D(v1->screen);
        code1 = Helper_EncodeClip2D(v2->screen);

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
            v1->screen[1] += (currentCtx->viewportX - v1->screen[0])*m;
            v1->screen[0] = currentCtx->viewportX;
        }
        else if (code0 & CLIP_RIGHT)
        {
            v1->screen[1] += (currentCtx->viewportW - v1->screen[0])*m;
            v1->screen[0] = currentCtx->viewportW;
        }
        else if (code0 & CLIP_BOTTOM)
        {
            if (m) v1->screen[0] += (currentCtx->viewportY - v1->screen[1]) / m;
            v1->screen[1] = currentCtx->viewportY;
        }
        else if (code0 & CLIP_TOP)
        {
            if (m) v1->screen[0] += (currentCtx->viewportH - v1->screen[1]) / m;
            v1->screen[1] = currentCtx->viewportH;
        }
    }

    return accept;
}

static PFboolean Process_ClipLine3D(PFvertex* restrict v1, PFvertex* restrict v2)
{
    PFfloat t1 = 0, t2 = 1;

    PFvec4f delta;
    pfVec4fSub(delta, v2->homogeneous, v1->homogeneous);

    if (!Helper_ClipCoord3D(v1->homogeneous[3] - v1->homogeneous[0], -delta[3] + delta[0], &t1, &t2)) return PF_FALSE;
    if (!Helper_ClipCoord3D(v1->homogeneous[3] + v1->homogeneous[0], -delta[3] - delta[0], &t1, &t2)) return PF_FALSE;

    if (!Helper_ClipCoord3D(v1->homogeneous[3] - v1->homogeneous[1], -delta[3] + delta[1], &t1, &t2)) return PF_FALSE;
    if (!Helper_ClipCoord3D(v1->homogeneous[3] + v1->homogeneous[1], -delta[3] - delta[1], &t1, &t2)) return PF_FALSE;

    if (!Helper_ClipCoord3D(v1->homogeneous[3] - v1->homogeneous[2], -delta[3] + delta[2], &t1, &t2)) return PF_FALSE;
    if (!Helper_ClipCoord3D(v1->homogeneous[3] + v1->homogeneous[2], -delta[3] - delta[2], &t1, &t2)) return PF_FALSE;

    if (t2 < 1)
    {
        PFvec4f d;
        pfVec4fScale(d, delta, t2);
        pfVec4fAdd(v2->homogeneous, v1->homogeneous, d);
    }

    if (t1 > 0)
    {
        PFvec4f d;
        pfVec4fScale(d, delta, t1);
        pfVec4fAdd(v1->homogeneous, v1->homogeneous, d);
    }

    return PF_TRUE;
}

static PFboolean Process_ClipPolygonW(PFvertex* restrict polygon, PFubyte* restrict vertexCounter)
{
    PFvertex input[PF_MAX_CLIPPED_POLYGON_VERTICES];
    memcpy(input, polygon, (*vertexCounter)*sizeof(PFvertex));

    PFubyte inputCounter = *vertexCounter;
    *vertexCounter = 0;

    const PFvertex *prevVt = &input[inputCounter-1];
    PFbyte prevDot = (prevVt->homogeneous[3] < PF_CLIP_EPSILON) ? -1 : 1;

    for (int_fast8_t i = 0; i < inputCounter; i++)
    {
        PFbyte currDot = (input[i].homogeneous[3] < PF_CLIP_EPSILON) ? -1 : 1;

        if (prevDot*currDot < 0)
        {
            polygon[(*vertexCounter)++] = Helper_LerpVertex(prevVt, &input[i], 
                (PF_CLIP_EPSILON - prevVt->homogeneous[3]) / (input[i].homogeneous[3] - prevVt->homogeneous[3]));
        }

        if (currDot > 0)
        {
            polygon[(*vertexCounter)++] = input[i];
        }

        prevDot = currDot;
        prevVt = &input[i];
    }

    return *vertexCounter > 0;
}

static PFboolean Process_ClipPolygonXYZ(PFvertex* restrict polygon, PFubyte* restrict vertexCounter)
{
    for (PFubyte iAxis = 0; iAxis < 3; iAxis++)
    {
        if (*vertexCounter == 0) return PF_FALSE;

        PFvertex input[PF_MAX_CLIPPED_POLYGON_VERTICES];
        const PFvertex *prevVt;
        PFubyte inputCounter;
        PFbyte prevDot;

        // Clip against first plane

        memcpy(input, polygon, (*vertexCounter)*sizeof(PFvertex));
        inputCounter = *vertexCounter;
        *vertexCounter = 0;

        prevVt = &input[inputCounter-1];
        prevDot = (prevVt->homogeneous[iAxis] <= prevVt->homogeneous[3]) ? 1 : -1;

        for (int_fast8_t i = 0; i < inputCounter; i++)
        {
            PFbyte currDot = (input[i].homogeneous[iAxis] <= input[i].homogeneous[3]) ? 1 : -1;

            if (prevDot*currDot <= 0)
            {
                polygon[(*vertexCounter)++] = Helper_LerpVertex(prevVt, &input[i], (prevVt->homogeneous[3] - prevVt->homogeneous[iAxis]) /
                    ((prevVt->homogeneous[3] - prevVt->homogeneous[iAxis]) - (input[i].homogeneous[3] - input[i].homogeneous[iAxis])));
            }

            if (currDot > 0)
            {
                polygon[(*vertexCounter)++] = input[i];
            }

            prevDot = currDot;
            prevVt = &input[i];
        }

        if (*vertexCounter == 0) return PF_FALSE;

        // Clip against opposite plane

        memcpy(input, polygon, (*vertexCounter)*sizeof(PFvertex));
        inputCounter = *vertexCounter;
        *vertexCounter = 0;

        prevVt = &input[inputCounter-1];
        prevDot = (-prevVt->homogeneous[iAxis] <= prevVt->homogeneous[3]) ? 1 : -1;

        for (int_fast8_t i = 0; i < inputCounter; i++)
        {
            PFbyte currDot = (-input[i].homogeneous[iAxis] <= input[i].homogeneous[3]) ? 1 : -1;

            if (prevDot*currDot <= 0)
            {
                polygon[(*vertexCounter)++] = Helper_LerpVertex(prevVt, &input[i], (prevVt->homogeneous[3] + prevVt->homogeneous[iAxis]) /
                    ((prevVt->homogeneous[3] + prevVt->homogeneous[iAxis]) - (input[i].homogeneous[3] + input[i].homogeneous[iAxis])));
            }

            if (currDot > 0)
            {
                polygon[(*vertexCounter)++] = input[i];
            }

            prevDot = currDot;
            prevVt = &input[i];
        }
    }

    return *vertexCounter > 0;
}

static PFboolean Process_ProjectPoint(PFvertex* restrict v, const PFmat4f* mvp)
{
    memcpy(v->homogeneous, v->position, sizeof(PFvec3f));
    v->homogeneous[3] = 1.0f;

    pfVec4fTransform(v->homogeneous, v->homogeneous, mvp);

    if (v->homogeneous[3] != 1.0f)
    {
        PFfloat invW = 1.0f / v->homogeneous[3];
        v->homogeneous[0] *= invW;
        v->homogeneous[1] *= invW;
    }

    Process_HomogeneousToScreen(v);

    return v->screen[0] >= currentCtx->viewportX
        && v->screen[0] <= currentCtx->viewportW
        && v->screen[1] >= currentCtx->viewportY
        && v->screen[1] <= currentCtx->viewportH;
}


static void Process_ProjectAndClipLine(PFvertex* restrict line, PFubyte* restrict vertexCounter, const PFmat4f* mvp)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        PFvertex *v = line + i;

        memcpy(v->homogeneous, v->position, sizeof(PFvec3f));
        v->homogeneous[3] = 1.0f;

        pfVec4fTransform(v->homogeneous, v->homogeneous, mvp);
    }

    if (line[0].homogeneous[3] == 1.0f && line[1].homogeneous[3] == 1.0f)
    {
        Process_HomogeneousToScreen(&line[0]);
        Process_HomogeneousToScreen(&line[1]);

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

        Process_HomogeneousToScreen(&line[0]);
        Process_HomogeneousToScreen(&line[1]);
    }
}

static PFboolean Process_ProjectAndClipTriangle(PFvertex* restrict polygon, PFubyte* restrict vertexCounter, const PFmat4f* mvp)
{
    for (int_fast8_t i = 0; i < *vertexCounter; i++)
    {
        PFvertex *v = polygon + i;

        memcpy(v->homogeneous, v->position, sizeof(PFvec3f));
        v->homogeneous[3] = 1.0f;

        pfVec4fTransform(v->homogeneous, v->homogeneous, mvp);
    }

    PFboolean is2D = (
        polygon[0].homogeneous[3] == 1.0f &&
        polygon[1].homogeneous[3] == 1.0f &&
        polygon[2].homogeneous[3] == 1.0f);

    if (is2D)
    {
        for (int_fast8_t i = 0; i < *vertexCounter; i++)
        {
            Process_HomogeneousToScreen(&polygon[i]);
        }
    }
    else
    {
        if (Process_ClipPolygonW(polygon, vertexCounter) && Process_ClipPolygonXYZ(polygon, vertexCounter))
        {
            for (int_fast8_t i = 0; i < *vertexCounter; i++)
            {
                // Calculation of the reciprocal of Z for the perspective correct
                polygon[i].homogeneous[2] = 1.0f / polygon[i].homogeneous[2];

                // Division of texture coordinates by the Z axis (perspective correct)
                pfVec2fScale(polygon[i].texcoord, polygon[i].texcoord, polygon[i].homogeneous[2]);

                // Division of XY coordinates by weight (perspective correct)
                PFfloat invW = 1.0f / polygon[i].homogeneous[3];
                polygon[i].homogeneous[0] *= invW;
                polygon[i].homogeneous[1] *= invW;

                Process_HomogeneousToScreen(&polygon[i]);
            }
        }
    }

    return is2D;
}


/* Internal line rasterizer function definitions */

static void Rasterize_LineFlat(const PFvertex* v1, const PFvertex* v2)
{
    const PFfloat dx = v2->screen[0] - v1->screen[0];
    const PFfloat dy = v2->screen[1] - v1->screen[1];

    if (dx == 0 && dy == 0)
    {
        pfFramebufferSetPixel(currentCtx->currentFramebuffer, v1->screen[0], v1->screen[1], v1->color);
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
            pfFramebufferSetPixel(currentCtx->currentFramebuffer, x, y, Helper_LerpColor(v1->color, v2->color, t));
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
            pfFramebufferSetPixel(currentCtx->currentFramebuffer, x, y, Helper_LerpColor(v1->color, v2->color, t));
        }
    }
}

static void Rasterize_LineDepth(const PFvertex* v1, const PFvertex* v2)
{
    const PFfloat dx = v2->screen[0] - v1->screen[0];
    const PFfloat dy = v2->screen[1] - v1->screen[1];

    if (dx == 0 && dy == 0)
    {
        pfFramebufferSetPixelDepth(currentCtx->currentFramebuffer, v1->screen[0], v1->screen[1], v1->homogeneous[2], v1->color);
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
            pfFramebufferSetPixelDepth(currentCtx->currentFramebuffer, x, y, z, Helper_LerpColor(v1->color, v2->color, t));
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
            pfFramebufferSetPixelDepth(currentCtx->currentFramebuffer, x, y, z, Helper_LerpColor(v1->color, v2->color, t));
        }
    }
}


/* Internal triangle 2D rasterizer function definitions */

static void Rasterize_TriangleColorFlat2D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    // Define the origin barycentric coordinates
    // as well as the barycentric increment steps
    // and also the rendering area xMin, xMax, ...

    PF_PREPARE_TRIANGLE_FRONT_2D();

    const PFcolor emission = currentCtx->frontMaterial.emission;

    // The BEGIN_XXX_LOOP macro provides access to certain useful variables and constants, including:
    //      - `PFtexture texTarget`, which is the destination buffer
    //      - `PFcolor finalColor`, which stores the color that will be put
    //      - `const PFfloat aW1, aW2, aW3` allowing barycentric interpolation
    //      - `const PFfloat z`, which is the interpolated depth

    PF_BEGIN_TRIANGLE_FLAT_LOOP();
    {
        const PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
        const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);

        finalColor = pfBlendAdditive(currentCtx->blendFunction(srcCol, dstCol), emission);
    }
    PF_END_TRIANGLE_FLAT_LOOP();
}

static void Rasterize_TriangleColorDepth2D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    PF_PREPARE_TRIANGLE_FRONT_2D();

    const PFcolor emission = currentCtx->frontMaterial.emission;

    PF_BEGIN_TRIANGLE_DEPTH_LOOP();
    {
        const PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
        const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);

        finalColor = pfBlendAdditive(currentCtx->blendFunction(srcCol, dstCol), emission);
    }
    PF_END_TRIANGLE_DEPTH_LOOP();
}

static void Rasterize_TriangleTextureFlat2D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    PF_PREPARE_TRIANGLE_FRONT_2D();

    const PFcolor emission = currentCtx->frontMaterial.emission;

    PF_BEGIN_TRIANGLE_FLAT_LOOP();
    {
        PFvec2f texcoord = { 0 };
        Helper_InterpolateVec2f(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
        PFcolor texel = pfTextureGetFragment(currentCtx->currentTexture, texcoord[0], texcoord[1]);

        const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);
        PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
        srcCol = pfBlendMultiplicative(texel, srcCol);

        finalColor = pfBlendAdditive(currentCtx->blendFunction(srcCol, dstCol), emission);
    }
    PF_END_TRIANGLE_FLAT_LOOP();
}

static void Rasterize_TriangleTextureDepth2D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    PF_PREPARE_TRIANGLE_FRONT_2D();

    const PFcolor emission = currentCtx->frontMaterial.emission;

    PF_BEGIN_TRIANGLE_DEPTH_LOOP();
    {
        PFvec2f texcoord = { 0 };
        Helper_InterpolateVec2f(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
        PFcolor texel = pfTextureGetFragment(currentCtx->currentTexture, texcoord[0], texcoord[1]);

        const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);
        PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
        srcCol = pfBlendMultiplicative(texel, srcCol);

        finalColor = pfBlendAdditive(currentCtx->blendFunction(srcCol, dstCol), emission);
    }
    PF_END_TRIANGLE_DEPTH_LOOP();
}


/* Internal front triangle 3D rasterizer function definitions */

static void Rasterize_TriangleColorFlat3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    PFboolean faceCullingDisabled = !(currentCtx->state & PF_CULL_FACE);

    if (faceCullingDisabled || currentCtx->cullFace == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        const PFcolor emission = currentCtx->frontMaterial.emission;

        PF_BEGIN_TRIANGLE_FLAT_LOOP();
        {
            const PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);

            finalColor = pfBlendAdditive(currentCtx->blendFunction(srcCol, dstCol), emission);
        }
        PF_END_TRIANGLE_FLAT_LOOP();
    }

    if (faceCullingDisabled || currentCtx->cullFace == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        const PFcolor emission = currentCtx->backMaterial.emission;

        PF_BEGIN_TRIANGLE_FLAT_LOOP();
        {
            const PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);

            finalColor = pfBlendAdditive(currentCtx->blendFunction(srcCol, dstCol), emission);
        }
        PF_END_TRIANGLE_FLAT_LOOP();
    }
}

static void Rasterize_TriangleColorDepth3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    PFboolean faceCullingDisabled = !(currentCtx->state & PF_CULL_FACE);

    if (faceCullingDisabled || currentCtx->cullFace == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        const PFcolor emission = currentCtx->frontMaterial.emission;

        PF_BEGIN_TRIANGLE_DEPTH_LOOP();
        {
            const PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);

            finalColor = pfBlendAdditive(currentCtx->blendFunction(srcCol, dstCol), emission);
        }
        PF_END_TRIANGLE_DEPTH_LOOP();
    }

    if (faceCullingDisabled || currentCtx->cullFace == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        const PFcolor emission = currentCtx->backMaterial.emission;

        PF_BEGIN_TRIANGLE_DEPTH_LOOP();
        {
            const PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);

            finalColor = pfBlendAdditive(currentCtx->blendFunction(srcCol, dstCol), emission);
        }
        PF_END_TRIANGLE_DEPTH_LOOP();
    }
}

static void Rasterize_TriangleTextureFlat3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    PFboolean faceCullingDisabled = !(currentCtx->state & PF_CULL_FACE);

    if (faceCullingDisabled || currentCtx->cullFace == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        const PFcolor emission = currentCtx->frontMaterial.emission;

        PF_BEGIN_TRIANGLE_FLAT_LOOP();
        {
            PFvec2f texcoord;
            Helper_InterpolateVec2f(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            const PFcolor texel = pfTextureGetFragment(currentCtx->currentTexture, texcoord[0], texcoord[1]);
            PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            srcCol = pfBlendMultiplicative(texel, srcCol);

            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);
            finalColor = pfBlendAdditive(currentCtx->blendFunction(srcCol, dstCol), emission);
        }
        PF_END_TRIANGLE_FLAT_LOOP();
    }

    if (faceCullingDisabled || currentCtx->cullFace == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        const PFcolor emission = currentCtx->backMaterial.emission;

        PF_BEGIN_TRIANGLE_FLAT_LOOP();
        {
            PFvec2f texcoord;
            Helper_InterpolateVec2f(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            const PFcolor texel = pfTextureGetFragment(currentCtx->currentTexture, texcoord[0], texcoord[1]);
            PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            srcCol = pfBlendMultiplicative(texel, srcCol);

            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);
            finalColor = pfBlendAdditive(currentCtx->blendFunction(srcCol, dstCol), emission);
        }
        PF_END_TRIANGLE_FLAT_LOOP();
    }
}

static void Rasterize_TriangleTextureDepth3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    PFboolean faceCullingDisabled = !(currentCtx->state & PF_CULL_FACE);

    if (faceCullingDisabled || currentCtx->cullFace == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        const PFcolor emission = currentCtx->frontMaterial.emission;

        PF_BEGIN_TRIANGLE_DEPTH_LOOP();
        {
            PFvec2f texcoord;
            Helper_InterpolateVec2f(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            PFcolor texel = pfTextureGetFragment(currentCtx->currentTexture, texcoord[0], texcoord[1]);
            srcCol = pfBlendMultiplicative(texel, srcCol);

            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);
            finalColor = pfBlendAdditive(currentCtx->blendFunction(srcCol, dstCol), emission);
        }
        PF_END_TRIANGLE_DEPTH_LOOP();
    }

    if (faceCullingDisabled || currentCtx->cullFace == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        const PFcolor emission = currentCtx->backMaterial.emission;

        PF_BEGIN_TRIANGLE_DEPTH_LOOP();
        {
            PFvec2f texcoord;
            Helper_InterpolateVec2f(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            PFcolor texel = pfTextureGetFragment(currentCtx->currentTexture, texcoord[0], texcoord[1]);
            srcCol = pfBlendMultiplicative(texel, srcCol);

            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);
            finalColor = pfBlendAdditive(currentCtx->blendFunction(srcCol, dstCol), emission);
        }
        PF_END_TRIANGLE_DEPTH_LOOP();
    }
}


/* Internal lighting process functions defintions */

static PFcolor Light_Compute(const PFlight* light, PFcolor ambient, PFcolor texel, const PFvec3f viewPos, const PFvec3f vertex, const PFvec3f normal)
{
    // Calculate view direction vector
    PFvec3f viewDir;
    pfVec3fSub(viewDir, viewPos, vertex);
    pfVec3fNormalize(viewDir, viewDir);

    // Compute ambient lighting contribution
    ambient = pfBlendMultiplicative(texel, ambient);

    // Compute diffuse lighting
    const PFfloat intensity = fmaxf(-pfVec3fDot(light->direction, normal), 0.0f);

    const PFcolor diffuse = {
        (PFubyte)(light->diffuse.r * intensity),
        (PFubyte)(light->diffuse.g * intensity),
        (PFubyte)(light->diffuse.b * intensity),
        255
    };

#ifdef PF_NO_BLINN_PHONG
    PFvec3f reflectDir;
    pfVec3fReflect(reflectDir, light->direction, normal);
    const PFfloat spec = powf(fmaxf(pfVec3fDot(reflectDir, viewDir), 0.0f),
                              currentCtx->frontMaterial.shininess);
#else
    // To work here we take the opposite direction
    PFvec3f negLightDir;
    pfVec3fNeg(negLightDir, light->direction);

    // Compute half vector (Blinn vector)
    PFvec3f halfVec;
    pfVec3fAdd(halfVec, negLightDir, viewDir);
    pfVec3fNormalize(halfVec, halfVec);

    // Compute specular term using half vector
    const PFfloat spec = powf(fmaxf(pfVec3fDot(normal, halfVec), 0.0f),
                              currentCtx->frontMaterial.shininess);
#endif

    const PFcolor specular = {
        (PFubyte)(light->specular.r * spec),
        (PFubyte)(light->specular.g * spec),
        (PFubyte)(light->specular.b * spec),
        255
    };

    // Combine ambient, diffuse, and specular components
    PFcolor finalColor = pfBlendMultiplicative(texel, diffuse);
    finalColor = pfBlendAdditive(finalColor, specular);
    return pfBlendAdditive(finalColor, ambient);
}


/* Internal enlightened triangle 3D rasterizer function definitions */

static void Rasterize_TriangleColorFlatLight3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFvec3f viewPos)
{
    PFboolean faceCullingDisabled = !(currentCtx->state & PF_CULL_FACE);

    if (faceCullingDisabled || currentCtx->cullFace == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        const PFcolor emission = currentCtx->frontMaterial.emission;

        // The 'PF_BEGIN_XXX_LIGHT_LOOP' macro additionally provides access to:
        //  - `const PFlight *light` which is the active light for the currently rendered pixel
        //  - `const PFcolor ambient` which is the ambient color of the light multiplied by that of the active material

        PF_BEGIN_TRIANGLE_FLAT_LIGHT_LOOP(currentCtx->frontMaterial);
        {
            const PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);

            PFvec3f normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, currentCtx->blendFunction(srcCol, dstCol), viewPos, vertex, normal);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_FLAT_LIGHT_LOOP();
    }

    if (faceCullingDisabled || currentCtx->cullFace == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        const PFcolor emission = currentCtx->backMaterial.emission;

        // The 'PF_BEGIN_XXX_LIGHT_LOOP' macro additionally provides access to:
        //  - `const PFlight *light` which is the active light for the currently rendered pixel
        //  - `const PFcolor ambient` which is the ambient color of the light multiplied by that of the active material

        PF_BEGIN_TRIANGLE_FLAT_LIGHT_LOOP(currentCtx->backMaterial);
        {
            const PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);

            PFvec3f normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, currentCtx->blendFunction(srcCol, dstCol), viewPos, vertex, normal);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_FLAT_LIGHT_LOOP();
    }
}

static void Rasterize_TriangleColorDepthLight3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFvec3f viewPos)
{
    PFboolean faceCullingDisabled = !(currentCtx->state & PF_CULL_FACE);

    if (faceCullingDisabled || currentCtx->cullFace == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        const PFcolor emission = currentCtx->frontMaterial.emission;

        PF_BEGIN_TRIANGLE_DEPTH_LIGHT_LOOP(currentCtx->frontMaterial);
        {
            const PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);

            PFvec3f normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, currentCtx->blendFunction(srcCol, dstCol), viewPos, vertex, normal);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_DEPTH_LIGHT_LOOP();
    }

    if (faceCullingDisabled || currentCtx->cullFace == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        const PFcolor emission = currentCtx->backMaterial.emission;

        PF_BEGIN_TRIANGLE_DEPTH_LIGHT_LOOP(currentCtx->backMaterial);
        {
            const PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);

            PFvec3f normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, currentCtx->blendFunction(srcCol, dstCol), viewPos, vertex, normal);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_DEPTH_LIGHT_LOOP();
    }
}

static void Rasterize_TriangleTextureFlatLight3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFvec3f viewPos)
{
    PFboolean faceCullingDisabled = !(currentCtx->state & PF_CULL_FACE);

    if (faceCullingDisabled || currentCtx->cullFace == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        const PFcolor emission = currentCtx->frontMaterial.emission;

        PF_BEGIN_TRIANGLE_FLAT_LIGHT_LOOP(currentCtx->frontMaterial);
        {
            PFvec2f texcoord;
            Helper_InterpolateVec2f(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            const PFcolor texel = pfTextureGetFragment(currentCtx->currentTexture, texcoord[0], texcoord[1]);
            PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            srcCol = pfBlendMultiplicative(texel, srcCol);

            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);

            PFvec3f normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, currentCtx->blendFunction(srcCol, dstCol), viewPos, vertex, normal);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_FLAT_LIGHT_LOOP();
    }

    if (faceCullingDisabled || currentCtx->cullFace == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        const PFcolor emission = currentCtx->backMaterial.emission;

        PF_BEGIN_TRIANGLE_FLAT_LIGHT_LOOP(currentCtx->backMaterial);
        {
            PFvec2f texcoord;
            Helper_InterpolateVec2f(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            const PFcolor texel = pfTextureGetFragment(currentCtx->currentTexture, texcoord[0], texcoord[1]);
            PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            srcCol = pfBlendMultiplicative(texel, srcCol);

            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);

            PFvec3f normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, currentCtx->blendFunction(srcCol, dstCol), viewPos, vertex, normal);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_FLAT_LIGHT_LOOP();
    }
}

static void Rasterize_TriangleTextureDepthLight3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFvec3f viewPos)
{
    PFboolean faceCullingDisabled = !(currentCtx->state & PF_CULL_FACE);

    if (faceCullingDisabled || currentCtx->cullFace == PF_BACK)
    {
        PF_PREPARE_TRIANGLE_FRONT_3D();

        const PFcolor emission = currentCtx->frontMaterial.emission;

        PF_BEGIN_TRIANGLE_DEPTH_LIGHT_LOOP(currentCtx->frontMaterial);
        {
            PFvec2f texcoord;
            Helper_InterpolateVec2f(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            const PFcolor texel = pfTextureGetFragment(currentCtx->currentTexture, texcoord[0], texcoord[1]);
            PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            srcCol = pfBlendMultiplicative(texel, srcCol);

            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);

            PFvec3f normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, currentCtx->blendFunction(srcCol, dstCol), viewPos, vertex, normal);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_DEPTH_LIGHT_LOOP();
    }

    if (faceCullingDisabled || currentCtx->cullFace == PF_FRONT)
    {
        PF_PREPARE_TRIANGLE_BACK_3D();

        const PFcolor emission = currentCtx->backMaterial.emission;

        PF_BEGIN_TRIANGLE_DEPTH_LIGHT_LOOP(currentCtx->backMaterial);
        {
            PFvec2f texcoord;
            Helper_InterpolateVec2f(texcoord, v1->texcoord, v2->texcoord, v3->texcoord, aW1, aW2, aW3);
            texcoord[0] *= z, texcoord[1] *= z; // Perspective correct

            const PFcolor texel = pfTextureGetFragment(currentCtx->currentTexture, texcoord[0], texcoord[1]);
            PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
            srcCol = pfBlendMultiplicative(texel, srcCol);

            const PFcolor dstCol = texTarget->pixelGetter(texTarget->pixels, xyOffset);

            PFvec3f normal, vertex;
            Helper_InterpolateVec3f(normal, v1->normal, v2->normal, v3->normal, aW1, aW2, aW3);
            Helper_InterpolateVec3f(vertex, v1->position, v2->position, v3->position, aW1, aW2, aW3);

            finalColor = Light_Compute(light, ambient, currentCtx->blendFunction(srcCol, dstCol), viewPos, vertex, normal);
            finalColor = pfBlendAdditive(finalColor, emission);
        }
        PF_END_TRIANGLE_DEPTH_LIGHT_LOOP();
    }
}


/* Internal processing and rasterization function definitions */

void ProcessRasterize(const PFmat4f* mvp, const PFmat4f* matNormal)
{
    switch (currentCtx->currentDrawMode)
    {
        case PF_POINTS:
        {
            PFvertex *processed = currentCtx->vertexBuffer;

            // Process point

            if (!Process_ProjectPoint(processed, mvp))
            {
                return;
            }

            // Draw point

            if (currentCtx->state & (PF_DEPTH_TEST))
            {
                pfFramebufferSetPixelDepth(
                    currentCtx->currentFramebuffer,
                    processed->screen[0],
                    processed->screen[1],
                    processed->homogeneous[2],
                    processed->color);
            }
            else
            {
                pfFramebufferSetPixel(
                    currentCtx->currentFramebuffer,
                    processed->screen[0],
                    processed->screen[1],
                    processed->color);
            }
        }
        break;

        case PF_LINES:
        {
            // Process vertices

            PFubyte processedCounter = 2;

            PFvertex processed[2] = {
                currentCtx->vertexBuffer[0],
                currentCtx->vertexBuffer[1]
            };

            Process_ProjectAndClipLine(processed, &processedCounter, mvp);
            if (processedCounter != 2) break;

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
        break;

        case PF_TRIANGLES:
        {
            PFubyte processedCounter = 3;

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
                    pfVec3fTransform(processed[i].normal, processed[i].normal, matNormal);
                    pfVec3fNormalize(processed[i].normal, processed[i].normal);
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
            if (processedCounter < 3) break;

            // Rasterize triangles

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

                for (PFbyte i = 0; i < processedCounter - 2; i++)
                {
                    rasterizer(&processed[0], &processed[i + 1], &processed[i + 2]);
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

                    PFmat4f invMV = pfMat4fInvert(&currentCtx->modelview);
                    PFvec3f viewPos = { invMV.m12, invMV.m13, invMV.m14 };

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

                    for (PFbyte i = 0; i < processedCounter - 2; i++)
                    {
                        rasterizer(&processed[0], &processed[i + 1], &processed[i + 2], viewPos);
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

                    for (PFbyte i = 0; i < processedCounter - 2; i++)
                    {
                        rasterizer(&processed[0], &processed[i + 1], &processed[i + 2]);
                    }
                }
            }
        }
        break;

        case PF_QUADS:
        {
            for (PFint i = 0; i < 2; i++)
            {
                PFubyte processedCounter = 3;

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
                        pfVec3fTransform(processed[j].normal, processed[j].normal, matNormal);
                        pfVec3fNormalize(processed[j].normal, processed[j].normal);
                    }
                }

                // Multiply vertices color with material diffuse

                for (PFubyte j = 0; j < processedCounter; j++)
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

                    for (PFbyte j = 0; j < processedCounter - 2; j++)
                    {
                        rasterizer(&processed[0], &processed[j + 1], &processed[j + 2]);
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

                        PFmat4f invMV = pfMat4fInvert(&currentCtx->modelview);
                        PFvec3f viewPos = { invMV.m12, invMV.m13, invMV.m14 };

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

                        for (PFbyte j = 0; j < processedCounter - 2; j++)
                        {
                            rasterizer(&processed[0], &processed[j + 1], &processed[j + 2], viewPos);
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

                        for (PFbyte j = 0; j < processedCounter - 2; j++)
                        {
                            rasterizer(&processed[0], &processed[j + 1], &processed[j + 2]);
                        }
                    }
                }
            }
        }
        break;
    }
}
