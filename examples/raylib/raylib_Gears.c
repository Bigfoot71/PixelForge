#include "pixelforge.h"
#define PF_RAYLIB_COMMON_IMPL
#include "raylib_common.h"

#define SCREEN_WIDTH    600
#define SCREEN_HEIGHT   600

static void Gear(float innerRadius, float outerRadius, float width, int teeth, float toothDepth)
{
    int i;
    float r0, r1, r2;
    float angle, da;
    float u, v, len;

    r0 = innerRadius;
    r1 = outerRadius - toothDepth/2.0f;
    r2 = outerRadius + toothDepth/2.0f;

    da = 2.0f*PFM_PI / teeth / 4.0f;

    pfShadeModel(PF_FLAT);

    pfNormal3f(0.0f, 0.0f, 1.0f);

    // Draw front face
    pfBegin(PF_QUAD_STRIP);
        for (i = 0; i <= teeth; i++)
        {
            angle = i * 2.0f*PFM_PI / teeth;
            pfVertex3f(r0*cos(angle), r0*sin(angle), width*0.5f);
            pfVertex3f(r1*cos(angle), r1*sin(angle), width*0.5f);
            pfVertex3f(r0*cos(angle), r0*sin(angle), width*0.5f);
            pfVertex3f(r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5f);
        }
    pfEnd();

    // Draw front sides of teeth
    pfBegin(PF_QUADS);
        da = 2.0f*PFM_PI / teeth / 4.0f;
        for (i = 0; i < teeth; i++)
        {
            angle = i * 2.0f*PFM_PI / teeth;

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
            angle = i * 2.0f*PFM_PI / teeth;
            pfVertex3f(r1*cos(angle), r1*sin(angle), -width*0.5f);
            pfVertex3f(r0*cos(angle), r0*sin(angle), -width*0.5f);
            pfVertex3f(r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5f);
            pfVertex3f(r0*cos(angle), r0*sin(angle), -width*0.5f);
        }
    pfEnd();

    // Draw back sides of teeth
    pfBegin(PF_QUADS);
    da = 2.0f*PFM_PI / teeth / 4.0f;
        for (i = 0; i < teeth; i++)
        {
            angle = i * 2.0f*PFM_PI / teeth;

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
            angle = i * 2.0f*PFM_PI / teeth;

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
            angle = i * 2.0f*PFM_PI / teeth;
            pfNormal3f(-cos(angle), -sin(angle), 0.0f);
            pfVertex3f(r0*cos(angle), r0*sin(angle), -width*0.5f);
            pfVertex3f(r0*cos(angle), r0*sin(angle), width*0.5f);
        }
    pfEnd();
}

/* Program */

static PFrenderlist gear1, gear2, gear3;
static float viewRotX = 20.0f;
static float viewRotY = 30.0f;
static float viewRotZ = 0.0f;
static float angle = 0.0f;

static void InitScene(void)
{
	static PFfloat pos[4] = {5, 5, 10, 0.0};

	static PFfloat red[4] = {1.0, 0.0, 0.0, 0.0};
	static PFfloat green[4] = {0.0, 1.0, 0.0, 0.0};
	static PFfloat blue[4] = {0.0, 0.0, 1.0, 0.0};
	static PFfloat white[4] = {1.0, 1.0, 1.0, 0.0};

	static PFfloat shininess = 5;

    pfEnable(PF_LIGHTING);
	pfEnable(PF_CULL_FACE);
	pfEnable(PF_DEPTH_TEST);

	pfEnableLight(PF_LIGHT0);
	pfLightfv(PF_LIGHT0, PF_POSITION, pos);
	pfLightfv(PF_LIGHT0, PF_DIFFUSE, white);
	// pfLightfv(PF_LIGHT0, PF_AMBIENT, white);
	pfLightfv(PF_LIGHT0, PF_SPECULAR, white);

	// Make the gears
	gear1 = pfGenList();
	pfNewList(gear1);
	pfMaterialfv(PF_FRONT, PF_DIFFUSE, red);
	pfMaterialfv(PF_FRONT, PF_SPECULAR, white);
	pfMaterialfv(PF_FRONT, PF_SHININESS, &shininess);
	pfColor3fv(red);
	Gear(1.0, 4.0, 1.0, 20, 0.7); // The largest gear.
	pfEndList();

	gear2 = pfGenList();
	pfNewList(gear2);
	pfMaterialfv(PF_FRONT, PF_DIFFUSE, green);
	pfMaterialfv(PF_FRONT, PF_SPECULAR, white);
	pfColor3fv(green);
	Gear(0.5, 2.0, 2.0, 10, 0.7); // The small gear with the smaller hole, to the right.
	pfEndList();

	gear3 = pfGenList();
	pfNewList(gear3);
	pfMaterialfv(PF_FRONT, PF_DIFFUSE, blue);
	pfMaterialfv(PF_FRONT, PF_SPECULAR, white);
	pfColor3fv(blue);
	Gear(1.3, 2.0, 0.5, 10, 0.7); // The small gear above with the large hole.
	pfEndList();

	// pfEnable(PF_NORMALIZE);
}

void Reshape(int width, int height)
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

void Draw(void)
{
	pfPushMatrix();
	pfRotatef(viewRotX, 1.0, 0.0, 0.0);
	pfRotatef(viewRotY, 0.0, 1.0, 0.0);
	pfRotatef(viewRotZ, 0.0, 0.0, 1.0 );

	pfPushMatrix();
	pfTranslatef(-3.0, -2.0, 0.0);
	pfRotatef(angle, 0.0, 0.0, 1.0);
	pfCallList(gear1);
	pfPopMatrix();

	pfPushMatrix();
	pfTranslatef(3.1, -2.0, 0.0);
	pfRotatef(-2.0 * angle - 9.0, 0.0, 0.0, 1.0);
	pfCallList(gear2);
	pfPopMatrix();

	pfPushMatrix();
	pfTranslatef(-3.1, 4.2, 0.0);
	pfRotatef(-2.0 * angle - 25.0, 0.0, 0.0, 1.0);
	pfCallList(gear3);
	pfPopMatrix();

	pfPopMatrix();
}

int main(void)
{
    // Init raylib window and set target FPS
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "PixelForge - Gears");

    // Create a rendering buffer in RAM as well as in VRAM (see raylib_common.h)
    PF_TargetBuffer target = PF_LoadTargetBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    PFcontext ctx = PF_InitFromTargetBuffer(target); // PixelForge context

    // Init and reshape
    InitScene();
    Reshape(SCREEN_WIDTH, SCREEN_HEIGHT);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        // Update
        angle += 90 * dt;
        if (IsKeyDown(KEY_UP)) viewRotX += 180.0f * dt;
        if (IsKeyDown(KEY_DOWN)) viewRotX -= 180.0f * dt;
        if (IsKeyDown(KEY_LEFT)) viewRotY += 180.0f * dt;
        if (IsKeyDown(KEY_RIGHT)) viewRotY -= 180.0f * dt;
        if (IsKeyDown(KEY_A)) viewRotZ += 180.0f * dt;
        if (IsKeyDown(KEY_D)) viewRotZ -= 180.0f * dt;

        // Draw
        pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);
        Draw();

        // Texture rendering via raylib
        BeginDrawing();
            ClearBackground(BLACK);
            PF_DrawTargetBuffer(target, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
            DrawFPS(10, 10);
        EndDrawing();
    }

    // Unload render lists
    pfDeleteList(gear1);
    pfDeleteList(gear2);
    pfDeleteList(gear3);

    // Unload the PixelForge context and the target buffer
    pfDeleteContext(ctx);
    PF_UnloadTargetBuffer(target);

    // Close raylib window
    CloseWindow();

    return 0;
}
