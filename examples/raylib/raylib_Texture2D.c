#include "pixelforge.h"
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
void Arrow_Draw(Arrow* arrow, PFtexture texture);

/* Main */

int main(void)
{
    // Init raylib window and set target FPS
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PixelForge - Texture 2D");
    SetTargetFPS(60);

    // Create a rendering buffer in RAM as well as in VRAM (see raylib_common.h)
    PF_TargetBuffer target = PF_LoadTargetBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    PFcontext ctx = PF_InitFromTargetBuffer(target); // PixelForge context
    PF_Reshape(SCREEN_WIDTH, SCREEN_HEIGHT);

    // We load background as an raylib Image to draw it with pfDrawPixels
    Image imBG = LoadImage(RESOURCES_PATH "images/PixelForge.png");

    // We load arrow texture
    PFtexture texArrow = PF_LoadTexture(RESOURCES_PATH "images/arrow.png");

    // We create an arrow that will follow the mouse cursor
    Arrow arrow = Arrow_Create();

    // Enable texture rendering and color blending (alpha-blending by default)
    pfEnable(PF_TEXTURE_2D | PF_BLEND);

    // Main loop
    while (!WindowShouldClose())
    {
        Arrow_Update(&arrow);

        // Clear the destination buffer (RAM)
        pfClear(PF_COLOR_BUFFER_BIT);

        // Draw the background with pfDrawPixels
        pfRasterPos2i(0, SCREEN_HEIGHT - 400);
        pfPixelZoom(800.0f/imBG.width, 400.0f/imBG.height);
        pfDrawPixels(imBG.width, imBG.height, PF_LUMINANCE_ALPHA, PF_UNSIGNED_BYTE, imBG.data);

        // Draw the arrow entity
        Arrow_Draw(&arrow, texArrow);

        // Texture rendering via raylib
        BeginDrawing();
            ClearBackground(BLACK);
            PF_DrawTargetBuffer(target, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            DrawFPS(10, 10);
        EndDrawing();
    }

    // Unload assets
    pfDeleteTexture(&texArrow, true);
    UnloadImage(imBG);

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
    arrow->angle = PFM_RAD2DEG(-Vector2LineAngle(arrow->position, GetMousePosition())) + 90.0f;
    float dist = Vector2Distance(arrow->position, GetMousePosition());

    if (dist > 32)
    {
        arrow->position = Vector2MoveTowards(arrow->position, GetMousePosition(), dist * 4.0f * GetFrameTime());
    }
}

void Arrow_Draw(Arrow* arrow, PFtexture texture)
{
    PF_DrawTextureEx(texture, arrow->position.x, arrow->position.y, 64, 64, 32, 32, arrow->angle);
}
