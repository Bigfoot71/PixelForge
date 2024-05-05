#define PF_RAYLIB_COMMON_IMPL
#include "raylib_common.h"

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

static bool lightEnabled = false;

static bool WallCollision(Camera3D* camera, const Image* imMap);

static void InitFlashLight(void);
static void ToggleFlashLight(void);
static void UpdateFlashLight(Camera3D camera);

int main(void)
{
    // Init raylib window and set target FPS
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PixelForge - First Person");
    SetTargetFPS(60);
    DisableCursor();

    // Create a rendering buffer in RAM as well as in VRAM (see raylib_common.h)
    PF_TargetBuffer target = PF_LoadTargetBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    PFctx *ctx = PF_InitFromTargetBuffer(target); // PixelForge context

    // Init "flashlight"
    InitFlashLight();

    Camera3D camera = {
        (Vector3) { 1.2f, 0.5f, 1.2f },
        (Vector3) { 1.5f, 0.5f, 1.5f },
        (Vector3) { 0.0f, 1.0f, 0.0f },
        60.0f,
        CAMERA_PERSPECTIVE
    };

    Image imMap = LoadImage(RESOURCES_PATH "images/cubicmap.png");
    Model model = LoadModelFromMesh(GenMeshCubicmap(imMap, (Vector3) { 1.0f, 1.0f, 1.0f }));

    PFtexture texMap = PF_LoadTexture(RESOURCES_PATH "images/cubicmap_atlas.png");

    while (!WindowShouldClose())
    {
        // Update

        Vector3 dir = {
            IsKeyDown(KEY_W) - IsKeyDown(KEY_S),
            IsKeyDown(KEY_D) - IsKeyDown(KEY_A),
            IsKeyDown(KEY_SPACE) - IsKeyDown(KEY_LEFT_SHIFT)
        };

        dir = Vector3Scale(Vector3Normalize(dir), 5.0f * GetFrameTime());

        UpdateCameraPro(&camera, dir, (Vector3) {
            .x = GetMouseDelta().x * 0.1f,
            .y = GetMouseDelta().y * 0.1f,
            .z = 0.0,
        }, 0.0f);

        if (camera.position.y < 0.5f)
        {
            camera.target.y += 0.5f - camera.position.y;
            camera.position.y = 0.5;
        }

        if (WallCollision(&camera, &imMap))
        {
            (void)WallCollision(&camera, &imMap);
        }

        if (IsKeyPressed(KEY_F))
        {
            ToggleFlashLight();
        }

        if (lightEnabled)
        {
            UpdateFlashLight(camera);
        }

        // Draw

        pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

        PF_BeginMode3D(camera);
        {
            pfEnable(PF_TEXTURE_2D);
            pfBindTexture(&texMap);

                PF_DrawModel(model, (Vector3) { 0 }, 1.0f, WHITE);

            pfBindTexture(0);
            pfDisable(PF_TEXTURE_2D);
        }
        PF_EndMode3D();

        BeginDrawing();
            ClearBackground(BLACK);
            PF_DrawTargetBuffer(target, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            DrawFPS(10, 10);
        EndDrawing();
    }

    // Unload assets
    pfDeleteTexture(&texMap);
    UnloadModel(model);

    // Unload the PixelForge context and the target buffer
    pfDeleteContext(ctx);
    PF_UnloadTargetBuffer(target);

    // Close raylib window
    CloseWindow();

    return 0;
}

bool WallCollision(Camera3D* camera, const Image* imMap)
{
    Vector2 pos2D = { .x = camera->position.x, .y = camera->position.z };
    Vector2 rdPos2D = { .x = round(pos2D.x), .y = round(pos2D.y) };

    int xMax = MIN(rdPos2D.x + 1, imMap->width - 1);
    int yMax = MIN(rdPos2D.y + 1, imMap->height - 1);

    Rectangle camRect = { .x = pos2D.x - 0.2f, .y = pos2D.y - 0.2f, .width = 0.4f, .height = 0.4f };
    Vector2 result_disp = { .x = 0, .y = 0 };

    for (int y = fmaxf(rdPos2D.y - 1, 0); y <= yMax; y += 1)
    {
        for (int x = fmaxf(rdPos2D.x - 1, 0); x <= xMax; x += 1)
        {
            if ((x != rdPos2D.x || y != rdPos2D.y) && ((PFubyte*)imMap->data)[y * imMap->width + x] > 0)
            {
                Rectangle tileRec = { .x = x - 0.5f, .y = y - 0.5f, .width = 1.0f, .height = 1.0f };

                Vector2 dist = { .x = pos2D.x - x, .y = pos2D.y - y };
                Vector2 minDist = { .x = (camRect.width + tileRec.width) * 0.5f, .y = (camRect.height + tileRec.height) * 0.5f };

                Vector2 collisionVec = { .x = 0, .y = 0 };

                if (fabsf(dist.x) < minDist.x && fabsf(dist.y) < minDist.y)
                {
                    Vector2 overlap = {
                        .x = minDist.x - fabsf(dist.x),
                        .y = minDist.y - fabsf(dist.y),
                    };

                    if (overlap.x < overlap.y)
                    {
                        collisionVec.x = (dist.x > 0) ? overlap.x : -overlap.x;
                    }
                    else
                    {
                        collisionVec.y = (dist.y > 0) ? overlap.y : -overlap.y;
                    }
                }

                if (fabsf(collisionVec.x) > fabsf(result_disp.x)) result_disp.x = collisionVec.x;
                if (fabsf(collisionVec.y) > fabsf(result_disp.y)) result_disp.y = collisionVec.y;
            }
        }
    }

    float adx = fabsf(result_disp.x);
    float ady = fabsf(result_disp.y);

    if (adx > ady)
    {
        camera->position.x += result_disp.x;
        camera->target.x += result_disp.x;
    }
    else
    {
        camera->position.z += result_disp.y;
        camera->target.z += result_disp.y;
    }

    return (adx > 0 && ady > 0);
}

void InitFlashLight(void)
{
    pfLightf(PF_LIGHT0, PF_SPOT_CUTOFF, 17.5f);
    pfLightf(PF_LIGHT0, PF_SPOT_OUTER_CUTOFF, 22.5f);
}

void ToggleFlashLight(void)
{
    lightEnabled = !lightEnabled;

    if (lightEnabled)
    {
        pfEnable(PF_LIGHTING);
        pfEnableLight(PF_LIGHT0);
    }
    else
    {
        pfDisable(PF_LIGHTING);
        pfDisableLight(PF_LIGHT0);
    }
}

void UpdateFlashLight(Camera3D camera)
{
    Vector3 direction = Vector3Normalize(Vector3Subtract(camera.target, camera.position));

    pfLightfv(PF_LIGHT0, PF_POSITION, &camera.position);
    pfLightfv(PF_LIGHT0, PF_SPOT_DIRECTION, &direction);
}