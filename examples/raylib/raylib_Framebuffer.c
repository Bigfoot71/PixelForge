#define PF_COMMON_IMPL
#include "../common.h"
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

    // Create a PixelForge context (see examples/common.h)
    PFctx *ctx = PF_Init(dest.data, dest.width, dest.height);
    PF_Reshape(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Create framebuffer
    PFframebuffer target = pfFramebufferGenBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, PF_PIXELFORMAT_R8G8B8A8);

    // Define the camera position and a phase for the rotation
    Vector3 camPos = { -2.0f, 1.5f, -2.0f };
    float timer = 0;

    while (!WindowShouldClose())
    {
        // Update camera position
        camPos.x = 2.0f * cos(timer);
        camPos.z = 2.0f * sin(timer);
        timer += GetFrameTime();

        // Clear the destination buffer (RAM)
        pfClearColor(0, 0, 0, 255);
        pfClear(PF_COLOR_BUFFER_BIT);

        // Renders the cube in the framebuffer
        pfEnableFramebuffer(&target);
            pfClearColor(255, 255, 255, 255);
            pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);
            PF_Begin3D(SCREEN_WIDTH, SCREEN_HEIGHT, 60.0);
            PF_Update3D(camPos.x, camPos.y, camPos.z, 0, 0, 0);
            PF_DrawCube(1.0f);
            PF_End3D();
        pfDisableFramebuffer();

        // Rendering the framebuffer in the main buffer with a different scale
        pfColor4ub(255, 255, 255, 255);
        PF_DrawTexture(&target.texture, (SCREEN_WIDTH - 400) / 2.0f, (SCREEN_HEIGHT - 300) / 2.0f, 400, 300);

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
    pfDeleteContext(ctx);
    UnloadTexture(gpuDest);
    UnloadImage(dest);

    CloseWindow();

    return 0;
}