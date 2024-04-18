#define PF_RAYLIB_COMMON_IMPL
#include "raylib_common.h"

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

/* Arrow entity declarations */

typedef struct {
    Vector2 position;
    float angle;
} Arrow;

Arrow Arrow_Create();
void Arrow_Update(Arrow* arrow);
void Arrow_Draw(Arrow* arrow, PFtexture* texture);

/* Main */

int main(void)
{
    // Init raylib window and set target FPS
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PixelForge - Texture 2D");
    SetTargetFPS(60);

    // Create a rendering buffer in RAM as well as in VRAM (see raylib_common.h)
    PF_TargetBuffer target = PF_LoadTargetBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    PFctx *ctx = PF_InitFromTargetBuffer(target); // PixelForge context

    // We load background and arrow textures
    PFtexture texBG = PF_LoadTexture(RESOURCES_PATH "images/PixelForge.png");
    PFtexture texArrow = PF_LoadTexture(RESOURCES_PATH "images/arrow.png");

    // We create an arrow that will follow the mouse cursor
    Arrow arrow = Arrow_Create();

    // Main loop
    while (!WindowShouldClose())
    {
        Arrow_Update(&arrow);

        // Clear the destination buffer (RAM)
        pfClear(PF_COLOR_BUFFER_BIT);

        // Draw something on each iteration of the main loop
        PF_DrawTexture(&texBG, 0, SCREEN_HEIGHT - 400, 800, 400);
        Arrow_Draw(&arrow, &texArrow);

        // Texture rendering via raylib
        BeginDrawing();
            ClearBackground(BLACK);
            PF_DrawTargetBuffer(target, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            DrawFPS(10, 10);
        EndDrawing();
    }

    // Unload assets
    pfDeleteTexture(&texBG);
    pfDeleteTexture(&texArrow);

    // Unload the PixelForge context and the target buffer
    pfDeleteContext(ctx);
    PF_UnloadTargetBuffer(target);

    // Close raylib window
    CloseWindow();

    return 0;
}

/* Arrow entity definitions */

Arrow Arrow_Create()
{
    return (Arrow) { (Vector2) { 400, 300 }, 0 };
}

void Arrow_Update(Arrow* arrow)
{
    arrow->angle = RAD2DEG(-Vector2LineAngle(arrow->position, GetMousePosition())) + 90.0f;
    float dist = Vector2Distance(arrow->position, GetMousePosition());

    if (dist > 32)
    {
        arrow->position = Vector2MoveTowards(arrow->position, GetMousePosition(), dist * 4.0f * GetFrameTime());
    }
}

void Arrow_Draw(Arrow* arrow, PFtexture* texture)
{
    PF_DrawTextureEx(texture, arrow->position.x, arrow->position.y, 64, 64, 32, 32, arrow->angle);
}
