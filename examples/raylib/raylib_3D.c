#define PF_RAYLIB_COMMON_IMPL
#include "raylib_common.h"

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

int main(void)
{
    // Init raylib window and set target FPS
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PixelForge - Basic 3D");
    SetTargetFPS(60);

    // Create a rendering buffer in RAM as well as in VRAM (see raylib_common.h)
    PF_TargetBuffer target = PF_LoadTargetBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    PFctx *ctx = PF_InitFromTargetBuffer(target); // PixelForge context

    // Define the camera position and a phase for the rotation
    Vector3 camPos = { -2.0f, 1.5f, -2.0f };
    float timer = 0;

    while (!WindowShouldClose())
    {
        // Update camera position
        camPos.x = 2.0f * cosf(timer);
        camPos.z = 2.0f * sinf(timer);
        timer += GetFrameTime();

        // Clear the destination buffer (RAM)
        pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

        // Draw something on each iteration of the main loop
        PF_Begin3D(SCREEN_WIDTH, SCREEN_HEIGHT, 60.0);
            PF_Update3D(camPos.x, camPos.y, camPos.z, 0, 0, 0);
            PF_DrawCube(1.0f);
        PF_End3D();

        // Texture rendering via raylib
        BeginDrawing();
            ClearBackground(BLACK);
            PF_DrawTargetBuffer(target, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        EndDrawing();
    }

    // Unload the PixelForge context and the target buffer
    pfDeleteContext(ctx);
    PF_UnloadTargetBuffer(target);

    // Close raylib window
    CloseWindow();

    return 0;
}