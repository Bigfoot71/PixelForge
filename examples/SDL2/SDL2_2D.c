#define PF_SDL2_COMMON_IMPL
#include "SDL2_common.h"

int main(void)
{
    // Create Window
    Window window = Window_Create("PixelForge - Basic 2D",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        800, 600, SDL_WINDOW_SHOWN);

    // Init clock system
    Clock clock = Clock_Create(10);

    // Creating the PixelForge context
    PFctx *ctx = PF_InitFromWindow(&window);

    // Draw a triangle once with color vertex interpolation
    pfBegin(PF_TRIANGLES);
        pfColor3f(1.0f, 0.0f, 0.0f); // Red
        pfVertex2f(-0.5f, -0.5f);
        pfColor3f(0.0f, 1.0f, 0.0f); // Green
        pfVertex2f(0.5f, -0.5f);
        pfColor3f(0.0f, 0.0f, 1.0f); // Blue
        pfVertex2f(0.0f, 0.5f);
    pfEnd();

    // We update the surface of the window
    Window_Update(&window);

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

        Clock_End(&clock);
    }

    // Freeing resources and closing SDL
    pfDeleteContext(ctx);
    Window_Destroy(&window);

    return 0;
}
