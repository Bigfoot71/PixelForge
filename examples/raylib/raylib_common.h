#ifndef PF_RAYLIB_COMMON_H
#define PF_RAYLIB_COMMON_H

#include "pixelforge.h"
#ifdef PF_RAYLIB_COMMON_H
#   define PF_COMMON_IMPL
#endif //PF_RAYLIB_COMMON_H

#include "../common.h"

#include <raylib.h>
#include <raymath.h>

/* Helper structs for raylib */

typedef struct {
    Image image;
    Texture tex;
} PF_TargetBuffer;

/* Easy PixelForge context creation for raylib */

PFcontext PF_InitFromTargetBuffer(PF_TargetBuffer destBuffer);

/* Load PixelForge texture */

PFtexture PF_LoadTexture(const char* fileName);

/* Destination buffer management functions */

PF_TargetBuffer PF_LoadTargetBuffer(int width, int height);
void PF_UnloadTargetBuffer(PF_TargetBuffer destBuffer);

void PF_DrawTargetBuffer(PF_TargetBuffer target, float x, float y, float w, float h);

/* Mode 3D management (raylib Camera3D compatibility) */

void PF_BeginMode3D(Camera3D camera);
void PF_EndMode3D(void);

/* Draw meshes/models functions */

void PF_DrawMesh(Mesh mesh, Material material, Matrix transform);
void PF_DrawModel(Model model, Vector3 position, float scale, Color tint);
void PF_DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint);


/* Functions implementation */

#ifdef PF_RAYLIB_COMMON_IMPL

PFcontext PF_InitFromTargetBuffer(PF_TargetBuffer destBuffer)
{
    return PF_Init(destBuffer.image.data, destBuffer.image.width, destBuffer.image.height, PF_RGBA, PF_UNSIGNED_BYTE);
}

/* Load PixelForge texture */

PFtexture PF_LoadTexture(const char* fileName)
{
    // NOTE 1: Here we load the image via raylib and then assign the pointer to the PFtexture structure.
    // Therefore, we should not unload the image loaded with raylib, but we will obviously need
    // to unload the PixelForge texture to which we have entrusted the memory pointer.

    Image image = LoadImage(fileName);

    PFpixelformat format = 0;
    PFdatatype type = 0;

    switch (image.format)
    {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
            format = PF_LUMINANCE;
            type = PF_UNSIGNED_BYTE;
            break;

        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
            format = PF_LUMINANCE_ALPHA;
            type = PF_UNSIGNED_BYTE;
            break;

        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
            format = PF_RGB;
            type = PF_UNSIGNED_SHORT_5_6_5;
            break;

        case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
            format = PF_RGB;
            type = PF_UNSIGNED_BYTE;
            break;

        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
            format = PF_RGBA;
            type = PF_UNSIGNED_SHORT_5_5_5_1;
            break;

        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
            format = PF_RGBA;
            type = PF_UNSIGNED_SHORT_4_4_4_4;
            break;

        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            format = PF_RGBA;
            type = PF_UNSIGNED_BYTE;
            break;

        case PIXELFORMAT_UNCOMPRESSED_R32:
            format = PF_RED;
            type = PF_FLOAT;
            break;

        case PIXELFORMAT_UNCOMPRESSED_R32G32B32:
            format = PF_RGB;
            type = PF_FLOAT;
            break;

        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
            format = PF_RGBA;
            type = PF_FLOAT;
            break;

        case PIXELFORMAT_UNCOMPRESSED_R16:
            format = PF_RED;
            type = PF_HALF_FLOAT;
            break;

        case PIXELFORMAT_UNCOMPRESSED_R16G16B16:
            format = PF_RGB;
            type = PF_HALF_FLOAT;
            break;

        case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
            format = PF_RGBA;
            type = PF_HALF_FLOAT;
            break;

        default:
            return (PFtexture) { 0 };
    }

    return pfGenTexture(image.data, image.width, image.height, format, type);
}

/* Destination buffer management functions */

PF_TargetBuffer PF_LoadTargetBuffer(int width, int height)
{
    PF_TargetBuffer target = { 0 };

    target.image = GenImageColor(width, height, BLANK);
    target.tex = LoadTextureFromImage(target.image);

    return target;
}

void PF_UnloadTargetBuffer(PF_TargetBuffer destBuffer)
{
    UnloadImage(destBuffer.image);
    UnloadTexture(destBuffer.tex);
}

void PF_DrawTargetBuffer(PF_TargetBuffer target, float x, float y, float w, float h)
{
    UpdateTexture(target.tex, target.image.data);

    DrawTexturePro(target.tex, (Rectangle) { 0, 0, (float)target.tex.width, (float)target.tex.height },
        (Rectangle) { x, y, w, h }, (Vector2) { 0 }, 0.0f, WHITE);
}

/* Mode 3D management (raylib Camera3D compatibility) */

void PF_BeginMode3D(Camera3D camera)
{
    pfMatrixMode(PF_PROJECTION);    // Switch to projection matrix
    pfPushMatrix();                 // Save previous matrix, which contains the settings for the 2d ortho projection
    pfLoadIdentity();               // Reset current matrix (projection)

    float aspect = (float)GetScreenWidth()/(float)GetScreenHeight();

    // NOTE: zNear and zFar values are important when computing depth buffer values
    if (camera.projection == CAMERA_PERSPECTIVE)
    {
        // Setup perspective projection
        double top = 0.01*tan(DEG2RAD(camera.fovy*0.5));
        double right = top*aspect;

        pfFrustum(-right, right, -top, top, 0.01, 1000.0);
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        // Setup orthographic projection
        double top = camera.fovy/2.0;
        double right = top*aspect;

        pfOrtho(-right, right, -top,top, 0.01, 1000.0);
    }

    pfMatrixMode(PF_MODELVIEW);         // Switch back to modelview matrix
    pfLoadIdentity();                   // Reset current matrix (modelview)

    // Setup Camera view
    PFMmat4 matView;
    pfmMat4LookAt(matView, (PFfloat*)(&camera.position), (PFfloat*)(&camera.target), (PFfloat*)(&camera.up));
    pfMultMatrixf((float*)(&matView));  // Multiply modelview matrix by view matrix (camera)

    pfEnable(PF_DEPTH_TEST);            // Enable DEPTH_TEST for 3D
}

void PF_EndMode3D(void)
{
    pfMatrixMode(PF_PROJECTION);    // Switch to projection matrix
    pfPopMatrix();                  // Restore previous matrix (projection) from matrix stack

    pfMatrixMode(PF_MODELVIEW);     // Switch back to modelview matrix
    pfLoadIdentity();               // Reset current matrix (modelview)

    pfDisable(PF_DEPTH_TEST);       // Disable DEPTH_TEST for 2D
}

/* Draw meshes/models functions */

void PF_EnableStatePointer(PFstate state, const void* pointer)
{
    pfEnable(state);

    switch (state)
    {
        case PF_VERTEX_ARRAY:
            pfVertexPointer(3, PF_FLOAT, 0, pointer);
            break;
        
        case PF_NORMAL_ARRAY:
            pfNormalPointer(PF_FLOAT, 0, pointer);
            break;

        case PF_TEXTURE_COORD_ARRAY:
            pfTexCoordPointer(PF_FLOAT, 0, pointer);
            break;

        case PF_COLOR_ARRAY:
            pfColorPointer(4, PF_UNSIGNED_BYTE, 0, pointer);
            break;

        default:
            break;
    }
}

void PF_DisableStatePointer(PFstate state)
{
    pfDisable(state);
}

void PF_DrawMesh(Mesh mesh, Material material, Matrix transform)
{
    if (mesh.animVertices)
    {
        PF_EnableStatePointer(PF_VERTEX_ARRAY, mesh.animVertices);
        PF_EnableStatePointer(PF_NORMAL_ARRAY, mesh.animNormals);
    }
    else
    {
        PF_EnableStatePointer(PF_VERTEX_ARRAY, mesh.vertices);
        PF_EnableStatePointer(PF_NORMAL_ARRAY, mesh.normals);
    }

    PF_EnableStatePointer(PF_TEXTURE_COORD_ARRAY, mesh.texcoords);
    PF_EnableStatePointer(PF_COLOR_ARRAY, mesh.colors);

    pfPushMatrix();
        pfMultMatrixf((float*)(&transform));
        pfColor4ub(material.maps[MATERIAL_MAP_DIFFUSE].color.r,
                   material.maps[MATERIAL_MAP_DIFFUSE].color.g,
                   material.maps[MATERIAL_MAP_DIFFUSE].color.b,
                   material.maps[MATERIAL_MAP_DIFFUSE].color.a);

        if (mesh.indices != NULL) pfDrawElements(PF_TRIANGLES, mesh.triangleCount*3, PF_UNSIGNED_SHORT, mesh.indices);
        else pfDrawArrays(PF_TRIANGLES, 0, mesh.vertexCount);
    pfPopMatrix();

    PF_DisableStatePointer(PF_VERTEX_ARRAY);
    PF_DisableStatePointer(PF_TEXTURE_COORD_ARRAY);
    PF_DisableStatePointer(PF_NORMAL_ARRAY);
    PF_DisableStatePointer(PF_COLOR_ARRAY);
}

void PF_DrawModel(Model model, Vector3 position, float scale, Color tint)
{
    Vector3 vScale = { scale, scale, scale };
    Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };
    PF_DrawModelEx(model, position, rotationAxis, 0.0f, vScale, tint);
}

void PF_DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
{
    // Calculate transformation matrix from function parameters
    // Get transform matrix (rotation -> scale -> translation)
    Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix matRotation = MatrixRotate(rotationAxis, DEG2RAD(rotationAngle));
    Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

    Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

    // Combine model transformation matrix (model.transform) with matrix generated by function parameters (matTransform)
    model.transform = MatrixMultiply(model.transform, matTransform);

    for (int i = 0; i < model.meshCount; i++)
    {
        Color color = model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;

        Color colorTint = WHITE;
        colorTint.r = (unsigned char)((((float)color.r/255.0f)*((float)tint.r/255.0f))*255.0f);
        colorTint.g = (unsigned char)((((float)color.g/255.0f)*((float)tint.g/255.0f))*255.0f);
        colorTint.b = (unsigned char)((((float)color.b/255.0f)*((float)tint.b/255.0f))*255.0f);
        colorTint.a = (unsigned char)((((float)color.a/255.0f)*((float)tint.a/255.0f))*255.0f);

        model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = colorTint;
        PF_DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], MatrixTranspose(model.transform));
        model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = color;
    }
}

#endif //PF_RAYLIB_COMMON_IMPL
#endif //PF_RAYLIB_COMMON_H