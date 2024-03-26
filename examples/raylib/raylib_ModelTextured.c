#include "pixelforge.h"
#define PF_COMMON_IMPL
#include "../common.h"
#include "raylib.h"
#include <string.h>

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

void PF_DrawMesh(Mesh mesh, Material material, Matrix transform);
void PF_DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint);
void PF_DrawModel(Model model, Vector3 position, float scale, Color tint);

int main(void)
{
    SetTraceLogLevel(LOG_NONE); // NOTE: Used to disable log when drawing text to dest image...

    // Init raylib window and set target FPS
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PixelForge - raylib example");
    SetTargetFPS(60);

    // Create a rendering buffer in RAM as well as in VRAM
    Image dest = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
    Texture gpuDest = LoadTextureFromImage(dest);

    // Create a PixelForge context (see examples/common.h)
    PFctx *ctx = PF_Init(dest.data, dest.width, dest.height);
    PF_Reshape(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Load a 3D model with raylib
    Model model = LoadModel(RESOURCES_PATH "models/castle.obj");
    Image imModelDiffuse = LoadImage(RESOURCES_PATH "images/castle_diffuse.png");
    PFtexture modelDiffuse = pfTextureGenFromBuffer(imModelDiffuse.data, imModelDiffuse.width, imModelDiffuse.height, imModelDiffuse.format);

    // Define the camera position and a phase for the rotation
    PFvec3f camPos = { 50.0f, 25.0f, 50.0f };
    float timer = 0;

    // Start the main loop
    while (!WindowShouldClose())
    {
        // Update camera position
        camPos.x = 50.0f * cos(timer);
        camPos.z = 50.0f * sin(timer);
        timer += GetFrameTime();

        // Clear the destination buffer (RAM)
        pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

        // Draw something on each iteration of the main loop
        PF_Begin3D(SCREEN_WIDTH, SCREEN_HEIGHT, 60.0);
        PF_Update3D(camPos.x, camPos.y, camPos.z, 0, 10.0f, 0);

        PF_DrawGrid(10.0f, 10.0f);

        pfEnableTexture(&modelDiffuse);
        PF_DrawModel(model, (Vector3) { 0 }, 1.0f, WHITE);
        pfDisableTexture();

        PF_End3D();

        // We display the FPS on the destination image
        ImageDrawText(&dest, TextFormat("FPS: %i", GetFPS()), 10, 10, 24,
            ColorFromHSV(MIN((GetFPS()/60.0f)*110.0f, 110.0f), 1.0f, 1.0f));

        // Updates the destination texture
        UpdateTexture(gpuDest, dest.data);

        // Texture rendering via raylib
        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexture(gpuDest, 0, 0, WHITE);
            DrawText("Visual glitch also present with this model in the original raylib example :-(", 0, SCREEN_HEIGHT-18, 18, WHITE);
        EndDrawing();
    }

    pfContextDestroy(ctx);
    UnloadImage(imModelDiffuse);
    UnloadTexture(gpuDest);
    UnloadImage(dest);
    CloseWindow();

    return 0;
}

void PF_DrawMesh(Mesh mesh, Material material, Matrix transform)
{
    if (mesh.animVertices)
    {
        pfEnableStatePointer(PF_VERTEX_ARRAY, mesh.animVertices);
        pfEnableStatePointer(PF_NORMAL_ARRAY, mesh.animNormals);
    }
    else
    {
        pfEnableStatePointer(PF_VERTEX_ARRAY, mesh.vertices);
        pfEnableStatePointer(PF_NORMAL_ARRAY, mesh.normals);
    }

    pfEnableStatePointer(PF_TEXTURE_COORD_ARRAY, mesh.texcoords);
    pfEnableStatePointer(PF_COLOR_ARRAY, mesh.colors);

    pfPushMatrix();
        pfMultMatrixMat4f((PFmat4f*)(&transform));
        pfColor4ub(material.maps[MATERIAL_MAP_DIFFUSE].color.r,
                   material.maps[MATERIAL_MAP_DIFFUSE].color.g,
                   material.maps[MATERIAL_MAP_DIFFUSE].color.b,
                   material.maps[MATERIAL_MAP_DIFFUSE].color.a);

        if (mesh.indices != NULL) pfDrawVertexArrayElements(0, mesh.triangleCount*3, mesh.indices);
        else pfDrawVertexArray(0, mesh.vertexCount);
    pfPopMatrix();

    pfDisableStatePointer(PF_VERTEX_ARRAY);
    pfDisableStatePointer(PF_TEXTURE_COORD_ARRAY);
    pfDisableStatePointer(PF_NORMAL_ARRAY);
    pfDisableStatePointer(PF_COLOR_ARRAY);

    pfDisableTexture();
}

void PF_DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
{
    // Calculate transformation matrix from function parameters
    // Get transform matrix (rotation -> scale -> translation)
    PFmat4f matScale = pfMat4fScale(scale.x, scale.y, scale.z);
    PFmat4f matRotation = pfMat4fRotate((PFvec3f*)(&rotationAxis), DEG2RAD(rotationAngle));
    PFmat4f matTranslation = pfMat4fTranslate(position.x, position.y, position.z);

    PFmat4f matTransform = pfMat4fMul(&matScale, &matRotation);
    matTransform = pfMat4fMul(&matTransform, &matTranslation);

    // Combine model transformation matrix (model.transform) with matrix generated by function parameters (matTransform)
    PFmat4f modelTransform = pfMat4fMul((PFmat4f*)(&model.transform), &matTransform);
    memcpy(&model.transform, &modelTransform, sizeof(Matrix));

    for (int i = 0; i < model.meshCount; i++)
    {
        Color color = model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;

        Color colorTint = WHITE;
        colorTint.r = (unsigned char)((((float)color.r/255.0f)*((float)tint.r/255.0f))*255.0f);
        colorTint.g = (unsigned char)((((float)color.g/255.0f)*((float)tint.g/255.0f))*255.0f);
        colorTint.b = (unsigned char)((((float)color.b/255.0f)*((float)tint.b/255.0f))*255.0f);
        colorTint.a = (unsigned char)((((float)color.a/255.0f)*((float)tint.a/255.0f))*255.0f);

        model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = colorTint;
        PF_DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], model.transform);
        model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = color;
    }
}

void PF_DrawModel(Model model, Vector3 position, float scale, Color tint)
{
    Vector3 vScale = { scale, scale, scale };
    Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };
    PF_DrawModelEx(model, position, rotationAxis, 0.0f, vScale, tint);
}
