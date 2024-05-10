#define PF_RAYLIB_COMMON_IMPL
#include "raylib_common.h"

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

int main(void)
{
    // Init raylib window and set target FPS
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PixelForge - Animated Model");
    SetTargetFPS(60);

    // Create a rendering buffer in RAM as well as in VRAM (see raylib_common.h)
    PF_TargetBuffer target = PF_LoadTargetBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    PFcontext ctx = PF_InitFromTargetBuffer(target); // PixelForge context

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
        {
            PF_Update3D(camPos.x, camPos.y, camPos.z, 0, 10.0f, 0);

            PF_DrawGrid(10.0f, 10.0f);
            PF_DrawModel(model, (Vector3) { 0 }, 5.0f, WHITE);
        }
        PF_End3D();

        // Texture rendering via raylib
        BeginDrawing();
            ClearBackground(BLACK);
            PF_DrawTargetBuffer(target, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); // Update and draw final texture
            DrawFPS(10, 10);
        EndDrawing();
    }

    // Unload assets
    UnloadModel(model);
    UnloadModelAnimations(modelAnimations, animsCount);

    // Unload the PixelForge context and the target buffer
    pfDeleteContext(ctx);
    PF_UnloadTargetBuffer(target);

    // Close raylib window
    CloseWindow();

    return 0;
}
