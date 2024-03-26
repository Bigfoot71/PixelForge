#ifndef PIXEL_FORGE_H
#define PIXEL_FORGE_H

#include <stdint.h>

#if defined(_WIN32)
#   if defined(__TINYC__)
#       define __declspec(x) __attribute__((x))
#   endif
#   if defined(PF_BUILD_SHARED)
#       define PF_API __declspec(dllexport)
#   elif defined(USE_LIBTYPE_SHARED)
#       define PF_API __declspec(dllimport)
#   endif
#else
#   if defined(PF_BUILD_SHARED)
#       define PF_API __attribute__((visibility("default")))
#   endif
#endif //OS

#ifndef PF_API
    #define PF_API
#endif //PF_API

#ifndef PF_MALLOC
#   define PF_MALLOC(size) malloc(size)
#endif //PF_MALLOC

#ifndef PF_CALLOC
#   define PF_CALLOC(count, size) calloc(count, size)
#endif //PF_CALLOC

#ifndef PF_FREE
#   define PF_FREE(size) free(size)
#endif //PF_FREE

#ifndef PF_MAX_MATRIX_STACK_SIZE
#   define PF_MAX_MATRIX_STACK_SIZE 8
#endif //PF_MAX_MATRIX_STACK_SIZE

#ifndef PF_MAX_CLIPPED_POLYGON_VERTEX
#   define PF_MAX_CLIPPED_POLYGON_VERTEX 12
#endif //PF_MAX_CLIPPED_POLYGON_VERTEX

#ifndef PF_PIXELFORMAT_R5G5B5A1_ALPHA_THRESHOLD
    #define PF_PIXELFORMAT_R5G5B5A1_ALPHA_THRESHOLD 50  // Threshold over 255 to set alpha as 0
#endif //PF_PIXELFORMAT_R5G5B5A1_ALPHA_THRESHOLD

#ifndef PF_CLIP_EPSILON
#   define PF_CLIP_EPSILON 1e-5f
#endif //PF_CLIP_EPSILON

#ifndef INV_255
#   define INV_255 (1.0 / 255)
#endif //INV_255

#ifndef M_PI
#   define M_PI 3.14159265358979323846
#endif //M_PI

#ifndef DEG2RAD
#   define DEG2RAD(deg) ((deg) * M_PI / 180.0)
#endif //DEG2RAD

#ifndef MIN
#   define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif //MIN

#ifndef MAX
#   define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif //MAX

#ifndef CLAMP
#   define CLAMP(x, lower, upper) (MAX((lower), MIN((x), (upper))))
#endif // CLAMP

/* Types definitions */

typedef enum {
    PF_FALSE = 0,
    PF_TRUE = 1
} PFboolean;

typedef int8_t      PFbyte;
typedef uint8_t     PFubyte;
typedef int16_t     PFshort;
typedef uint16_t    PFushort;
typedef int32_t     PFint;
typedef uint32_t    PFuint;
typedef int64_t     PFint64;
typedef uint64_t    PFuint64;
typedef uint32_t    PFsizei;
typedef uint32_t    PFenum;
typedef intptr_t    PFintptr;
typedef uintptr_t   PFsizeiptr;
typedef float       PFfloat;
typedef double      PFdouble;

/* Log system definitions */

typedef enum {
    PF_LOG_DEBUG,
    PF_LOG_INFO,
    PF_LOG_WARNING,
    PF_LOG_ERROR
} PFlog;

#ifndef PF_LOG
#   include <stdio.h>
#   define PF_LOG(level, format, ...)                                   \
        switch(level) {                                                 \
            case PF_LOG_DEBUG:                                          \
                printf("DEBUG: " format "\n", ##__VA_ARGS__);           \
                break;                                                  \
            case PF_LOG_INFO:                                           \
                printf("INFO: " format "\n", ##__VA_ARGS__);            \
                break;                                                  \
            case PF_LOG_WARNING:                                        \
                printf("WARNING: " format "\n", ##__VA_ARGS__);         \
                break;                                                  \
            case PF_LOG_ERROR:                                          \
                fprintf(stderr, "ERROR: " format "\n", ##__VA_ARGS__);  \
                break;                                                  \
            default:                                                    \
                break;                                                  \
        }
#endif //PF_LOG

/* Context defintions */

typedef struct PFctx PFctx;

/* Math definitions */

typedef struct {
    PFfloat x, y;
} PFvec2f;

typedef struct {
    PFfloat x, y, z;
} PFvec3f;

typedef struct {
    PFfloat x, y, z, w;
} PFvec4f;

typedef struct {
    PFfloat m0, m4, m8, m12;
    PFfloat m1, m5, m9, m13;
    PFfloat m2, m6, m10, m14;
    PFfloat m3, m7, m11, m15;
} PFmat4f;

/* Render definitions */

typedef enum {
    PF_VERTEX_ARRAY         = 0x01,
    PF_NORMAL_ARRAY         = 0x02,
    PF_COLOR_ARRAY          = 0x04,
    PF_TEXTURE_COORD_ARRAY  = 0x08
} PFarraytype;

typedef enum {
    PF_MODELVIEW,
    PF_PROJECTION,
} PFmatrixmode;

typedef enum {
    PF_LINES     = 2,
    PF_TRIANGLES = 3,
    PF_QUADS     = 4,
} PFdrawmode;

typedef struct {
    PFubyte r, g, b, a;
} PFcolor;

typedef struct {
    PFvec4f position;
    PFvec3f normal;
    PFvec2f texcoord;
    PFcolor color;
} PFvertex;

typedef PFcolor (*PFblendfunc)(PFcolor, PFcolor);

/* Texture definitions */

typedef struct PFtexture PFtexture;

typedef void (*PFpixelsetter)(void*, PFsizei, PFcolor);
typedef PFcolor (*PFpixelgetter)(const void*, PFsizei);

typedef enum {
    PF_PIXELFORMAT_UNKNOWN = 0,
    PF_PIXELFORMAT_GRAYSCALE,
    PF_PIXELFORMAT_GRAY_ALPHA,
    PF_PIXELFORMAT_R5G6B5,
    PF_PIXELFORMAT_R8G8B8,
    PF_PIXELFORMAT_R5G5B5A1,
    PF_PIXELFORMAT_R4G4B4A4,
    PF_PIXELFORMAT_R8G8B8A8,
    PF_PIXELFORMAT_R32,
    PF_PIXELFORMAT_R32G32B32,
    PF_PIXELFORMAT_R32G32B32A32,
    PF_PIXELFORMAT_R16,
    PF_PIXELFORMAT_R16G16B16,
    PF_PIXELFORMAT_R16G16B16A16,
} PFpixelformat;

struct PFtexture {
    PFpixelsetter pixelSetter;
    PFpixelgetter pixelGetter;
    void *pixels;
    PFuint width;
    PFuint height;
    PFpixelformat format;
};

/* Framebuffer defintions */

typedef struct {
    PFtexture texture;
    PFfloat *zbuffer;
} PFframebuffer;

#if defined(__cplusplus)
extern "C" {
#endif //__cplusplus

/* Context API functions */

PF_API PFctx* pfContextCreate(void* screenBuffer, PFuint screenWidth, PFuint screenHeight, PFpixelformat screenFormat);
PF_API void pfContextDestroy(PFctx* ctx);

PF_API PFctx* pfGetCurrent(void);
PF_API void pfMakeCurrent(PFctx* ctx);
PF_API PFboolean pfIsCurrent(PFctx* ctx);

/* Render API functions */

PF_API void pfMatrixMode(PFmatrixmode mode);

PF_API void pfPushMatrix(void);
PF_API void pfPopMatrix(void);

PF_API void pfLoadIdentity(void);

PF_API void pfTranslatef(PFfloat x, PFfloat y, PFfloat z);
PF_API void pfRotatef(PFfloat angle, PFfloat x, PFfloat y, PFfloat z);
PF_API void pfScalef(PFfloat x, PFfloat y, PFfloat z);

PF_API void pfMultMatrixf(const float* mat);
PF_API void pfMultMatrixMat4f(const PFmat4f* mat);

PF_API void pfFrustum(PFdouble left, PFdouble right, PFdouble bottom, PFdouble top, PFdouble znear, PFdouble zfar);
PF_API void pfOrtho(PFdouble left, PFdouble right, PFdouble bottom, PFdouble top, PFdouble znear, PFdouble zfar);

PF_API void pfGetViewport(PFuint* x, PFuint* y, PFuint* width, PFuint* height);
PF_API void pfViewport(PFuint x, PFuint y, PFuint width, PFuint height);

PF_API void pfSetDefaultPixelGetter(PFpixelgetter func);
PF_API void pfSetDefaultPixelSetter(PFpixelsetter func);

PF_API PFblendfunc pfGetBlendFunction();
PF_API void pfSetBlendFunction(PFblendfunc func);

PF_API void pfDrawVertexArrayElements(PFsizei offset, PFsizei count, const void *buffer);
PF_API void pfDrawVertexArray(PFsizei offset, PFsizei count);

PF_API void pfEnableStatePointer(PFarraytype vertexAttribType, const void* buffer);
PF_API void pfDisableStatePointer(PFarraytype vertexAttribType);

PF_API PFframebuffer* pfGetActiveFramebuffer(void);
PF_API void pfEnableFramebuffer(PFframebuffer* framebuffer);
PF_API void pfDisableFramebuffer(void);

PF_API PFtexture* pfGetActiveTexture(void);
PF_API void pfEnableTexture(PFtexture* texture);
PF_API void pfDisableTexture();

PF_API void pfEnableWireMode(void);
PF_API void pfDisableWireMode(void);

PF_API void pfEnableDepthTest(void);
PF_API void pfDisableDepthTest(void);

PF_API void pfClear(void);
PF_API void pfClearColor(PFubyte r, PFubyte g, PFubyte b, PFubyte a);

PF_API void pfBegin(PFdrawmode mode);
PF_API void pfEnd(void);

PF_API void pfVertex2i(PFint x, PFint y);
PF_API void pfVertex2f(PFfloat x, PFfloat y);
PF_API void pfVertexVec2f(const PFvec2f* v);

PF_API void pfVertex3f(PFfloat x, PFfloat y, PFfloat z);
PF_API void pfVertexVec3f(const PFvec3f* v);

PF_API void pfColor3f(PFfloat r, PFfloat g, PFfloat b);
PF_API void pfColor4f(PFfloat r, PFfloat g, PFfloat b , PFfloat a);
PF_API void pfColor4ub(PFubyte r, PFubyte g, PFubyte b, PFubyte a);
PF_API void pfColor(PFcolor color);

PF_API void pfTexCoord2f(PFfloat u, PFfloat v);
PF_API void pfTexCoordVec2f(const PFvec2f* v);

PF_API void pfNormal3f(PFfloat x, PFfloat y, PFfloat z);
PF_API void pfNormalVec3f(const PFvec3f* v);

/* Blending functions */

PF_API PFcolor pfBlend(PFcolor source, PFcolor destination);
PF_API PFcolor pfBlendAlpha(PFcolor source, PFcolor destination);
PF_API PFcolor pfBlendAdditive(PFcolor source, PFcolor destination);
PF_API PFcolor pfBlendSubtractive(PFcolor source, PFcolor destination);
PF_API PFcolor pfBlendMultiplicative(PFcolor source, PFcolor destination);

/* Framebuffer functions */

PFframebuffer pfFramebufferGenBuffer(PFuint width, PFuint height, PFpixelformat format);
PF_API void pfFramebufferDestroy(PFframebuffer* framebuffer);

PF_API void pfFramebufferClear(PFframebuffer* framebuffer, PFcolor color);

PF_API void pfFramebufferSetPixelDepth(PFframebuffer* framebuffer, PFuint x, PFuint y, PFfloat z, PFcolor color);
PF_API float pfFramebufferGetDepth(const PFframebuffer* framebuffer, PFuint x, PFuint y);

PF_API void pfFramebufferSetPixel(PFframebuffer* framebuffer, PFuint x, PFuint y, PFcolor color);
PF_API PFcolor pfFrambufferGetPixel(const PFframebuffer* framebuffer, PFuint x, PFuint y);

/* Texture functions */

PF_API PFtexture pfTextureGenFromBuffer(void* pixels, PFuint width, PFuint height, PFpixelformat format);
PF_API PFtexture pfTextureGenBuffer(PFuint width, PFuint height, PFpixelformat format);
PF_API PFtexture pfTextureGenColorBuffer(PFuint width, PFuint height, PFcolor color, PFpixelformat format);

PF_API void pfTextureDestroy(PFtexture* texture);

PF_API void pfTextureSetPixel(PFtexture* texture, PFuint x, PFuint y, PFcolor color);
PF_API PFcolor pfTextureGetPixel(const PFtexture* texture, PFuint x, PFuint y);

PF_API void pfTextureSetFragment(PFtexture* texture, PFfloat u, PFfloat v, PFcolor color);
PF_API PFcolor pfTextureGetFragment(const PFtexture* texture, PFfloat u, PFfloat v);

/* Math functions */

PF_API PFvec2f pfVec2fNeg(const PFvec2f* v);
PF_API PFvec2f pfVec2fAdd(const PFvec2f* v1, const PFvec2f* v2);
PF_API PFvec2f pfVec2fSub(const PFvec2f* v1, const PFvec2f* v2);
PF_API PFvec2f pfVec2fMul(const PFvec2f* v1, const PFvec2f* v2);
PF_API PFvec2f pfVec2fDiv(const PFvec2f* v1, const PFvec2f* v2);
PF_API PFvec2f pfVec2fScale(const PFvec2f* v, PFfloat scalar);
PF_API PFvec2f pfVec2fNormalize(const PFvec2f* v);
PF_API PFfloat pfVec2fDot(const PFvec2f* v1, const PFvec2f* v2);
PF_API PFvec2f pfVec2Transform(const PFvec2f* v, const PFmat4f* mat);

PF_API PFvec3f pfVec3fNeg(const PFvec3f* v);
PF_API PFvec3f pfVec3fAdd(const PFvec3f* v1, const PFvec3f* v2);
PF_API PFvec3f pfVec3fSub(const PFvec3f* v1, const PFvec3f* v2);
PF_API PFvec3f pfVec3fMul(const PFvec3f* v1, const PFvec3f* v2);
PF_API PFvec3f pfVec3fDiv(const PFvec3f* v1, const PFvec3f* v2);
PF_API PFvec3f pfVec3fScale(const PFvec3f* v, PFfloat scalar);
PF_API PFvec3f pfVec3fNormalize(const PFvec3f* v);
PF_API PFfloat pfVec3fDot(const PFvec3f* v1, const PFvec3f* v2);
PF_API PFvec3f pfVec3fCross(const PFvec3f* v1, const PFvec3f* v2);
PF_API PFvec3f pfVec3fTransform(const PFvec3f* v1, const PFmat4f* mat);

PF_API PFvec4f pfVec4fNeg(const PFvec4f* v);
PF_API PFvec4f pfVec4fAdd(const PFvec4f* v1, const PFvec4f* v2);
PF_API PFvec4f pfVec4fSub(const PFvec4f* v1, const PFvec4f* v2);
PF_API PFvec4f pfVec4fMul(const PFvec4f* v1, const PFvec4f* v2);
PF_API PFvec4f pfVec4fDiv(const PFvec4f* v1, const PFvec4f* v2);
PF_API PFvec4f pfVec4fScale(const PFvec4f* v, PFfloat scalar);
PF_API PFvec4f pfVec4fNormalize(const PFvec4f* v);
PF_API PFfloat pfVec4fDot(const PFvec4f* v1, const PFvec4f* v2);
PF_API PFvec4f pfVec4fTransform(const PFvec4f* v, const PFmat4f* mat);

PF_API PFfloat pfMat4fDeterminant(const PFmat4f* mat);
PF_API PFfloat pfMat4fTrace(const PFmat4f* mat);
PF_API PFmat4f pfMat4fTranspose(const PFmat4f* mat);
PF_API PFmat4f pfMat4fInvert(const PFmat4f* mat);
PF_API PFmat4f pfMat4fIdentity(void);
PF_API PFmat4f pfMat4fAdd(const PFmat4f* left, const PFmat4f* right);
PF_API PFmat4f pfMat4fSub(const PFmat4f* left, const PFmat4f* right);
PF_API PFmat4f pfMat4fMul(const PFmat4f* left, const PFmat4f* right);
PF_API PFmat4f pfMat4fTranslate(PFfloat x, PFfloat y, PFfloat z);
PF_API PFmat4f pfMat4fRotate(const PFvec3f* axis, PFfloat angle);
PF_API PFmat4f pfMat4fRotateX(PFfloat angle);
PF_API PFmat4f pfMat4fRotateY(PFfloat angle);
PF_API PFmat4f pfMat4fRotateZ(PFfloat angle);
PF_API PFmat4f pfMat4fRotateXYZ(const PFvec3f* angle);
PF_API PFmat4f pfMat4fRotateZYX(const PFvec3f* angle);
PF_API PFmat4f pfMat4fScale(PFfloat x, PFfloat y, PFfloat z);
PF_API PFmat4f pfMat4fFrustum(PFdouble left, PFdouble right, PFdouble bottom, PFdouble top, PFdouble near, PFdouble far);
PF_API PFmat4f pfMat4fPerspective(PFdouble fovY, PFdouble aspect, PFdouble nearPlane, PFdouble farPlane);
PF_API PFmat4f pfMat4fOrtho(PFdouble left, PFdouble right, PFdouble bottom, PFdouble top, PFdouble nearPlane, PFdouble farPlane);
PF_API PFmat4f pfMat4fLookAt(const PFvec3f* eye, const PFvec3f* target, const PFvec3f* up);

#if defined(__cplusplus)
}
#endif //__cplusplus

#endif //PIXEL_FORGE_H
