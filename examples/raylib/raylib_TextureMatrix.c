#define PF_RAYLIB_COMMON_IMPL
#include "raylib_common.h"

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

int main(void)
{
    // Init raylib window and set target FPS
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PixelForge - Water");
    SetTargetFPS(60);

    // Create a rendering buffer in RAM as well as in VRAM (see raylib_common.h)
    PF_TargetBuffer target = PF_LoadTargetBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    PFcontext ctx = PF_InitFromTargetBuffer(target); // PixelForge context

    // Define the camera position and a phase for the rotation
    const Vector3 camPos = { -20.0f, 10.0f, -20.0f };

    // Load texture and enable texture render mode
    PFtexture texture = PF_LoadTexture(RESOURCES_PATH "images/water.png");
    pfEnable(PF_TEXTURE_2D);

    // Texture translation
    float translate = 0.0f;

    while (!WindowShouldClose())
    {
        // Clear the destination buffer (RAM)
        pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

        // Draw something on each iteration of the main loop
        PF_Begin3D(SCREEN_WIDTH, SCREEN_HEIGHT, 60.0);
        {
            PF_Update3D(camPos.x, camPos.y, camPos.z, 0, 0, 0);

            pfMatrixMode(PF_TEXTURE);
            pfLoadIdentity();

            translate += GetFrameTime();
            pfTranslatef(translate, translate, 0);

            pfMatrixMode(PF_MODELVIEW);
            pfBindTexture(texture);
            pfBegin(PF_QUADS);

                pfTexCoord2f(0, 0);
                pfVertex3f(-1000, 0, -1000);

                pfTexCoord2f(0, 200);
                pfVertex3f(-1000, 0, 1000);

                pfTexCoord2f(200, 200);
                pfVertex3f(1000, 0, 1000);

                pfTexCoord2f(200, 0);
                pfVertex3f(1000, 0, -1000);

            pfEnd();
            pfBindTexture(NULL);
        }
        PF_End3D();

        // Texture rendering via raylib
        BeginDrawing();
            ClearBackground(BLACK);
            PF_DrawTargetBuffer(target, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            DrawFPS(10, 10);
        EndDrawing();
    }

    // Unload the PixelForge context and the target buffer
    pfDeleteContext(ctx);
    PF_UnloadTargetBuffer(target);

    // Close raylib window
    CloseWindow();

    return 0;
}