#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#define PF_SDL2_COMMON_IMPL
#include "SDL2_common.h"

static void DrawGear(float innerRadius, float outerRadius, float width, int teeth, float toothDepth)
{
    int i;
    float r0, r1, r2;
    float angle, da;
    float u, v, len;

    r0 = innerRadius;
    r1 = outerRadius - toothDepth/2.0;
    r2 = outerRadius + toothDepth/2.0;

    da = 2.0*M_PI / teeth / 4.0;

    pfShadeModel(PF_FLAT);

    pfNormal3f(0.0, 0.0, 1.0);

    // Draw front face
    pfBegin(PF_QUAD_STRIP);
        for (i = 0; i <= teeth; i++)
        {
            angle = i * 2.0*M_PI / teeth;
            pfVertex3f(r0*cos(angle), r0*sin(angle), width*0.5);
            pfVertex3f(r1*cos(angle), r1*sin(angle), width*0.5);
            pfVertex3f(r0*cos(angle), r0*sin(angle), width*0.5);
            pfVertex3f(r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5);
        }
    pfEnd();

    // Draw front sides of teeth
    pfBegin(PF_QUADS);
        da = 2.0*M_PI / teeth / 4.0;
        for (i = 0; i < teeth; i++)
        {
            angle = i * 2.0*M_PI / teeth;

            pfVertex3f(r1*cos(angle),      r1*sin(angle),      width*0.5);
            pfVertex3f(r2*cos(angle+da),   r2*sin(angle+da),   width*0.5);
            pfVertex3f(r2*cos(angle+2*da), r2*sin(angle+2*da), width*0.5);
            pfVertex3f(r1*cos(angle+3*da), r1*sin(angle+3*da), width*0.5);
        }
    pfEnd();

    pfNormal3f(0.0, 0.0, -1.0);

    // Draw back face
    pfBegin(PF_QUAD_STRIP);
        for (i = 0; i <= teeth; i++)
        {
            angle = i * 2.0*M_PI / teeth;
            pfVertex3f(r1*cos(angle), r1*sin(angle), -width*0.5);
            pfVertex3f(r0*cos(angle), r0*sin(angle), -width*0.5);
            pfVertex3f(r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5);
            pfVertex3f(r0*cos(angle), r0*sin(angle), -width*0.5);
        }
    pfEnd();

    // Draw back sides of teeth
    pfBegin(PF_QUADS);
    da = 2.0*M_PI / teeth / 4.0;
        for (i = 0; i < teeth; i++)
        {
            angle = i * 2.0*M_PI / teeth;

            pfVertex3f(r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5);
            pfVertex3f(r2*cos(angle+2*da), r2*sin(angle+2*da), -width*0.5);
            pfVertex3f(r2*cos(angle+da),   r2*sin(angle+da),   -width*0.5);
            pfVertex3f(r1*cos(angle),      r1*sin(angle),      -width*0.5);
        }
    pfEnd();

    // Draw outward faces of teeth
    pfBegin(PF_QUAD_STRIP);
        for (i=0;i<teeth;i++)
        {
            angle = i * 2.0*M_PI / teeth;

            pfVertex3f(r1*cos(angle),      r1*sin(angle),       width*0.5);
            pfVertex3f(r1*cos(angle),      r1*sin(angle),      -width*0.5);
            u = r2*cos(angle+da) - r1*cos(angle);
            v = r2*sin(angle+da) - r1*sin(angle);
            len = sqrt(u*u + v*v);
            u /= len;
            v /= len;
            pfNormal3f(v, -u, 0.0);
            pfVertex3f(r2*cos(angle+da),   r2*sin(angle+da),    width*0.5);
            pfVertex3f(r2*cos(angle+da),   r2*sin(angle+da),   -width*0.5);
            pfNormal3f(cos(angle), sin(angle), 0.0);
            pfVertex3f(r2*cos(angle+2*da), r2*sin(angle+2*da),  width*0.5);
            pfVertex3f(r2*cos(angle+2*da), r2*sin(angle+2*da), -width*0.5);
            u = r1*cos(angle+3*da) - r2*cos(angle+2*da);
            v = r1*sin(angle+3*da) - r2*sin(angle+2*da);
            pfNormal3f(v, -u, 0.0);
            pfVertex3f(r1*cos(angle+3*da), r1*sin(angle+3*da),  width*0.5);
            pfVertex3f(r1*cos(angle+3*da), r1*sin(angle+3*da), -width*0.5);
            pfNormal3f(cos(angle), sin(angle), 0.0);
        }

        pfVertex3f(r1*cos(0), r1*sin(0), width*0.5);
        pfVertex3f(r1*cos(0), r1*sin(0), -width*0.5);
    pfEnd();

    pfShadeModel(PF_SMOOTH);

    // Draw inside radius cylinder
    pfBegin(PF_QUAD_STRIP);
        for (i = 0; i <= teeth; i++)
        {
            angle = i * 2.0*M_PI / teeth;
            pfNormal3f(-cos(angle), -sin(angle), 0.0);
            pfVertex3f(r0*cos(angle), r0*sin(angle), -width*0.5);
            pfVertex3f(r0*cos(angle), r0*sin(angle), width*0.5);
        }
    pfEnd();
}


/* Program */

static float view_rotx = 20.0;
static float view_roty = 30.0;
static float angle = 0.0;

void init(void)
{
   static PFMvec3 pos = {5.0, 5.0, 10.0 };
   static PFMvec3 dir = { 0 };

    pfmVec3Sub(dir, dir, pos);
    pfmVec3Normalize(dir, dir);

    pfLightfv(PF_LIGHT0, PF_POSITION, pos);
    pfLightfv(PF_LIGHT0, PF_SPOT_DIRECTION, dir);
    pfEnable(PF_CULL_FACE);
    pfEnable(PF_LIGHTING);
    pfEnableLight(PF_LIGHT0);
    pfEnable(PF_DEPTH_TEST);
}

void reshape(int width, int height)
{
    float aspectRatio = (float)height / width;

    pfViewport(0, 0, (float)width, (float)height);
    pfMatrixMode(PF_PROJECTION);
    pfLoadIdentity();
    pfFrustum(-1.0, 1.0, -aspectRatio, aspectRatio, 5.0, 60.0);
    pfMatrixMode(PF_MODELVIEW);
    pfLoadIdentity();
    pfTranslatef(0.0, 0.0, -40.0);
}

void draw(void)
{
    pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

    pfEnable(PF_COLOR_MATERIAL);
    pfColorMaterial(PF_FRONT_AND_BACK, PF_AMBIENT_AND_DIFFUSE);

    pfPushMatrix();

        pfRotatef(view_rotx, 1.0, 0.0, 0.0);
        pfRotatef(view_roty, 0.0, 1.0, 0.0);

        pfPushMatrix();
            pfTranslatef(-3.0, -2.0, 0.0);
            pfRotatef(angle, 0.0, 0.0, 1.0);
            pfColor3ub(255, 0, 0);
            DrawGear(1.0, 4.0, 1.0, 20, 0.7);
        pfPopMatrix();

        pfPushMatrix();
            pfTranslatef(3.1, -2.0, 0.0);
            pfRotatef(-2.0*angle-9.0, 0.0, 0.0, 1.0);
            pfColor3ub(0, 255, 0);
            DrawGear(0.5, 2.0, 2.0, 10, 0.7);
        pfPopMatrix();

        pfPushMatrix();
            pfTranslatef(-3.1, 4.2, 0.0);
            pfRotatef(-2.0*angle-25.0, 0.0, 0.0, 1.0);
            pfColor3ub(0, 0, 255);
            DrawGear(1.3, 2.0, 0.5, 10, 0.7);
        pfPopMatrix();

    pfPopMatrix();

    pfDisable(PF_COLOR_MATERIAL);
}

int main(void)
{
    // Create Window
    Window window = Window_Create("PixelForge - Gears",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        300, 300, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    // Init clock system
    Clock clock = Clock_Create(60);

    // Creating the PixelForge context
    PFctx *ctx = PF_InitFromWindow(&window);

    // Init and reshape
    init();
    reshape(window.surface->w, window.surface->h);

    // Main loop
    SDL_Event e;
    int quit = 0;
    while (!quit)
    {
        Clock_Begin(&clock);

        // Waiting for an event
        while (SDL_PollEvent(&e) != 0)
        {
            switch (e.type)
            {
                case SDL_QUIT:
                    quit = 1;
                    break;

                case SDL_WINDOWEVENT:
                    if (e.window.event == SDL_WINDOWEVENT_RESIZED)
                    {
                        // TODO: Handle this case
                    }
                    break;

                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym)
                    {
                        case SDLK_UP:
                            view_rotx += 5.0;
                            break;
                        case SDLK_DOWN:
                            view_rotx -= 5.0;
                            break;
                        case SDLK_LEFT:
                            view_roty += 5.0;
                            break;
                        case SDLK_RIGHT:
                            view_roty -= 5.0;
                            break;
                    }
                    break;

                default:
                    break;
            }
        }

        // Update and draw
        angle += 2;
        draw();

        // We update the surface of the window
        Window_Update(&window);

        Clock_End(&clock);
    }

    // Freeing resources and closing SDL
    pfDeleteContext(ctx);
    Window_Destroy(&window);

    return 0;
}
