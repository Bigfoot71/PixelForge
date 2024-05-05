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

typedef struct {
    const void  *buffer;        ///< Buffer containing the vertices
    PFsizei     stride;         ///< Byte stride between each vertex
    PFint       size;           ///< Number of elements per vertex
    PFdatatype  type;           ///< Data type stored by the buffer
} PFvertexattribbuffer;

typedef struct {
    PFvertexattribbuffer positions;
    PFvertexattribbuffer normals;
    PFvertexattribbuffer colors;
    PFvertexattribbuffer texcoords;
} PFvertexattribs;

typedef struct {
    PFMvec4 homogeneous;
    PFMvec2 screen;
    PFMvec4 position;
    PFMvec3 normal;
    PFMvec2 texcoord;
    PFcolor color;
} PFvertex;

typedef struct {
    PFMvec3 position;
    PFMvec3 direction;
    PFfloat cutoff;
    PFfloat outerCutoff;
    PFfloat attConstant;
    PFfloat attLinear;
    PFfloat attQuadratic;
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

typedef struct {
    PFface face;                                        ///< Face(s) whose material color must be followed
    PFenum mode;                                        ///< Material color which must follow the current color (see 'pfColorMaterial')
} PFmatcolfollowing;

// TODO: Reorganize the context structure

struct PFctx {

    PFframebuffer *currentFramebuffer;                              ///< Pointer to the current framebuffer
    PFtexture *currentTexture;                                      ///< Pointer to the current texture
    PFMmat4 *currentMatrix;                                         ///< Pointer to the current matrix
    void *auxFramebuffer;                                           ///< Auxiliary buffer for double buffering

    PFblendfunc blendFunction;                                      ///< Blend function for alpha blending
    PFdepthfunc depthFunction;                                      ///< Function for depth testing

    PFint vpPos[2];                                                 ///< Represents the top-left corner of the viewport
    PFsizei vpDim[2];                                               ///< Represents the dimensions of the viewport (minus one)
    PFint vpMin[2];                                                 ///< Represents the minimum renderable point of the viewport (top-left)
    PFint vpMax[2];                                                 ///< Represents the maximum renderable point of the viewport (bottom-right)

    PFvertexattribs vertexAttribs;                                  ///< Vertex attributes used by 'pfDrawArrays' or 'pfDrawElements' (e.g., normal, texture coordinates)
    PFvertex vertexBuffer[6];                                       ///< Buffer used for storing primitive vertices, used for processing and rendering
    PFsizei vertexCounter;                                          ///< Number of vertices in 'ctx.vertexBuffer'

    PFMvec3 currentNormal;                                          ///< Current normal assigned by 'pfNormal'                  - (Stored in 'ctx.vertexBuffer' after the call to 'pfVertex')
    PFMvec2 currentTexcoord;                                        ///< Current texture coordinates assigned by 'pfTexCoord'   - (Stored in 'ctx.vertexBuffer' after the call to 'pfVertex')
    PFcolor currentColor;                                           ///< Current color assigned by by 'pfColor'                 - (Stored in 'ctx.vertexBuffer' after the call to 'pfVertex')

    PFframebuffer mainFramebuffer;                                  ///< Screen buffer for rendering

    PFcolor clearColor;                                             ///< Color used to clear the screen
    PFfloat clearDepth;                                             ///< Depth value used to clear the screen

    PFfloat pointSize;                                              ///< Rasterized point size
    PFfloat lineWidth;                                              ///< Rasterized line width

    PFMvec4 rasterPos;                                              ///< Current raster position (for pfDrawPixels)
    PFMvec2 pixelZoom;                                              ///< Pixel zoom factor (for pfDrawPixels)

    PFdrawmode currentDrawMode;                                     ///< Current drawing mode (e.g., lines, triangles)
    PFpolygonmode polygonMode[2];                                   ///< Polygon mode for faces [0: front] [1: back]

    PFmaterial faceMaterial[2];                                     ///< Material properties for faces [0: front] [1: back]
    PFmatcolfollowing materialColorFollowing;                       ///< Material color which must follow the current color (see 'pfColorMaterial')

    PFlight lights[PF_MAX_LIGHT_STACK];                             ///< Array of lights
    PFint lastActiveLight;                                          ///< Index of the last active light

    PFMmat4 projection;                                             ///< Default projection matrix
    PFMmat4 model;                                                  ///< Default model matrix (the one used if we push in PF_MODELVIEW mode)
    PFMmat4 view;                                                   ///< Default view matrix (the default one used in PF_MODELVIEW mode)

    PFMmat4 stackProjection[PF_MAX_PROJECTION_STACK_SIZE];          ///< Projection matrix stack for push/pop operations
    PFMmat4 stackModelview[PF_MAX_MODELVIEW_STACK_SIZE];            ///< Modelview matrix stack for push/pop operations
    PFsizei stackProjectionCounter;                                 ///< Counter for matrix stack operations
    PFsizei stackModelviewCounter;                                  ///< Counter for matrix stack operations

    PFmatrixmode currentMatrixMode;                                 ///< Current matrix mode (e.g., PF_MODELVIEW, PF_PROJECTION)
    PFboolean modelMatrixUsed;                                      ///< Flag indicating if the model matrix is used

    PFshademode shadingMode;                                        ///< Type of shading (e.g., flat, smooth)
    PFface cullFace;                                                ///< Faces to cull

    PFerrcode errCode;                                              ///< Last error code
    PFuint state;                                                   ///< Current context state
};

#endif //PF_INTERNAL_CONTEXT_H
