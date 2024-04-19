#define PF_RAYLIB_COMMON_IMPL
#include "raylib_common.h"

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

int main(void)
{
    // Init raylib window and set target FPS
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PixelForge - Textured Model");
    SetTargetFPS(60);

    // Create a rendering buffer in RAM as well as in VRAM (see raylib_common.h)
    PF_TargetBuffer target = PF_LoadTargetBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    PFctx *ctx = PF_InitFromTargetBuffer(target); // PixelForge context

    // Load a 3D model with raylib
    Model model = LoadModel(RESOURCES_PATH "models/character.obj");
    PFtexture modelDiffuse = PF_LoadTexture(RESOURCES_PATH "images/character.png");

    // Define the camera position and a phase for the rotation
    Vector3 camPos = { 35.0f, 30.0f, 35.0f };
    float timer = 0;

    // Activate texture rendering
    pfEnable(PF_TEXTURE_2D);

    // Start the main loop
    while (!WindowShouldClose())
    {
        // Update camera position
        camPos.x = 35.0f * cosf(timer);
        camPos.z = 35.0f * sinf(timer);
        timer += GetFrameTime();

        // Clear the destination buffer (RAM)
        pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

        // Draw something on each iteration of the main loop
        PF_Begin3D(SCREEN_WIDTH, SCREEN_HEIGHT, 60.0);
        {
            PF_Update3D(camPos.x, camPos.y, camPos.z, 0, 12.5f, 0);

            PF_DrawGrid(10.0f, 10.0f);

            pfBindTexture(&modelDiffuse);
                PF_DrawModel(model, (Vector3) { 0 }, 1.5f, WHITE);
            pfBindTexture(0);
        }
        PF_End3D();

        // Texture rendering via raylib
        BeginDrawing();
        {
            ClearBackground(BLACK);

            PF_DrawTargetBuffer(target, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); // Update and draw final texture
            DrawFPS(10, 10);

            DrawText("Model made by Ilya Anchouz Danilov", 4, SCREEN_HEIGHT-28, 24, WHITE);
        }
        EndDrawing();
    }

    // Unload assets
    UnloadModel(model);
    pfDeleteTexture(&modelDiffuse);

    // Unload the PixelForge context and the target buffer
    pfDeleteContext(ctx);
    PF_UnloadTargetBuffer(target);

    // Close raylib window
    CloseWindow();

    return 0;
}

