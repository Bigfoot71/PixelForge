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

#include "../pixelforge.h"
#include "./config.h"
#include "../pfm.h"

/*
    Internal context struct and other structs used by them
*/

/**
 * @brief Structure representing a vertex attribute buffer.
 */
typedef struct {
    const void  *buffer;                ///< Buffer containing the vertices
    PFsizei     stride;                 ///< Byte stride between each vertex
    PFint       size;                   ///< Number of elements per vertex
    PFdatatype  type;                   ///< Data type stored by the buffer
} PFvertexattribbuffer;

/**
 * @brief Structure representing vertex attributes.
 */
typedef struct {
    PFvertexattribbuffer positions;     ///< Position attribute buffer
    PFvertexattribbuffer normals;       ///< Normal attribute buffer
    PFvertexattribbuffer colors;        ///< Color attribute buffer
    PFvertexattribbuffer texcoords;     ///< Texture coordinates attribute buffer
} PFvertexattribs;

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
} PFvertex;

/**
 * @brief Structure representing a light source.
 */
typedef struct PFlight PFlight;
struct PFlight {
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
    PFlight *next;                      ///< Pointer to the next light in a linked list
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
} PFmaterial;

/**
 * @brief Structure representing material color following.
 */
typedef struct {
    PFface face;                        ///< Face(s) whose material color must be followed
    PFenum mode;                        ///< Material color which must follow the current color (see 'pfColorMaterial')
} PFmatcolfollowing;

/**
 * @brief Structure representing fog properties.
 */
typedef struct {
    PFfogmode mode;                    ///< Fog mode (see 'PFfogmode')
    PFfloat density;                   ///< Density of fog (used only with exponential fog modes)
    PFfloat start;                     ///< Distance at which fog starts
    PFfloat end;                       ///< Distance at which fog ends
    PFcolor color;                     ///< Color of the fog
} PFfog;

/**
 * @brief Structure representing the main rendering context of the library.
 * TODO: Reorganize the context structure
 */
typedef struct {

    PFframebuffer *currentFramebuffer;                      ///< Pointer to the current framebuffer
    PFtexture *currentTexture;                              ///< Pointer to the current texture
    PFMmat4 *currentMatrix;                                 ///< Pointer to the current matrix
    void *auxFramebuffer;                                   ///< Auxiliary buffer for double buffering

    PFblendfunc blendFunction;                              ///< Blend function for alpha blending
    PFdepthfunc depthFunction;                              ///< Function for depth testing

    PFint vpPos[2];                                         ///< Represents the top-left corner of the viewport
    PFsizei vpDim[2];                                       ///< Represents the dimensions of the viewport (minus one)
    PFint vpMin[2];                                         ///< Represents the minimum renderable point of the viewport (top-left)
    PFint vpMax[2];                                         ///< Represents the maximum renderable point of the viewport (bottom-right)

    PFvertexattribs vertexAttribs;                          ///< Vertex attributes used by 'pfDrawArrays' or 'pfDrawElements' (e.g., normal, texture coordinates)
    PFvertex vertexBuffer[6];                               ///< Buffer used for storing primitive vertices, used for processing and rendering
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

    PFmaterial faceMaterial[2];                             ///< Material properties for faces [0: front] [1: back]
    PFmatcolfollowing materialColorFollowing;               ///< Material color which must follow the current color (see 'pfColorMaterial')

    PFlight lights[PF_MAX_LIGHT_STACK];                     ///< Array of lights
    PFlight *activeLights;                                  ///< Pointer to the currently active light in the list of lights (see PFlight->next)

    PFfog fog;                                              ///< Fog properties (see PFfog)

    PFMmat4 matProjection;                                  ///< Projection matrix, user adjustable
    PFMmat4 matTexture;                                     ///< Texture matrix, user adjustable
    PFMmat4 matNormal;                                      ///< Normal matrix, calculated and used internally
    PFMmat4 matModel;                                       ///< Model matrix, user adjustable (the one used if we push in PF_MODELVIEW mode)
    PFMmat4 matView;                                        ///< View matrix, user adjustable (the default one used in PF_MODELVIEW mode)

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

} PFctx;

/* Current thread local-thread declaration */

extern PF_CTX_DECL PFctx *currentCtx;


#endif //PF_INTERNAL_CONTEXT_H
