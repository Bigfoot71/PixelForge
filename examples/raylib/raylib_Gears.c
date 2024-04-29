#define PF_RAYLIB_COMMON_IMPL
#include "raylib_common.h"

#define SCREEN_WIDTH    600
#define SCREEN_HEIGHT   600

static void Gear_Draw(float innerRadius, float outerRadius, float width, int teeth, float toothDepth)
{
    int i;
    float r0, r1, r2;
    float angle, da;
    float u, v, len;

    r0 = innerRadius;
    r1 = outerRadius - toothDepth/2.0f;
    r2 = outerRadius + toothDepth/2.0f;

    da = 2.0f*M_PI / teeth / 4.0f;

    pfShadeModel(PF_FLAT);

    pfNormal3f(0.0f, 0.0f, 1.0f);

    // Draw front face
    pfBegin(PF_QUAD_STRIP);
        for (i = 0; i <= teeth; i++)
        {
            angle = i * 2.0f*M_PI / teeth;
            pfVertex3f(r0*cos(angle), r0*sin(angle), width*0.5f);
            pfVertex3f(r1*cos(angle), r1*sin(angle), width*0.5f);
            pfVertex3f(r0*cos(angle), r0*sin(angle), width*0.5f);
            pfVertex3f(r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5f);
        }
    pfEnd();

    // Draw front sides of teeth
    pfBegin(PF_QUADS);
        da = 2.0f*M_PI / teeth / 4.0f;
        for (i = 0; i < teeth; i++)
        {
            angle = i * 2.0f*M_PI / teeth;

            pfVertex3f(r1*cos(angle),      r1*sin(angle),      width*0.5f);
            pfVertex3f(r2*cos(angle+da),   r2*sin(angle+da),   width*0.5f);
            pfVertex3f(r2*cos(angle+2*da), r2*sin(angle+2*da), width*0.5f);
            pfVertex3f(r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5f);
        }
    pfEnd();

    pfNormal3f(0.0f, 0.0f, -1.0f);

    // Draw back face
    pfBegin(PF_QUAD_STRIP);
        for (i = 0; i <= teeth; i++)
        {
            angle = i * 2.0f*M_PI / teeth;
            pfVertex3f(r1*cos(angle), r1*sin(angle), -width*0.5f);
            pfVertex3f(r0*cos(angle), r0*sin(angle), -width*0.5f);
            pfVertex3f(r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5f);
            pfVertex3f(r0*cos(angle), r0*sin(angle), -width*0.5f);
        }
    pfEnd();

    // Draw back sides of teeth
    pfBegin(PF_QUADS);
    da = 2.0f*M_PI / teeth / 4.0f;
        for (i = 0; i < teeth; i++)
        {
            angle = i * 2.0f*M_PI / teeth;

            pfVertex3f(r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5f);
            pfVertex3f(r2*cos(angle+2*da), r2*sin(angle+2*da), -width*0.5f);
            pfVertex3f(r2*cos(angle+da),   r2*sin(angle+da),   -width*0.5f);
            pfVertex3f(r1*cos(angle),      r1*sin(angle),      -width*0.5f);
        }
    pfEnd();

    // Draw outward faces of teeth
    pfBegin(PF_QUAD_STRIP);
        for (i=0;i<teeth;i++)
        {
            angle = i * 2.0f*M_PI / teeth;

            pfVertex3f(r1*cos(angle),      r1*sin(angle),       width*0.5f);
            pfVertex3f(r1*cos(angle),      r1*sin(angle),      -width*0.5f);
            u = r2*cos(angle+da) - r1*cos(angle);
            v = r2*sin(angle+da) - r1*sin(angle);
            len = sqrt(u*u + v*v);
            u /= len;
            v /= len;
            pfNormal3f(v, -u, 0.0f);
            pfVertex3f(r2*cos(angle+da),   r2*sin(angle+da),    width*0.5f);
            pfVertex3f(r2*cos(angle+da),   r2*sin(angle+da),   -width*0.5f);
            pfNormal3f(cos(angle), sin(angle), 0.0f);
            pfVertex3f(r2*cos(angle+2*da), r2*sin(angle+2*da),  width*0.5f);
            pfVertex3f(r2*cos(angle+2*da), r2*sin(angle+2*da), -width*0.5f);
            u = r1*cos(angle+3*da) - r2*cos(angle+2*da);
            v = r1*sin(angle+3*da) - r2*sin(angle+2*da);
            pfNormal3f(v, -u, 0.0f);
            pfVertex3f(r1*cos(angle+3*da), r1*sin(angle+3*da),  width*0.5f);
            pfVertex3f(r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5f);
            pfNormal3f(cos(angle), sin(angle), 0.0f);
        }

        pfVertex3f(r1*cos(0), r1*sin(0), width*0.5f);
        pfVertex3f(r1*cos(0), r1*sin(0), -width*0.5f);
    pfEnd();

    pfShadeModel(PF_SMOOTH);

    // Draw inside radius cylinder
    pfBegin(PF_QUAD_STRIP);
        for (i = 0; i <= teeth; i++)
        {
            angle = i * 2.0f*M_PI / teeth;
            pfNormal3f(-cos(angle), -sin(angle), 0.0f);
            pfVertex3f(r0*cos(angle), r0*sin(angle), -width*0.5f);
            pfVertex3f(r0*cos(angle), r0*sin(angle), width*0.5f);
        }
    pfEnd();
}


/* Program */

static float viewRotX = 20.0f;
static float viewRotY = 30.0f;
static float viewRotZ = 0.0f;
static float angle = 0.0f;

void Gears_Init(void)
{
   PFMvec3 pos = { 5.0f, 5.0f, 10.0f };
   PFMvec3 dir = { 0 };

    pfmVec3Sub(dir, dir, pos);
    pfmVec3Normalize(dir, dir);

    pfLightfv(PF_LIGHT0, PF_POSITION, pos);
    pfLightfv(PF_LIGHT0, PF_SPOT_DIRECTION, dir);
    pfEnable(PF_CULL_FACE);
    pfEnable(PF_LIGHTING);
    pfEnableLight(PF_LIGHT0);
    pfEnable(PF_DEPTH_TEST);
}

void Gears_Reshape(int width, int height)
{
    float aspectRatio = (float)height / width;

    pfViewport(0, 0, width, height);
    pfMatrixMode(PF_PROJECTION);
    pfLoadIdentity();
    pfFrustum(-1.0f, 1.0f, -aspectRatio, aspectRatio, 5.0f, 60.0f);
    pfMatrixMode(PF_MODELVIEW);
    pfLoadIdentity();
    pfTranslatef(0.0f, 0.0f, -40.0f);
}

void Gears_Draw(void)
{
    pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

    pfEnable(PF_COLOR_MATERIAL);
    pfColorMaterial(PF_FRONT_AND_BACK, PF_AMBIENT_AND_DIFFUSE);

    pfPushMatrix();

        pfRotatef(viewRotX, 1.0f, 0.0f, 0.0f);
        pfRotatef(viewRotY, 0.0f, 1.0f, 0.0f);
        pfRotatef(viewRotZ, 0.0f, 0.0f, 1.0f);

        pfPushMatrix();
            pfTranslatef(-3.0f, -2.0f, 0.0f);
            pfRotatef(angle, 0.0f, 0.0f, 1.0f);
            pfColor3ub(255, 0, 0);
            Gear_Draw(1.0f, 4.0f, 1.0f, 20, 0.7f);
        pfPopMatrix();

        pfPushMatrix();
            pfTranslatef(3.1f, -2.0f, 0.0f);
            pfRotatef(-2.0f*angle-9.0f, 0.0f, 0.0f, 1.0f);
            pfColor3ub(0, 255, 0);
            Gear_Draw(0.5f, 2.0f, 2.0f, 10, 0.7f);
        pfPopMatrix();

        pfPushMatrix();
            pfTranslatef(-3.1f, 4.2, 0.0f);
            pfRotatef(-2.0f*angle-25.0f, 0.0f, 0.0f, 1.0f);
            pfColor3ub(0, 0, 255);
            Gear_Draw(1.3f, 2.0f, 0.5f, 10, 0.7f);
        pfPopMatrix();

    pfPopMatrix();

    pfDisable(PF_COLOR_MATERIAL);
}

int main(void)
{
    // Init raylib window and set target FPS
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PixelForge - Gears");
    SetTargetFPS(60);

    // Create a rendering buffer in RAM as well as in VRAM (see raylib_common.h)
    PF_TargetBuffer target = PF_LoadTargetBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    PFctx *ctx = PF_InitFromTargetBuffer(target); // PixelForge context

    // Init and reshape
    Gears_Init();
    Gears_Reshape(SCREEN_WIDTH, SCREEN_HEIGHT);

    while (!WindowShouldClose())
    {
        if (IsKeyDown(KEY_UP)) viewRotX += 5.0f;
        if (IsKeyDown(KEY_DOWN)) viewRotX -= 5.0f;
        if (IsKeyDown(KEY_LEFT)) viewRotY += 5.0f;
        if (IsKeyDown(KEY_RIGHT)) viewRotY -= 5.0f;
        if (IsKeyDown(KEY_Z) && !IsKeyDown(KEY_LEFT_SHIFT)) viewRotZ += 5.0f;
        if (IsKeyDown(KEY_Z) && IsKeyDown(KEY_LEFT_SHIFT)) viewRotZ -= 5.0f;

        // Update and draw
        angle += 90 * GetFrameTime();
        Gears_Draw();

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