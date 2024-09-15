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

#ifndef PF_INTERNAL_CONTEXT_H
#define PF_INTERNAL_CONTEXT_H

#include "../../pixelforge.h"
#include "../config.h"
#include "../vector.h"
#include "../../pfm.h"
#include "../color.h"
#include "../simd.h"

/*
    This file contains all the internal structures and functions
    not exposed that are used by the public context API.
*/

/**
 * @brief PFItex (texture) forward declaration
 */
struct PFItex;

/**
 * @brief Defines a function pointer type for color blending functions.
 *
 * This function pointer type is used to specify a function that blends two colors together.
 * The function should take two colors as input and return the resulting blended color.
 *
 * @param src The source color to blend. This is the color that will be blended with the destination color.
 * @param dst The destination color to blend with. This is the color that will be blended with the source color.
 * 
 * @return The resulting blended color after applying the blending function to the source and destination colors.
 */
typedef PFcolor (*PFIblendfunc)(PFcolor src, PFcolor dst);

/**
 * @brief Function pointer type for depth testing.
 *
 * This type defines a function pointer for a depth test function that compares
 * two floating-point values representing source and destination depths.
 *
 * @param src The source depth value.
 * @param dst The destination depth value.
 * @return A boolean value indicating the result of the depth test.
 */
typedef PFboolean (*PFIdepthfunc)(PFfloat src, PFfloat dst);

/**
 * @brief Function pointer type for getting a pixel from a texture.
 *
 * @param pixels A pointer to the pixel data of the texture.
 * @param index The index of the pixel to retrieve.
 * @return The color value of the pixel at the specified index.
 */
typedef PFcolor (*PFIpixelgetter)(const void* pixels, PFsizei index);

/**
 * @brief Function pointer type for setting a pixel in a texture.
 *
 * @param pixels A pointer to the pixel data of the texture.
 * @param index The index of the pixel to set.
 * @param color The color value to set the pixel to.
 */
typedef void (*PFIpixelsetter)(void* pixels, PFsizei index, PFcolor color);

/**
 * @brief Texture sampler function pointer type.
 * This function samples a texture at the given (u, v) coordinates.
 *
 * @param tex A pointer to the texture to sample from.
 * @param u The u coordinate for texture sampling.
 * @param v The v coordinate for texture sampling.
 * @return The color sampled from the texture at the specified coordinates.
 */
typedef PFcolor (*PFItexturesampler)(const struct PFItex* tex, PFfloat u, PFfloat v);

#if PF_SIMD_SUPPORT

/**
 * @brief Defines a function pointer type for SIMD color blending functions.
 *
 * This function pointer type is used to specify a function that blends multiple colors together
 * using SIMD operations. It processes arrays of colors efficiently
 * in parallel.
 *
 * @param[out] out The output color array where the resulting blended colors will be stored.
 * @param[in] src The source color array to blend. This is the array of colors that will be blended with
 *                the destination color array.
 * @param[in] dst The destination color array to blend with. This is the array of colors that will be
 *                blended with the source color array.
 *
 * @return void This function does not return a value; instead, it writes the blended results directly
 *               to the `out` array.
 */
typedef void (*PFIblendfunc_simd)(PFcolor_simd out, const PFcolor_simd src, const PFcolor_simd dst);

/**
 * @brief Function pointer type for SIMD depth testing.
 *
 * This type defines a function pointer for a SIMD depth test function that compares
 * two SIMD floating-point values representing source and destination depths.
 *
 * @param src The source depth value as a SIMD floating-point type.
 * @param dst The destination depth value as a SIMD floating-point type.
 * @return A SIMD floating-point value representing the result of the depth test.
 */
typedef PFIsimdvf (*PFIdepthfunc_simd)(PFIsimdvf src, PFIsimdvf dst);

/**
 * @brief Function pointer type for getting multiple pixels from a texture using SIMD instructions.
 *
 * This function retrieves multiple pixels at once from a texture, using SIMD (Single Instruction, Multiple Data) instructions.
 *
 * @param pixels A pointer to the pixel data of the texture.
 * @param offsets A SIMD register containing the indices of the pixels to retrieve.
 * @return A SIMD register containing the color values of the pixels at the specified indices.
 */
typedef PFIsimdvi (*PFIpixelgetter_simd)(const void* pixels, PFIsimdvi offsets);

/**
 * @brief Function pointer type for setting multiple pixels in a texture using SIMD instructions.
 *
 * This function sets multiple pixels at once in a texture, using SIMD (Single Instruction, Multiple Data) instructions.
 *
 * @param pixels A pointer to the pixel data of the texture.
 * @param offset The starting index of the pixels to set.
 * @param colors A SIMD register containing the color values to set the pixels to.
 * @param mask A SIMD register used to mask which pixels should be set.
 */
typedef void (*PFIpixelsetter_simd)(void* pixels, PFsizei offset, PFIsimdvi colors, PFIsimdvi mask);

/**
 * @brief SIMD texture sampler function pointer type.
 * This function samples a texture using SIMD (Single Instruction, Multiple Data) coordinates.
 *
 * @param tex A pointer to the texture to sample from.
 * @param texcoords A SIMD vector containing the texture coordinates.
 * @return The SIMD integer result sampled from the texture at the specified coordinates.
 */
typedef PFIsimdvi (*PFItexturesampler_simd)(const struct PFItex* tex, const PFsimdv2f texcoords);

#endif //PF_SIMD_SUPPORT

/**
 * @brief Structure representing a texture.
 */
struct PFItex {

    PFIpixelgetter           getter;
    PFIpixelsetter           setter;
    PFItexturesampler        sampler;

#if PF_SIMD_SUPPORT
    PFIpixelgetter_simd      getterSimd;
    PFIpixelsetter_simd      setterSimd;
    PFItexturesampler_simd   samplerSimd;
#endif //PF_SIMD_SUPPORT

    void                    *pixels;
    PFfloat                 tx, ty;
    PFsizei                 w, h;
    PFdatatype              type;
    PFpixelformat           format;

};

/**
 * @brief Structure representing a vertex attribute buffer.
 */
typedef struct {
    const void  *buffer;                ///< Buffer containing the vertices
    PFsizei     stride;                 ///< Byte stride between each vertex
    PFint       size;                   ///< Number of elements per vertex
    PFdatatype  type;                   ///< Data type stored by the buffer
} PFIvertexattribbuffer;

/**
 * @brief Structure representing vertex attributes.
 */
typedef struct {
    PFIvertexattribbuffer positions;     ///< Position attribute buffer
    PFIvertexattribbuffer normals;       ///< Normal attribute buffer
    PFIvertexattribbuffer colors;        ///< Color attribute buffer
    PFIvertexattribbuffer texcoords;     ///< Texture coordinates attribute buffer
} PFIvertexattribs;

/**
 * @brief Structure representing a vertex.
 */
typedef struct {
    PFMvec4 homogeneous;                ///< Homogeneous coordinates
    PFMvec2 screen;                     ///< Screen coordinates
    PFMvec4 position;                   ///< Position coordinates
    PFMvec3 normal;                     ///< Normal vector
    PFMvec2 texcoord;                   ///< Texture coordinates
    PFcolor color;                      ///< Color
} PFIvertex;

/**
 * @brief Structure representing a light source.
 */
typedef struct PFIlight PFIlight;
struct PFIlight {
    PFMvec3 position;                   ///< Position of the light source
    PFMvec3 direction;                  ///< Direction of the light source
    PFfloat innerCutOff;                ///< Inner cut off angle of the light cone
    PFfloat outerCutOff;                ///< Outer cut off angle of the light cone
    PFfloat attConstant;                ///< Constant attenuation factor
    PFfloat attLinear;                  ///< Linear attenuation factor
    PFfloat attQuadratic;               ///< Quadratic attenuation factor
    PFcolor ambient;                    ///< Ambient color of the light
    PFcolor diffuse;                    ///< Diffuse color of the light
    PFcolor specular;                   ///< Specular color of the light
    PFIlight *next;                      ///< Pointer to the next light in a linked list
};

/**
 * @brief Structure representing material properties.
 */
typedef struct {
    PFcolor ambient;                    ///< Ambient material color
    PFcolor diffuse;                    ///< Diffuse material color
    PFcolor specular;                   ///< Specular material color
    PFcolor emission;                   ///< Emissive material color
    PFfloat shininess;                  ///< Material shininess coefficient
} PFImaterial;

/**
 * @brief Structure representing material color following.
 */
typedef struct {
    PFface face;                        ///< Face(s) whose material color must be followed
    PFenum mode;                        ///< Material color which must follow the current color (see 'pfColorMaterial')
} PFImatcolfollowing;

/**
 * @brief Structure representing fog properties.
 */
typedef struct {
    PFfogmode mode;                    ///< Fog mode (see 'PFfogmode')
    PFfloat density;                   ///< Density of fog (used only with exponential fog modes)
    PFfloat start;                     ///< Distance at which fog starts
    PFfloat end;                       ///< Distance at which fog ends
    PFcolor color;                     ///< Color of the fog
} PFIfog;

/**
 * @brief Internal structure representing a single render call within a render list.
 *
 * This structure holds all the information needed for a single rendering operation.
 * It is used internally by the rendering engine to store the state and data associated
 * with a specific rendering command. Each render call corresponds to a specific set of 
 * vertices, textures, materials, and other attributes that define how an object is rendered.
 *
 * This structure is typically created when a render command is issued and is stored in a render list.
 * The render list is then executed later to draw the objects as specified by these render calls.
 */
typedef struct {
    PFImaterial faceMaterial[2];  ///< Materials for the front and back faces of the object
    PFIvector   positions;        ///< Vertex positions for the render call
    PFIvector   texcoords;        ///< Texture coordinates for each vertex
    PFIvector   normals;          ///< Normal vectors for each vertex (used for lighting)
    PFIvector   colors;           ///< Color values for each vertex (used for per-vertex coloring)
    PFtexture   texture;          ///< Handle to the texture applied to this render call
    PFdrawmode  drawMode;         ///< Drawing mode (defines how the vertices are interpreted)
} PFIrendercall;

/**
 * @brief Internal representation of a render list.
 *
 * `PFIrenderlist` is a type alias for `PFIvector`, which represents a dynamic collection of `PFIrendercall` structures.
 * This type is used internally to store a sequence of rendering commands (render calls).
 *
 * Each element of this vector corresponds to a single render call, which contains the necessary 
 * information (such as vertex positions, normals, textures, etc.) to render a specific object or part of an object.
 *
 * Render lists are compiled by the user through the API (`pfNewList`, `pfEndList`, etc.), and then executed 
 * later with `pfCallList`. The underlying `PFIvector` allows for efficient dynamic storage and retrieval of 
 * these render commands.
 */
typedef PFIvector PFIrenderlist;

/**
 * @brief Structure for backing up the current rendering context state.
 *
 * This structure is used to save the state of the rendering context before 
 * starting the recording of a render list with `pfNewList`. Once the list recording 
 * is complete (`pfEndList`), the saved state can be restored to ensure the rendering 
 * context returns to its previous configuration.
 *
 * Fields in this structure represent the current state of the materials, texture 
 * coordinates, normals, vertex colors, and other settings at the time the context 
 * is backed up. This allows the rendering system to preserve the exact state and 
 * avoid any unintended side effects from recording a render list.
 */
typedef struct {
    PFImaterial faceMaterial[2];   //< Materials for the front and back faces.
    PFMvec2     currentTexcoord;   //< Current texture coordinate (2D vector).
    PFMvec3     currentNormal;     //< Current normal vector (3D vector).
    PFcolor     currentColor;      //< Current vertex color.
    PFtexture   currentTexture;    //< Handle to the currently bound texture.
    PFuint      state;             //< Bitfield representing the current state flags.
} PFIctxbackup;

/**
 * @brief Structure representing the main rendering context of the library.
 * TODO: Reorganize the context structure
 */
typedef struct {

    PFframebuffer *currentFramebuffer;                      ///< Pointer to the current framebuffer
    PFtexture currentTexture;                               ///< Pointer to the current texture
    PFMmat4 *currentMatrix;                                 ///< Pointer to the current matrix
    void *auxFramebuffer;                                   ///< Auxiliary buffer for double buffering

    PFIblendfunc blendFunction;                             ///< SISD Blend function for color blending
    PFIdepthfunc depthFunction;                             ///< SISD Function for depth testing

#if PF_SIMD_SUPPORT
    PFIblendfunc_simd blendSimdFunction;                    ///< SIMD Blend function for color blending
    PFIdepthfunc_simd depthSimdFunction;                    ///< SIMD Function for depth testing
#endif //PF_SIMD_SUPPORT

    PFint vpPos[2];                                         ///< Represents the top-left corner of the viewport
    PFsizei vpDim[2];                                       ///< Represents the dimensions of the viewport (minus one)
    PFint vpMin[2];                                         ///< Represents the minimum renderable point of the viewport (top-left)
    PFint vpMax[2];                                         ///< Represents the maximum renderable point of the viewport (bottom-right)

    PFIvertexattribs vertexAttribs;                         ///< Vertex attributes used by 'pfDrawArrays' or 'pfDrawElements' (e.g., normal, texture coordinates)
    PFIvertex vertexBuffer[6];                              ///< Buffer used for storing primitive vertices, used for processing and rendering
    PFsizei vertexCounter;                                  ///< Number of vertices in 'ctx.vertexBuffer'

    PFMvec3 currentNormal;                                  ///< Current normal assigned by 'pfNormal'                  - (Stored in 'ctx.vertexBuffer' after the call to 'pfVertex')
    PFMvec2 currentTexcoord;                                ///< Current texture coordinates assigned by 'pfTexCoord'   - (Stored in 'ctx.vertexBuffer' after the call to 'pfVertex')
    PFcolor currentColor;                                   ///< Current color assigned by by 'pfColor'                 - (Stored in 'ctx.vertexBuffer' after the call to 'pfVertex')

    PFframebuffer mainFramebuffer;                          ///< Screen buffer for rendering
    PFframebuffer *bindedFramebuffer;                       ///< Framebuffer currently bound if PF_FRAMEBUFFER state is active

    PFcolor clearColor;                                     ///< Color used to clear the screen
    PFfloat clearDepth;                                     ///< Depth value used to clear the screen

    PFfloat pointSize;                                      ///< Rasterized point size
    PFfloat lineWidth;                                      ///< Rasterized line width

    PFMvec4 rasterPos;                                      ///< Current raster position (for pfDrawPixels)
    PFMvec2 pixelZoom;                                      ///< Pixel zoom factor (for pfDrawPixels)

    PFdrawmode currentDrawMode;                             ///< Current drawing mode (e.g., lines, triangles)
    PFpolygonmode polygonMode[2];                           ///< Polygon mode for faces [0: front] [1: back]

    PFImaterial faceMaterial[2];                            ///< Material properties for faces [0: front] [1: back]
    PFImatcolfollowing materialColorFollowing;              ///< Material color which must follow the current color (see 'pfColorMaterial')

    PFIlight lights[PF_MAX_LIGHT_STACK];                    ///< Array of lights
    PFIlight *activeLights;                                 ///< Pointer to the currently active light in the list of lights (see PFIlight->next)

    PFIfog fog;                                             ///< Fog properties (see PFIfog)

    PFIrenderlist *currentRenderList;                       ///< Pointer to the render list where we are currently writing (NULL if no list is currently being written)
    PFIctxbackup ctxBackup;                                 ///< Used to store some context data before writing in a render list, and restore these values ​​after writing

    PFMmat4 matProjection;                                  ///< Projection matrix, user adjustable
    PFMmat4 matTexture;                                     ///< Texture matrix, user adjustable
    PFMmat4 matModel;                                       ///< Model matrix, user adjustable (the one used if we push in PF_MODELVIEW mode)
    PFMmat4 matView;                                        ///< View matrix, user adjustable (the default one used in PF_MODELVIEW mode)

    PFMmat4 matMVP;                                         ///< Model view projection matrix, calculated and used internally
    PFMmat4 matNormal;                                      ///< Normal matrix, calculated and used internally

    PFMmat4 stackProjection[PF_MAX_PROJECTION_STACK_SIZE];  ///< Projection matrix stack for push/pop operations
    PFMmat4 stackModelview[PF_MAX_MODELVIEW_STACK_SIZE];    ///< Modelview matrix stack for push/pop operations
    PFMmat4 stackTexture[PF_MAX_TEXTURE_STACK_SIZE];        ///< Texture matrix stack for push/pop operations
    PFsizei stackProjectionCounter;                         ///< Counter for matrix stack operations
    PFsizei stackModelviewCounter;                          ///< Counter for matrix stack operations
    PFsizei stackTextureCounter;                            ///< Counter for matrix stack operations

    PFmatrixmode currentMatrixMode;                         ///< Current matrix mode (e.g., PF_MODELVIEW, PF_PROJECTION)
    PFboolean modelMatrixUsed;                              ///< Flag indicating if the model matrix is used

    PFshademode shadingMode;                                ///< Type of shading (e.g., flat, smooth)
    PFface cullFace;                                        ///< Faces to cull

    PFerrcode errCode;                                      ///< Last error code
    PFuint state;                                           ///< Current context state

} PFIctx;


/* Current thread local-thread declaration */

extern PF_CTX_DECL PFIctx *G_currentCtx;


/* Internal context functions */

void pfiMakeContextBackup(void);
void pfiRestoreContext(void);

void pfiHomogeneousToScreen(PFIvertex* v);
void pfiProcessAndRasterize(void);


#endif //PF_INTERNAL_CONTEXT_H
