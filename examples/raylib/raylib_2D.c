#define PF_RAYLIB_COMMON_IMPL
#include "raylib_common.h"

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

int main(void)
{
    // Init raylib window and set target FPS
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PixelForge - Basic 2D");
    SetTargetFPS(60);

    // Create a rendering buffer in RAM as well as in VRAM (see raylib_common.h)
    PF_TargetBuffer target = PF_LoadTargetBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    PFctx *ctx = PF_InitFromTargetBuffer(target); // PixelForge context

    // Draw a triangle once with color vertex interpolation
    pfBegin(PF_TRIANGLES);
        pfColor3f(1.0f, 0.0f, 0.0f); // Red
        pfVertex2f(-0.5f, -0.5f);
        pfColor3f(0.0f, 1.0f, 0.0f); // Green
        pfVertex2f(0.5f, -0.5f);
        pfColor3f(0.0f, 0.0f, 1.0f); // Blue
        pfVertex2f(0.0f, 0.5f);
    pfEnd();

    while (!WindowShouldClose())
    {
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