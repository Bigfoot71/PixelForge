#define PF_RAYLIB_COMMON_IMPL
#include "raylib_common.h"

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

int main(void)
{
    // Init raylib window and set target FPS
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PixelForge - Framebuffer");
    SetTargetFPS(60);

    // Create a rendering buffer in RAM as well as in VRAM (see raylib_common.h)
    PF_TargetBuffer target = PF_LoadTargetBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    PFctx *ctx = PF_InitFromTargetBuffer(target); // PixelForge context

    // Create framebuffer
    PFframebuffer fbTarget = pfGenFramebuffer(
        SCREEN_WIDTH, SCREEN_HEIGHT,
        PF_PIXELFORMAT_R8G8B8A8);

    // Define the camera position and a phase for the rotation
    Vector3 camPos = { -2.0f, 1.5f, -2.0f };
    float timer = 0;

    while (!WindowShouldClose())
    {
        // Update camera position
        camPos.x = 2.0f * cos(timer);
        camPos.z = 2.0f * sin(timer);
        timer += GetFrameTime();

        // Renders the cube in the framebuffer
        pfEnableFramebuffer(&fbTarget);
        {
            pfClearColor(255, 255, 255, 255);
            pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

            PF_Begin3D(SCREEN_WIDTH, SCREEN_HEIGHT, 60.0);
                PF_Update3D(camPos.x, camPos.y, camPos.z, 0, 0, 0);
                PF_DrawCube(1.0f);
            PF_End3D();
        }
        pfDisableFramebuffer();

        // Clear the main destination buffer (RAM)
        pfClearColor(0, 0, 0, 255);
        pfClear(PF_COLOR_BUFFER_BIT);

        // Rendering the framebuffer in the main buffer with a different scale
        pfColor3ub(255, 255, 255);
        PF_DrawTexture(&fbTarget.texture, (SCREEN_WIDTH - 400) / 2.0f, (SCREEN_HEIGHT - 300) / 2.0f, 400, 300);

        // Texture rendering via raylib
        BeginDrawing();
            ClearBackground(BLACK);
            PF_DrawTargetBuffer(target, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            DrawFPS(10, 10);
        EndDrawing();
    }

    // Unload assets
    pfDeleteFramebuffer(&fbTarget);

    // Unload the PixelForge context and the target buffer
    pfDeleteContext(ctx);
    PF_UnloadTargetBuffer(target);

    // Close raylib window
    CloseWindow();

    return 0;
}
