#include "pixelforge.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <float.h>
#include <math.h>

/* Internal data */

typedef enum {
    PF_TEXTURE_MODE = 0x01,
    PF_DEPTH_TEST = 0x02,
    PF_WIRE_MODE = 0x04,
    PF_STEREO = 0x08
} PFrenderstate;

typedef struct {
    const void *positions;
    const void *normals;
    const void *colors;
    const void *texcoords;
} PFvertexattribs;

struct PFctx {

    PFframebuffer screenBuffer;
    PFframebuffer *currentFramebuffer;

    PFuint viewportX, viewportY;
    PFuint viewportW, viewportH;

    PFdrawmode currentDrawMode;
    PFblendfunc blendFunction;
    PFcolor clearColor;

    PFvertex vertexBuffer[6];
    PFuint vertexCount;

    PFvec3f currentNormal;
    PFvec2f currentTexcoord;
    PFcolor currentColor;
    PFfloat currentDepth;

    PFint currentMatrixMode;                    // Current matrix mode
    PFmat4f *currentMatrix;                     // Current matrix pointer
    PFmat4f modelview;                          // Default modelview matrix
    PFmat4f projection;                         // Default projection matrix
    PFmat4f transform;                          // Transform matrix to be used with pfTranslate, pfRotate, pfScale
    PFboolean transformRequired;                // Require transform matrix application to current draw-call vertex (if required)
    PFmat4f stack[PF_MAX_MATRIX_STACK_SIZE];    // Matrix stack for push/pop
    PFint stackCounter;                         // Matrix stack counter

    PFvertexattribs vertexAttribs;
    PFtexture *currentTexture;

    //PFmat4f projectionStereo[2];                 // VR stereo rendering eyes projection matrices
    //PFmat4f viewOffsetStereo[2];                 // VR stereo rendering eyes view offset matrices

    PFushort vertexAttribState;
    PFushort renderState;

    PFboolean depthTest;
    PFboolean wireMode;

    PFboolean renderBeginned;

};

typedef enum {
    CLIP_INSIDE = 0x00, // 0000
    CLIP_LEFT   = 0x01, // 0001
    CLIP_RIGHT  = 0x02, // 0010
    CLIP_BOTTOM = 0x04, // 0100
    CLIP_TOP    = 0x08, // 1000
} PFclipcode;

static PFctx *currentCtx = NULL;

/* Internal helper function declarations */

static PFvertex Helper_LerpVertex(const PFvertex* start, const PFvertex* end, PFfloat t);
static PFcolor Helper_LerpColor(PFcolor a, PFcolor b, PFfloat t);

static PFvec2f Helper_InterpolateVec2f(const PFvec2f* v1, const PFvec2f* v2, const PFvec2f* v3, PFfloat w1, PFfloat w2, PFfloat w3);
static PFcolor Helper_InterpolateColor(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3);

static void Helper_SwapVertex(PFvertex* a, PFvertex* b);
static void Helper_SwapByte(PFubyte* a, PFubyte* b);

// Used by 'Process_ClipLine2D'
static PFubyte Helper_EncodeClip2D(const PFvec4f* v);

// Used by 'Process_ClipLine3D'
// 'q' represents a homogeneous coordinate weight from which a homogeneous coordinate 'x', 'y', or 'z' has been subtracted
// 'p' represents the delta between two homogeneous coordinate weights from which the corresponding homogeneous delta 'x', 'y', or 'z' has been subtracted
static PFboolean Helper_ClipCoord3D(PFfloat q, PFfloat p, PFfloat* t1, PFfloat* t2);

/* Internal vertex processing function declarations */

static void Process_HomogeneousToScreen(PFvec4f* homogeneous);

static PFboolean Process_ClipLine2D(PFvertex* v1, PFvertex* v2);
static PFboolean Process_ClipLine3D(PFvertex* v1, PFvertex* v2);

static PFboolean Process_ClipPolygonW(PFvertex* polygon, PFubyte* vertexCounter);
static PFboolean Process_ClipPolygonXYZ(PFvertex* polygon, PFubyte* vertexCounter);

static void Process_ProjectAndClipLine(PFvertex* line, PFubyte* vertexCounter, const PFmat4f* mvp);
static PFboolean Process_ProjectAndClipTriangle(PFvertex* polygon, PFubyte* vertexCounter, const PFmat4f* mvp);

/* Internal line rasterizer function declarations */

static void Rasterize_LineFlat(const PFvertex* v1, const PFvertex* v2);
static void Rasterize_LineDepth(const PFvertex* v1, const PFvertex* v2);

/* Internal triangle 2D rasterizer function declarations */

static void Rasterize_TriangleColorFlat2D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
static void Rasterize_TriangleColorDepth2D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
static void Rasterize_TriangleTextureFlat2D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
static void Rasterize_TriangleTextureDepth2D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);

/* Internal triangle 3D rasterizer function declarations */

static void Rasterize_TriangleColorFlat3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
static void Rasterize_TriangleColorDepth3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
static void Rasterize_TriangleTextureFlat3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);
static void Rasterize_TriangleTextureDepth3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3);

/* Internal processing and rasterization function declarations */

static void ProcessRasterize(const PFmat4f* mvp);

/* Context API functions */

PFctx* pfContextCreate(void* screenBuffer, PFuint screenWidth, PFuint screenHeight, PFpixelformat screenFormat)
{
    PFctx *ctx = (PFctx*)PF_MALLOC(sizeof(PFctx));

    ctx->screenBuffer = (PFframebuffer) { 0 };

    ctx->screenBuffer.texture = pfTextureGenFromBuffer(
        screenBuffer, screenWidth, screenHeight, screenFormat);

    const PFsizei bufferSize = screenWidth*screenHeight;
    ctx->screenBuffer.zbuffer = (PFfloat*)PF_MALLOC(bufferSize*sizeof(PFctx));
    for (PFsizei i = 0; i < bufferSize; i++) ctx->screenBuffer.zbuffer[i] = FLT_MAX;

    ctx->currentDrawMode = 0;
    ctx->blendFunction = pfBlendAlpha;
    ctx->clearColor = (PFcolor) { 0 };

    ctx->currentFramebuffer = &ctx->screenBuffer;

    ctx->viewportX = 0;
    ctx->viewportY = 0;
    ctx->viewportW = screenWidth - 1;
    ctx->viewportH = screenHeight - 1;

    ctx->vertexCount = 0;

    ctx->currentNormal = (PFvec3f) { 0 };
    ctx->currentTexcoord = (PFvec2f) { 0 };
    ctx->currentColor = (PFcolor) { 255, 255, 255, 255 };
    ctx->currentDepth = 0.0f;

    ctx->currentMatrixMode = PF_MODELVIEW;
    ctx->currentMatrix = NULL;
    ctx->modelview = pfMat4fIdentity();
    ctx->projection = pfMat4fIdentity();
    ctx->transform = pfMat4fIdentity();
    ctx->transformRequired = PF_FALSE;
    ctx->stackCounter = 0;

    ctx->vertexAttribs = (PFvertexattribs) { 0 };
    ctx->currentTexture = NULL;

    //ctx->projectionStereo;
    //ctx->viewOffsetStereo;

    ctx->vertexAttribState = 0x00;
    ctx->renderState = 0x00;

    ctx->wireMode = PF_FALSE;

    ctx->renderBeginned = PF_FALSE;

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


/* Render API functions */

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
        PF_LOG(PF_LOG_ERROR, "[pfPushMatrix] PFmat4f stack overflow (PF_MAX_MATRIX_STACK_SIZE=%i)", PF_MAX_MATRIX_STACK_SIZE);
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
    PFmat4f rotation = pfMat4fRotate(&axis, DEG2RAD(angle));

    // NOTE: We transpose matrix with multiplication order
    *currentCtx->currentMatrix = pfMat4fMul(&rotation, currentCtx->currentMatrix);
}

void pfScalef(PFfloat x, PFfloat y, PFfloat z)
{
    PFmat4f scale = pfMat4fScale(x, y, z);

    // NOTE: We transpose matrix with multiplication order
    *currentCtx->currentMatrix = pfMat4fMul(&scale, currentCtx->currentMatrix);
}

void pfMultMatrixf(const float* mat)
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

PFblendfunc pfGetBlendFunction()
{
    return currentCtx->blendFunction;
}

void pfSetBlendFunction(PFblendfunc func)
{
    currentCtx->blendFunction = func;
}

void pfDrawVertexArrayElements(PFsizei offset, PFsizei count, const void *buffer)
{
    if (!(currentCtx->vertexAttribState & PF_VERTEX_ARRAY)) return;
    const PFvec3f *positions = (PFvec3f*)(currentCtx->vertexAttribs.positions) + offset;

    const PFvec3f *normals = (currentCtx->vertexAttribState & PF_NORMAL_ARRAY)
        ? (PFvec3f*)(currentCtx->vertexAttribs.normals) + offset : NULL;

    const PFcolor *colors = (currentCtx->vertexAttribState & PF_COLOR_ARRAY)
        ? (PFcolor*)(currentCtx->vertexAttribs.colors) + offset : NULL;

    const PFvec2f *texcoords = (currentCtx->vertexAttribState & PF_TEXTURE_COORD_ARRAY)
        ? (PFvec2f*)(currentCtx->vertexAttribs.texcoords) + offset : NULL;

    pfBegin((currentCtx->renderState & PF_WIRE_MODE)
        ? PF_LINES : PF_TRIANGLES);

    for (PFsizei i = 0; i < count; i++)
    {
        const PFushort j = ((PFushort*)buffer)[i];

        const PFvec3f *p3 = positions + j;
        const PFvec4f p4 = { p3->x, p3->y, p3->z, 1.0f };

        currentCtx->vertexBuffer[currentCtx->vertexCount++] = (PFvertex) {
            (currentCtx->transformRequired) ? pfVec4fTransform(&p4, &currentCtx->transform) : p4,
            (normals) ? normals[i] : (PFvec3f) { 0 }, (texcoords) ? texcoords[i] : (PFvec2f) { 0 },
            (colors) ? pfBlendMultiplicative(colors[j], currentCtx->currentColor) : currentCtx->currentColor,
        };

        if (currentCtx->vertexCount == currentCtx->currentDrawMode)
        {
            currentCtx->vertexCount = 0;
            PFmat4f mvp = pfMat4fMul(&currentCtx->modelview, &currentCtx->projection);
            ProcessRasterize(&mvp);
        }
    }

    pfEnd();
}

void pfDrawVertexArray(PFsizei offset, PFsizei count)
{
    if (!(currentCtx->vertexAttribState & PF_VERTEX_ARRAY)) return;
    const PFvec3f *positions = (PFvec3f*)(currentCtx->vertexAttribs.positions) + offset;

    const PFvec3f *normals = (currentCtx->vertexAttribState & PF_NORMAL_ARRAY)
        ? (PFvec3f*)(currentCtx->vertexAttribs.normals) + offset : NULL;

    const PFcolor *colors = (currentCtx->vertexAttribState & PF_COLOR_ARRAY)
        ? (PFcolor*)(currentCtx->vertexAttribs.colors) + offset : NULL;

    const PFvec2f *texcoords = (currentCtx->vertexAttribState & PF_TEXTURE_COORD_ARRAY)
        ? (PFvec2f*)(currentCtx->vertexAttribs.texcoords) + offset : NULL;

    pfBegin((currentCtx->renderState & PF_WIRE_MODE)
        ? PF_LINES : PF_TRIANGLES);

    for (PFsizei i = 0; i < count; i++)
    {
        const PFvec3f *p3 = positions + i;
        const PFvec4f p4 = { p3->x, p3->y, p3->z, 1.0f };

        currentCtx->vertexBuffer[currentCtx->vertexCount++] = (PFvertex) {
            (currentCtx->transformRequired) ? pfVec4fTransform(&p4, &currentCtx->transform) : p4,
            (normals) ? normals[i] : (PFvec3f) { 0 }, (texcoords) ? texcoords[i] : (PFvec2f) { 0 },
            (colors) ? pfBlendMultiplicative(colors[i], currentCtx->currentColor) : currentCtx->currentColor,
        };

        if (currentCtx->vertexCount == currentCtx->currentDrawMode)
        {
            currentCtx->vertexCount = 0;
            PFmat4f mvp = pfMat4fMul(&currentCtx->modelview, &currentCtx->projection);
            ProcessRasterize(&mvp);
        }
    }

    pfEnd();
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

void pfEnableTexture(PFtexture* texture)
{
    if (!texture) { pfDisableTexture(); return; }
    currentCtx->renderState |= PF_TEXTURE_MODE;
    currentCtx->currentTexture = texture;
}

void pfDisableTexture()
{
    currentCtx->renderState &= ~PF_TEXTURE_MODE;
    currentCtx->currentTexture = NULL;
}

void pfEnableWireMode(void)
{
    currentCtx->renderState |= PF_WIRE_MODE;
}

void pfDisableWireMode(void)
{
    currentCtx->renderState &= ~PF_WIRE_MODE;
}

void pfEnableDepthTest(void)
{
    currentCtx->renderState |= PF_DEPTH_TEST;
}

void pfDisableDepthTest(void)
{
    currentCtx->renderState &= ~PF_DEPTH_TEST;
}

void pfClear(void)
{
    pfFramebufferClear(currentCtx->currentFramebuffer, currentCtx->clearColor);
}

void pfClearColor(PFubyte r, PFubyte g, PFubyte b, PFubyte a)
{
    currentCtx->clearColor = (PFcolor) { r, g, b, a };
}

void pfBegin(PFdrawmode mode)
{
    currentCtx->vertexCount = 0;
    currentCtx->currentDrawMode = mode;
    currentCtx->renderBeginned = PF_TRUE;
}

void pfEnd(void)
{
    currentCtx->renderBeginned = PF_FALSE;
    currentCtx->vertexCount = 0;
}

void pfVertex2i(PFint x, PFint y)
{
    PFvec3f v = (PFvec3f) { (PFfloat)x, (PFfloat)y, currentCtx->currentDepth };
    pfVertexVec3f(&v);
}

void pfVertex2f(PFfloat x, PFfloat y)
{
    PFvec3f v = (PFvec3f) { x, y, currentCtx->currentDepth };
    pfVertexVec3f(&v);
}

void pfVertexVec2f(const PFvec2f* v)
{
    PFvec3f v3 = (PFvec3f) { v->x, v->y, currentCtx->currentDepth };
    pfVertexVec3f(&v3);
}

void pfVertex3f(PFfloat x, PFfloat y, PFfloat z)
{
    PFvec3f v = (PFvec3f) { x, y, z };
    pfVertexVec3f(&v);
}

void pfVertexVec3f(const PFvec3f* v)
{
    PFvec4f v4 = { v->x, v->y, v->z, 1.0f };

    currentCtx->vertexBuffer[currentCtx->vertexCount++] = (PFvertex) {
        (currentCtx->transformRequired) ? pfVec4fTransform(&v4, &currentCtx->transform) : v4,
        currentCtx->currentNormal, currentCtx->currentTexcoord, currentCtx->currentColor
    };

    if (currentCtx->vertexCount == currentCtx->currentDrawMode)
    {
        currentCtx->vertexCount = 0;
        PFmat4f mvp = pfMat4fMul(&currentCtx->modelview, &currentCtx->projection);
        ProcessRasterize(&mvp);
    }
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

void pfColor4f(PFfloat r, PFfloat g, PFfloat b , PFfloat a)
{
    currentCtx->currentColor = (PFcolor) {
        (PFubyte)(r*255),
        (PFubyte)(g*255),
        (PFubyte)(b*255),
        (PFubyte)(a*255)
    };
}

void pfColor4ub(PFubyte r, PFubyte g, PFubyte b, PFubyte a)
{
    currentCtx->currentColor = (PFcolor) { r, g, b, a };
}

void pfColor(PFcolor color)
{
    currentCtx->currentColor = color;
}

void pfTexCoord2f(PFfloat u, PFfloat v)
{
    currentCtx->currentTexcoord = (PFvec2f) { u, v };
}

void pfTexCoordVec2f(const PFvec2f* v)
{
    currentCtx->currentTexcoord = *v;
}

void pfNormal3f(PFfloat x, PFfloat y, PFfloat z)
{
    currentCtx->currentNormal = (PFvec3f) { x, y, z };
}

void pfNormalVec3f(const PFvec3f* v)
{
    currentCtx->currentNormal = *v;
}


/* Internal helper function definitions */

PFvertex Helper_LerpVertex(const PFvertex* start, const PFvertex* end, PFfloat t)
{
    PFvertex result;

    // Interpolate position
    result.position.x = start->position.x + t * (end->position.x - start->position.x);
    result.position.y = start->position.y + t * (end->position.y - start->position.y);
    result.position.z = start->position.z + t * (end->position.z - start->position.z);
    result.position.w = start->position.w + t * (end->position.w - start->position.w);

    // Interpolate normal
    result.normal.x = start->normal.x + t * (end->normal.x - start->normal.x);
    result.normal.y = start->normal.y + t * (end->normal.y - start->normal.y);
    result.normal.z = start->normal.z + t * (end->normal.z - start->normal.z);

    // Interpolate texcoord
    result.texcoord.x = start->texcoord.x + t * (end->texcoord.x - start->texcoord.x);
    result.texcoord.y = start->texcoord.y + t * (end->texcoord.y - start->texcoord.y);

    // Interpolate color
    result.color.r = start->color.r + t * (end->color.r - start->color.r);
    result.color.g = start->color.g + t * (end->color.g - start->color.g);
    result.color.b = start->color.b + t * (end->color.b - start->color.b);
    result.color.a = start->color.a + t * (end->color.a - start->color.a);

    return result;
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

PFvec2f Helper_InterpolateVec2f(const PFvec2f* v1, const PFvec2f* v2, const PFvec2f* v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    return (PFvec2f) {
        w1*v1->x + w2*v2->x + w3*v3->x,
        w1*v1->y + w2*v2->y + w3*v3->y
    };
}

PFcolor Helper_InterpolateColor(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    return (PFcolor) {
        (PFubyte)((w1*v1.r*INV_255 + w2*v2.r*INV_255 + w3*v3.r*INV_255)*255),
        (PFubyte)((w1*v1.g*INV_255 + w2*v2.g*INV_255 + w3*v3.g*INV_255)*255),
        (PFubyte)((w1*v1.b*INV_255 + w2*v2.b*INV_255 + w3*v3.b*INV_255)*255),
        (PFubyte)((w1*v1.a*INV_255 + w2*v2.a*INV_255 + w3*v3.a*INV_255)*255)
    };
}

void Helper_SwapVertex(PFvertex* a, PFvertex* b)
{
    PFvertex tmp = *a;
    *a = *b; *b = tmp;
}

void Helper_SwapByte(PFubyte* a, PFubyte* b)
{
    PFubyte tmp = *a;
    *a = *b; *b = tmp;
}

// Used by 'Process_ClipLine2D'
PFubyte Helper_EncodeClip2D(const PFvec4f* v)
{
    PFubyte code = CLIP_INSIDE;
    if (v->x < currentCtx->viewportX) code |= CLIP_LEFT;
    if (v->x > currentCtx->viewportW) code |= CLIP_RIGHT;
    if (v->y < currentCtx->viewportY) code |= CLIP_BOTTOM;
    if (v->y > currentCtx->viewportH) code |= CLIP_TOP;
    return code;
}

// Used by 'Process_ClipLine3D'
// 'q' represents a homogeneous coordinate weight from which a homogeneous coordinate 'x', 'y', or 'z' has been subtracted
// 'p' represents the delta between two homogeneous coordinate weights from which the corresponding homogeneous delta 'x', 'y', or 'z' has been subtracted
PFboolean Helper_ClipCoord3D(PFfloat q, PFfloat p, PFfloat* t1, PFfloat* t2)
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

void Process_HomogeneousToScreen(PFvec4f* homogeneous)
{
    homogeneous->x = currentCtx->viewportX + (homogeneous->x + 1.0f) * 0.5f * currentCtx->viewportW;
    homogeneous->y = currentCtx->viewportY + (1.0f - homogeneous->y) * 0.5f * currentCtx->viewportH;
}

PFboolean Process_ClipLine2D(PFvertex* v1, PFvertex* v2)
{
    PFboolean accept = PF_FALSE;
    PFubyte code0, code1;
    PFfloat m = 0;

    if (v1->position.x != v2->position.x)
    {
        m = (v2->position.y - v1->position.y) / (v2->position.x - v1->position.x);
    }

    for (;;)
    {
        code0 = Helper_EncodeClip2D(&v1->position);
        code1 = Helper_EncodeClip2D(&v2->position);

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
            v1->position.y += (currentCtx->viewportX - v1->position.x) * m;
            v1->position.x = currentCtx->viewportX;
        }
        else if (code0 & CLIP_RIGHT)
        {
            v1->position.y += (currentCtx->viewportW - v1->position.x) * m;
            v1->position.x = currentCtx->viewportW;
        }
        else if (code0 & CLIP_BOTTOM)
        {
            if (m) v1->position.x += (currentCtx->viewportY - v1->position.y) / m;
            v1->position.y = currentCtx->viewportY;
        }
        else if (code0 & CLIP_TOP)
        {
            if (m) v1->position.x += (currentCtx->viewportH - v1->position.y) / m;
            v1->position.y = currentCtx->viewportH;
        }
    }

    return accept;
}

PFboolean Process_ClipLine3D(PFvertex* v1, PFvertex* v2)
{
    PFfloat t1 = 0, t2 = 1;

    const PFvec4f delta = pfVec4fSub(&v2->position, &v1->position);

    if (!Helper_ClipCoord3D(v1->position.w - v1->position.x, -delta.w + delta.x, &t1, &t2)) return PF_FALSE;
    if (!Helper_ClipCoord3D(v1->position.w + v1->position.x, -delta.w - delta.x, &t1, &t2)) return PF_FALSE;

    if (!Helper_ClipCoord3D(v1->position.w - v1->position.y, -delta.w + delta.y, &t1, &t2)) return PF_FALSE;
    if (!Helper_ClipCoord3D(v1->position.w + v1->position.y, -delta.w - delta.y, &t1, &t2)) return PF_FALSE;

    if (!Helper_ClipCoord3D(v1->position.w - v1->position.z, -delta.w + delta.z, &t1, &t2)) return PF_FALSE;
    if (!Helper_ClipCoord3D(v1->position.w + v1->position.z, -delta.w - delta.z, &t1, &t2)) return PF_FALSE;

    if (t2 < 1)
    {
        PFvec4f d = pfVec4fScale(&delta, t2);
        v2->position = pfVec4fAdd(&v1->position, &d);
    }
    if (t1 > 0)
    {
        PFvec4f d = pfVec4fScale(&delta, t1);
        v1->position = pfVec4fAdd(&v1->position, &d);
    }

    return PF_TRUE;
}

PFboolean Process_ClipPolygonW(PFvertex* polygon, PFubyte* vertexCounter)
{
    PFvertex input[12];
    memcpy(input, polygon, sizeof(input));

    PFubyte inputCounter = *vertexCounter;
    *vertexCounter = 0;

    const PFvertex *prevVt = &input[inputCounter-1];
    PFbyte currDot, prevDot = (prevVt->position.w < PF_CLIP_EPSILON) ? -1 : 1;

    for (PFubyte i = 0; i < inputCounter; i++)
    {
        currDot = (input[i].position.w < PF_CLIP_EPSILON) ? -1 : 1;

        if (prevDot * currDot < 0)
        {
            polygon[(*vertexCounter)++] = Helper_LerpVertex(prevVt, &input[i], 
                (PF_CLIP_EPSILON - prevVt->position.w) / (prevVt->position.w - input[i].position.w));
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

PFboolean Process_ClipPolygonXYZ(PFvertex* polygon, PFubyte* vertexCounter)
{
    for (PFubyte iAxis = 0; iAxis < 3; iAxis++)
    {
        if (*vertexCounter == 0) return PF_FALSE;

        PFvertex input[12];
        memcpy(input, polygon, sizeof(input));

        PFubyte inputCounter = *vertexCounter;
        *vertexCounter = 0;

        const PFvertex *prevVt = &input[inputCounter-1];
        PFbyte prevDotHigh = (((PFfloat*)(&prevVt->position))[iAxis] <= prevVt->position.w) ? 1 : -1;
        PFbyte prevDotLow = (-((PFfloat*)(&prevVt->position))[iAxis] <= prevVt->position.w) ? 1 : -1;

        for (PFubyte i = 0; i < inputCounter; i++)
        {
            const PFbyte currDotHigh = (((PFfloat*)(&input[i].position))[iAxis] <= input[i].position.w) ? 1 : -1;
            const PFbyte currDotLow = (-((PFfloat*)(&input[i].position))[iAxis] <= input[i].position.w) ? 1 : -1;

            if (prevDotHigh * currDotHigh < 0)
            {
                polygon[(*vertexCounter)++] = Helper_LerpVertex(prevVt, &input[i], 
                    (prevVt->position.w - ((PFfloat*)(&prevVt->position))[iAxis]) / ((prevVt->position.w - ((PFfloat*)(&prevVt->position))[iAxis]) - (input[i].position.w - ((PFfloat*)(&input[i].position))[iAxis])));
            }

            if (prevDotLow * currDotLow < 0)
            {
                polygon[(*vertexCounter)++] = Helper_LerpVertex(prevVt, &input[i], 
                    (prevVt->position.w + ((PFfloat*)(&prevVt->position))[iAxis]) / ((prevVt->position.w + ((PFfloat*)(&prevVt->position))[iAxis]) - (input[i].position.w + ((PFfloat*)(&input[i].position))[iAxis])));
            }

            if (currDotHigh > 0 && currDotLow > 0)
            {
                polygon[(*vertexCounter)++] = input[i];
            }

            prevDotHigh = currDotHigh;
            prevDotLow = currDotLow;
            prevVt = &input[i];
        }
    }

    return *vertexCounter > 0;
}

void Process_ProjectAndClipLine(PFvertex* line, PFubyte* vertexCounter, const PFmat4f* mvp)
{
    line[0].position = pfVec4fTransform(&line[0].position, mvp);
    line[1].position = pfVec4fTransform(&line[1].position, mvp);

    if (line[0].position.w == 1.0f && line[1].position.w == 1.0f)
    {
        Process_HomogeneousToScreen(&line[0].position);
        Process_HomogeneousToScreen(&line[1].position);

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

        for (PFubyte i = 0; i < 2; i++)
        {
            // Division of XY coordinates by weight (perspective correct)
            PFfloat invW = 1.0f / line[i].position.w;
            line[i].position.x *= invW;
            line[i].position.y *= invW;
        }

        Process_HomogeneousToScreen(&line[0].position);
        Process_HomogeneousToScreen(&line[1].position);
    }
}

PFboolean Process_ProjectAndClipTriangle(PFvertex* polygon, PFubyte* vertexCounter, const PFmat4f* mvp)
{
    for (PFubyte i = 0; i < *vertexCounter; i++)
    {
        polygon[i].position = pfVec4fTransform(&polygon[i].position, mvp);
    }

    PFboolean is2D = (
        polygon[0].position.w == 1.0f &&
        polygon[1].position.w == 1.0f &&
        polygon[2].position.w == 1.0f);

    if (is2D)
    {
        for (PFubyte i = 0; i < *vertexCounter; i++)
        {
            Process_HomogeneousToScreen(&polygon[i].position);
        }
    }
    else
    {
        if (Process_ClipPolygonW(polygon, vertexCounter) && Process_ClipPolygonXYZ(polygon, vertexCounter))
        {
            for (PFubyte i = 0; i < *vertexCounter; i++)
            {
                // Calculation of the reciprocal of Z for the perspective correct
                polygon[i].position.z = 1.0f / polygon[i].position.z;

                // Division of texture coordinates by the Z axis (perspective correct)
                polygon[i].texcoord = pfVec2fScale(&polygon[i].texcoord, polygon[i].position.z);

                // Division of XY coordinates by weight (perspective correct)
                PFfloat invW = 1.0f / polygon[i].position.w;
                polygon[i].position.x *= invW;
                polygon[i].position.y *= invW;

                Process_HomogeneousToScreen(&polygon[i].position);
            }
        }
    }

    return is2D;
}


/* Internal line rasterizer function definitions */

void Rasterize_LineFlat(const PFvertex* v1, const PFvertex* v2)
{
    const PFfloat dx = v2->position.x - v1->position.x;
    const PFfloat dy = v2->position.y - v1->position.y;

    if (dx == 0 && dy == 0)
    {
        pfFramebufferSetPixel(currentCtx->currentFramebuffer, v1->position.x, v1->position.y, v1->color);
        return;
    }

    const PFfloat adx = fabsf(dx);
    const PFfloat ady = fabsf(dy);

    if (adx > ady)
    {
        const PFfloat invAdx = 1.0f / adx;
        const PFfloat slope = dy / dx;

        PFint xMin, xMax;
        if (v1->position.x < v2->position.x)
        {
            xMin = v1->position.x, xMax = v2->position.x;
        }
        else
        {
            xMin = v2->position.x, xMax = v1->position.x;
        }

        for (PFint x = xMin; x <= xMax; x++)
        {
            const PFfloat t = (x - xMin)*invAdx;
            const PFint y = v1->position.y + (x - v1->position.x)*slope;
            pfFramebufferSetPixel(currentCtx->currentFramebuffer, x, y, Helper_LerpColor(v1->color, v2->color, t));
        }
    }
    else
    {
        const PFfloat invAdy = 1.0f / ady;
        const PFfloat slope = dx / dy;

        PFint yMin, yMax;
        if (v1->position.y < v2->position.y)
        {
            yMin = v1->position.y, yMax = v2->position.y;
        }
        else
        {
            yMin = v2->position.y, yMax = v1->position.y;
        }

        for (PFint y = yMin; y <= yMax; y++)
        {
            const PFfloat t = (y - yMin)*invAdy;
            const PFint x = v1->position.x + (y - v1->position.y)*slope;
            pfFramebufferSetPixel(currentCtx->currentFramebuffer, x, y, Helper_LerpColor(v1->color, v2->color, t));
        }
    }
}

void Rasterize_LineDepth(const PFvertex* v1, const PFvertex* v2)
{
    const PFfloat dx = v2->position.x - v1->position.x;
    const PFfloat dy = v2->position.y - v1->position.y;

    if (dx == 0 && dy == 0)
    {
        pfFramebufferSetPixelDepth(currentCtx->currentFramebuffer, v1->position.x, v1->position.y, v1->position.z, v1->color);
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
        if (v1->position.x < v2->position.x)
        {
            xMin = v1->position.x, xMax = v2->position.x;
            zMin = v1->position.z, zMax = v2->position.z;
        }
        else
        {
            xMin = v2->position.x, xMax = v1->position.x;
            zMin = v2->position.z, zMax = v1->position.z;
        }

        for (PFint x = xMin; x <= xMax; x++)
        {
            const PFfloat t = (x - xMin)*invAdx;
            const PFfloat z = zMin + t*(zMax - zMin);
            const PFint y = v1->position.y + (x - v1->position.x)*slope;
            pfFramebufferSetPixelDepth(currentCtx->currentFramebuffer, x, y, z, Helper_LerpColor(v1->color, v2->color, t));
        }
    }
    else
    {
        const PFfloat invAdy = 1.0f / ady;
        const PFfloat slope = dx / dy;

        PFint yMin, yMax;
        PFfloat zMin, zMax;
        if (v1->position.y < v2->position.y)
        {
            yMin = v1->position.y, yMax = v2->position.y;
            zMin = v1->position.z, zMax = v2->position.z;
        }
        else
        {
            yMin = v2->position.y, yMax = v1->position.y;
            zMin = v2->position.z, zMax = v1->position.z;
        }

        for (PFint y = yMin; y <= yMax; y++)
        {
            const PFfloat t = (y - yMin)*invAdy;
            const PFfloat z = zMin + t*(zMax - zMin);
            const PFint x = v1->position.x + (y - v1->position.y)*slope;
            pfFramebufferSetPixelDepth(currentCtx->currentFramebuffer, x, y, z, Helper_LerpColor(v1->color, v2->color, t));
        }
    }
}


/* Internal triangle 2D rasterizer function definitions */

void Rasterize_TriangleColorFlat2D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    // Get integer 2D position coordinates
    const PFint x1 = (PFint)v1->position.x, y1 = (PFint)v1->position.y;
    const PFint x2 = (PFint)v2->position.x, y2 = (PFint)v2->position.y;
    const PFint x3 = (PFint)v3->position.x, y3 = (PFint)v3->position.y;

    // Check if vertices are in clockwise order or degenerate, in which case the triangle cannot be rendered
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) >= 0) return;

    // Calculate the 2D bounding box of the triangle clamped to the viewport dimensions
    const PFuint xMin = (PFuint)CLAMP(MIN(x1, MIN(x2, x3)), (PFint)currentCtx->viewportX, (PFint)(currentCtx->viewportX + currentCtx->viewportW));
    const PFuint yMin = (PFuint)CLAMP(MIN(y1, MIN(y2, y3)), (PFint)currentCtx->viewportY, (PFint)(currentCtx->viewportY + currentCtx->viewportH));
    const PFuint xMax = (PFuint)CLAMP(MAX(x1, MAX(x2, x3)), (PFint)currentCtx->viewportX, (PFint)(currentCtx->viewportX + currentCtx->viewportW));
    const PFuint yMax = (PFuint)CLAMP(MAX(y1, MAX(y2, y3)), (PFint)currentCtx->viewportY, (PFint)(currentCtx->viewportY + currentCtx->viewportH));

    // If triangle is entirely outside the viewport we can stop now
    if (xMin == xMax && yMin == yMax) return;

    // Calculate original edge weights relative to bounds.min
    // Will be used to obtain barycentric coordinates by incrementing then averaging them
    PFint w1Row = (xMin - x2)*(y3 - y2) - (x3 - x2)*(yMin - y2);
    PFint w2Row = (xMin - x3)*(y1 - y3) - (x1 - x3)*(yMin - y3);
    PFint w3Row = (xMin - x1)*(y2 - y1) - (x2 - x1)*(yMin - y1);

    // Calculate weight increment steps for each edge
    const PFuint stepWX1 = y3 - y2, stepWY1 = x2 - x3;
    const PFuint stepWX2 = y1 - y3, stepWY2 = x3 - x1;
    const PFuint stepWX3 = y2 - y1, stepWY3 = x1 - x2;

    // Fill the triangle with either color or image based on the provided parameters
    for (PFuint y = yMin; y <= yMax; y++)
    {
        const PFuint yOffset = y*currentCtx->currentFramebuffer->texture.width;
        PFint w1 = w1Row, w2 = w2Row, w3 = w3Row;

        for (PFuint x = xMin; x <= xMax; x++)
        {
            if ((w1 | w2 | w3) >= 0)
            {
                const PFuint xyOffset = yOffset + x;
                const PFfloat invSum = 1.0f/(w1 + w2 + w3);
                const PFfloat aW1 = w1*invSum, aW2 = w2*invSum, aW3 = w3*invSum;

                const PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
                const PFcolor dstCol = currentCtx->currentFramebuffer->texture.pixelGetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset);
                currentCtx->currentFramebuffer->texture.pixelSetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset, currentCtx->blendFunction(srcCol, dstCol));
            }

            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3;
        }

        w1Row += stepWY1, w2Row += stepWY2, w3Row += stepWY3;
    }
}

void Rasterize_TriangleColorDepth2D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    // Get integer 2D position coordinates
    const PFint x1 = (PFint)v1->position.x, y1 = (PFint)v1->position.y;
    const PFint x2 = (PFint)v2->position.x, y2 = (PFint)v2->position.y;
    const PFint x3 = (PFint)v3->position.x, y3 = (PFint)v3->position.y;

    // Check if vertices are in clockwise order or degenerate, in which case the triangle cannot be rendered
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) >= 0) return;

    // Calculate the 2D bounding box of the triangle clamped to the viewport dimensions
    const PFuint xMin = (PFuint)CLAMP(MIN(x1, MIN(x2, x3)), (PFint)currentCtx->viewportX, (PFint)(currentCtx->viewportX + currentCtx->viewportW));
    const PFuint yMin = (PFuint)CLAMP(MIN(y1, MIN(y2, y3)), (PFint)currentCtx->viewportY, (PFint)(currentCtx->viewportY + currentCtx->viewportH));
    const PFuint xMax = (PFuint)CLAMP(MAX(x1, MAX(x2, x3)), (PFint)currentCtx->viewportX, (PFint)(currentCtx->viewportX + currentCtx->viewportW));
    const PFuint yMax = (PFuint)CLAMP(MAX(y1, MAX(y2, y3)), (PFint)currentCtx->viewportY, (PFint)(currentCtx->viewportY + currentCtx->viewportH));

    // If triangle is entirely outside the viewport we can stop now
    if (xMin == xMax && yMin == yMax) return;

    // Calculate original edge weights relative to bounds.min
    // Will be used to obtain barycentric coordinates by incrementing then averaging them
    PFint w1Row = (xMin - x2)*(y3 - y2) - (x3 - x2)*(yMin - y2);
    PFint w2Row = (xMin - x3)*(y1 - y3) - (x1 - x3)*(yMin - y3);
    PFint w3Row = (xMin - x1)*(y2 - y1) - (x2 - x1)*(yMin - y1);

    // Calculate weight increment steps for each edge
    const PFuint stepWX1 = y3 - y2, stepWY1 = x2 - x3;
    const PFuint stepWX2 = y1 - y3, stepWY2 = x3 - x1;
    const PFuint stepWX3 = y2 - y1, stepWY3 = x1 - x2;

    // Fill the triangle with either color or image based on the provided parameters
    for (PFuint y = yMin; y <= yMax; y++)
    {
        const PFuint yOffset = y*currentCtx->currentFramebuffer->texture.width;
        PFint w1 = w1Row, w2 = w2Row, w3 = w3Row;

        for (PFuint x = xMin; x <= xMax; x++)
        {
            if ((w1 | w2 | w3) >= 0)
            {
                const PFuint xyOffset = yOffset + x;
                const PFfloat invSum = 1.0f/(w1 + w2 + w3);
                const PFfloat aW1 = w1*invSum, aW2 = w2*invSum, aW3 = w3*invSum;
                const PFfloat z = aW1*v1->position.z + aW2*v2->position.z + aW3*v3->position.z;

                if (z < currentCtx->currentFramebuffer->zbuffer[xyOffset])
                {
                    const PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
                    const PFcolor dstCol = currentCtx->currentFramebuffer->texture.pixelGetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset);
                    currentCtx->currentFramebuffer->texture.pixelSetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset, currentCtx->blendFunction(srcCol, dstCol));
                    currentCtx->currentFramebuffer->zbuffer[xyOffset] = z;
                }
            }

            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3;
        }

        w1Row += stepWY1, w2Row += stepWY2, w3Row += stepWY3;
    }
}

void Rasterize_TriangleTextureFlat2D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    // Get integer 2D position coordinates
    const PFint x1 = (PFint)v1->position.x, y1 = (PFint)v1->position.y;
    const PFint x2 = (PFint)v2->position.x, y2 = (PFint)v2->position.y;
    const PFint x3 = (PFint)v3->position.x, y3 = (PFint)v3->position.y;

    // Check if vertices are in clockwise order or degenerate, in which case the triangle cannot be rendered
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) >= 0) return;

    // Calculate the 2D bounding box of the triangle clamped to the viewport dimensions
    const PFuint xMin = (PFuint)CLAMP(MIN(x1, MIN(x2, x3)), (PFint)currentCtx->viewportX, (PFint)(currentCtx->viewportX + currentCtx->viewportW));
    const PFuint yMin = (PFuint)CLAMP(MIN(y1, MIN(y2, y3)), (PFint)currentCtx->viewportY, (PFint)(currentCtx->viewportY + currentCtx->viewportH));
    const PFuint xMax = (PFuint)CLAMP(MAX(x1, MAX(x2, x3)), (PFint)currentCtx->viewportX, (PFint)(currentCtx->viewportX + currentCtx->viewportW));
    const PFuint yMax = (PFuint)CLAMP(MAX(y1, MAX(y2, y3)), (PFint)currentCtx->viewportY, (PFint)(currentCtx->viewportY + currentCtx->viewportH));

    // If triangle is entirely outside the viewport we can stop now
    if (xMin == xMax && yMin == yMax) return;

    // Calculate original edge weights relative to bounds.min
    // Will be used to obtain barycentric coordinates by incrementing then averaging them
    PFint w1Row = (xMin - x2)*(y3 - y2) - (x3 - x2)*(yMin - y2);
    PFint w2Row = (xMin - x3)*(y1 - y3) - (x1 - x3)*(yMin - y3);
    PFint w3Row = (xMin - x1)*(y2 - y1) - (x2 - x1)*(yMin - y1);

    // Calculate weight increment steps for each edge
    const PFuint stepWX1 = y3 - y2, stepWY1 = x2 - x3;
    const PFuint stepWX2 = y1 - y3, stepWY2 = x3 - x1;
    const PFuint stepWX3 = y2 - y1, stepWY3 = x1 - x2;

    // Fill the triangle with either color or image based on the provided parameters
    for (PFuint y = yMin; y <= yMax; y++)
    {
        const PFuint yOffset = y*currentCtx->currentFramebuffer->texture.width;
        PFint w1 = w1Row, w2 = w2Row, w3 = w3Row;

        for (PFuint x = xMin; x <= xMax; x++)
        {
            if ((w1 | w2 | w3) >= 0)
            {
                const PFuint xyOffset = yOffset + x;
                const PFfloat invSum = 1.0f/(w1 + w2 + w3);
                const PFfloat aW1 = w1*invSum, aW2 = w2*invSum, aW3 = w3*invSum;

                const PFvec2f texcoord = Helper_InterpolateVec2f(&v1->texcoord, &v2->texcoord, &v3->texcoord, aW1, aW2, aW3);
                PFcolor texel = pfTextureGetFragment(currentCtx->currentTexture, texcoord.x, texcoord.y);

                PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
                srcCol = pfBlendMultiplicative(texel, srcCol);

                const PFcolor dstCol = currentCtx->currentFramebuffer->texture.pixelGetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset);
                const PFcolor finalColor = currentCtx->blendFunction(srcCol, dstCol);

                currentCtx->currentFramebuffer->texture.pixelSetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset, finalColor);
            }

            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3;
        }

        w1Row += stepWY1, w2Row += stepWY2, w3Row += stepWY3;
    }
}

void Rasterize_TriangleTextureDepth2D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    // Get integer 2D position coordinates
    const PFint x1 = (PFint)v1->position.x, y1 = (PFint)v1->position.y;
    const PFint x2 = (PFint)v2->position.x, y2 = (PFint)v2->position.y;
    const PFint x3 = (PFint)v3->position.x, y3 = (PFint)v3->position.y;

    // Check if vertices are in clockwise order or degenerate, in which case the triangle cannot be rendered
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) >= 0) return;

    // Calculate the 2D bounding box of the triangle clamped to the viewport dimensions
    const PFuint xMin = (PFuint)CLAMP(MIN(x1, MIN(x2, x3)), (PFint)currentCtx->viewportX, (PFint)(currentCtx->viewportX + currentCtx->viewportW));
    const PFuint yMin = (PFuint)CLAMP(MIN(y1, MIN(y2, y3)), (PFint)currentCtx->viewportY, (PFint)(currentCtx->viewportY + currentCtx->viewportH));
    const PFuint xMax = (PFuint)CLAMP(MAX(x1, MAX(x2, x3)), (PFint)currentCtx->viewportX, (PFint)(currentCtx->viewportX + currentCtx->viewportW));
    const PFuint yMax = (PFuint)CLAMP(MAX(y1, MAX(y2, y3)), (PFint)currentCtx->viewportY, (PFint)(currentCtx->viewportY + currentCtx->viewportH));

    // If triangle is entirely outside the viewport we can stop now
    if (xMin == xMax && yMin == yMax) return;

    // Calculate original edge weights relative to bounds.min
    // Will be used to obtain barycentric coordinates by incrementing then averaging them
    PFint w1Row = (xMin - x2)*(y3 - y2) - (x3 - x2)*(yMin - y2);
    PFint w2Row = (xMin - x3)*(y1 - y3) - (x1 - x3)*(yMin - y3);
    PFint w3Row = (xMin - x1)*(y2 - y1) - (x2 - x1)*(yMin - y1);

    // Calculate weight increment steps for each edge
    const PFuint stepWX1 = y3 - y2, stepWY1 = x2 - x3;
    const PFuint stepWX2 = y1 - y3, stepWY2 = x3 - x1;
    const PFuint stepWX3 = y2 - y1, stepWY3 = x1 - x2;

    // Fill the triangle with either color or image based on the provided parameters
    for (PFuint y = yMin; y <= yMax; y++)
    {
        const PFuint yOffset = y*currentCtx->currentFramebuffer->texture.width;
        PFint w1 = w1Row, w2 = w2Row, w3 = w3Row;

        for (PFuint x = xMin; x <= xMax; x++)
        {
            if ((w1 | w2 | w3) >= 0)
            {
                const PFuint xyOffset = yOffset + x;
                const PFfloat invSum = 1.0f/(w1 + w2 + w3);
                const PFfloat aW1 = w1*invSum, aW2 = w2*invSum, aW3 = w3*invSum;
                const PFfloat z = aW1*v1->position.z + aW2*v2->position.z + aW3*v3->position.z;

                if (z < currentCtx->currentFramebuffer->zbuffer[xyOffset])
                {
                    const PFvec2f texcoord = Helper_InterpolateVec2f(&v1->texcoord, &v2->texcoord, &v3->texcoord, aW1, aW2, aW3);
                    PFcolor texel = pfTextureGetFragment(currentCtx->currentTexture, texcoord.x, texcoord.y);

                    PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
                    srcCol = pfBlendMultiplicative(texel, srcCol);

                    const PFcolor dstCol = currentCtx->currentFramebuffer->texture.pixelGetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset);
                    const PFcolor finalColor = currentCtx->blendFunction(srcCol, dstCol);

                    currentCtx->currentFramebuffer->texture.pixelSetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset, finalColor);
                    currentCtx->currentFramebuffer->zbuffer[xyOffset] = z;
                }
            }

            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3;
        }

        w1Row += stepWY1, w2Row += stepWY2, w3Row += stepWY3;
    }
}


/* Internal triangle 3D rasterizer function definitions */

void Rasterize_TriangleColorFlat3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    // Get integer 2D position coordinates
    const PFint x1 = (PFint)v1->position.x, y1 = (PFint)v1->position.y;
    const PFint x2 = (PFint)v2->position.x, y2 = (PFint)v2->position.y;
    const PFint x3 = (PFint)v3->position.x, y3 = (PFint)v3->position.y;

    // Check if vertices are in clockwise order or degenerate, in which case the triangle cannot be rendered
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) >= 0) return;

    // Calculate the 2D bounding box of the triangle clamped to the viewport dimensions
    const PFuint xMin = MIN(x1, MIN(x2, x3));
    const PFuint yMin = MIN(y1, MIN(y2, y3));
    const PFuint xMax = MAX(x1, MAX(x2, x3));
    const PFuint yMax = MAX(y1, MAX(y2, y3));

    // If triangle is entirely outside the viewport we can stop now
    if (xMin == xMax && yMin == yMax) return;

    // Calculate original edge weights relative to bounds.min
    // Will be used to obtain barycentric coordinates by incrementing then averaging them
    PFint w1Row = (xMin - x2)*(y3 - y2) - (x3 - x2)*(yMin - y2);
    PFint w2Row = (xMin - x3)*(y1 - y3) - (x1 - x3)*(yMin - y3);
    PFint w3Row = (xMin - x1)*(y2 - y1) - (x2 - x1)*(yMin - y1);

    // Calculate weight increment steps for each edge
    const PFuint stepWX1 = y3 - y2, stepWY1 = x2 - x3;
    const PFuint stepWX2 = y1 - y3, stepWY2 = x3 - x1;
    const PFuint stepWX3 = y2 - y1, stepWY3 = x1 - x2;

    // Fill the triangle with either color or image based on the provided parameters
    for (PFuint y = yMin; y <= yMax; y++)
    {
        const PFuint yOffset = y*currentCtx->currentFramebuffer->texture.width;
        PFint w1 = w1Row, w2 = w2Row, w3 = w3Row;

        for (PFuint x = xMin; x <= xMax; x++)
        {
            if ((w1 | w2 | w3) >= 0)
            {
                const PFuint xyOffset = yOffset + x;
                const PFfloat invSum = 1.0f/(w1 + w2 + w3);
                const PFfloat aW1 = w1*invSum, aW2 = w2*invSum, aW3 = w3*invSum;
                const PFfloat z = 1.0f/(aW1*v1->position.z + aW2*v2->position.z + aW3*v3->position.z);

                const PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
                const PFcolor dstCol = currentCtx->currentFramebuffer->texture.pixelGetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset);
                currentCtx->currentFramebuffer->texture.pixelSetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset, currentCtx->blendFunction(srcCol, dstCol));
                currentCtx->currentFramebuffer->zbuffer[xyOffset] = z;
            }

            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3;
        }

        w1Row += stepWY1, w2Row += stepWY2, w3Row += stepWY3;
    }
}

void Rasterize_TriangleColorDepth3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    // Get integer 2D position coordinates
    const PFint x1 = (PFint)v1->position.x, y1 = (PFint)v1->position.y;
    const PFint x2 = (PFint)v2->position.x, y2 = (PFint)v2->position.y;
    const PFint x3 = (PFint)v3->position.x, y3 = (PFint)v3->position.y;

    // Check if vertices are in clockwise order or degenerate, in which case the triangle cannot be rendered
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) >= 0) return;

    // Calculate the 2D bounding box of the triangle clamped to the viewport dimensions
    const PFuint xMin = MIN(x1, MIN(x2, x3));
    const PFuint yMin = MIN(y1, MIN(y2, y3));
    const PFuint xMax = MAX(x1, MAX(x2, x3));
    const PFuint yMax = MAX(y1, MAX(y2, y3));

    // If triangle is entirely outside the viewport we can stop now
    if (xMin == xMax && yMin == yMax) return;

    // Calculate original edge weights relative to bounds.min
    // Will be used to obtain barycentric coordinates by incrementing then averaging them
    PFint w1Row = (xMin - x2)*(y3 - y2) - (x3 - x2)*(yMin - y2);
    PFint w2Row = (xMin - x3)*(y1 - y3) - (x1 - x3)*(yMin - y3);
    PFint w3Row = (xMin - x1)*(y2 - y1) - (x2 - x1)*(yMin - y1);

    // Calculate weight increment steps for each edge
    const PFuint stepWX1 = y3 - y2, stepWY1 = x2 - x3;
    const PFuint stepWX2 = y1 - y3, stepWY2 = x3 - x1;
    const PFuint stepWX3 = y2 - y1, stepWY3 = x1 - x2;

    // Fill the triangle with either color or image based on the provided parameters
    for (PFuint y = yMin; y <= yMax; y++)
    {
        const PFuint yOffset = y*currentCtx->currentFramebuffer->texture.width;
        PFint w1 = w1Row, w2 = w2Row, w3 = w3Row;

        for (PFuint x = xMin; x <= xMax; x++)
        {
            if ((w1 | w2 | w3) >= 0)
            {
                const PFuint xyOffset = yOffset + x;
                const PFfloat invSum = 1.0f/(w1 + w2 + w3);
                const PFfloat aW1 = w1*invSum, aW2 = w2*invSum, aW3 = w3*invSum;
                const PFfloat z = 1.0f/(aW1*v1->position.z + aW2*v2->position.z + aW3*v3->position.z);

                if (z < currentCtx->currentFramebuffer->zbuffer[xyOffset])
                {
                    const PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
                    const PFcolor dstCol = currentCtx->currentFramebuffer->texture.pixelGetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset);
                    currentCtx->currentFramebuffer->texture.pixelSetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset, currentCtx->blendFunction(srcCol, dstCol));
                    currentCtx->currentFramebuffer->zbuffer[xyOffset] = z;
                }
            }

            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3;
        }

        w1Row += stepWY1, w2Row += stepWY2, w3Row += stepWY3;
    }
}

void Rasterize_TriangleTextureFlat3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    // Get integer 2D position coordinates
    const PFint x1 = (PFint)v1->position.x, y1 = (PFint)v1->position.y;
    const PFint x2 = (PFint)v2->position.x, y2 = (PFint)v2->position.y;
    const PFint x3 = (PFint)v3->position.x, y3 = (PFint)v3->position.y;

    // Check if vertices are in clockwise order or degenerate, in which case the triangle cannot be rendered
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) >= 0) return;

    // Calculate the 2D bounding box of the triangle clamped to the viewport dimensions
    const PFuint xMin = MIN(x1, MIN(x2, x3));
    const PFuint yMin = MIN(y1, MIN(y2, y3));
    const PFuint xMax = MAX(x1, MAX(x2, x3));
    const PFuint yMax = MAX(y1, MAX(y2, y3));

    // If triangle is entirely outside the viewport we can stop now
    if (xMin == xMax && yMin == yMax) return;

    // Calculate original edge weights relative to bounds.min
    // Will be used to obtain barycentric coordinates by incrementing then averaging them
    PFint w1Row = (xMin - x2)*(y3 - y2) - (x3 - x2)*(yMin - y2);
    PFint w2Row = (xMin - x3)*(y1 - y3) - (x1 - x3)*(yMin - y3);
    PFint w3Row = (xMin - x1)*(y2 - y1) - (x2 - x1)*(yMin - y1);

    // Calculate weight increment steps for each edge
    const PFuint stepWX1 = y3 - y2, stepWY1 = x2 - x3;
    const PFuint stepWX2 = y1 - y3, stepWY2 = x3 - x1;
    const PFuint stepWX3 = y2 - y1, stepWY3 = x1 - x2;

    // Fill the triangle with either color or image based on the provided parameters
    for (PFuint y = yMin; y <= yMax; y++)
    {
        const PFuint yOffset = y*currentCtx->currentFramebuffer->texture.width;
        PFint w1 = w1Row, w2 = w2Row, w3 = w3Row;

        for (PFuint x = xMin; x <= xMax; x++)
        {
            if ((w1 | w2 | w3) >= 0)
            {
                const PFuint xyOffset = yOffset + x;
                const PFfloat invSum = 1.0f/(w1 + w2 + w3);
                const PFfloat aW1 = w1*invSum, aW2 = w2*invSum, aW3 = w3*invSum;
                const PFfloat z = 1.0f/(aW1*v1->position.z + aW2*v2->position.z + aW3*v3->position.z);

                const PFvec2f texcoord = Helper_InterpolateVec2f(&v1->texcoord, &v2->texcoord, &v3->texcoord, aW1, aW2, aW3);
                PFcolor texel = pfTextureGetFragment(currentCtx->currentTexture, texcoord.x, texcoord.y);

                PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
                srcCol = pfBlendMultiplicative(texel, srcCol);

                const PFcolor dstCol = currentCtx->currentFramebuffer->texture.pixelGetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset);
                const PFcolor finalColor = currentCtx->blendFunction(srcCol, dstCol);

                currentCtx->currentFramebuffer->texture.pixelSetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset, finalColor);
                currentCtx->currentFramebuffer->zbuffer[xyOffset] = z;
            }

            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3;
        }

        w1Row += stepWY1, w2Row += stepWY2, w3Row += stepWY3;
    }
}

void Rasterize_TriangleTextureDepth3D(const PFvertex* v1, const PFvertex* v2, const PFvertex* v3)
{
    // Get integer 2D position coordinates
    const PFint x1 = (PFint)v1->position.x, y1 = (PFint)v1->position.y;
    const PFint x2 = (PFint)v2->position.x, y2 = (PFint)v2->position.y;
    const PFint x3 = (PFint)v3->position.x, y3 = (PFint)v3->position.y;

    // Check if vertices are in clockwise order or degenerate, in which case the triangle cannot be rendered
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) >= 0) return;

    // Calculate the 2D bounding box of the triangle clamped to the viewport dimensions
    const PFuint xMin = MIN(x1, MIN(x2, x3));
    const PFuint yMin = MIN(y1, MIN(y2, y3));
    const PFuint xMax = MAX(x1, MAX(x2, x3));
    const PFuint yMax = MAX(y1, MAX(y2, y3));

    // If triangle is entirely outside the viewport we can stop now
    if (xMin == xMax && yMin == yMax) return;

    // Calculate original edge weights relative to bounds.min
    // Will be used to obtain barycentric coordinates by incrementing then averaging them
    PFint w1Row = (xMin - x2)*(y3 - y2) - (x3 - x2)*(yMin - y2);
    PFint w2Row = (xMin - x3)*(y1 - y3) - (x1 - x3)*(yMin - y3);
    PFint w3Row = (xMin - x1)*(y2 - y1) - (x2 - x1)*(yMin - y1);

    // Calculate weight increment steps for each edge
    const PFuint stepWX1 = y3 - y2, stepWY1 = x2 - x3;
    const PFuint stepWX2 = y1 - y3, stepWY2 = x3 - x1;
    const PFuint stepWX3 = y2 - y1, stepWY3 = x1 - x2;

    // Fill the triangle with either color or image based on the provided parameters
    for (PFuint y = yMin; y <= yMax; y++)
    {
        const PFuint yOffset = y*currentCtx->currentFramebuffer->texture.width;
        PFint w1 = w1Row, w2 = w2Row, w3 = w3Row;

        for (PFuint x = xMin; x <= xMax; x++)
        {
            if ((w1 | w2 | w3) >= 0)
            {
                const PFuint xyOffset = yOffset + x;
                const PFfloat invSum = 1.0f/(w1 + w2 + w3);
                const PFfloat aW1 = w1*invSum, aW2 = w2*invSum, aW3 = w3*invSum;
                const PFfloat z = 1.0f/(aW1*v1->position.z + aW2*v2->position.z + aW3*v3->position.z);

                if (z < currentCtx->currentFramebuffer->zbuffer[xyOffset])
                {
                    PFvec2f texcoord = Helper_InterpolateVec2f(&v1->texcoord, &v2->texcoord, &v3->texcoord, aW1, aW2, aW3);
                    texcoord = pfVec2fScale(&texcoord, z); // Perspective correct

                    PFcolor srcCol = Helper_InterpolateColor(v1->color, v2->color, v3->color, aW1, aW2, aW3);
                    PFcolor texel = pfTextureGetFragment(currentCtx->currentTexture, texcoord.x, texcoord.y);
                    srcCol = pfBlendMultiplicative(texel, srcCol);

                    const PFcolor dstCol = currentCtx->currentFramebuffer->texture.pixelGetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset);
                    const PFcolor finalColor = currentCtx->blendFunction(srcCol, dstCol);

                    currentCtx->currentFramebuffer->texture.pixelSetter(currentCtx->currentFramebuffer->texture.pixels, xyOffset, finalColor);
                    currentCtx->currentFramebuffer->zbuffer[xyOffset] = z;
                }
            }

            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3;
        }

        w1Row += stepWY1, w2Row += stepWY2, w3Row += stepWY3;
    }
}

/* Internal processing and rasterization function definitions */

void ProcessRasterize(const PFmat4f* mvp)
{
    switch (currentCtx->currentDrawMode)
    {
        case PF_LINES:
        {
            PFubyte processedCounter = 2;

            PFvertex processed[2] = {
                currentCtx->vertexBuffer[0],
                currentCtx->vertexBuffer[1]
            };

            Process_ProjectAndClipLine(processed, &processedCounter, mvp);

            if (processedCounter == 2)
            {
                if (currentCtx->renderState & (PF_DEPTH_TEST))
                {
                    Rasterize_LineDepth(&processed[0], &processed[1]);
                }
                else
                {
                    Rasterize_LineFlat(&processed[0], &processed[1]);
                }
            }
        }
        break;

        case PF_TRIANGLES:
        {
            PFubyte processedCounter = 3;

            PFvertex processed[12] = {
                currentCtx->vertexBuffer[0],
                currentCtx->vertexBuffer[1],
                currentCtx->vertexBuffer[2]
            };

            PFboolean is2D = Process_ProjectAndClipTriangle(processed, &processedCounter, mvp);

            if (is2D)
            {
                if ((currentCtx->renderState & (PF_TEXTURE_MODE | PF_DEPTH_TEST)) == (PF_TEXTURE_MODE | PF_DEPTH_TEST))
                {
                    for (PFbyte i = 0; i < processedCounter - 2; i++)
                    {
                        Rasterize_TriangleTextureDepth2D(&processed[0], &processed[i + 1], &processed[i + 2]);
                    }
                }
                else if (currentCtx->renderState & (PF_TEXTURE_MODE))
                {
                    for (PFbyte i = 0; i < processedCounter - 2; i++)
                    {
                        Rasterize_TriangleTextureFlat2D(&processed[0], &processed[i + 1], &processed[i + 2]);
                    }
                }
                else if (currentCtx->renderState & (PF_DEPTH_TEST))
                {
                    for (PFbyte i = 0; i < processedCounter - 2; i++)
                    {
                        Rasterize_TriangleColorDepth2D(&processed[0], &processed[i + 1], &processed[i + 2]);
                    }
                }
                else
                {
                    for (PFbyte i = 0; i < processedCounter - 2; i++)
                    {
                        Rasterize_TriangleColorFlat2D(&processed[0], &processed[i + 1], &processed[i + 2]);
                    }
                }
            }
            else
            {
                if ((currentCtx->renderState & (PF_TEXTURE_MODE | PF_DEPTH_TEST)) == (PF_TEXTURE_MODE | PF_DEPTH_TEST))
                {
                    for (PFbyte i = 0; i < processedCounter - 2; i++)
                    {
                        Rasterize_TriangleTextureDepth3D(&processed[0], &processed[i + 1], &processed[i + 2]);
                    }
                }
                else if (currentCtx->renderState & (PF_TEXTURE_MODE))
                {
                    for (PFbyte i = 0; i < processedCounter - 2; i++)
                    {
                        Rasterize_TriangleTextureFlat3D(&processed[0], &processed[i + 1], &processed[i + 2]);
                    }
                }
                else if (currentCtx->renderState & (PF_DEPTH_TEST))
                {
                    for (PFbyte i = 0; i < processedCounter - 2; i++)
                    {
                        Rasterize_TriangleColorDepth3D(&processed[0], &processed[i + 1], &processed[i + 2]);
                    }
                }
                else
                {
                    for (PFbyte i = 0; i < processedCounter - 2; i++)
                    {
                        Rasterize_TriangleColorFlat3D(&processed[0], &processed[i + 1], &processed[i + 2]);
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

                PFvertex processed[12] = {
                    currentCtx->vertexBuffer[0],
                    currentCtx->vertexBuffer[i + 1],
                    currentCtx->vertexBuffer[i + 2]
                };

                PFboolean is2D = Process_ProjectAndClipTriangle(processed, &processedCounter, mvp);

                if (is2D)
                {
                    if ((currentCtx->renderState & (PF_TEXTURE_MODE | PF_DEPTH_TEST)) == (PF_TEXTURE_MODE | PF_DEPTH_TEST))
                    {
                        for (PFbyte j = 0; j < processedCounter - j; j++)
                        {
                            Rasterize_TriangleTextureDepth2D(&processed[0], &processed[j + 1], &processed[j + 2]);
                        }
                    }
                    else if (currentCtx->renderState & (PF_TEXTURE_MODE))
                    {
                        for (PFbyte j = 0; j < processedCounter - 2; j++)
                        {
                            Rasterize_TriangleTextureFlat2D(&processed[0], &processed[j + 1], &processed[j + 2]);
                        }
                    }
                    else if (currentCtx->renderState & (PF_DEPTH_TEST))
                    {
                        for (PFbyte j = 0; j < processedCounter - 2; j++)
                        {
                            Rasterize_TriangleColorDepth2D(&processed[0], &processed[j + 1], &processed[j + 2]);
                        }
                    }
                    else
                    {
                        for (PFbyte j = 0; j < processedCounter - 2; j++)
                        {
                            Rasterize_TriangleColorFlat2D(&processed[0], &processed[j + 1], &processed[j + 2]);
                        }
                    }
                }
                else
                {
                    if ((currentCtx->renderState & (PF_TEXTURE_MODE | PF_DEPTH_TEST)) == (PF_TEXTURE_MODE | PF_DEPTH_TEST))
                    {
                        for (PFbyte j = 0; j < processedCounter - 2; j++)
                        {
                            Rasterize_TriangleTextureDepth3D(&processed[0], &processed[j + 1], &processed[j + 2]);
                        }
                    }
                    else if (currentCtx->renderState & (PF_TEXTURE_MODE))
                    {
                        for (PFbyte j = 0; j < processedCounter - 2; j++)
                        {
                            Rasterize_TriangleTextureFlat3D(&processed[0], &processed[j + 1], &processed[j + 2]);
                        }
                    }
                    else if (currentCtx->renderState & (PF_DEPTH_TEST))
                    {
                        for (PFbyte j = 0; j < processedCounter - 2; j++)
                        {
                            Rasterize_TriangleColorDepth3D(&processed[0], &processed[j + 1], &processed[j + 2]);
                        }
                    }
                    else
                    {
                        for (PFbyte j = 0; j < processedCounter - 2; j++)
                        {
                            Rasterize_TriangleColorFlat3D(&processed[0], &processed[j + 1], &processed[j + 2]);
                        }
                    }
                }
            }
        }
        break;
    }
}
