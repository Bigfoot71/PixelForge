
#include "../pixelforge.h"
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

    PFframebuffer mainFramebuffer;                      ///< Screen buffer for rendering
    PFframebuffer *currentFramebuffer;                  ///< Pointer to the current framebuffer

    PFint viewportX, viewportY;                         ///< X and Y coordinates of the viewport
    PFsizei viewportW, viewportH;                       ///< Width and height of the viewport

    PFdrawmode currentDrawMode;                         ///< Current drawing mode (e.g., lines, triangles)
    PFblendfunc blendFunction;                          ///< Blend function for alpha blending
    PFdepthfunc depthFunction;
    PFcolor clearColor;                                 ///< Color used to clear the screen
    PFfloat clearDepth;

    PFfloat pointSize;                                  ///< Rasterized point size
    PFpolygonmode polygonMode[2];                       ///< Polygon mode for faces [0: front] [1: back]

    PFMvec3 currentNormal;                              ///< Current normal vector for lighting calculations
    PFMvec2 currentTexcoord;                            ///< Current texture coordinates
    PFcolor currentColor;                               ///< Current color for vertex rendering

    PFvertex vertexBuffer[6];                           ///< Vertex buffer for geometry
    int_fast8_t vertexCount;                            ///< Number of vertices in the buffer

    PFMvec4 rasterPos;                                  ///< Current raster position (for pfDrawPixels)
    PFMvec2 pixelZoom;                                  ///< Pixel zoom factor (for pfDrawPixels)

    PFlight lights[PF_MAX_LIGHT_STACK];
    PFint lastActiveLight;

    PFmaterial faceMaterial[2];
    PFmatcolfollowing materialColorFollowing;           ///< Material color which must follow the current color (see 'pfColorMaterial')

    PFmatrixmode currentMatrixMode;                     ///< Current matrix mode (e.g., PF_MODELVIEW, PF_PROJECTION)
    PFMmat4 *currentMatrix;                             ///< Pointer to the current matrix
    PFMmat4 projection;                                 ///< Default projection matrix
    PFMmat4 modelview;                                  ///< Default modelview matrix
    PFMmat4 transform;                                  ///< Transformation matrix for translation, rotation, and scaling
    PFboolean transformRequired;                        ///< Flag indicating whether transformation is required for vertices
    PFMmat4 stack[PF_MAX_MATRIX_STACK_SIZE];            ///< Matrix stack for push/pop operations
    PFsizei stackCounter;                               ///< Counter for matrix stack operations

    PFvertexattribs vertexAttribs;                      ///< Vertex attributes (e.g., normal, texture coordinates)
    PFtexture *currentTexture;                          ///< Pointer to the current texture

    PFushort state;                                     ///< Current context state

    PFshademode shadingMode;                            ///< Defines the type of shading, whether the colors are interpolated or not
    PFface cullFace;                                    ///< Faces to cull

    PFerrcode errCode;                                  ///< Contains the last error code that occurred

};
