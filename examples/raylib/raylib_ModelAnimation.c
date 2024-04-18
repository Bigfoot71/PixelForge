#define PF_COMMON_IMPL
#include "../common.h"
#include <raylib.h>
#include <raymath.h>
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
    Model model = LoadModel(RESOURCES_PATH "models/robot.glb");

    // Load the animations of the 3D model with raylib
    int animsCount = 0;
    unsigned int animIndex = 0;
    unsigned int animCurrentFrame = 0;
    ModelAnimation *modelAnimations = LoadModelAnimations(RESOURCES_PATH "models/robot.glb", &animsCount);

    // Define some values for the camera
    Vector3 camPos = { 25.0f, 25.0f, 25.0f };
    Vector3 camTar = { 0.0f, 10.0f, 0.0f };

    // Enable lighting system
    pfEnable(PF_LIGHTING);

    // Config one light
    Vector3 camDir = Vector3Subtract(camTar, camPos);
    camDir = Vector3Normalize(camDir);

    pfEnableLight(0);
    pfLightfv(0, PF_POSITION, &camPos);
    pfLightfv(0, PF_SPOT_DIRECTION, &camDir);

    //const PFMvec3 col = { 1.0f, 0.0f, 0.0f };
    //pfMaterialfv(PF_FRONT, PF_DIFFUSE, &col);

    // Start the main loop
    while (!WindowShouldClose())
    {
        // Select current animation
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) animIndex = (animIndex + 1)%animsCount;
        else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) animIndex = (animIndex + animsCount - 1)%animsCount;

        // Update model animation
        ModelAnimation anim = modelAnimations[animIndex];
        animCurrentFrame = (animCurrentFrame + 1)%anim.frameCount;
        UpdateModelAnimation(model, anim, animCurrentFrame);

        // Clear the destination buffer (RAM)
        pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

        // Draw something on each iteration of the main loop
        PF_Begin3D(SCREEN_WIDTH, SCREEN_HEIGHT, 60.0);
        PF_Update3D(camPos.x, camPos.y, camPos.z, 0, 10.0f, 0);
        PF_DrawGrid(10.0f, 10.0f);
        PF_DrawModel(model, (Vector3) { 0 }, 5.0f, WHITE);
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
        EndDrawing();
    }

    pfDeleteContext(ctx);
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
        pfMultMatrixf((float*)(&transform));
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
