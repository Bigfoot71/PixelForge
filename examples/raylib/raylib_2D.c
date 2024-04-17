#include "pixelforge.h"
#include <raylib.h>

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

int main(void)
{
    SetTraceLogLevel(LOG_NONE); // NOTE: Used to disable log when drawing text to dest image...

    // Init raylib window and set target FPS
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PixelForge - raylib example");
    SetTargetFPS(60);

    // Create a rendering buffer in RAM as well as in VRAM
    Image dest = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
    Texture gpuDest = LoadTextureFromImage(dest);

    // Creating the PixelForge context
    PFctx *ctx = pfContextCreate(dest.data, dest.width, dest.height, dest.format);
    pfMakeCurrent(ctx);

    while (!WindowShouldClose())
    {
        // Clear the destination buffer
        pfClear(PF_COLOR_BUFFER_BIT);

        // Draw something on each iteration of the main loop
        pfBegin(PF_TRIANGLES);
        pfColor3f(1.0f, 0.0f, 0.0f); // Red
        pfVertex2f(-0.5f, -0.5f);
        pfColor3f(0.0f, 1.0f, 0.0f); // Green
        pfVertex2f(0.5f, -0.5f);
        pfColor3f(0.0f, 0.0f, 1.0f); // Blue
        pfVertex2f(0.0f, 0.5f);
        pfEnd();

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

    // Free allocated data
    pfContextDestroy(ctx);
    UnloadTexture(gpuDest);
    UnloadImage(dest);

    CloseWindow();

    return 0;
}