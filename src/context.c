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
#include "internal/config.h"
#include "internal/pixel.h"
#include "internal/blend.h"
#include "internal/depth.h"
#include "internal/simd.h"
#include "pixelforge.h"
#include "pfm.h"

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <float.h>

/* Current thread local-thread definition (declared in context.h) */

PF_CTX_DECL PFctx *G_currentCtx = NULL;

/* Some helper functions */

static void pfiResetVertexBufferForNextElement()
{
    switch (G_currentCtx->currentDrawMode) {
        case PF_TRIANGLE_FAN:
        case PF_TRIANGLE_STRIP:
            G_currentCtx->vertexCounter = 1;
            G_currentCtx->vertexBuffer[0] = G_currentCtx->vertexBuffer[3];
            break;
        case PF_QUAD_FAN:
        case PF_QUAD_STRIP:
            G_currentCtx->vertexCounter = 2;
            G_currentCtx->vertexBuffer[0] = G_currentCtx->vertexBuffer[4];
            G_currentCtx->vertexBuffer[1] = G_currentCtx->vertexBuffer[5];
            break;
        default:
            G_currentCtx->vertexCounter = 0;
            break;
    }
}

static PFsizei pfiGetDrawModeVertexCount(PFdrawmode mode)
{
    switch (mode) {
        case PF_POINTS:         return 1;
        case PF_LINES:          return 2;
        case PF_TRIANGLES:      return 3;
        case PF_TRIANGLE_FAN:
        case PF_TRIANGLE_STRIP: return 4;
        case PF_QUADS:          return 4;
        case PF_QUAD_FAN:
        case PF_QUAD_STRIP:     return 6;
    }
    return 0;
}

static PFsizei pfiGetDataTypeSize(PFdatatype type)
{
    switch (type) {
        case PF_UNSIGNED_BYTE:          return sizeof(PFubyte);
        case PF_UNSIGNED_SHORT_5_6_5:
        case PF_UNSIGNED_SHORT_5_5_5_1:
        case PF_UNSIGNED_SHORT_4_4_4_4:
        case PF_UNSIGNED_SHORT:         return sizeof(PFushort);
        case PF_UNSIGNED_INT:           return sizeof(PFuint);
        case PF_BYTE:                   return sizeof(PFbyte);
        case PF_HALF_FLOAT:
        case PF_SHORT:                  return sizeof(PFshort);
        case PF_INT:                    return sizeof(PFint);
        case PF_FLOAT:                  return sizeof(PFfloat);
        case PF_DOUBLE:                 return sizeof(PFdouble);
    }
    return 0;
}

// NOTE: Only updates the MVP by default but also updates the normal matrix if necessary
static void pfiUpdateMatrices(PFboolean matNormal)
{
    if (G_currentCtx->modelMatrixUsed) {
        pfmMat4MulR(G_currentCtx->matMVP, G_currentCtx->matModel, G_currentCtx->matView);
        pfmMat4Mul(G_currentCtx->matMVP, G_currentCtx->matMVP, G_currentCtx->matProjection);
        if (matNormal && G_currentCtx->state & PF_LIGHTING) {
            pfmMat4Invert(G_currentCtx->matNormal, G_currentCtx->matModel);
            pfmMat4Transpose(G_currentCtx->matNormal, G_currentCtx->matNormal);
        }
    } else {
        pfmMat4MulR(G_currentCtx->matMVP, G_currentCtx->matView, G_currentCtx->matProjection);
        if (matNormal && G_currentCtx->state & PF_LIGHTING) {
            pfmMat4Identity(G_currentCtx->matNormal);
        }
    }
}

/* Context API functions */

PFcontext pfCreateContext(void* targetBuffer, PFsizei width, PFsizei height, PFpixelformat format, PFdatatype type)
{
    /* Memory allocation for the context */

    PFctx *ctx = (PFctx*)PF_MALLOC(sizeof(PFctx));
    if (!ctx) return NULL;

    /* Initialization of the main framebuffer */

    ctx->mainFramebuffer = (PFframebuffer) { 0 };
    ctx->mainFramebuffer.texture = pfGenTexture(targetBuffer, width, height, format, type);

    const PFsizei bufferSize = width*height;
    ctx->mainFramebuffer.zbuffer = (PFfloat*)PF_MALLOC(bufferSize * sizeof(PFfloat));

    if (!ctx->mainFramebuffer.zbuffer) {
        PF_FREE(ctx);
        return NULL;
    }

    /* Initialization of the z-buffer with maximum values */

    for (PFsizei i = 0; i < bufferSize; i++) {
        ctx->mainFramebuffer.zbuffer[i] = FLT_MAX;
    }

    /* Definition of the current framebuffer */

    ctx->currentFramebuffer = &ctx->mainFramebuffer;

    /* Initialization of viewport members */

    ctx->vpPos[0] = ctx->vpMin[0] = 0;
    ctx->vpPos[1] = ctx->vpMin[1] = 0;
    ctx->vpDim[0] = ctx->vpMax[0] = width - 1;
    ctx->vpDim[1] = ctx->vpMax[1] = height - 1;

    /* Initialization of default rendering members */

    ctx->currentDrawMode = 0;

    ctx->blendFunction = pfiBlendAlpha;
    ctx->depthFunction = pfiDepthTest_LT;

#if PF_SIMD_SUPPORT
    ctx->depthSimdFunction = pfiDepthTest_LT_simd;
    ctx->blendSimdFunction = pfiBlendAlpha_simd;
#endif //PF_SIMD_SUPPORT

    ctx->clearColor = (PFcolor) { 0 };
    ctx->clearDepth = FLT_MAX;

    ctx->pointSize = 1.0f;
    ctx->lineWidth = 1.0f;

    ctx->polygonMode[0] = PF_FILL;
    ctx->polygonMode[1] = PF_FILL;

    /* Initialization of vertex attributes */

    memset(ctx->currentNormal, 0, sizeof(PFMvec3));
    memset(ctx->currentTexcoord, 0, sizeof(PFMvec2));
    ctx->currentColor = (PFcolor) { 255, 255, 255, 255 };
    ctx->vertexCounter = 0;

    /* Initialization of raster position */

    memset(ctx->rasterPos, 0, sizeof(PFMvec4));
    ctx->pixelZoom[0] = ctx->pixelZoom[1] = 1.0f;

    /* Initialization of lights */

    for (PFsizei i = 0; i < PF_MAX_LIGHT_STACK; i++) {
        ctx->lights[i] = (PFlight) {
            .position = { 0 },
            .direction = { 0 },
            .innerCutOff = M_PI,
            .outerCutOff = M_PI,
            .attConstant = 1,
            .attLinear = 0,
            .attQuadratic = 0,
            .ambient = (PFcolor) { 51, 51, 51, 255 },
            .diffuse = (PFcolor) { 255, 255, 255, 255 },
            .specular = (PFcolor) { 255, 255, 255, 255 },
            .next = NULL
        };
    }
    ctx->activeLights = NULL;

    /* Initialization of fog properties */

    ctx->fog = (PFfog) {
        .mode = PF_LINEAR,
        .density = 1.0f,
        .start = 0.0f,
        .end = 1.0f,
        .color = (PFcolor) { 0 }
    };

    /* Initialization of materials */

    ctx->faceMaterial[0] = ctx->faceMaterial[1] = (PFmaterial) {
        .ambient = (PFcolor) { 255, 255, 255, 255 },
        .diffuse = (PFcolor) { 255, 255, 255, 255 },
        .specular = (PFcolor) { 255, 255, 255, 255 },
        .emission = (PFcolor) { 0, 0, 0, 255 },
#ifdef PF_PHONG_REFLECTION
        .shininess = 16.0f,
#else
        .shininess = 64.0f,
#endif
    };

    ctx->materialColorFollowing = (PFmatcolfollowing) {
        .face = PF_FRONT_AND_BACK,
        .mode = PF_AMBIENT_AND_DIFFUSE
    };

    /* Initialization of default matrices */

    ctx->currentMatrixMode = PF_MODELVIEW;
    ctx->currentMatrix = &ctx->matView;

    pfmMat4Ortho(ctx->matProjection, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    pfmMat4Identity(ctx->matTexture);
    pfmMat4Identity(ctx->matNormal);
    pfmMat4Identity(ctx->matModel);
    pfmMat4Identity(ctx->matView);

    /* Initialization of matrix stack counters */

    ctx->modelMatrixUsed = PF_FALSE;
    ctx->stackProjectionCounter = 0;
    ctx->stackModelviewCounter = 0;
    ctx->stackTextureCounter = 0;

    /* Initialization of vertex and texture attributes */

    ctx->vertexAttribs = (PFvertexattribs) { 0 };
    ctx->currentTexture = NULL;

    /* Initialization of the context state */

    ctx->state = 0x00;
    ctx->state |= PF_CULL_FACE;
    ctx->shadingMode = PF_SMOOTH;
    ctx->cullFace = PF_BACK;
    ctx->errCode = PF_NO_ERROR;

    return ctx;
}

void pfDeleteContext(PFcontext ctx)
{
    if (ctx) {
        if (((PFctx*)ctx)->mainFramebuffer.zbuffer) {
            PF_FREE(((PFctx*)ctx)->mainFramebuffer.zbuffer);
            ((PFctx*)ctx)->mainFramebuffer = (PFframebuffer) { 0 };
        }
        PF_FREE(ctx);
    }
}

void pfSetMainBuffer(void* targetBuffer, PFsizei width, PFsizei height, PFpixelformat format, PFdatatype type)
{
    /* Check if targetBuffer, width, or height is invalid */

    if (targetBuffer == NULL || width == 0 || height == 0) {
        G_currentCtx->errCode = PF_INVALID_VALUE;
        return;
    }

    /* Store the old width and height of the main framebuffer */

    PFsizei oldW = ((struct PFtex*)G_currentCtx->mainFramebuffer.texture)->w;
    PFsizei oldH = ((struct PFtex*)G_currentCtx->mainFramebuffer.texture)->h;

    /* Check if the dimensions of the new buffer differ from the old one */

    if (oldW != width || oldH != height) {
        // Calculate the new buffer size
        const PFsizei bufferSize = width*height;

        // Reallocate memory for the z-buffer
        PFfloat *zbuffer = (PFfloat*)PF_REALLOC(G_currentCtx->mainFramebuffer.zbuffer, bufferSize);

        // Check if reallocation failed
        if (zbuffer == NULL) {
            G_currentCtx->errCode = PF_ERROR_OUT_OF_MEMORY;
            return;
        }

        // Initialize the new areas of the z-buffer
        if (width > oldW) {
            for (PFsizei y = 0; y < height; y++) {
                for (PFsizei x = oldW; x < width; x++) {
                    zbuffer[y * width + x] = G_currentCtx->clearDepth;
                }
            }
        }

        if (height > oldH) {
            const PFsizei xMax = (oldW > width) ? oldW : width;
            for (PFsizei y = oldH; y < height; y++) {
                for (PFsizei x = 0; x < xMax; x++) {
                    zbuffer[y * width + x] = G_currentCtx->clearDepth;
                }
            }
        }

        // Update the z-buffer pointer
        G_currentCtx->mainFramebuffer.zbuffer = zbuffer;
    }

    /* Generate the new texture for the main framebuffer */

    G_currentCtx->mainFramebuffer.texture = pfGenTexture(targetBuffer, width, height, format, type);

    /* Reset the auxiliary framebuffer, it needs to be redefined after changing the main buffer */

    G_currentCtx->auxFramebuffer = NULL;
}


PF_API void pfSetAuxBuffer(void *auxFramebuffer)
{
    G_currentCtx->auxFramebuffer = auxFramebuffer;
}

PF_API void pfSwapBuffers(void)
{
    if (G_currentCtx->auxFramebuffer == NULL) {
        G_currentCtx->errCode = PF_INVALID_OPERATION;
        return;
    }

    struct PFtex* tex = G_currentCtx->currentFramebuffer->texture;

    void *tmp = tex->pixels;
    tex->pixels = G_currentCtx->auxFramebuffer;
    G_currentCtx->auxFramebuffer = tmp;
}

PFcontext pfGetCurrentContext(void)
{
    return G_currentCtx;
}

void pfMakeCurrent(PFcontext ctx)
{
    G_currentCtx = ctx;
}

PFboolean pfIsEnabled(PFstate state)
{
    return G_currentCtx->state & state;
}

void pfEnable(PFstate state)
{
    G_currentCtx->state |= state;

    if (state & PF_FRAMEBUFFER) {
        G_currentCtx->currentFramebuffer = G_currentCtx->bindedFramebuffer
            ? G_currentCtx->bindedFramebuffer : &G_currentCtx->mainFramebuffer;
    }
}

void pfDisable(PFstate state)
{
    G_currentCtx->state &= ~state;

    if (state & PF_FRAMEBUFFER) {
        G_currentCtx->currentFramebuffer = &G_currentCtx->mainFramebuffer;
    }
}


/* Getter API functions (see also 'getter.c') */

PFerrcode pfGetError(void)
{
    PFerrcode errCode = G_currentCtx->errCode;
    G_currentCtx->errCode = PF_NO_ERROR;
    return errCode;
}


/* Matrix management API functions */

// REVIEW: Returned errors when calling matrix functions between pfBegin and pfEnd (?)
//         Because the internal matrices are only updated upon calling pfBegin,
//         but the user can still attempt to include pfiUpdateMatrices
//         on their side, although the usefulness is questionable...

void pfMatrixMode(PFmatrixmode mode)
{
    switch (mode) {
        case PF_PROJECTION:
            G_currentCtx->currentMatrix = &G_currentCtx->matProjection;
            break;
        case PF_MODELVIEW:
            G_currentCtx->currentMatrix = G_currentCtx->modelMatrixUsed
                ? &G_currentCtx->matModel : &G_currentCtx->matView;
            break;
        case PF_TEXTURE:
            G_currentCtx->currentMatrix = &G_currentCtx->matTexture;
            break;
        default:
            G_currentCtx->errCode = PF_INVALID_ENUM;
            return;
    }

    G_currentCtx->currentMatrixMode = mode;
}

void pfPushMatrix(void)
{
    switch (G_currentCtx->currentMatrixMode) {
        case PF_PROJECTION: {
            if (G_currentCtx->stackProjectionCounter >= PF_MAX_PROJECTION_STACK_SIZE) {
                G_currentCtx->errCode = PF_STACK_OVERFLOW;
                return;
            }
            pfmMat4Copy(G_currentCtx->stackProjection[G_currentCtx->stackProjectionCounter], G_currentCtx->matProjection);
            G_currentCtx->stackProjectionCounter++;
        }
        break;

        case PF_MODELVIEW: {
            if (G_currentCtx->stackModelviewCounter >= PF_MAX_MODELVIEW_STACK_SIZE) {
                G_currentCtx->errCode = PF_STACK_OVERFLOW;
                return;
            }
            if (G_currentCtx->modelMatrixUsed) {
                pfmMat4Copy(G_currentCtx->stackModelview[G_currentCtx->stackModelviewCounter], G_currentCtx->matModel);
                G_currentCtx->stackModelviewCounter++;
            } else {
                G_currentCtx->currentMatrix = &G_currentCtx->matModel;
                G_currentCtx->modelMatrixUsed = PF_TRUE;
            }
        }
        break;

        case PF_TEXTURE: {
            if (G_currentCtx->stackTextureCounter >= PF_MAX_TEXTURE_STACK_SIZE) {
                G_currentCtx->errCode = PF_STACK_OVERFLOW;
                return;
            }
            pfmMat4Copy(G_currentCtx->stackTexture[G_currentCtx->stackTextureCounter], G_currentCtx->matTexture);
            G_currentCtx->stackTextureCounter++;
        }
        break;
    }
}

void pfPopMatrix(void)
{
    switch (G_currentCtx->currentMatrixMode) {
        case PF_PROJECTION: {
            if (G_currentCtx->stackProjectionCounter <= 0) {
                G_currentCtx->errCode = PF_STACK_UNDERFLOW;
                return;
            }
            G_currentCtx->stackProjectionCounter--;
            pfmMat4Copy(G_currentCtx->matProjection, G_currentCtx->stackProjection[G_currentCtx->stackProjectionCounter]);
        }
        break;

        case PF_MODELVIEW: {
            if (G_currentCtx->stackModelviewCounter == 0)  {
                if (!G_currentCtx->modelMatrixUsed) {
                    G_currentCtx->errCode = PF_STACK_UNDERFLOW;
                    return;
                }
                pfmMat4Identity(G_currentCtx->matModel);
                G_currentCtx->modelMatrixUsed = PF_FALSE;
                G_currentCtx->currentMatrix = &G_currentCtx->matView;
            } else {
                G_currentCtx->stackModelviewCounter--;
                pfmMat4Copy(G_currentCtx->matModel, G_currentCtx->stackModelview[G_currentCtx->stackModelviewCounter]);
            }
        }
        break;

        case PF_TEXTURE: {
            if (G_currentCtx->stackTextureCounter <= 0) {
                G_currentCtx->errCode = PF_STACK_UNDERFLOW;
                return;
            }
            G_currentCtx->stackTextureCounter--;
            pfmMat4Copy(G_currentCtx->matTexture, G_currentCtx->stackTexture[G_currentCtx->stackTextureCounter]);
        }
        break;
    }
}

void pfLoadIdentity(void)
{
    pfmMat4Identity(*G_currentCtx->currentMatrix);
}

void pfTranslatef(PFfloat x, PFfloat y, PFfloat z)
{
    PFMmat4 translation;
    pfmMat4Translate(translation, x, y, z);

    // NOTE: We transpose matrix with multiplication order
    pfmMat4Mul(*G_currentCtx->currentMatrix, translation, *G_currentCtx->currentMatrix);
}

void pfRotatef(PFfloat angle, PFfloat x, PFfloat y, PFfloat z)
{
    PFMvec3 axis = { x, y, z }; // TODO: review

    PFMmat4 rotation;
    pfmMat4Rotate(rotation, axis, DEG2RAD(angle));

    // NOTE: We transpose matrix with multiplication order
    pfmMat4Mul(*G_currentCtx->currentMatrix, rotation, *G_currentCtx->currentMatrix);
}

void pfScalef(PFfloat x, PFfloat y, PFfloat z)
{
    PFMmat4 scale;
    pfmMat4Scale(scale, x, y, z);

    // NOTE: We transpose matrix with multiplication order
    pfmMat4Mul(*G_currentCtx->currentMatrix, scale, *G_currentCtx->currentMatrix);
}

void pfMultMatrixf(const PFfloat* mat)
{
    pfmMat4Mul(*G_currentCtx->currentMatrix, *G_currentCtx->currentMatrix, mat);
}

void pfFrustum(PFdouble left, PFdouble right, PFdouble bottom, PFdouble top, PFdouble znear, PFdouble zfar)
{
    PFMmat4 frustum;
    pfmMat4Frustum(frustum, left, right, bottom, top, znear, zfar);
    pfmMat4Mul(*G_currentCtx->currentMatrix, *G_currentCtx->currentMatrix, frustum);
}

void pfOrtho(PFdouble left, PFdouble right, PFdouble bottom, PFdouble top, PFdouble znear, PFdouble zfar)
{
    PFMmat4 ortho;
    pfmMat4Ortho(ortho, left, right, bottom, top, znear, zfar);
    pfmMat4Mul(*G_currentCtx->currentMatrix, *G_currentCtx->currentMatrix, ortho);
}


/* Render configuration API functions */

void pfViewport(PFint x, PFint y, PFsizei width, PFsizei height)
{
    if (x <= -(PFint)width || y <= -(PFint)height) {
        G_currentCtx->errCode = PF_INVALID_OPERATION;
        return;
    }

    G_currentCtx->vpPos[0] = x;
    G_currentCtx->vpPos[1] = y;

    G_currentCtx->vpDim[0] = width - 1;
    G_currentCtx->vpDim[1] = height - 1;

    G_currentCtx->vpMin[0] = MAX(x, 0);
    G_currentCtx->vpMin[1] = MAX(y, 0);

    G_currentCtx->vpMax[0] = MIN(x + width, ((struct PFtex*)G_currentCtx->mainFramebuffer.texture)->w - 1);
    G_currentCtx->vpMax[1] = MIN(y + height, ((struct PFtex*)G_currentCtx->mainFramebuffer.texture)->h - 1);
}

void pfPolygonMode(PFface face, PFpolygonmode mode)
{
    if (!(mode == PF_POINT || mode == PF_LINE || mode == PF_FILL)) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    switch (face) {
        case PF_FRONT:
            G_currentCtx->polygonMode[0] = mode;
            break;

        case PF_BACK:
            G_currentCtx->polygonMode[1] = mode;
            break;

        case PF_FRONT_AND_BACK:
            G_currentCtx->polygonMode[0] = mode;
            G_currentCtx->polygonMode[1] = mode;
            break;

        default:
            G_currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfShadeModel(PFshademode mode)
{
    G_currentCtx->shadingMode = mode;
}

void pfLineWidth(PFfloat width)
{
    if (width <= 0.0f) {
        G_currentCtx->errCode = PF_INVALID_VALUE;
        return;
    }
    G_currentCtx->lineWidth = width;
}

void pfPointSize(PFfloat size)
{
    if (size <= 0.0f) {
        G_currentCtx->errCode = PF_INVALID_VALUE;
        return;
    }
    G_currentCtx->pointSize = size;
}

void pfCullFace(PFface face)
{
    if (face < PF_FRONT || face > PF_BACK) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }
    G_currentCtx->cullFace = face;
}

void pfBlendFunc(PFblendmode mode)
{
    if (!pfiIsBlendModeValid(mode)) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    G_currentCtx->blendFunction = GC_blendFuncs[mode];

#   if PF_SIMD_SUPPORT
        G_currentCtx->blendSimdFunction = GC_blendFuncs_simd[mode];
#   endif //PF_SIMD_SUPPORT
}

void pfDepthFunc(PFdepthmode mode)
{
    if (!pfiIsDepthModeValid(mode)) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    G_currentCtx->depthFunction = GC_depthTestFuncs[mode];

#   if PF_SIMD_SUPPORT
        G_currentCtx->depthSimdFunction = GC_depthTestFuncs_simd[mode];
#   endif //PF_SIMD_SUPPORT
}

void pfBindFramebuffer(PFframebuffer* framebuffer)
{
    G_currentCtx->bindedFramebuffer = framebuffer;

    if (G_currentCtx->state & PF_FRAMEBUFFER) {
        G_currentCtx->currentFramebuffer = framebuffer
            ? framebuffer : &G_currentCtx->mainFramebuffer;
    }
}

void pfBindTexture(PFtexture texture)
{
    G_currentCtx->currentTexture = texture;
}

void pfClear(PFclearflag flag)
{
    // If no flag is set, return early (nothing to clear)
    if (!flag) return;

    // Retrieve the current framebuffer and its associated texture
    PFframebuffer *framebuffer = G_currentCtx->currentFramebuffer;
    struct PFtex *tex = framebuffer->texture;
    PFsizei size = tex->w * tex->h;

#   if PF_SIMD_SUPPORT

    // SIMD-aligned size calculation
    PFsizei simdAlignedSize = size - (size % PF_SIMD_SIZE);

    // If both color and depth buffers should be cleared
    if (flag & (PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT)) {
        PFsizei pixelBytes = pfiGetPixelBytes(tex->format, tex->type);
        PFsimdvi vcolor = pfiSimdSet1_I32(*(PFuint*)&G_currentCtx->clearColor);
        PFsimdvf vdepth = pfiSimdSet1_F32(G_currentCtx->clearDepth);
        PFubyte *pbuffer = (PFubyte*)tex->pixels;
        PFfloat *zbuffer = framebuffer->zbuffer;
#       ifdef _OPENMP
#           pragma omp parallel for if(size >= PF_OPENMP_CLEAR_BUFFER_SIZE_THRESHOLD)
#       endif //_OPENMP
        for (PFsizei i = PF_SIMD_SIZE; i < simdAlignedSize; i += PF_SIMD_SIZE) {
            tex->setterSimd(tex->pixels, i, vcolor, *(PFsimdvi*)GC_simd_i32_0xffffffff);
            pfiSimdStore_F32(zbuffer + i, vdepth);
        }
        for (PFsizei i = simdAlignedSize; i < size; i++) {
            memcpy(pbuffer + i * pixelBytes, pbuffer, pixelBytes);
            zbuffer[i] = zbuffer[0];
        }
    }
    // If only the color buffer should be cleared
    else if (flag & PF_COLOR_BUFFER_BIT) {
        PFsizei pixelBytes = pfiGetPixelBytes(tex->format, tex->type);
        PFsimdvi vcolor = pfiSimdSet1_I32(*(PFuint*)&G_currentCtx->clearColor);
        PFubyte *pbuffer = (PFubyte*)tex->pixels;
#       ifdef _OPENMP
#           pragma omp parallel for if(size >= PF_OPENMP_CLEAR_BUFFER_SIZE_THRESHOLD)
#       endif //_OPENMP
        for (PFsizei i = PF_SIMD_SIZE; i < simdAlignedSize; i += PF_SIMD_SIZE) {
            tex->setterSimd(tex->pixels, i, vcolor, *(PFsimdvi*)GC_simd_i32_0xffffffff);
        }
        for (PFsizei i = simdAlignedSize; i < size; i++) {
            memcpy(pbuffer + i * pixelBytes, pbuffer, pixelBytes);
        }
    }
    // If only the depth buffer should be cleared
    else if (flag & PF_DEPTH_BUFFER_BIT) {
        PFsimdvf vdepth = pfiSimdSet1_F32(G_currentCtx->clearDepth);
        PFfloat *zbuffer = framebuffer->zbuffer;
#       ifdef _OPENMP
#          pragma omp parallel for if(size >= PF_OPENMP_CLEAR_BUFFER_SIZE_THRESHOLD)
#       endif //_OPENMP
        for (PFsizei i = PF_SIMD_SIZE; i < simdAlignedSize; i += PF_SIMD_SIZE) {
            pfiSimdStore_F32(zbuffer + i, vdepth);
        }
        for (PFsizei i = simdAlignedSize; i < size; i++) {
            zbuffer[i] = zbuffer[0];
        }
    }

#else

    // If both color and depth buffers should be cleared
    if (flag & (PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT)) {
        PFsizei pixelBytes = pfiGetPixelBytes(tex->format, tex->type);
        tex->setter(tex->pixels, 0, G_currentCtx->clearColor);
        framebuffer->zbuffer[0] = G_currentCtx->clearDepth;
        PFubyte *pbuffer = (PFubyte*)tex->pixels;
        PFfloat *zbuffer = framebuffer->zbuffer;
        PFfloat zvalue = zbuffer[0];
#       ifdef _OPENMP
#           pragma omp parallel for if(size >= PF_OPENMP_CLEAR_BUFFER_SIZE_THRESHOLD)
#       endif //_OPENMP
        for (PFsizei i = 1; i < size; i++) {
            memcpy(pbuffer + i * pixelBytes, pbuffer, pixelBytes);
            zbuffer[i] = zvalue;
        }
    }
    // If only the color buffer should be cleared
    else if (flag & PF_COLOR_BUFFER_BIT) {
        void *pbuffer = tex->pixels;
        PFsizei pixelBytes = pfiGetPixelBytes(tex->format, tex->type);
        tex->setter(pbuffer, 0, G_currentCtx->clearColor);
#       ifdef _OPENMP
#           pragma omp parallel for if(size >= PF_OPENMP_CLEAR_BUFFER_SIZE_THRESHOLD)
#       endif //_OPENMP
        for (PFsizei i = 0; i < size; i++) {
            memcpy((PFubyte*)pbuffer + i * pixelBytes, pbuffer, pixelBytes);
        }
    }
    // If only the depth buffer should be cleared
    else if (flag & PF_DEPTH_BUFFER_BIT) {
        PFfloat *zbuffer = framebuffer->zbuffer;
        zbuffer[0] = G_currentCtx->clearDepth;
#       ifdef _OPENMP
#           pragma omp parallel for if(size >= PF_OPENMP_CLEAR_BUFFER_SIZE_THRESHOLD)
#       endif //_OPENMP
        for (PFsizei i = 0; i < size; i++) {
            memcpy(zbuffer + i, zbuffer, sizeof(PFfloat));
        }
    }

#endif //PF_SIMD_SUPPORT
}

void pfClearDepth(PFfloat depth)
{
    G_currentCtx->clearDepth = depth;
}

void pfClearColor(PFubyte r, PFubyte g, PFubyte b, PFubyte a)
{
    G_currentCtx->clearColor = (PFcolor) { r, g, b, a };
}


/* Light management API functions */

void pfEnableLight(PFsizei light)
{
    if (light >= PF_MAX_LIGHT_STACK) {                      // Check if the specified light index is within the valid range.
        G_currentCtx->errCode = PF_INVALID_VALUE;
        return;
    }

    PFlight *desiredLight = G_currentCtx->lights + light;   // Get the pointer to the desired light from the lights array.
    PFlight **nodeLight = &G_currentCtx->activeLights;      // Get a pointer to the pointer pointing to the head of the active lights list.

    PFboolean enabled = PF_FALSE;                           // Flag to track if the light is not already enabled.

    while (*nodeLight != NULL) {                            // Iterate through the active lights list to check if the desired light is already enabled.
        if (*nodeLight == desiredLight) {                   // If the current light in the list matches the desired light, it's already enabled.
            enabled = PF_TRUE;                              // Set the flag to indicate that the light is not valid for enabling.
            break;
        }
        nodeLight = &(*nodeLight)->next;                    // Move to the next light in the list.
    }

    if (!enabled) {                                         // If the desired light is not already enabled, add it to the active lights list.
        desiredLight->next = NULL;                          // Set the 'next' pointer of the desired light to NULL since it will be added at the end.
        *nodeLight = desiredLight;                          // Update the pointer pointing to the head of the active lights list to include the desired light.
        return;
    }

    G_currentCtx->errCode = PF_INVALID_OPERATION;           // If the desired light is already enabled, set an error code indicating invalid operation.
}

void pfDisableLight(PFsizei light)
{
    if (light >= PF_MAX_LIGHT_STACK) {                      // Check if the specified light index is within the valid range.
        G_currentCtx->errCode = PF_INVALID_VALUE;
        return;
    }

    PFlight *desiredLight = G_currentCtx->lights + light;   // Get the pointer to the desired light from the lights array.
    PFlight **nodeLight = &G_currentCtx->activeLights;      // Get a pointer to the pointer pointing to the head of the active lights list.

    while (*nodeLight != NULL) {                            // Iterate through the active lights list to find and remove the desired light.
        if (*nodeLight == desiredLight) {                   // If the current light in the list matches the desired light, remove it from the list.
            *nodeLight = desiredLight->next;                // Update the pointer to skip the desired light in the list.
            desiredLight->next = NULL;                      // Set the 'next' pointer of the desired light to NULL to detach it from the list.
            return;
        }
        nodeLight = &(*nodeLight)->next;                    // Move to the next light in the list.
    }

    G_currentCtx->errCode = PF_INVALID_OPERATION;           // If the desired light is not found in the active lights list, set an error code indicating invalid operation.
}

PFboolean pfIsEnabledLight(PFsizei light)
{
    if (light >= PF_MAX_LIGHT_STACK) {
        G_currentCtx->errCode = PF_INVALID_VALUE;
        return PF_FALSE;
    }

    PFlight *desiredLight = G_currentCtx->lights + light;

    for (PFlight *light = G_currentCtx->activeLights; light != NULL; light = light->next) {
        if (light == desiredLight) return PF_TRUE;
    }

    return PF_FALSE;
}

void pfLightf(PFsizei light, PFenum param, PFfloat value)
{
    if (light >= PF_MAX_LIGHT_STACK) {
        G_currentCtx->errCode = PF_STACK_OVERFLOW;
        return;
    }

    PFlight *l = &G_currentCtx->lights[light];

    switch (param) {
        case PF_SPOT_INNER_CUTOFF:
            if ((value >= 0 && value <= 90) || value == 180) {
                l->innerCutOff = cosf(DEG2RAD(value));
            } else {
                G_currentCtx->errCode = PF_INVALID_VALUE;
            }
            break;
        case PF_SPOT_OUTER_CUTOFF:
            if ((value >= 0 && value <= 90) || value == 180) {
                l->outerCutOff = cosf(DEG2RAD(value));
            } else {
                G_currentCtx->errCode = PF_INVALID_VALUE;
            }
            break;
        case PF_CONSTANT_ATTENUATION:
            l->attConstant = value;
            break;
        case PF_LINEAR_ATTENUATION:
            l->attLinear = value;
            break;
        case PF_QUADRATIC_ATTENUATION:
            l->attQuadratic = value;
            break;
        default:
            // NOTE: The definition 'PF_AMBIENT_AND_DIFFUSE' is reserved for 'pfMaterialfv'
            G_currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfLightfv(PFsizei light, PFenum param, const void* value)
{
    if (light >= PF_MAX_LIGHT_STACK) {
        G_currentCtx->errCode = PF_STACK_OVERFLOW;
        return;
    }

    PFlight *l = &G_currentCtx->lights[light];

    switch (param) {
        case PF_POSITION:
            memcpy(l->position, value, sizeof(PFMvec3));
            break;
        case PF_SPOT_DIRECTION:
            memcpy(l->direction, value, sizeof(PFMvec3));
            break;
        case PF_SPOT_INNER_CUTOFF: {
            PFfloat v = *(PFfloat*)value;
            if ((v >= 0 && v <= 90) || v == 180) {
                l->innerCutOff = cosf(DEG2RAD(v));
            } else {
                G_currentCtx->errCode = PF_INVALID_VALUE;
            }
        }
        break;
        case PF_SPOT_OUTER_CUTOFF: {
            PFfloat v = *(PFfloat*)value;
            if ((v >= 0 && v <= 90) || v == 180) {
                l->outerCutOff = cosf(DEG2RAD(v));
            } else {
                G_currentCtx->errCode = PF_INVALID_VALUE;
            }
        }
        break;
        case PF_CONSTANT_ATTENUATION:
            l->attConstant = *(const PFfloat*)value;
            break;
        case PF_LINEAR_ATTENUATION:
            l->attLinear = *(const PFfloat*)value;
            break;
        case PF_QUADRATIC_ATTENUATION:
            l->attQuadratic = *(const PFfloat*)value;
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
            G_currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfMaterialf(PFface face, PFenum param, PFfloat value)
{
    PFmaterial *material0 = NULL;
    PFmaterial *material1 = NULL;

    switch (face) {
        case PF_FRONT:
            material0 = &G_currentCtx->faceMaterial[PF_FRONT];
            material1 = &G_currentCtx->faceMaterial[PF_FRONT];
            break;
        case PF_BACK:
            material0 = &G_currentCtx->faceMaterial[PF_BACK];
            material1 = &G_currentCtx->faceMaterial[PF_BACK];
            break;
        case PF_FRONT_AND_BACK:
            material0 = &G_currentCtx->faceMaterial[PF_FRONT];
            material1 = &G_currentCtx->faceMaterial[PF_BACK];
            break;
        default:
            G_currentCtx->errCode = PF_INVALID_ENUM;
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
            G_currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfMaterialfv(PFface face, PFenum param, const void *value)
{
    PFmaterial *material0 = NULL;
    PFmaterial *material1 = NULL;

    switch (face) {
        case PF_FRONT:
            material0 = &G_currentCtx->faceMaterial[PF_FRONT];
            material1 = &G_currentCtx->faceMaterial[PF_FRONT];
            break;
        case PF_BACK:
            material0 = &G_currentCtx->faceMaterial[PF_BACK];
            material1 = &G_currentCtx->faceMaterial[PF_BACK];
            break;
        case PF_FRONT_AND_BACK:
            material0 = &G_currentCtx->faceMaterial[PF_FRONT];
            material1 = &G_currentCtx->faceMaterial[PF_BACK];
            break;
        default:
            G_currentCtx->errCode = PF_INVALID_ENUM;
            return;
    }

    switch (param) {
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
            G_currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfColorMaterial(PFface face, PFenum mode)
{
    if (face < PF_FRONT || face > PF_FRONT_AND_BACK) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    if (mode < PF_AMBIENT_AND_DIFFUSE || mode > PF_EMISSION) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    G_currentCtx->materialColorFollowing.face = face;
    G_currentCtx->materialColorFollowing.mode = mode;
}


/* Vertex array drawing API functions */

void pfVertexPointer(PFint size, PFenum type, PFsizei stride, const void* pointer)
{
    if (size < 2 || size > 4) {
        G_currentCtx->errCode = PF_INVALID_VALUE;
        return;
    }

    if (!(type == PF_SHORT || type == PF_INT || type == PF_FLOAT || type == PF_DOUBLE)) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    G_currentCtx->vertexAttribs.positions = (PFvertexattribbuffer) {
        .buffer = pointer,
        .stride = stride,
        .size = size,
        .type = type
    };
}

void pfNormalPointer(PFenum type, PFsizei stride, const void* pointer)
{
    if (!(type == PF_FLOAT || type == PF_DOUBLE)) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    G_currentCtx->vertexAttribs.normals = (PFvertexattribbuffer) {
        .buffer = pointer,
        .stride = stride,
        .size = 3,
        .type = type
    };
}

void pfTexCoordPointer(PFenum type, PFsizei stride, const void* pointer)
{
    if (!(type == PF_FLOAT || type == PF_DOUBLE)) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    G_currentCtx->vertexAttribs.texcoords = (PFvertexattribbuffer) {
        .buffer = pointer,
        .stride = stride,
        .size = 2,
        .type = type
    };
}

void pfColorPointer(PFint size, PFenum type, PFsizei stride, const void* pointer)
{
    if (size < 3 || size > 4) {
        G_currentCtx->errCode = PF_INVALID_VALUE;
        return;
    }

    if (!(type == PF_UNSIGNED_BYTE || type == PF_UNSIGNED_SHORT || type == PF_UNSIGNED_INT || type == PF_FLOAT || type == PF_DOUBLE)) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    G_currentCtx->vertexAttribs.colors = (PFvertexattribbuffer) {
        .buffer = pointer,
        .stride = stride,
        .size = size,
        .type = type
    };
}

void pfDrawElements(PFdrawmode mode, PFsizei count, PFdatatype type, const void* indices)
{
    if (!(type == PF_UNSIGNED_BYTE || type == PF_UNSIGNED_SHORT || type == PF_UNSIGNED_INT)) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    if (!(G_currentCtx->state & PF_VERTEX_ARRAY)) {
        G_currentCtx->errCode = PF_INVALID_OPERATION;
        return;
    }

    const PFvertexattribbuffer *positions = &G_currentCtx->vertexAttribs.positions;
    const PFvertexattribbuffer *texcoords = &G_currentCtx->vertexAttribs.texcoords;
    const PFvertexattribbuffer *normals = &G_currentCtx->vertexAttribs.normals;
    const PFvertexattribbuffer *colors = &G_currentCtx->vertexAttribs.colors;

    PFboolean useTexCoordArray = G_currentCtx->state & PF_TEXTURE_COORD_ARRAY && texcoords->buffer;
    PFboolean useNormalArray = G_currentCtx->state & PF_NORMAL_ARRAY && normals->buffer;
    PFboolean useColorArray = G_currentCtx->state & PF_COLOR_ARRAY && colors->buffer;

    PFsizei indicesTypeSize = pfiGetDataTypeSize(type);
    PFsizei drawModeVertexCount = pfiGetDrawModeVertexCount(mode);

    for (PFsizei i = 0; i < drawModeVertexCount; i++) {
        G_currentCtx->vertexBuffer[i] = (PFvertex) { 0 };
        G_currentCtx->vertexBuffer[i].color = G_currentCtx->currentColor;
    }

    pfBegin(mode);

    for (PFsizei i = 0; i < count; i++) {
        PFvertex *vertex = G_currentCtx->vertexBuffer + (G_currentCtx->vertexCounter++);

        // Get vertex index
        const void *p = (const PFubyte*)indices + i*indicesTypeSize;

        PFsizei j = 0;
        switch(type) {
            case PF_UNSIGNED_BYTE:  j = *((const PFubyte*)p);  break;
            case PF_UNSIGNED_SHORT: j = *((const PFushort*)p); break;
            case PF_UNSIGNED_INT:   j = *((const PFuint*)p);   break;
            default:
                G_currentCtx->errCode = PF_INVALID_ENUM;
                return;
        }

        // Fill the vertex with given vertices data
        memset(&vertex->position, 0, sizeof(PFMvec3));
        vertex->position[3] = 1.0f;

        switch (positions->type) {
            case PF_SHORT: {
                for (int_fast8_t k = 0; k < positions->size; k++) {
                    vertex->position[k] = ((const PFshort*)positions->buffer)[j*positions->size + k];
                }
            }
            break;
            case PF_INT: {
                for (int_fast8_t k = 0; k < positions->size; k++) {
                    vertex->position[k] = ((const PFint*)positions->buffer)[j*positions->size + k];
                }
            }
            break;
            case PF_FLOAT: {
                for (int_fast8_t k = 0; k < positions->size; k++) {
                    vertex->position[k] = ((const PFfloat*)positions->buffer)[j*positions->size + k];
                }
            }
            break;
            case PF_DOUBLE: {
                for (int_fast8_t k = 0; k < positions->size; k++) {
                    vertex->position[k] = ((const PFdouble*)positions->buffer)[j*positions->size + k];
                }
            }
            break;
            default:
                G_currentCtx->errCode = PF_INVALID_ENUM;
                return;
        }

        if (useNormalArray) {
            memset(&vertex->normal, 0, sizeof(PFMvec3));

            switch (normals->type) {
                case PF_FLOAT: {
                    for (int_fast8_t k = 0; k < 3; k++) {
                        vertex->normal[k] = ((const PFfloat*)normals->buffer)[j*3 + k];
                    }
                }
                break;
                case PF_DOUBLE: {
                    for (int_fast8_t k = 0; k < 3; k++) {
                        vertex->normal[k] = ((const PFdouble*)normals->buffer)[j*3 + k];
                    }
                }
                break;
                default:
                    G_currentCtx->errCode = PF_INVALID_ENUM;
                    return;
            }
        }

        if (useTexCoordArray) {
            memset(&vertex->texcoord, 0, sizeof(PFMvec2));

            switch (texcoords->type) {
                case PF_FLOAT: {
                    for (int_fast8_t k = 0; k < 2; k++)
                    {
                        vertex->texcoord[k] = ((const PFfloat*)texcoords->buffer)[j*2 + k];
                    }
                }
                break;
                case PF_DOUBLE: {
                    for (int_fast8_t k = 0; k < 2; k++) {
                        vertex->texcoord[k] = ((const PFdouble*)texcoords->buffer)[j*2 + k];
                    }
                }
                break;
                default:
                    G_currentCtx->errCode = PF_INVALID_ENUM;
                    return;
            }
        }

        if (useColorArray) {
            memset(&vertex->color, 0xFF, sizeof(PFcolor));

            switch (colors->type) {
                case PF_UNSIGNED_BYTE: {
                    for (int_fast8_t k = 0; k < colors->size; k++) {
                        ((PFubyte*)&vertex->color)[k] = ((const PFubyte*)colors->buffer)[j*colors->size + k];
                    }
                }
                break;
                case PF_UNSIGNED_SHORT: {
                    for (int_fast8_t k = 0; k < colors->size; k++) {
                        ((PFubyte*)&vertex->color)[k] = ((const PFushort*)colors->buffer)[j*colors->size + k] >> 8;
                    }
                }
                break;
                case PF_UNSIGNED_INT: {
                    for (int_fast8_t k = 0; k < colors->size; k++) {
                        ((PFubyte*)&vertex->color)[k] = ((const PFuint*)colors->buffer)[j*colors->size + k] >> 24;
                    }
                }
                break;
                case PF_FLOAT: {
                    for (int_fast8_t k = 0; k < colors->size; k++) {
                        ((PFubyte*)&vertex->color)[k] = ((const PFfloat*)colors->buffer)[j*colors->size + k] * 255;
                    }
                }
                break;
                case PF_DOUBLE: {
                    for (int_fast8_t k = 0; k < colors->size; k++) {
                        ((PFubyte*)&vertex->color)[k] = ((const PFdouble*)colors->buffer)[j*colors->size + k] * 255;
                    }
                }
                break;
                default:
                    G_currentCtx->errCode = PF_INVALID_ENUM;
                    return;
            }
        }

        // If the number of vertices has reached that necessary for, we process the shape
        if (G_currentCtx->vertexCounter == drawModeVertexCount) {
            pfiProcessAndRasterize();
            pfiResetVertexBufferForNextElement();
        }
    }

    pfEnd();
}

void pfDrawArrays(PFdrawmode mode, PFint first, PFsizei count)
{
    if (!(G_currentCtx->state & PF_VERTEX_ARRAY)) {
        G_currentCtx->errCode = PF_INVALID_OPERATION;
        return;
    }

    const PFvertexattribbuffer *positions = &G_currentCtx->vertexAttribs.positions;
    const PFvertexattribbuffer *texcoords = &G_currentCtx->vertexAttribs.texcoords;
    const PFvertexattribbuffer *normals = &G_currentCtx->vertexAttribs.normals;
    const PFvertexattribbuffer *colors = &G_currentCtx->vertexAttribs.colors;

    PFboolean useTexCoordArray = G_currentCtx->state & PF_TEXTURE_COORD_ARRAY && texcoords->buffer;
    PFboolean useNormalArray = G_currentCtx->state & PF_NORMAL_ARRAY && normals->buffer;
    PFboolean useColorArray = G_currentCtx->state & PF_COLOR_ARRAY && colors->buffer;

    PFsizei drawModeVertexCount = pfiGetDrawModeVertexCount(mode);

    for (PFsizei i = 0; i < drawModeVertexCount; i++) {
        G_currentCtx->vertexBuffer[i] = (PFvertex) { 0 };
        G_currentCtx->vertexBuffer[i].color = G_currentCtx->currentColor;
    }

    pfBegin(mode);

    for (PFsizei i = 0; i < count; i++) {
        PFvertex *vertex = G_currentCtx->vertexBuffer + (G_currentCtx->vertexCounter++);

        // Fill the vertex with given vertices data
        memset(&vertex->position, 0, sizeof(PFMvec3));
        vertex->position[3] = 1.0f;

        switch (positions->type) {
            case PF_SHORT: {
                for (int_fast8_t j = 0; j < positions->size; j++) {
                    vertex->position[j] =
                        ((const PFshort*)positions->buffer + first*positions->size)[i*positions->size + j];
                }
            }
            break;
            case PF_INT: {
                for (int_fast8_t j = 0; j < positions->size; j++) {
                    vertex->position[j] =
                        ((const PFint*)positions->buffer + first*positions->size)[i*positions->size + j];
                }
            }
            break;
            case PF_FLOAT: {
                for (int_fast8_t j = 0; j < positions->size; j++) {
                    vertex->position[j] =
                        ((const PFfloat*)positions->buffer + first*positions->size)[i*positions->size + j];
                }
            }
            break;
            case PF_DOUBLE: {
                for (int_fast8_t j = 0; j < positions->size; j++) {
                    vertex->position[j] =
                        ((const PFdouble*)positions->buffer + first*positions->size)[i*positions->size + j];
                }
            }
            break;
            default:
                G_currentCtx->errCode = PF_INVALID_ENUM;
                break;
        }

        if (useNormalArray) {
            memset(&vertex->normal, 0, sizeof(PFMvec3));

            switch (normals->type) {
                case PF_FLOAT:
                {
                    for (int_fast8_t j = 0; j < 3; j++) {
                        vertex->normal[j] =
                            ((const PFfloat*)normals->buffer + first*3)[i*3 + j];
                    }
                }
                break;
                case PF_DOUBLE: {
                    for (int_fast8_t j = 0; j < 3; j++) {
                        vertex->normal[j] =
                            ((const PFdouble*)normals->buffer + first*3)[i*3 + j];
                    }
                }
                break;
                default:
                    G_currentCtx->errCode = PF_INVALID_ENUM;
                    break;
            }
        }

        if (useTexCoordArray) {
            memset(&vertex->texcoord, 0, sizeof(PFMvec2));

            switch (texcoords->type) {
                case PF_FLOAT: {
                    for (int_fast8_t j = 0; j < 2; j++) {
                        vertex->texcoord[j] =
                            ((const PFfloat*)texcoords->buffer + first*2)[i*2 + j];
                    }
                }
                break;
                case PF_DOUBLE: {
                    for (int_fast8_t j = 0; j < 2; j++) {
                        vertex->texcoord[j] =
                            ((const PFdouble*)texcoords->buffer + first*2)[i*2 + j];
                    }
                }
                break;
                default:
                    G_currentCtx->errCode = PF_INVALID_ENUM;
                    break;
            }
        }

        if (useColorArray) {
            memset(&vertex->color, 0xFF, sizeof(PFcolor));

            switch (colors->type) {
                case PF_UNSIGNED_BYTE: {
                    for (int_fast8_t j = 0; j < colors->size; j++) {
                        ((PFubyte*)&vertex->color)[j] =
                            ((const PFubyte*)colors->buffer + first*colors->size)[i*colors->size + j];
                    }
                }
                break;
                case PF_UNSIGNED_SHORT: {
                    for (int_fast8_t j = 0; j < colors->size; j++) {
                        ((PFubyte*)&vertex->color)[j] =
                            ((const PFushort*)colors->buffer + first*colors->size)[i*colors->size + j] >> 8;
                    }
                }
                break;
                case PF_UNSIGNED_INT: {
                    for (int_fast8_t j = 0; j < colors->size; j++) {
                        ((PFubyte*)&vertex->color)[j] =
                            ((const PFuint*)colors->buffer + first*colors->size)[i*colors->size + j] >> 24;
                    }
                }
                break;
                case PF_FLOAT: {
                    for (int_fast8_t j = 0; j < colors->size; j++) {
                        ((PFubyte*)&vertex->color)[j] =
                            ((const PFfloat*)colors->buffer + first*colors->size)[i*colors->size + j] * 255;
                    }
                }
                break;
                case PF_DOUBLE: {
                    for (int_fast8_t j = 0; j < colors->size; j++) {
                        ((PFubyte*)&vertex->color)[j] =
                            ((const PFdouble*)colors->buffer + first*colors->size)[i*colors->size + j] * 255;
                    }
                }
                break;
                default:
                    G_currentCtx->errCode = PF_INVALID_ENUM;
                    break;
            }
        }

        // If the number of vertices has reached that necessary for, we process the shape
        if (G_currentCtx->vertexCounter == drawModeVertexCount) {
            pfiProcessAndRasterize();
            pfiResetVertexBufferForNextElement();
        }
    }

    pfEnd();
}


/* Primitives drawing API functions */

void pfBegin(PFdrawmode mode)
{
    if (mode < PF_POINTS || mode > PF_QUAD_STRIP) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    pfiUpdateMatrices(
        !(mode == PF_POINTS || mode == PF_LINES));

    G_currentCtx->currentDrawMode = mode;
    G_currentCtx->vertexCounter = 0;
}

void pfEnd(void)
{
    G_currentCtx->vertexCounter = 0;
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
    PFvertex *vertex = G_currentCtx->vertexBuffer + (G_currentCtx->vertexCounter++);

    // Fill the vertex with given vertices data
    memcpy(vertex->position, v, sizeof(PFMvec4));
    memcpy(vertex->normal, G_currentCtx->currentNormal, sizeof(PFMvec3));
    memcpy(vertex->texcoord, G_currentCtx->currentTexcoord, sizeof(PFMvec2));
    memcpy(&vertex->color, &G_currentCtx->currentColor, sizeof(PFcolor));

    // If the number of vertices has reached that necessary for, we process the shape
    if (G_currentCtx->vertexCounter == pfiGetDrawModeVertexCount(G_currentCtx->currentDrawMode)) {
        pfiProcessAndRasterize();
        pfiResetVertexBufferForNextElement();
    }
}

// NOTE: Used by `pfColor` to assign material colors
//       when the `PF_COLOR_MATERIAL` state is enabled.
static void pfiSetMaterialColor(PFcolor color)
{
    PFmaterial *m1 = NULL, *m2 = NULL;

    switch (G_currentCtx->materialColorFollowing.face) {
        case PF_BACK:
        case PF_FRONT:
            m1 = m2 = &G_currentCtx->faceMaterial
                [G_currentCtx->materialColorFollowing.face];
            break;
        case PF_FRONT_AND_BACK:
            m1 = &G_currentCtx->faceMaterial[PF_FRONT];
            m2 = &G_currentCtx->faceMaterial[PF_BACK];
            break;
    }

    switch (G_currentCtx->materialColorFollowing.mode) {
        case PF_AMBIENT_AND_DIFFUSE:
            m1->ambient = m2->ambient = color;
            m1->diffuse = m2->diffuse = color;
            break;
        case PF_AMBIENT:
            m1->ambient = m2->ambient = color;
            break;
        case PF_DIFFUSE:
            m1->diffuse = m2->diffuse = color;
            break;
        case PF_SPECULAR:
            m1->specular = m2->specular = color;
            break;
        case PF_EMISSION:
            m1->emission = m2->emission = color;
            break;
    }
}

static inline void pfiSetCurrentColor(PFcolor color)
{
    if (G_currentCtx->state & PF_COLOR_MATERIAL) {
        pfiSetMaterialColor(color);
    } else {
        G_currentCtx->currentColor = color;
    }
}

void pfColor3ub(PFubyte r, PFubyte g, PFubyte b)
{
    pfiSetCurrentColor((PFcolor) {
        r, g, b, 255
    });
}

void pfColor3ubv(const PFubyte* v)
{
    pfiSetCurrentColor((PFcolor) {
        v[0], v[1], v[2], 255
    });
}

void pfColor3us(PFushort r, PFushort g, PFushort b)
{
    pfiSetCurrentColor((PFcolor) {
        (PFubyte)(r >> 8),
        (PFubyte)(g >> 8),
        (PFubyte)(b >> 8),
        255
    });
}

void pfColor3usv(const PFushort* v)
{
    pfiSetCurrentColor((PFcolor) {
        (PFubyte)(v[0] >> 8),
        (PFubyte)(v[1] >> 8),
        (PFubyte)(v[2] >> 8),
        255
    });
}

void pfColor3ui(PFuint r, PFuint g, PFuint b)
{
    pfiSetCurrentColor((PFcolor) {
        (PFubyte)(r >> 24),
        (PFubyte)(g >> 24),
        (PFubyte)(b >> 24),
        255
    });
}

void pfColor3uiv(const PFuint* v)
{
    pfiSetCurrentColor((PFcolor) {
        (PFubyte)(v[0] >> 24),
        (PFubyte)(v[1] >> 24),
        (PFubyte)(v[2] >> 24),
        255
    });
}

void pfColor3f(PFfloat r, PFfloat g, PFfloat b)
{
    pfiSetCurrentColor((PFcolor) {
        (PFubyte)(r*255),
        (PFubyte)(g*255),
        (PFubyte)(b*255),
        255
    });
}

void pfColor3fv(const PFfloat* v)
{
    pfiSetCurrentColor((PFcolor) {
        (PFubyte)(v[0]*255),
        (PFubyte)(v[1]*255),
        (PFubyte)(v[2]*255),
        255
    });
}

void pfColor4ub(PFubyte r, PFubyte g, PFubyte b, PFubyte a)
{
    pfiSetCurrentColor((PFcolor) {
        r, g, b, a
    });
}

void pfColor4ubv(const PFubyte* v)
{
    pfiSetCurrentColor((PFcolor) {
        v[0], v[1], v[2], v[3]
    });
}

void pfColor4us(PFushort r, PFushort g, PFushort b, PFushort a)
{
    pfiSetCurrentColor((PFcolor) {
        (PFubyte)(r >> 8),
        (PFubyte)(g >> 8),
        (PFubyte)(b >> 8),
        (PFubyte)(a >> 8)
    });
}

void pfColor4usv(const PFushort* v)
{
    pfiSetCurrentColor((PFcolor) {
        (PFubyte)(v[0] >> 8),
        (PFubyte)(v[1] >> 8),
        (PFubyte)(v[2] >> 8),
        (PFubyte)(v[3] >> 8)
    });
}

void pfColor4ui(PFuint r, PFuint g, PFuint b, PFuint a)
{
    pfiSetCurrentColor((PFcolor) {
        (PFubyte)(r >> 24),
        (PFubyte)(g >> 24),
        (PFubyte)(b >> 24),
        (PFubyte)(a >> 24)
    });
}

void pfColor4uiv(const PFuint* v)
{
    pfiSetCurrentColor((PFcolor) {
        (PFubyte)(v[0] >> 24),
        (PFubyte)(v[1] >> 24),
        (PFubyte)(v[2] >> 24),
        (PFubyte)(v[3] >> 24)
    });
}

void pfColor4f(PFfloat r, PFfloat g, PFfloat b, PFfloat a)
{
    pfiSetCurrentColor((PFcolor) {
        (PFubyte)(r*255),
        (PFubyte)(g*255),
        (PFubyte)(b*255),
        (PFubyte)(a*255)
    });
}

void pfColor4fv(const PFfloat* v)
{
    pfiSetCurrentColor((PFcolor) {
        (PFubyte)(v[0]*255),
        (PFubyte)(v[1]*255),
        (PFubyte)(v[2]*255),
        (PFubyte)(v[3]*255)
    });
}

void pfTexCoord2f(PFfloat u, PFfloat v)
{
    G_currentCtx->currentTexcoord[0] = u;
    G_currentCtx->currentTexcoord[1] = v;

    pfmVec2Transform(
        G_currentCtx->currentTexcoord,
        G_currentCtx->currentTexcoord,
        G_currentCtx->matTexture);
}

void pfTexCoordfv(const PFfloat* v)
{
    memcpy(G_currentCtx->currentTexcoord, v, sizeof(PFMvec2));

    pfmVec2Transform(
        G_currentCtx->currentTexcoord,
        G_currentCtx->currentTexcoord,
        G_currentCtx->matTexture);
}

void pfNormal3f(PFfloat x, PFfloat y, PFfloat z)
{
    G_currentCtx->currentNormal[0] = x;
    G_currentCtx->currentNormal[1] = y;
    G_currentCtx->currentNormal[2] = z;

    if (G_currentCtx->state & PF_NORMALIZE) {
        pfmVec3Normalize(
            G_currentCtx->currentNormal,
            G_currentCtx->currentNormal);
    }
}

void pfNormal3fv(const PFfloat* v)
{
    memcpy(G_currentCtx->currentNormal, v, sizeof(PFMvec3));
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
    pfiUpdateMatrices(PF_FALSE);

    // Project corner points
    PFMvec4 v1 = { x1, y1, 0.0f, 1.0f };
    PFMvec4 v2 = { x2, y2, 0.0f, 1.0f };
    pfmVec4Transform(v1, v1, G_currentCtx->matMVP);
    pfmVec4Transform(v2, v2, G_currentCtx->matMVP);

    // Calculate screen coordinates from projected coordinates
    PFint iX1 = G_currentCtx->vpPos[0] + (v1[0] + 1.0f) * 0.5f * G_currentCtx->vpDim[0];
    PFint iY1 = G_currentCtx->vpPos[1] + (1.0f - v1[1]) * 0.5f * G_currentCtx->vpDim[1];
    PFint iX2 = G_currentCtx->vpPos[0] + (v2[0] + 1.0f) * 0.5f * G_currentCtx->vpDim[0];
    PFint iY2 = G_currentCtx->vpPos[1] + (1.0f - v2[1]) * 0.5f * G_currentCtx->vpDim[1];

    // Ensure iX1 <= iX2 and iY1 <= iY2
    if (iX2 < iX1) iX1 ^= iX2, iX2 ^= iX1, iX1 ^= iX2;
    if (iY2 < iY1) iY1 ^= iY2, iY2 ^= iY1, iY1 ^= iY2;

    // Clamp screen coordinates to viewport boundaries
    iX1 = CLAMP(iX1, G_currentCtx->vpMin[0], G_currentCtx->vpMax[0]);
    iY1 = CLAMP(iY1, G_currentCtx->vpMin[1], G_currentCtx->vpMax[1]);
    iX2 = CLAMP(iX2, G_currentCtx->vpMin[0], G_currentCtx->vpMax[0]);
    iY2 = CLAMP(iY2, G_currentCtx->vpMin[1], G_currentCtx->vpMax[1]);

    // Retrieve framebuffer texture and current drawing color tint
    struct PFtex *tex = G_currentCtx->currentFramebuffer->texture;
    PFcolor color = G_currentCtx->currentColor;

    // Draw rectangle
    for (PFint y = iY1; y <= iY2; y++) {
        PFsizei yOffset = y * tex->w;
        for (PFint x = iX1; x <= iX2; x++) {
            tex->setter(tex->pixels, yOffset + x, color);
        }
    }
}

void pfRectfv(const PFfloat* v1, const PFfloat* v2)
{
    pfRectf(v1[0], v1[1], v2[0], v2[1]);
}


/* Drawing pixels API functions */

void pfDrawPixels(PFsizei width, PFsizei height, PFpixelformat format, PFdatatype type, const void* pixels)
{
    // Check if width or height is 0, which is an invalid value
    if (width == 0 || height == 0) {
        G_currentCtx->errCode = PF_INVALID_VALUE;
        return;
    }

    // Check if the pixel format and type are valid enums
    if (!pfiIsPixelFormatValid(format, type)) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    // Retrieve the appropriate pixel getter function for the given buffer format
    PFpixelgetter getPixelSrc = GC_pixelGetters[format][type];
    if (!getPixelSrc) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    // Get the transformation matrix from model to view (ModelView) and projection
    pfiUpdateMatrices(PF_FALSE);

    // Project raster point from model space to screen space
    PFMvec4 rasterPos;
    pfmVec4Copy(rasterPos, G_currentCtx->rasterPos);                  // Copy raster position
    pfmVec4Transform(rasterPos, rasterPos, G_currentCtx->matMVP);     // Apply transformation matrix

    // Calculate screen coordinates from projected coordinates
    // Transform X and Y from range [-1, 1] to screen space coordinates
    PFint xScreen = G_currentCtx->vpPos[0] + (rasterPos[0] + 1.0f)*0.5f*G_currentCtx->vpDim[0];
    PFint yScreen = G_currentCtx->vpPos[1] + (1.0f - rasterPos[1])*0.5f*G_currentCtx->vpDim[1];
    PFfloat zPos = rasterPos[2]; // Z position remains unchanged

    // Calculate the destination rectangle (clipped to viewport boundaries)
    PFint xMin = CLAMP(xScreen, G_currentCtx->vpMin[0], G_currentCtx->vpMax[0]);
    PFint yMin = CLAMP(yScreen, G_currentCtx->vpMin[1], G_currentCtx->vpMax[1]);
    PFint xMax = CLAMP(xScreen + width*G_currentCtx->pixelZoom[0], G_currentCtx->vpMin[0], G_currentCtx->vpMax[0]);
    PFint yMax = CLAMP(yScreen + height*G_currentCtx->pixelZoom[1], G_currentCtx->vpMin[1], G_currentCtx->vpMax[1]);

    // Calculate inverse lengths for texture sampling
    PFfloat invXLen = 1.0f/(PFfloat)(width*G_currentCtx->pixelZoom[0]);
    PFfloat invYLen = 1.0f/(PFfloat)(height*G_currentCtx->pixelZoom[1]);

    // Calculate the dimensions minus 1 to scale the texture coordinates
    PFsizei widthM1 = width - 1;
    PFsizei heightM1 = height - 1;

    // Get current framebuffer and depth buffer
    struct PFtex *texDst = G_currentCtx->currentFramebuffer->texture;
    PFfloat *zBuffer = G_currentCtx->currentFramebuffer->zbuffer;

    // Check if depth test is enabled
    PFboolean noDepthTest = !(G_currentCtx->state & PF_DEPTH_TEST);

    // Get the color mixing function (if necessary)
    PFblendfunc blendFunction = G_currentCtx->state & PF_BLEND ?
        G_currentCtx->blendFunction : NULL;

    // Loop through each pixel in the destination rectangle
#   ifdef _OPENMP
#       pragma omp parallel for if ((yMax - yMin)*(xMax - xMin) >= PF_OPENMP_RASTER_THRESHOLD_AREA)
#   endif // _OPENMP
    for (PFint y = yMin; y <= yMax; y++) {
        // Calculate texture V coordinate based on screen Y coordinate
        PFfloat v = (PFfloat)(y - yScreen)*invYLen;
        PFsizei ySrcOffset = (PFsizei)(v*heightM1)*width; // Offset into source texture

        // Calculate destination offset for this scanline
        PFsizei yDstOffset = y*texDst->w;

        for (PFint x = xMin; x <= xMax; x++) {
            // Calculate destination offset for this pixel
            PFsizei xyDstOffset = yDstOffset + x;

            // Perform depth test or skip if disabled
            if (noDepthTest || G_currentCtx->depthFunction(zPos, zBuffer[xyDstOffset])) {
                // Calculate texture U coordinate based on screen X coordinate
                PFfloat u = (PFfloat)(x - xScreen)*invXLen;

                // Calculate offset into source texture for this pixel
                PFsizei xySrcOffset = ySrcOffset + (PFsizei)(u*widthM1);

                // Update depth buffer with new depth value
                zBuffer[xyDstOffset] = zPos;

                // Retrieve source color
                PFcolor color = getPixelSrc(pixels, xySrcOffset);

                // Blend source and destination colors and update framebuffer
                texDst->setter(texDst->pixels, xyDstOffset, blendFunction
                    ? blendFunction(color, texDst->getter(texDst->pixels, xyDstOffset)) : color);
            }
        }
    }
}

void pfPixelZoom(PFfloat xFactor, PFfloat yFactor)
{
    G_currentCtx->pixelZoom[0] = xFactor;
    G_currentCtx->pixelZoom[1] = yFactor;
}

void pfRasterPos2i(PFint x, PFint y)
{
    G_currentCtx->rasterPos[0] = (PFfloat)x;
    G_currentCtx->rasterPos[1] = (PFfloat)y;
    G_currentCtx->rasterPos[2] = 0.0f;
    G_currentCtx->rasterPos[3] = 1.0f;
}

void pfRasterPos2f(PFfloat x, PFfloat y)
{
    G_currentCtx->rasterPos[0] = x;
    G_currentCtx->rasterPos[1] = y;
    G_currentCtx->rasterPos[2] = 0.0f;
    G_currentCtx->rasterPos[3] = 1.0f;
}

void pfRasterPos2fv(const PFfloat* v)
{
    memcpy(G_currentCtx->rasterPos, v, sizeof(PFMvec2));
    G_currentCtx->rasterPos[2] = 0.0f;
    G_currentCtx->rasterPos[3] = 1.0f;
}

void pfRasterPos3i(PFint x, PFint y, PFint z)
{
    G_currentCtx->rasterPos[0] = (PFfloat)x;
    G_currentCtx->rasterPos[1] = (PFfloat)y;
    G_currentCtx->rasterPos[2] = (PFfloat)z;
    G_currentCtx->rasterPos[3] = 1.0f;
}

void pfRasterPos3f(PFfloat x, PFfloat y, PFfloat z)
{
    G_currentCtx->rasterPos[0] = x;
    G_currentCtx->rasterPos[1] = y;
    G_currentCtx->rasterPos[2] = z;
    G_currentCtx->rasterPos[3] = 1.0f;
}

void pfRasterPos3fv(const PFfloat* v)
{
    memcpy(G_currentCtx->rasterPos, v, sizeof(PFMvec3));
    G_currentCtx->rasterPos[3] = 1.0f;
}

void pfRasterPos4i(PFint x, PFint y, PFint z, PFint w)
{
    G_currentCtx->rasterPos[0] = (PFfloat)x;
    G_currentCtx->rasterPos[1] = (PFfloat)y;
    G_currentCtx->rasterPos[2] = (PFfloat)z;
    G_currentCtx->rasterPos[3] = (PFfloat)w;
}

void pfRasterPos4f(PFfloat x, PFfloat y, PFfloat z, PFfloat w)
{
    G_currentCtx->rasterPos[0] = x;
    G_currentCtx->rasterPos[1] = y;
    G_currentCtx->rasterPos[2] = z;
    G_currentCtx->rasterPos[3] = w;
}

void pfRasterPos4fv(const PFfloat* v)
{
    memcpy(G_currentCtx->rasterPos, v, sizeof(PFMvec4));
}


/* Fog API functions */

void pfFogi(PFfogparam pname, PFint param)
{
    switch (pname) {
        case PF_FOG_MODE:
            if (param >= PF_LINEAR && param <= PF_EXP2) {
                G_currentCtx->fog.mode = param;
            } else {
                G_currentCtx->errCode = PF_INVALID_VALUE;
            }
            break;
        case PF_FOG_DENSITY:
            if (param == 0 || param == 1) {
                G_currentCtx->fog.mode = param;
            } else {
                G_currentCtx->errCode = PF_INVALID_VALUE;
            }
            break;
        case PF_FOG_START:
            G_currentCtx->fog.start = param;
            break;
        case PF_FOG_END:
            G_currentCtx->fog.end = param;
            break;
        default:
            G_currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfFogf(PFfogparam pname, PFfloat param)
{
    switch (pname) {
        case PF_FOG_DENSITY:
            if (param >= 0 && param <= 1) {
                G_currentCtx->fog.mode = param;
            } else {
                G_currentCtx->errCode = PF_INVALID_VALUE;
            }
            break;
        case PF_FOG_START:
            G_currentCtx->fog.start = param;
            break;
        case PF_FOG_END:
            G_currentCtx->fog.end = param;
            break;
        default:
            G_currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfFogiv(PFfogparam pname, PFint* param)
{
    switch (pname) {
        case PF_FOG_MODE:
            if (*param >= PF_LINEAR || *param <= PF_EXP2) {
                G_currentCtx->fog.mode = *param;
            } else {
                G_currentCtx->errCode = PF_INVALID_VALUE;
            }
            break;
        case PF_FOG_DENSITY:
            if (*param >= 0 || *param <= 1) {
                G_currentCtx->fog.mode = *param;
            } else {
                G_currentCtx->errCode = PF_INVALID_VALUE;
            }
            break;
        case PF_FOG_START:
            G_currentCtx->fog.start = *param;
            break;
        case PF_FOG_END:
            G_currentCtx->fog.end = *param;
            break;
        case PF_FOG_COLOR:
            G_currentCtx->fog.color.r = param[0];
            G_currentCtx->fog.color.g = param[1];
            G_currentCtx->fog.color.b = param[2];
            G_currentCtx->fog.color.a = param[3];
            break;
        default:
            G_currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfFogfv(PFfogparam pname, PFfloat* param)
{
    switch (pname) {
        case PF_FOG_DENSITY:
            if (*param >= 0 || *param <= 1) {
                G_currentCtx->fog.mode = *param;
            } else {
                G_currentCtx->errCode = PF_INVALID_VALUE;
            }
            break;
        case PF_FOG_START:
            G_currentCtx->fog.start = *param;
            break;
        case PF_FOG_END:
            G_currentCtx->fog.end = *param;
            break;
        case PF_FOG_COLOR:
            G_currentCtx->fog.color.r = 255*param[0];
            G_currentCtx->fog.color.g = 255*param[1];
            G_currentCtx->fog.color.b = 255*param[2];
            G_currentCtx->fog.color.a = 255*param[3];
            break;
        default:
            G_currentCtx->errCode = PF_INVALID_ENUM;
            break;
    }
}

void pfFogProcess(void)
{
    struct PFtex* tex = G_currentCtx->currentFramebuffer->texture;

    PFint width = tex->w;
    PFint height = tex->h;

    void *pixels = tex->pixels;
    const PFfloat *zBuffer = G_currentCtx->currentFramebuffer->zbuffer;

    PFpixelgetter getter = tex->getter;
    PFpixelsetter setter = tex->setter;

#ifdef _OPENMP
#   define BEGIN_FOG_LOOP() \
    _Pragma("omp parallel for collapse(2) firstprivate(fogColor)") \
    for (PFint y = 0; y < height; y++) { \
        for (PFint x = 0; x < width; x++) { \
            PFsizei xyOffset = y*width + x;

#define END_FOG_LOOP() \
        } \
    }
#else
#   define BEGIN_FOG_LOOP() \
    PFsizei yOffset = 0; \
    for (PFint y = 0; y < height; y++, yOffset += width) { \
        for (PFint x = 0; x < width; x++) { \
            PFsizei xyOffset = yOffset + x;

#define END_FOG_LOOP() \
        } \
    }
#endif

    PFfloat density = G_currentCtx->fog.density;
    PFcolor fogColor = G_currentCtx->fog.color;
    PFfogmode mode = G_currentCtx->fog.mode;
    PFfloat start = G_currentCtx->fog.start;
    PFfloat end = G_currentCtx->fog.end;
    PFfloat invLen = 1/(end - start);
    PFubyte alpha = fogColor.a;

    BEGIN_FOG_LOOP()
    {
        PFfloat depth = zBuffer[xyOffset];
        if (depth >= end) {
            fogColor.a = alpha;
            setter(pixels, xyOffset, alpha == 255 ? fogColor
                : pfiBlendAlpha(fogColor, getter(pixels, xyOffset)));
        } else if (depth > start) {
            PFfloat t = 0;
            switch (mode) {
                case PF_LINEAR:
                    t = (depth - start)*invLen;
                    break;
                case PF_EXP:
                    t = 1.0f - exp(-density*(depth - start));
                    break;
                case PF_EXP2:
                    t = 1.0f - exp2(-density*(depth - start));
                    break;
            }
            fogColor.a = (PFubyte)(t * alpha);
            PFcolor color = getter(pixels, xyOffset);
            setter(pixels, xyOffset, pfiBlendAlpha(fogColor, color));
        }
    }
    END_FOG_LOOP()
}


/* Misc API functions */

void pfReadPixels(PFint x, PFint y, PFsizei width, PFsizei height, PFpixelformat format, PFdatatype type, void* pixels)
{
    if (!pfiIsPixelFormatValid(format, type)) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    // Get the appropriate pixel setter function for the given format
    PFpixelsetter dstPixelSetter = GC_pixelSetters[format][type];
    if (!dstPixelSetter) {
        G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    /* Retrieve information about the source framebuffer */

    const struct PFtex *texSrc = G_currentCtx->currentFramebuffer->texture;
    PFpixelgetter srcPixelGetter = texSrc->getter;
    const void *srcPixels = texSrc->pixels;
    PFsizei srcWidth = texSrc->w;

    /* Calculate the minimum and maximum coordinates of the region to be read */

    PFsizei xMin = CLAMP(x, 0, (PFint)texSrc->w - 1);
    PFsizei yMin = CLAMP(y, 0, (PFint)texSrc->h - 1);
    PFsizei xMax = CLAMP(x + (PFint)width, 0, (PFint)texSrc->w);
    PFsizei yMax = CLAMP(y + (PFint)height, 0, (PFint)texSrc->h);

    /* Reads pixels from the framebuffer and copies them to the destination */

#ifdef _OPENMP
#   pragma omp parallel for collapse(2)
    for (PFsizei ySrc = yMin; ySrc < yMax; ySrc++) {
        for (PFsizei xSrc = xMin; xSrc < xMax; xSrc++) {
            PFcolor color = srcPixelGetter(srcPixels, ySrc * srcWidth + xSrc);
            dstPixelSetter(pixels, (ySrc - yMin) * width + (xSrc - xMin), color);
        }
    }
#else
    for (PFsizei ySrc = yMin, yDst = 0; ySrc < yMax; ySrc++, yDst++) {
        for (PFsizei xSrc = xMin, xDst = 0; xSrc < xMax; xSrc++, xDst++) {
            PFcolor color = srcPixelGetter(srcPixels, ySrc * srcWidth + xSrc);
            dstPixelSetter(pixels, yDst * width + xDst, color);
        }
    }
#endif
}

void pfPostProcess(PFpostprocessfunc postProcessFunction)
{
    struct PFtex* tex = G_currentCtx->currentFramebuffer->texture;

    PFint width = tex->w;
    PFint height = tex->h;

    void *pixels = tex->pixels;
    const PFfloat *zBuffer = G_currentCtx->currentFramebuffer->zbuffer;

    PFpixelgetter getter = tex->getter;
    PFpixelsetter setter = tex->setter;

#ifdef _OPENMP
#   define BEGIN_POSTPROCESS_LOOP() \
    _Pragma("omp parallel for collapse(2)") \
    for (PFint y = 0; y < height; y++) { \
        for (PFint x = 0; x < width; x++) { \
            PFsizei xyOffset = y*width + x;

#define END_POSTPROCESS_LOOP() \
        } \
    }
#else
#   define BEGIN_POSTPROCESS_LOOP() \
    PFsizei yOffset = 0; \
    for (PFint y = 0; y < height; y++, yOffset += width) { \
        for (PFint x = 0; x < width; x++) { \
            PFsizei xyOffset = yOffset + x;

#define END_POSTPROCESS_LOOP() \
        } \
    }
#endif

    BEGIN_POSTPROCESS_LOOP()
    {
        PFcolor color = getter(pixels, xyOffset);
        PFfloat depth = zBuffer[xyOffset];

        color = postProcessFunction(x, y, depth, color);
        setter(pixels, xyOffset, color);
    }
    END_POSTPROCESS_LOOP()
}
