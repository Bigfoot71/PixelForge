#include "pixelforge.h"
#include <SDL2/SDL.h>

/* Helper functions */

typedef struct {
    Uint32 ticksAtLastFrame;
    float deltaTime;
    Uint32 maxFPS;
} Clock;

Clock Clock_Create(Uint32 maxFPS)
{
    return (Clock) {
        .ticksAtLastFrame = SDL_GetTicks(),
        .deltaTime = 0.0f,
        .maxFPS = maxFPS
    };
}

void Clock_Begin(Clock* clock)
{
    clock->ticksAtLastFrame = SDL_GetTicks();
}

void Clock_End(Clock* clock)
{
    Uint32 ticksAtThisFrame = SDL_GetTicks();
    Uint32 ticksSinceLastFrame = ticksAtThisFrame - clock->ticksAtLastFrame;
    float targetDeltaTime = 1000.0f / clock->maxFPS;

    if (ticksSinceLastFrame < targetDeltaTime)
    {
        SDL_Delay((Uint32)(targetDeltaTime - ticksSinceLastFrame));
    }

    clock->ticksAtLastFrame = SDL_GetTicks();
    clock->deltaTime = (clock->ticksAtLastFrame - ticksAtThisFrame) / 1000.0f;
}

/* Custom pixel setter/setter functions */

void SetScreenPixel(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFubyte*)pixels)[offset*4] = color.b;
    ((PFubyte*)pixels)[offset*4 + 1] = color.g;
    ((PFubyte*)pixels)[offset*4 + 2] = color.r;
    ((PFubyte*)pixels)[offset*4 + 3] = color.a;
}

PFcolor GetScreenPixel(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        ((PFubyte*)pixels)[offset*4 + 2],
        ((PFubyte*)pixels)[offset*4 + 1],
        ((PFubyte*)pixels)[offset*4],
        ((PFubyte*)pixels)[offset*4 + 3]
    };
}

/* Main */

int main(void)
{
    // Initializing SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Creating the window
    SDL_Window* window = SDL_CreateWindow("PixelForge - SDL2 example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Getting the window area
    SDL_Surface* surface = SDL_GetWindowSurface(window);
    if (surface == NULL)
    {
        fprintf(stderr, "Error getting window area: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Init clock system
    Clock clock = Clock_Create(60);

    // Creating the PixelForge context
    PFctx *ctx = pfContextCreate(surface->pixels, surface->w, surface->h, PF_PIXELFORMAT_UNKNOWN);
    pfMakeCurrent(ctx);

    // Init viewport and projection matrix
    pfViewport(0, 0, surface->w, surface->h);
    pfMatrixMode(PF_PROJECTION);
    pfLoadIdentity();
    pfOrtho(-1, 1, -1, 1, -1, 1);
    pfMatrixMode(PF_MODELVIEW);
    pfLoadIdentity();

    // Defining our own pixel getter/setter functions
    pfSetDefaultPixelGetter(GetScreenPixel);
    pfSetDefaultPixelSetter(SetScreenPixel);

    // Main loop
    SDL_Event e;
    int quit = 0;
    while (!quit)
    {
        Clock_Begin(&clock);

        // Waiting for an event
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = 1;
            }
        }

        // Clear the destination buffer
        pfClear();

        // Draw something on each iteration of the main loop
        pfBegin(PF_TRIANGLES);
        pfColor3f(1.0f, 0.0f, 0.0f); // Red
        pfVertex2f(-0.5f, -0.5f);
        pfColor3f(0.0f, 1.0f, 0.0f); // Green
        pfVertex2f(0.5f, -0.5f);
        pfColor3f(0.0f, 0.0f, 1.0f); // Blue
        pfVertex2f(0.0f, 0.5f);
        pfEnd();

        // We update the surface of the window
        SDL_UpdateWindowSurface(window);

        Clock_End(&clock);
    }

    // Freeing resources and closing SDL
    pfContextDestroy(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}