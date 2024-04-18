#define PF_COMMON_IMPL
#include "../common.h"
#include <raylib.h>
#include <raymath.h>

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

/* Arrow "entity" */

typedef struct {
    Vector2 position;
    float angle;
} Arrow;

Arrow Arrow_Create()
{
    return (Arrow) {
        (Vector2) { 400, 300 },
        0
    };
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

/* Main */

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
    pfSetBlendFunction(pfBlendAlpha);
    pfEnable(PF_TEXTURE_2D);

    // We load a background texture
    Image imBG = LoadImage(RESOURCES_PATH "images/PixelForge.png");
    PFtexture texBG = pfGenTexture(imBG.data, imBG.width, imBG.height, imBG.format);   // NOTE: PFpixelform is compatible with raylib PixelFormat!

    // We load an arrow texture
    Image imArrow = LoadImage(RESOURCES_PATH "images/arrow.png");
    PFtexture texArrow = pfGenTexture(imArrow.data, imArrow.width, imArrow.height, imArrow.format);

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

    // Free loaded image
    UnloadImage(imArrow);
    UnloadImage(imBG);

    // NOTE: No need to destroy tex here
    //pfDeleteTexture(&texArrow);
    //pfDeleteTexture(&texBG);

    // Free allocated data
    pfDeleteContext(ctx);
    UnloadTexture(gpuDest);
    UnloadImage(dest);

    CloseWindow();

    return 0;
}