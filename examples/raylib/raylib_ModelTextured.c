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
    Model model = LoadModel(RESOURCES_PATH "models/FemaleOpenGameArt.obj");
    PFtexture modelDiffuse = PF_LoadTexture(RESOURCES_PATH "images/castle_diffuse.png");

    // Define the camera position and a phase for the rotation
    Vector3 camPos = { 50.0f, 25.0f, 50.0f };
    float timer = 0;

    // Activate texture rendering
    pfEnable(PF_TEXTURE_2D);

    // Start the main loop
    while (!WindowShouldClose())
    {
        // Update camera position
        camPos.x = 50.0f * cosf(timer);
        camPos.z = 50.0f * sinf(timer);
        timer += GetFrameTime();

        // Clear the destination buffer (RAM)
        pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

        // Draw something on each iteration of the main loop
        PF_Begin3D(SCREEN_WIDTH, SCREEN_HEIGHT, 60.0);
        {
            PF_Update3D(camPos.x, camPos.y, camPos.z, 0, 10.0f, 0);

            PF_DrawGrid(10.0f, 10.0f);

            pfBindTexture(&modelDiffuse);
                PF_DrawModel(model, (Vector3) { 0 }, 1.0f, WHITE);
            pfBindTexture(0);
        }
        PF_End3D();

        // Texture rendering via raylib
        BeginDrawing();
        {
            ClearBackground(BLACK);

            PF_DrawTargetBuffer(target, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); // Update and draw final texture
            DrawFPS(10, 10);

            DrawText("Visual glitch also present with this model in the original raylib example :-(", 0, SCREEN_HEIGHT-18, 18, WHITE);
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

