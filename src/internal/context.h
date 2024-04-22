
#include "../pixelforge.h"
#include "../pfm.h"

/*
    Internal context struct and other structs used by them
*/

typedef struct {
    const void *positions;
    const void *normals;
    const void *colors;
    const void *texcoords;
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

struct PFctx {

    PFframebuffer screenBuffer;                 // Screen buffer for rendering
    PFframebuffer *currentFramebuffer;          // Pointer to the current framebuffer

    PFint viewportX, viewportY;                 // X and Y coordinates of the viewport
    PFsizei viewportW, viewportH;               // Width and height of the viewport

    PFdrawmode currentDrawMode;                 // Current drawing mode (e.g., lines, triangles)
    PFblendfunc blendFunction;                  // Blend function for alpha blending
    PFcolor clearColor;                         // Color used to clear the screen

    PFfloat pointSize;                          // Rasterized point size

    PFMvec3 currentNormal;                      // Current normal vector for lighting calculations
    PFMvec2 currentTexcoord;                    // Current texture coordinates
    PFcolor currentColor;                       // Current color for vertex rendering

    PFvertex vertexBuffer[6];                   // Vertex buffer for geometry
    PFsizei vertexCount;                        // Number of vertices in the buffer

    PFMvec4 rasterPos;                          // Current raster position (for pfDrawPixels)
    PFMvec2 pixelZoom;                          // Pixel zoom factor (for pfDrawPixels)

    PFlight lights[PF_MAX_LIGHTS];
    PFint lastActiveLight;

    PFmaterial frontMaterial;
    PFmaterial backMaterial;

    PFmatrixmode currentMatrixMode;             // Current matrix mode (e.g., PF_MODELVIEW, PF_PROJECTION)
    PFMmat4 *currentMatrix;                     // Pointer to the current matrix
    PFMmat4 projection;                         // Default projection matrix
    PFMmat4 modelview;                          // Default modelview matrix
    PFMmat4 transform;                          // Transformation matrix for translation, rotation, and scaling
    PFboolean transformRequired;                // Flag indicating whether transformation is required for vertices
    PFMmat4 stack[PF_MAX_MATRIX_STACK_SIZE];    // Matrix stack for push/pop operations
    PFsizei stackCounter;                       // Counter for matrix stack operations

    PFvertexattribs vertexAttribs;              // Vertex attributes (e.g., normal, texture coordinates)
    PFtexture *currentTexture;                  // Pointer to the current texture

    PFushort vertexAttribState;                 // State of vertex attributes
    PFushort state;                             // Current rendering state

    PFface cullFace;                            // Faces to cull

    PFerrcode errCode;                          // Contains the last error code that occurred

};
