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

#ifndef PF_REALLOC
#   define PF_REALLOC(ptr, newSize) realloc(ptr, newSize)
#endif //PF_REALLOC

#ifndef PF_FREE
#   define PF_FREE(size) free(size)
#endif //PF_FREE

#ifndef INV_255
#   define INV_255 (1.0 / 255)
#endif //INV_255

#ifndef MIN_255
#   define MIN_255(n) ( \
    (PFubyte)((PFint)(n) | ((255 - (PFint)(n)) >> 31)))
#endif //MIN_255

#ifndef MAX_0
#   define MAX_0(n) (\
    (PFubyte)((PFint)(n) & -((PFint)(n) >= 0)))
#endif //MAX_0

#ifndef MIN
#   define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif //MIN

#ifndef MAX
#   define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif //MAX

#ifndef CLAMP
#   define CLAMP(x, min, max) ( \
    (x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
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

typedef enum {
    PF_UNSIGNED_BYTE,
    PF_UNSIGNED_SHORT,
    PF_UNSIGNED_INT,
    PF_BYTE,
    PF_SHORT,
    PF_INT,
    PF_FLOAT,
    PF_DOUBLE
} PFdatatype;

/* Context definitions */

typedef struct PFctx PFctx;     // NOTE: This type is opaque, API functions are used to modify its state

typedef enum {
    PF_TEXTURE              = 0x0001,
    PF_FRAMEBUFFER          = 0x0002,
    PF_DEPTH_TEST           = 0x0004,
    PF_CULL_FACE            = 0x0008,
    PF_NORMALIZE            = 0x0010,
    PF_LIGHTING             = 0x0020,
    PF_COLOR_MATERIAL       = 0x0040,
    PF_VERTEX_ARRAY         = 0x0080,
    PF_NORMAL_ARRAY         = 0x0100,
    PF_COLOR_ARRAY          = 0x0200,
    PF_TEXTURE_COORD_ARRAY  = 0x0400,
    //PF_INDEX_ARRAY        = 0x0800,
} PFstate;

typedef enum {
    PF_BLEND                            = 10000,      // TODO REVIEW: Move to PFstate if you decide to review the color mixing mechanism
    PF_VIEWPORT,
    PF_COLOR_CLEAR_VALUE,
    //PF_INDEX_CLEAR_VALUE,
    PF_DEPTH_CLEAR_VALUE,
    PF_CULL_FACE_MODE,
    PF_CURRENT_COLOR,
    //PF_CURRENT_INDEX,
    PF_CURRENT_NORMAL,
    PF_CURRENT_TEXTURE_COORDS,
    //PF_CURRENT_RASTER_COLOR,
    //PF_CURRENT_RASTER_DISTANCE,
    //PF_CURRENT_RASTER_INDEX,
    PF_CURRENT_RASTER_POSITION,
    //PF_CURRENT_RASTER_POSITION_VALID,
    //PF_CURRENT_RASTER_TEXTURE_COORDS,
    PF_BLEND_FUNC,
    PF_DEPTH_FUNC,
    PF_POLYGON_MODE,
    //PF_POLYGON_OFFSET_FACTOR,
    //PF_POLYGON_OFFSET_UNITS,
    //PF_POLYGON_OFFSET_FILL,
    //PF_POLYGON_OFFSET_LINE,
    //PF_POLYGON_OFFSET_POINT,
    //PF_POLYGON_SMOOTH,
    //PF_POLYGON_SMOOTH_HINT,
    //PF_POLYGON_STIPPLE,
    PF_POINT_SIZE,
    //PF_POINT_SIZE_GRANULARITY,
    //PF_POINT_SIZE_RANGE,
    //PF_POINT_SMOOTH,
    //PF_POINT_SMOOTH_HINT,
    //PF_LIGHT_MODEL_AMBIENT,
    //PF_LIGHT_MODEL_LOCAL_VIEWER,
    //PF_LIGHT_MODEL_TWO_SIDE,
    //PF_LINE_SMOOTH,
    //PF_LINE_SMOOTH_HINT,
    //PF_LINE_STIPPLE,
    //PF_LINE_STIPPLE_PATTERN,
    //PF_LINE_STIPPLE_REPEAT,
    PF_LINE_WIDTH,
    //PF_LINE_WIDTH_GRANULARITY,
    //PF_LINE_WIDTH_RANGE,
    PF_MATRIX_MODE,
    PF_PROJECTION_MATRIX,
    PF_MODELVIEW_MATRIX,
    PF_MAX_PROJECTION_STACK_DEPTH,
    PF_MAX_MODELVIEW_STACK_DEPTH,
    //PF_MAX_TEXTURE_STACK_DEPTH,
    PF_SHADE_MODEL,
    PF_MAX_LIGHTS,
    PF_VERTEX_ARRAY_SIZE,
    PF_VERTEX_ARRAY_STRIDE,
    PF_VERTEX_ARRAY_TYPE,
    PF_NORMAL_ARRAY_STRIDE,
    PF_NORMAL_ARRAY_TYPE,
    //PF_TEXTURE_COORD_ARRAY_SIZE,
    PF_TEXTURE_COORD_ARRAY_STRIDE,
    PF_TEXTURE_COORD_ARRAY_TYPE,
    PF_COLOR_ARRAY_SIZE,
    PF_COLOR_ARRAY_STRIDE,
    PF_COLOR_ARRAY_TYPE,
    //PF_INDEX_ARRAY_STRIDE,
    //PF_INDEX_ARRAY_TYPE,
    //PF_INDEX_MODE,
    //PF_INDEX_OFFSET,
    //PF_INDEX_SHIFT,
    PF_ZOOM_X,
    PF_ZOOM_Y
} PFgettable;

/* Error enum */

typedef enum {

    PF_NO_ERROR,
    PF_INVALID_ENUM,
    PF_INVALID_VALUE,
    PF_STACK_OVERFLOW,
    PF_STACK_UNDERFLOW,
    PF_INVALID_OPERATION,
    PF_ERROR_OUT_OF_MEMORY,

#ifndef NDEBUG
    PF_DEBUG_NO_ERROR,
    PF_DEBUG_INVALID_ENUM,
    PF_DEBUG_INVALID_VALUE,
    PF_DEBUG_STACK_OVERFLOW,
    PF_DEBUG_STACK_UNDERFLOW,
    PF_DEBUG_INVALID_OPERATION,
    PF_DEBUG_ERROR_OUT_OF_MEMORY,
#endif //NDEBUG

} PFerrcode;

/* Render definitions */

typedef enum {
    PF_COLOR_BUFFER_BIT = 0x01,
    PF_DEPTH_BUFFER_BIT = 0x02
} PFclearflag;

typedef enum {
    PF_MODELVIEW,
    PF_PROJECTION,
} PFmatrixmode;

typedef enum {
    PF_POINTS,
    PF_LINES,
    PF_TRIANGLES,
    PF_TRIANGLE_FAN,
    PF_TRIANGLE_STRIP,
    PF_QUADS,
    PF_QUAD_FAN,
    PF_QUAD_STRIP
} PFdrawmode;

typedef enum {
    PF_POINT,
    PF_LINE,
    PF_FILL
} PFpolygonmode;

typedef enum {
    PF_FLAT,
    PF_SMOOTH
} PFshademode;

typedef enum {                  // NOTE 1: PF_FRONT and PF_BACK are used as indices internally for `ctx->polygonMode`.
    PF_FRONT            = 0,    // NOTE 2: Also, we can invert PF_FRONT in code using '!' to obtain PF_BACK.
    PF_BACK             = 1,    // NOTE 3: Similarly, this can be done for PF_BACK to obtain PF_FRONT.
    PF_FRONT_AND_BACK
} PFface;

typedef enum {
    PF_LIGHT0 = 0,
    PF_LIGHT1,
    PF_LIGHT2,
    PF_LIGHT3,
    PF_LIGHT4,
    PF_LIGHT5,
    PF_LIGHT6,
    PF_LIGHT7,
    PF_LIGHT8
} PFlights;

typedef enum {  // NOTE: Common to light and material
    PF_AMBIENT_AND_DIFFUSE      = 1,
    PF_AMBIENT                  = 2,
    PF_DIFFUSE                  = 3,
    PF_SPECULAR                 = 4
} PFrendercolor;

typedef enum {
    PF_EMISSION                 = 5,
    PF_SHININESS                = 6
} PFmaterialparam;

typedef enum {
    PF_POSITION                 = 7,
    PF_SPOT_DIRECTION           = 8,
    //PF_SPOT_EXPONENT          = 9,
    PF_SPOT_INNER_CUTOFF        = 10,
    PF_SPOT_OUTER_CUTOFF        = 11,
    PF_CONSTANT_ATTENUATION     = 12,
    PF_LINEAR_ATTENUATION       = 13,
    PF_QUADRATIC_ATTENUATION    = 14
} PFlightparam;

typedef struct {
    PFubyte r, g, b, a;
} PFcolor;

typedef PFcolor (*PFblendfunc)(PFcolor, PFcolor);
typedef PFboolean (*PFdepthfunc)(PFfloat, PFfloat);
typedef PFcolor (*PFpostprocessfunc)(PFint, PFint, PFfloat, PFcolor);

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
    PFsizei width;
    PFsizei height;
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

// TODO: Add a note about the error codes that can be defined by the context-related API functions if they fail.

/* Context API functions */

/**
 * @brief Creates a rendering context.
 *
 * @warning This function needs a context to be defined.
 *
 * @param targetBuffer Pointer to the target buffer.
 * @param width        Width of the target buffer.
 * @param height       Height of the target buffer.
 * @param pixelFormat  Pixel format of the target buffer.
 *
 * @return Pointer to the created rendering context.
 */
PF_API PFctx* pfCreateContext(void* targetBuffer, PFsizei width, PFsizei height, PFpixelformat pixelFormat);

/**
 * @brief Deletes a rendering context.
 *
 * @warning This function needs a context to be defined.
 *
 * @param ctx Pointer to the rendering context to be deleted.
 */
PF_API void pfDeleteContext(PFctx* ctx);

/**
 * @brief Sets the main buffer for rendering.
 *
 * @param targetBuffer Pointer to the target buffer.
 * @param width        Width of the target buffer.
 * @param height       Height of the target buffer.
 * @param pixelFormat  Pixel format of the target buffer.
 * 
 * @note If you have defined custom pixel getters/setters, you will need to reassign them.
 *       If reallocation of the main depth buffer fails, the old pixel buffer will be retained.
 *       Does not free the previous pixel buffer provided, its lifespan management is your responsibility.
 *       If you have defined an auxiliary buffer it will be automatically deactivated when changing the main buffer.
 */
PF_API void pfSetMainBuffer(void* targetBuffer, PFsizei width, PFsizei height, PFpixelformat pixelFormat);

/**
 * @brief Sets the auxiliary buffer for rendering.
 *
 * This function is used to define an auxiliary buffer for rendering, typically for double buffering.
 *
 * @warning This function requires a defined context.
 *
 * @param auxFramebuffer Pointer to the auxiliary buffer.
 */
PF_API void pfSetAuxBuffer(void *auxFramebuffer);

/**
 * @brief Swaps the front and back buffers.
 *
 * @warning This function needs a context to be defined.
 */
PF_API void pfSwapBuffers(void);

/**
 * @brief Retrieves the current rendering context.
 *
 * @warning This function needs a context to be defined.
 *
 * @return Pointer to the current rendering context.
 */
PF_API PFctx* pfGetCurrentContext(void);

/**
 * @brief Sets the current rendering context.
 *
 * @warning This function needs a context to be defined.
 *
 * @param ctx Pointer to the rendering context to be made current.
 */
PF_API void pfMakeCurrent(PFctx* ctx);

/**
 * @brief Checks if a rendering state is enabled.
 *
 * @warning This function needs a context to be defined.
 *
 * @param state The rendering state to check.
 *
 * @return PF_TRUE if the state is enabled, otherwise PF_FALSE.
 */
PF_API PFboolean pfIsEnabled(PFstate state);

/**
 * @brief Enables a rendering state.
 *
 * @warning This function needs a context to be defined.
 *
 * @param state The rendering state to enable.
 */
PF_API void pfEnable(PFstate state);

/**
 * @brief Disables a rendering state.
 *
 * @warning This function needs a context to be defined.
 *
 * @param state The rendering state to disable.
 */
PF_API void pfDisable(PFstate state);



/* Getter API functions */

/**
 * @brief Retrieves a boolean parameter value.
 *
 * @warning This function needs a context to be defined.
 *
 * @param pname  The name of the parameter to retrieve.
 * @param params Pointer to an array where the parameter value will be stored.
 */
PF_API void pfGetBooleanv(PFenum pname, PFboolean* params);

/**
 * @brief Retrieves an integer parameter value.
 *
 * @warning This function needs a context to be defined.
 *
 * @param pname  The name of the parameter to retrieve.
 * @param params Pointer to an array where the parameter value will be stored.
 */
PF_API void pfGetIntegerv(PFenum pname, PFint* params);

/**
 * @brief Retrieves a floating-point parameter value.
 *
 * @warning This function needs a context to be defined.
 *
 * @param pname  The name of the parameter to retrieve.
 * @param params Pointer to an array where the parameter value will be stored.
 */
PF_API void pfGetFloatv(PFenum pname, PFfloat* params);

/**
 * @brief Retrieves a double-precision floating-point parameter value.
 *
 * @warning This function needs a context to be defined.
 *
 * @param pname  The name of the parameter to retrieve.
 * @param params Pointer to an array where the parameter value will be stored.
 */
PF_API void pfGetDoublev(PFenum pname, PFdouble* params);

/**
 * @brief Retrieves a pointer parameter value.
 *
 * @warning This function needs a context to be defined.
 *
 * @param pname  The name of the parameter to retrieve.
 * @param params Pointer to a pointer where the parameter value will be stored.
 */
PF_API void pfGetPointerv(PFenum pname, const void** params);

/**
 * @brief Retrieves the error code for the last rendering operation.
 *
 * @warning This function needs a context to be defined.
 *
 * @return Error code for the last rendering operation.
 */
PF_API PFerrcode pfGetError(void);



/* Matrix management API functions */

/**
 * @brief Sets the current matrix mode.
 *
 * @warning This function needs a context to be defined.
 *
 * @param mode The matrix mode to set.
 */
PF_API void pfMatrixMode(PFmatrixmode mode);

/**
 * @brief Pushes the current matrix onto the matrix stack.
 *
 * @warning This function needs a context to be defined.
 */
PF_API void pfPushMatrix(void);

/**
 * @brief Pops the top matrix from the matrix stack.
 *
 * @warning This function needs a context to be defined.
 */
PF_API void pfPopMatrix(void);

/**
 * @brief Loads the identity matrix onto the current matrix.
 *
 * @warning This function needs a context to be defined.
 */
PF_API void pfLoadIdentity(void);

/**
 * @brief Translates the current matrix by the specified translation vector.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x Translation along the x-axis.
 * @param y Translation along the y-axis.
 * @param z Translation along the z-axis.
 */
PF_API void pfTranslatef(PFfloat x, PFfloat y, PFfloat z);

/**
 * @brief Rotates the current matrix by the specified angle around the specified axis.
 *
 * @warning This function needs a context to be defined.
 *
 * @param angle Angle of rotation in degrees.
 * @param x     X-coordinate of the rotation axis.
 * @param y     Y-coordinate of the rotation axis.
 * @param z     Z-coordinate of the rotation axis.
 */
PF_API void pfRotatef(PFfloat angle, PFfloat x, PFfloat y, PFfloat z);

/**
 * @brief Scales the current matrix by the specified scaling factors.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x Scaling factor along the x-axis.
 * @param y Scaling factor along the y-axis.
 * @param z Scaling factor along the z-axis.
 */
PF_API void pfScalef(PFfloat x, PFfloat y, PFfloat z);

/**
 * @brief Multiplies the current matrix by the specified matrix.
 *
 * @warning This function needs a context to be defined.
 *
 * @param mat Pointer to the 16-element floating-point array representing the matrix to multiply by.
 */
PF_API void pfMultMatrixf(const PFfloat* mat);

/**
 * @brief Sets up a perspective projection matrix.
 *
 * @warning This function needs a context to be defined.
 *
 * @param left   Coordinate for the left vertical clipping plane.
 * @param right  Coordinate for the right vertical clipping plane.
 * @param bottom Coordinate for the bottom horizontal clipping plane.
 * @param top    Coordinate for the top horizontal clipping plane.
 * @param znear  Distance to the near depth clipping plane.
 * @param zfar   Distance to the far depth clipping plane.
 */
PF_API void pfFrustum(PFdouble left, PFdouble right, PFdouble bottom, PFdouble top, PFdouble znear, PFdouble zfar);

/**
 * @brief Sets up an orthographic projection matrix.
 *
 * @warning This function needs a context to be defined.
 *
 * @param left   Coordinate for the left vertical clipping plane.
 * @param right  Coordinate for the right vertical clipping plane.
 * @param bottom Coordinate for the bottom horizontal clipping plane.
 * @param top    Coordinate for the top horizontal clipping plane.
 * @param znear  Distance to the near depth clipping plane.
 * @param zfar   Distance to the far depth clipping plane.
 */
PF_API void pfOrtho(PFdouble left, PFdouble right, PFdouble bottom, PFdouble top, PFdouble znear, PFdouble zfar);



/* Render configuration API functions */

/**
 * @brief Sets the viewport, defining the rectangular area of the window into which rendering will occur.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x      X-coordinate of the bottom-left corner of the viewport.
 * @param y      Y-coordinate of the bottom-left corner of the viewport.
 * @param width  Width of the viewport.
 * @param height Height of the viewport.
 */
PF_API void pfViewport(PFint x, PFint y, PFsizei width, PFsizei height);

/**
 * @brief Sets the default pixel getter function, used for reading pixel data from the main framebuffer.
 *
 * @warning This function needs a context to be defined.
 *
 * @param func Pointer to the pixel getter function.
 */
PF_API void pfSetDefaultPixelGetter(PFpixelgetter func);

/**
 * @brief Sets the default pixel setter function, used for writing pixel data to the main framebuffer.
 *
 * @warning This function needs a context to be defined.
 *
 * @param func Pointer to the pixel setter function.
 */
PF_API void pfSetDefaultPixelSetter(PFpixelsetter func);

/**
 * @brief Specifies the drawing mode for polygons.
 *
 * This function sets the drawing mode for polygons, similar to OpenGL 1.x.
 * It defines how the polygons are rendered: PF_FILL to fill the entire geometry,
 * PF_LINE to render only the lines between vertices, or PF_POINT to render points at the vertices of the geometry.
 *
 * @warning This function requires a defined context.
 *
 * @param face The face to set the drawing mode for (front or back).
 * @param mode The drawing mode for the specified face.
 */
PF_API void pfPolygonMode(PFface face, PFpolygonmode mode);

/**
 * @brief Specifies the shading mode for rendering.
 *
 * This function sets the shading mode for rendering, similar to OpenGL 1.x.
 * It accepts two modes: PF_SMOOTH for smooth shading, performing color interpolation between vertices,
 * or PF_FLAT for flat shading, where each primitive is rendered using the color of the nearest vertex in geometry.
 *
 * @warning This function requires a defined context.
 *
 * @param mode The shading mode to set.
 */
PF_API void pfShadeModel(PFshademode mode);

/**
 * @brief Sets the line width for rasterized lines.
 *
 * @warning This function needs a context to be defined.
 *
 * @param width The width of lines, in pixels.
 */
PF_API void pfLineWidth(PFfloat width);

/**
 * @brief Sets the size of rasterized points.
 *
 * @warning This function needs a context to be defined.
 *
 * @param size The size of points, in pixels.
 */
PF_API void pfPointSize(PFfloat size);

/**
 * @brief Specifies which faces to cull (front or back) during polygon rasterization.
 *
 * @warning This function needs a context to be defined.
 *
 * @param face The face to cull.
 */
PF_API void pfCullFace(PFface face);

/**
 * @brief Specifies the blending function used for color blending.
 *
 * @warning This function needs a context to be defined.
 *
 * @param func The blending function to use.
 */
PF_API void pfBlendFunc(PFblendfunc func);

/**
 * @brief Specifies the depth testing function.
 *
 * @warning This function needs a context to be defined.
 *
 * @param func The depth testing function to use.
 */
PF_API void pfDepthFunc(PFdepthfunc func);

/**
 * @brief Binds the specified framebuffer for subsequent rendering operations.
 *
 * Once the framebuffer is bound to the current context, the PF_FRAMEBUFFER state
 * must be active for the bound framebuffer to be considered during rendering.
 *
 * @warning This function requires a context to be defined.
 *
 * @param framebuffer Pointer to the framebuffer to bind.
 */
PF_API void pfBindFramebuffer(PFframebuffer* framebuffer);

/**
 * @brief Binds the specified texture for subsequent rendering operations.
 *
 * Once the texture is bound to the current context, the PF_TEXTURE state
 * must be active for the bound texture to be considered during rendering.
 *
 * @warning This function needs a context to be defined.
 *
 * @param texture Pointer to the texture to bind.
 */
PF_API void pfBindTexture(PFtexture* texture);

/**
 * @brief Clears the specified buffers of the current framebuffer.
 *
 * @warning This function needs a context to be defined.
 *
 * @param flag Bitwise OR of values specifying which buffers to clear (e.g., PF_COLOR_BUFFER_BIT, PF_DEPTH_BUFFER_BIT).
 */
PF_API void pfClear(PFclearflag flag);

/**
 * @brief Sets the clear depth value for the depth buffer of the current framebuffer.
 *
 * @warning This function needs a context to be defined.
 *
 * @param depth The depth value to clear to.
 */
PF_API void pfClearDepth(PFfloat depth);

/**
 * @brief Sets the clear color for the color buffer of the current framebuffer.
 *
 * @warning This function needs a context to be defined.
 *
 * @param r Red component of the clear color.
 * @param g Green component of the clear color.
 * @param b Blue component of the clear color.
 * @param a Alpha component of the clear color.
 */
PF_API void pfClearColor(PFubyte r, PFubyte g, PFubyte b, PFubyte a);



/* Light management API functions */

/**
 * @brief Enables the specified light source.
 *
 * @warning This function needs a context to be defined.
 *
 * @param light Index of the light source to enable.
 */
PF_API void pfEnableLight(PFsizei light);

/**
 * @brief Disables the specified light source.
 *
 * @warning This function needs a context to be defined.
 *
 * @param light Index of the light source to disable.
 */
PF_API void pfDisableLight(PFsizei light);

/**
 * @brief Checks if the specified light source is enabled.
 *
 * @warning This function needs a context to be defined.
 *
 * @param light Index of the light source to check.
 * @return PFboolean True if the light source is enabled, false otherwise.
 */
PF_API PFboolean pfIsEnabledLight(PFsizei light);

/**
 * @brief Sets a float parameter for the specified light source.
 *
 * @warning This function needs a context to be defined.
 *
 * @param light Index of the light source.
 * @param param Parameter to set (e.g., PF_LIGHT_POSITION, PF_LIGHT_DIFFUSE).
 * @param value Value to set for the parameter.
 */
PF_API void pfLightf(PFsizei light, PFenum param, PFfloat value);

/**
 * @brief Sets a float vector parameter for the specified light source.
 *
 * @warning This function needs a context to be defined.
 *
 * @param light Index of the light source.
 * @param param Parameter to set (e.g., PF_LIGHT_POSITION, PF_LIGHT_DIFFUSE).
 * @param value Pointer to the value array to set for the parameter.
 */
PF_API void pfLightfv(PFsizei light, PFenum param, const void* value);

/**
 * @brief Sets a float parameter for the specified face material.
 *
 * @warning This function needs a context to be defined.
 *
 * @param face Face whose material to set (PF_FRONT_FACE or PF_BACK_FACE).
 * @param param Parameter to set (e.g., PF_MATERIAL_AMBIENT, PF_MATERIAL_SPECULAR).
 * @param value Value to set for the parameter.
 */
PF_API void pfMaterialf(PFface face, PFenum param, PFfloat value);

/**
 * @brief Sets a float vector parameter for the specified face material.
 *
 * @warning This function needs a context to be defined.
 *
 * @param face Face whose material to set (PF_FRONT_FACE or PF_BACK_FACE).
 * @param param Parameter to set (e.g., PF_MATERIAL_AMBIENT, PF_MATERIAL_SPECULAR).
 * @param value Pointer to the value array to set for the parameter.
 */
PF_API void pfMaterialfv(PFface face, PFenum param, const void* value);

/**
 * @brief Sets the material color to follow the current color.
 *
 * @warning This function needs a context to be defined.
 *
 * @param face Face whose material color must follow the current color (PF_FRONT_FACE or PF_BACK_FACE).
 * @param mode Material color mode to set (e.g., PF_EMISSION, PF_AMBIENT).
 */
PF_API void pfColorMaterial(PFface face, PFenum mode);



/* Vertex array drawing API functions */

/**
 * @brief Specifies the location and data format of the vertex array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param size Number of coordinates per vertex (2, 3, or 4).
 * @param type Data type of each coordinate (PF_SHORT, PF_INT, PF_FLOAT, or PF_DOUBLE).
 * @param stride Byte offset between consecutive vertices.
 * @param pointer Pointer to the first coordinate of the first vertex.
 */
PF_API void pfVertexPointer(PFint size, PFenum type, PFsizei stride, const void* pointer);

/**
 * @brief Specifies the location and data format of the normal array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param type Data type of each normal (PF_FLOAT or PF_DOUBLE).
 * @param stride Byte offset between consecutive normals.
 * @param pointer Pointer to the first normal.
 */
PF_API void pfNormalPointer(PFenum type, PFsizei stride, const void* pointer);

/**
 * @brief Specifies the location and data format of the texture coordinate array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param type Data type of each texture coordinate (PF_FLOAT or PF_DOUBLE).
 * @param stride Byte offset between consecutive texture coordinates.
 * @param pointer Pointer to the first texture coordinate.
 */
PF_API void pfTexCoordPointer(PFenum type, PFsizei stride, const void* pointer);

/**
 * @brief Specifies the location and data format of the color array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param size Number of color components per vertex (3 or 4).
 * @param type Data type of each color component (PF_UNSIGNED_BYTE, PF_UNSIGNED_SHORT, PF_UNSIGNED_INT, PF_FLOAT, or PF_DOUBLE).
 * @param stride Byte offset between consecutive colors.
 * @param pointer Pointer to the first color component.
 */
PF_API void pfColorPointer(PFint size, PFenum type, PFsizei stride, const void* pointer);

/**
 * @brief Renders primitives from array data using indices.
 *
 * @warning This function needs a context to be defined.
 *
 * @param mode Type of primitives to render (e.g., PF_POINTS, PF_TRIANGLES).
 * @param count Number of elements to be rendered.
 * @param type Data type of the element indices (e.g., PF_UNSIGNED_BYTE, PF_UNSIGNED_SHORT).
 * @param indices Pointer to the first index in the array of element indices.
 */
PF_API void pfDrawElements(PFdrawmode mode, PFsizei count, PFdatatype type, const void* indices);

/**
 * @brief Renders primitives from array data.
 *
 * @warning This function needs a context to be defined.
 *
 * @param mode Type of primitives to render (e.g., PF_POINTS, PF_TRIANGLES).
 * @param first Index of the first vertex to render.
 * @param count Number of vertices to render.
 */
PF_API void pfDrawArrays(PFdrawmode mode, PFint first, PFsizei count);



/* Primitives drawing API functions */

/**
 * @brief Begins the definition of a primitive.
 *
 * @warning This function needs a context to be defined.
 *
 * @param mode Type of primitives to be defined (e.g., PF_POINTS, PF_TRIANGLES).
 */
PF_API void pfBegin(PFdrawmode mode);

/**
 * @brief Ends the definition of a primitive.
 *
 * @warning This function needs a context to be defined.
 */
PF_API void pfEnd(void);

/**
 * @brief Specifies a 2D vertex with integer coordinates.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x X-coordinate of the vertex.
 * @param y Y-coordinate of the vertex.
 */
PF_API void pfVertex2i(PFint x, PFint y);

/**
 * @brief Specifies a 2D vertex with floating-point coordinates.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x X-coordinate of the vertex.
 * @param y Y-coordinate of the vertex.
 */
PF_API void pfVertex2f(PFfloat x, PFfloat y);

/**
 * @brief Specifies a 2D vertex with coordinates provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the (x, y) coordinates of the vertex.
 */
PF_API void pfVertex2fv(const PFfloat* v);

/**
 * @brief Specifies a 3D vertex with integer coordinates.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x X-coordinate of the vertex.
 * @param y Y-coordinate of the vertex.
 * @param z Z-coordinate of the vertex.
 */
PF_API void pfVertex3i(PFint x, PFint y, PFint z);

/**
 * @brief Specifies a 3D vertex with floating-point coordinates.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x X-coordinate of the vertex.
 * @param y Y-coordinate of the vertex.
 * @param z Z-coordinate of the vertex.
 */
PF_API void pfVertex3f(PFfloat x, PFfloat y, PFfloat z);

/**
 * @brief Specifies a 3D vertex with coordinates provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the (x, y, z) coordinates of the vertex.
 */
PF_API void pfVertex3fv(const PFfloat* v);

/**
 * @brief Specifies a 4D vertex with integer coordinates.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x X-coordinate of the vertex.
 * @param y Y-coordinate of the vertex.
 * @param z Z-coordinate of the vertex.
 * @param w W-coordinate of the vertex.
 */
PF_API void pfVertex4i(PFint x, PFint y, PFint z, PFint w);

/**
 * @brief Specifies a 4D vertex with floating-point coordinates.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x X-coordinate of the vertex.
 * @param y Y-coordinate of the vertex.
 * @param z Z-coordinate of the vertex.
 * @param w W-coordinate of the vertex.
 */
PF_API void pfVertex4f(PFfloat x, PFfloat y, PFfloat z, PFfloat w);

/**
 * @brief Specifies a 4D vertex with coordinates provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the (x, y, z, w) coordinates of the vertex.
 */
PF_API void pfVertex4fv(const PFfloat* v);

/**
 * @brief Specifies a color using unsigned byte components.
 *
 * @warning This function needs a context to be defined.
 *
 * @param r Red component value (0-255).
 * @param g Green component value (0-255).
 * @param b Blue component value (0-255).
 */
PF_API void pfColor3ub(PFubyte r, PFubyte g, PFubyte b);

/**
 * @brief Specifies a color using unsigned byte components provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the red, green, and blue components (0-255).
 */
PF_API void pfColor3ubv(const PFubyte* v);

/**
 * @brief Specifies a color using unsigned short components.
 *
 * @warning This function needs a context to be defined.
 *
 * @param r Red component value (0-65535).
 * @param g Green component value (0-65535).
 * @param b Blue component value (0-65535).
 */
PF_API void pfColor3us(PFushort r, PFushort g, PFushort b);

/**
 * @brief Specifies a color using unsigned short components provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the red, green, and blue components (0-65535).
 */
PF_API void pfColor3usv(const PFushort* v);

/**
 * @brief Specifies a color using unsigned int components.
 *
 * @warning This function needs a context to be defined.
 *
 * @param r Red component value (0-4294967295).
 * @param g Green component value (0-4294967295).
 * @param b Blue component value (0-4294967295).
 */
PF_API void pfColor3ui(PFuint r, PFuint g, PFuint b);

/**
 * @brief Specifies a color using unsigned int components provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the red, green, and blue components (0-4294967295).
 */
PF_API void pfColor3uiv(const PFuint* v);

/**
 * @brief Specifies a color using floating-point components.
 *
 * @warning This function needs a context to be defined.
 *
 * @param r Red component value (0.0-1.0).
 * @param g Green component value (0.0-1.0).
 * @param b Blue component value (0.0-1.0).
 */
PF_API void pfColor3f(PFfloat r, PFfloat g, PFfloat b);

/**
 * @brief Specifies a color using floating-point components provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the red, green, and blue components (0.0-1.0).
 */
PF_API void pfColor3fv(const PFfloat* v);

/**
 * @brief Specifies a color with alpha using unsigned byte components.
 *
 * @warning This function needs a context to be defined.
 *
 * @param r Red component value (0-255).
 * @param g Green component value (0-255).
 * @param b Blue component value (0-255).
 * @param a Alpha component value (0-255).
 */
PF_API void pfColor4ub(PFubyte r, PFubyte g, PFubyte b, PFubyte a);

/**
 * @brief Specifies a color with alpha using unsigned byte components provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the red, green, blue, and alpha components (0-255).
 */
PF_API void pfColor4ubv(const PFubyte* v);

/**
 * @brief Specifies a color with alpha using unsigned short components.
 *
 * @warning This function needs a context to be defined.
 *
 * @param r Red component value (0-65535).
 * @param g Green component value (0-65535).
 * @param b Blue component value (0-65535).
 * @param a Alpha component value (0-65535).
 */
PF_API void pfColor4us(PFushort r, PFushort g, PFushort b, PFushort a);

/**
 * @brief Specifies a color with alpha using unsigned short components provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the red, green, blue, and alpha components (0-65535).
 */
PF_API void pfColor4usv(const PFushort* v);

/**
 * @brief Specifies a color with alpha using unsigned int components.
 *
 * @warning This function needs a context to be defined.
 *
 * @param r Red component value (0-4294967295).
 * @param g Green component value (0-4294967295).
 * @param b Blue component value (0-4294967295).
 * @param a Alpha component value (0-4294967295).
 */
PF_API void pfColor4ui(PFuint r, PFuint g, PFuint b, PFuint a);

/**
 * @brief Specifies a color with alpha using unsigned int components provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the red, green, blue, and alpha components (0-4294967295).
 */
PF_API void pfColor4uiv(const PFuint* v);

/**
 * @brief Specifies a color with alpha using floating-point components.
 *
 * @warning This function needs a context to be defined.
 *
 * @param r Red component value (0.0-1.0).
 * @param g Green component value (0.0-1.0).
 * @param b Blue component value (0.0-1.0).
 * @param a Alpha component value (0.0-1.0).
 */
PF_API void pfColor4f(PFfloat r, PFfloat g, PFfloat b, PFfloat a);

/**
 * @brief Specifies a color with alpha using floating-point components provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the red, green, blue, and alpha components (0.0-1.0).
 */
PF_API void pfColor4fv(const PFfloat* v);

/**
 * @brief Specifies 2D texture coordinates using floating-point components.
 *
 * @warning This function needs a context to be defined.
 *
 * @param u U texture coordinate value (0.0-1.0).
 * @param v V texture coordinate value (0.0-1.0).
 */
PF_API void pfTexCoord2f(PFfloat u, PFfloat v);

/**
 * @brief Specifies 2D texture coordinates using floating-point components provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the U and V texture coordinates (0.0-1.0).
 */
PF_API void pfTexCoordfv(const PFfloat* v);

/**
 * @brief Specifies a normal vector using floating-point components.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x X component of the normal vector.
 * @param y Y component of the normal vector.
 * @param z Z component of the normal vector.
 */
PF_API void pfNormal3f(PFfloat x, PFfloat y, PFfloat z);

/**
 * @brief Specifies a normal vector using floating-point components provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the X, Y, and Z components of the normal vector.
 */
PF_API void pfNormal3fv(const PFfloat* v);



/* Supplementary primitive drawing API functions */

/**
 * @brief Draws a rectangle with integer coordinates.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x1 X coordinate of the first corner.
 * @param y1 Y coordinate of the first corner.
 * @param x2 X coordinate of the second corner.
 * @param y2 Y coordinate of the second corner.
 */
PF_API void pfRects(PFshort x1, PFshort y1, PFshort x2, PFshort y2);

/**
 * @brief Draws a rectangle with integer coordinates provided as arrays.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v1 Array containing the X and Y coordinates of the first corner.
 * @param v2 Array containing the X and Y coordinates of the second corner.
 */
PF_API void pfRectsv(const PFshort* v1, const PFshort* v2);

/**
 * @brief Draws a rectangle with floating-point coordinates.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x1 X coordinate of the first corner.
 * @param y1 Y coordinate of the first corner.
 * @param x2 X coordinate of the second corner.
 * @param y2 Y coordinate of the second corner.
 */
PF_API void pfRectf(PFfloat x1, PFfloat y1, PFfloat x2, PFfloat y2);

/**
 * @brief Draws a rectangle with floating-point coordinates provided as arrays.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v1 Array containing the X and Y coordinates of the first corner.
 * @param v2 Array containing the X and Y coordinates of the second corner.
 */
PF_API void pfRectfv(const PFfloat* v1, const PFfloat* v2);



/* Drawing pixels API functions */

/**
 * @brief Draws pixels on the screen.
 *
 * @warning This function needs a context to be defined.
 *
 * @param width Width of the pixel data.
 * @param height Height of the pixel data.
 * @param format Pixel format of the data.
 * @param pixels Pointer to the pixel data.
 */
PF_API void pfDrawPixels(PFsizei width, PFsizei height, PFpixelformat format, const void* pixels);

/**
 * @brief Sets the zoom factor for pixel drawing.
 *
 * @warning This function needs a context to be defined.
 *
 * @param xfactor Zoom factor along the X-axis.
 * @param yfactor Zoom factor along the Y-axis.
 */
PF_API void pfPixelZoom(PFfloat xfactor, PFfloat yfactor);

/**
 * @brief Sets the current raster position using integer coordinates.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x X coordinate of the raster position.
 * @param y Y coordinate of the raster position.
 */
PF_API void pfRasterPos2i(PFint x, PFint y);

/**
 * @brief Sets the current raster position using floating-point coordinates.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x X coordinate of the raster position.
 * @param y Y coordinate of the raster position.
 */
PF_API void pfRasterPos2f(PFfloat x, PFfloat y);

/**
 * @brief Sets the current raster position using floating-point coordinates provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the X and Y coordinates of the raster position.
 */
PF_API void pfRasterPos2fv(const PFfloat* v);

/**
 * @brief Sets the current raster position using integer coordinates.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x X coordinate of the raster position.
 * @param y Y coordinate of the raster position.
 * @param z Z coordinate of the raster position.
 */
PF_API void pfRasterPos3i(PFint x, PFint y, PFint z);

/**
 * @brief Sets the current raster position using floating-point coordinates.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x X coordinate of the raster position.
 * @param y Y coordinate of the raster position.
 * @param z Z coordinate of the raster position.
 */
PF_API void pfRasterPos3f(PFfloat x, PFfloat y, PFfloat z);

/**
 * @brief Sets the current raster position using floating-point coordinates provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the X, Y, and Z coordinates of the raster position.
 */
PF_API void pfRasterPos3fv(const PFfloat* v);

/**
 * @brief Sets the current raster position using integer coordinates.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x X coordinate of the raster position.
 * @param y Y coordinate of the raster position.
 * @param z Z coordinate of the raster position.
 * @param w W coordinate of the raster position.
 */
PF_API void pfRasterPos4i(PFint x, PFint y, PFint z, PFint w);

/**
 * @brief Sets the current raster position using floating-point coordinates.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x X coordinate of the raster position.
 * @param y Y coordinate of the raster position.
 * @param z Z coordinate of the raster position.
 * @param w W coordinate of the raster position.
 */
PF_API void pfRasterPos4f(PFfloat x, PFfloat y, PFfloat z, PFfloat w);

/**
 * @brief Sets the current raster position using floating-point coordinates provided as an array.
 *
 * @warning This function needs a context to be defined.
 *
 * @param v Array containing the X, Y, Z, and W coordinates of the raster position.
 */
PF_API void pfRasterPos4fv(const PFfloat* v);



/* Misc API functions */

/**
 * @brief Copies pixel data from the current framebuffer to a buffer.
 *
 * This function makes a copy of the current framebuffer to a buffer provided by the user.
 * The format of the pixel data in the framebuffer and the format of the destination buffer must be specified.
 *
 * @warning This function needs a context to be defined.
 *
 * @param x The starting X coordinate of the region to read.
 * @param y The starting Y coordinate of the region to read.
 * @param width The width of the region to read.
 * @param height The height of the region to read.
 * @param format The pixel format of the framebuffer data.
 * @param pixels Pointer to the destination buffer where the pixel data will be copied.
 */
PF_API void pfReadPixels(PFint x, PFint y, PFsizei width, PFsizei height, PFpixelformat format, void* pixels);

/**
 * @brief Applies a post-processing function to modify pixels in the current framebuffer.
 *
 * This function applies a user-defined post-processing function to each pixel in the current framebuffer.
 * The post-processing function can modify the color and depth of each pixel.
 *
 * @warning This function requires a graphics context to be defined.
 *
 * @param postProcessFunction A pointer to the user-defined post-processing function.
 *                            The function signature should be PFcolor postProcessFunction(PFint x, PFint y, PFfloat depth, PFcolor color),
 *                            where x and y are the coordinates of the pixel, depth is the depth value of the pixel, and color is the original color of the pixel.
 *                            The function should return the modified color of the pixel.
 */
PF_API void pfPostProcess(PFpostprocessfunc postProcessFunction);


/*
 *  From here the shifted functions are not dependent on the current context defined by `pfMakeCurrent`
 */


/* Framebuffer functions */

/**
 * @brief Generates a framebuffer object with the specified dimensions and pixel format.
 *
 * This function creates a new framebuffer object with the given width, height, and pixel format.
 * The framebuffer can be used for off-screen rendering or as a render target.
 * By default, the color texture of the generated framebuffer is initialized with zeros,
 * and the depth buffer is initialized with 'FLT_MAX' values.
 *
 * @param width The width of the framebuffer.
 * @param height The height of the framebuffer.
 * @param format The pixel format of the framebuffer.
 * @return PFframebuffer The generated framebuffer object.
 */
PFframebuffer pfGenFramebuffer(PFsizei width, PFsizei height, PFpixelformat format);

/**
 * @brief Deletes a framebuffer object.
 *
 * This function deallocates memory associated with a framebuffer object.
 * After deletion, the framebuffer object becomes invalid.
 *
 * @param framebuffer Pointer to the framebuffer object to delete.
 */
PF_API void pfDeleteFramebuffer(PFframebuffer* framebuffer);

/**
 * @brief Checks if a framebuffer object is valid.
 *
 * This function determines whether a framebuffer object is valid or not.
 * An invalid framebuffer object may occur if it has been deleted or has not been initialized properly.
 *
 * @param framebuffer Pointer to the framebuffer object to check.
 * @return PFboolean True if the framebuffer object is valid, false otherwise.
 */
PF_API PFboolean pfIsValidFramebuffer(PFframebuffer* framebuffer);

/**
 * @brief Clears the color and depth buffers of a framebuffer with the specified color and depth value.
 *
 * This function clears both the color and depth buffers of the framebuffer to the specified color and depth value.
 * It is useful for preparing the framebuffer for rendering.
 *
 * @param framebuffer Pointer to the framebuffer object to clear.
 * @param color The color to clear the framebuffer with.
 * @param depth The depth value to clear the depth buffer with.
 */
PF_API void pfClearFramebuffer(PFframebuffer* framebuffer, PFcolor color, PFfloat depth);

/**
 * @brief Retrieves the color value of a pixel from the framebuffer.
 *
 * This function retrieves the color value of a specific pixel from the framebuffer.
 * The pixel coordinates (x, y) are provided.
 *
 * @param framebuffer Pointer to the framebuffer object.
 * @param x The X coordinate of the pixel.
 * @param y The Y coordinate of the pixel.
 * @return PFcolor The color value of the pixel.
 */
PF_API PFcolor pfGetFramebufferPixel(const PFframebuffer* framebuffer, PFsizei x, PFsizei y);

/**
 * @brief Retrieves the depth value of a pixel from the framebuffer.
 *
 * This function retrieves the depth value of a specific pixel from the framebuffer.
 * The pixel coordinates (x, y) are provided.
 *
 * @param framebuffer Pointer to the framebuffer object.
 * @param x The X coordinate of the pixel.
 * @param y The Y coordinate of the pixel.
 * @return PFfloat The depth value of the pixel.
 */
PF_API PFfloat pfGetFramebufferDepth(const PFframebuffer* framebuffer, PFsizei x, PFsizei y);

/**
 * @brief Sets the color and depth values of a pixel in the framebuffer, subject to depth testing.
 *
 * This function sets the color and depth values of a specific pixel in the framebuffer,
 * with an additional depth testing step based on the specified depth function.
 * The pixel coordinates (x, y), depth value (z), color value, and depth function are provided.
 *
 * @param framebuffer Pointer to the framebuffer object.
 * @param x The X coordinate of the pixel.
 * @param y The Y coordinate of the pixel.
 * @param z The depth value of the pixel.
 * @param color The color value of the pixel.
 * @param depthFunc The depth comparison function used for depth testing.
 */
PF_API void pfSetFramebufferPixelDepthTest(PFframebuffer* framebuffer, PFsizei x, PFsizei y, PFfloat z, PFcolor color, PFdepthfunc depthFunc);

/**
 * @brief Sets the color and depth values of a pixel in the framebuffer.
 *
 * This function sets the color and depth values of a specific pixel in the framebuffer.
 * The pixel coordinates (x, y) and the color value are provided.
 *
 * @param framebuffer Pointer to the framebuffer object.
 * @param x The X coordinate of the pixel.
 * @param y The Y coordinate of the pixel.
 * @param z The depth value of the pixel.
 * @param color The color value of the pixel.
 */
PF_API void pfSetFramebufferPixelDepth(PFframebuffer* framebuffer, PFsizei x, PFsizei y, PFfloat z, PFcolor color);

/**
 * @brief Sets the color value of a pixel in the framebuffer without modifying its depth value.
 *
 * This function sets the color value of a specific pixel in the framebuffer,
 * without modifying its depth value.
 * The pixel coordinates (x, y) and the color value are provided.
 *
 * @param framebuffer Pointer to the framebuffer object.
 * @param x The X coordinate of the pixel.
 * @param y The Y coordinate of the pixel.
 * @param color The color value of the pixel.
 */
PF_API void pfSetFramebufferPixel(PFframebuffer* framebuffer, PFsizei x, PFsizei y, PFcolor color);



/* Texture functions */

/**
 * @brief Generates a texture object with the specified pixel data, width, height, and format.
 *
 * This function creates a new texture object using the provided pixel data, width, height, and format.
 * The pixel data represents the texture image, and its format should match the specified format parameter.
 *
 * @param pixels Pointer to the pixel data of the texture.
 * @param width The width of the texture.
 * @param height The height of the texture.
 * @param format The pixel format of the texture.
 * @return PFtexture The generated texture object.
 */
PF_API PFtexture pfGenTexture(void* pixels, PFsizei width, PFsizei height, PFpixelformat format);

/**
 * @brief Generates an empty texture buffer with the specified width, height, and format.
 *
 * This function creates a new texture buffer without any initial pixel data.
 * The texture buffer is created with the provided width, height, and pixel format.
 * The buffer allocated for the texture is initialized with zeros by default.
 *
 * @param width The width of the texture buffer.
 * @param height The height of the texture buffer.
 * @param format The pixel format of the texture buffer.
 * @return PFtexture The generated texture object.
 */
PF_API PFtexture pfGenTextureBuffer(PFsizei width, PFsizei height, PFpixelformat format);

/**
 * @brief Generates a texture buffer filled with a solid color.
 *
 * This function creates a new texture buffer filled with the specified color.
 * The texture buffer is created with the provided width, height, color, and pixel format.
 *
 * @param width The width of the texture buffer.
 * @param height The height of the texture buffer.
 * @param color The color to fill the texture buffer with.
 * @param format The pixel format of the texture buffer.
 * @return PFtexture The generated texture object.
 */
PF_API PFtexture pfGenColorTextureBuffer(PFsizei width, PFsizei height, PFcolor color, PFpixelformat format);

/**
 * @brief Deletes a texture object.
 *
 * This function deallocates memory associated with a texture object.
 * After deletion, the texture object becomes invalid.
 *
 * @param texture Pointer to the texture object to delete.
 */
PF_API void pfDeleteTexture(PFtexture* texture);

/**
 * @brief Checks if a texture object is valid.
 *
 * This function determines whether a texture object is valid or not.
 * An invalid texture object may occur if it has been deleted or has not been initialized properly.
 *
 * @param texture Pointer to the texture object to check.
 * @return PFboolean True if the texture object is valid, false otherwise.
 */
PF_API PFboolean pfIsValidTexture(PFtexture* texture);

/**
 * @brief Sets the color value of a pixel in the texture.
 *
 * This function sets the color value of a specific pixel in the texture.
 * The pixel coordinates (x, y) and the color value are provided.
 *
 * @param texture Pointer to the texture object.
 * @param x The X coordinate of the pixel.
 * @param y The Y coordinate of the pixel.
 * @param color The color value of the pixel.
 */
PF_API void pfSetTexturePixel(PFtexture* texture, PFsizei x, PFsizei y, PFcolor color);

/**
 * @brief Retrieves the color value of a pixel from the texture.
 *
 * This function retrieves the color value of a specific pixel from the texture.
 * The pixel coordinates (x, y) are provided.
 *
 * @param texture Pointer to the texture object.
 * @param x The X coordinate of the pixel.
 * @param y The Y coordinate of the pixel.
 * @return PFcolor The color value of the pixel.
 */
PF_API PFcolor pfGetTexturePixel(const PFtexture* texture, PFsizei x, PFsizei y);

/**
 * @brief Sets the color value of a sampled texture coordinate.
 *
 * This function sets the color value of a specific texture coordinate (u, v) in the texture.
 * The texture coordinates (u, v) and the color value are provided.
 *
 * @note: To use this function correctly, textures must be power-of-two (POT).
 *        If you absolutely need support for non-POT textures, you can define
 *        `PF_SUPPORT_NO_POT_TEXTURE`. Just be aware that sample retrieval
 *        will be slower as it will perform two modulo operations per call
 *        instead of two bit-wise AND operations.
 *
 * @param texture Pointer to the texture object.
 * @param u The U coordinate of the texture.
 * @param v The V coordinate of the texture.
 * @param color The color value of the texture sample.
 */
PF_API void pfSetTextureSample(PFtexture* texture, PFfloat u, PFfloat v, PFcolor color);

/**
 * @brief Retrieves the color value of a sampled texture coordinate.
 *
 * This function retrieves the color value of a specific texture coordinate (u, v) from the texture.
 * The texture coordinates (u, v) are provided.
 *
 * @note: To use this function correctly, textures must be power-of-two (POT).
 *        If you absolutely need support for non-POT textures, you can define
 *        `PF_SUPPORT_NO_POT_TEXTURE`. Just be aware that sample retrieval
 *        will be slower as it will perform two modulo operations per call
 *        instead of two bit-wise AND operations.
 *
 * @param texture Pointer to the texture object.
 * @param u The U coordinate of the texture.
 * @param v The V coordinate of the texture.
 * @return PFcolor The color value of the texture sample.
 */
PF_API PFcolor pfGetTextureSample(const PFtexture* texture, PFfloat u, PFfloat v);


/*
 *  Blending functions
 *
 *  These functions are intended to be used with `pfBlendFunc` to define the blend mode.
 *  You can also define your own functions following the signature of `PFblendfunc`.
 */

PF_API PFcolor pfBlendDisabled(PFcolor source, PFcolor destination);

PF_API PFcolor pfBlend(PFcolor source, PFcolor destination);
PF_API PFcolor pfBlendAlpha(PFcolor source, PFcolor destination);
PF_API PFcolor pfBlendAdditive(PFcolor source, PFcolor destination);
PF_API PFcolor pfBlendSubtractive(PFcolor source, PFcolor destination);
PF_API PFcolor pfBlendMultiplicative(PFcolor source, PFcolor destination);

/*
 *  Depth testing functions
 *
 *  These functions are intended to be used with `pfDepthFunc` to define the depth testing mode.
 *  You can also define your own functions following the signature of `PFdepthfunc`.
*/

PF_API PFboolean pfDepthLess(PFfloat source, PFfloat destination);
PF_API PFboolean pfDepthEqual(PFfloat source, PFfloat destination);
PF_API PFboolean pfDepthLequal(PFfloat source, PFfloat destination);
PF_API PFboolean pfDepthGreater(PFfloat source, PFfloat destination);
PF_API PFboolean pfDepthNotequal(PFfloat source, PFfloat destination);
PF_API PFboolean pfDepthGequal(PFfloat source, PFfloat destination);


#if defined(__cplusplus)
}
#endif //__cplusplus

#endif //PIXEL_FORGE_H
