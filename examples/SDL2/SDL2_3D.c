#define PF_SDL2_COMMON_IMPL
#include "SDL2_common.h"

int main(void)
{
    // Create Window
    Window window = Window_Create("PixelForge - Basic 3D",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        800, 600, SDL_WINDOW_SHOWN);

    // Init clock system
    Clock clock = Clock_Create(60);

    // Creating the PixelForge context
    PFctx *ctx = PF_InitFromWindow(&window);

    // Define the camera position and a phase for the rotation
    PFMvec3 camPos = { -2.0f, 1.5f, -2.0f };
    float timer = 0;

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

        // Update camera position
        camPos[0] = 2.0f * cosf(timer);
        camPos[2] = 2.0f * sinf(timer);
        timer += 2.0f * clock.deltaTime;

        // Clear the destination buffer
        pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

        // Draw something on each iteration of the main loop
        PF_Begin3D(window.surface->w, window.surface->h, 60.0);
            PF_Update3D(camPos[0], camPos[1], camPos[2], 0, 0, 0);
            PF_DrawCube(1.0f);
        PF_End3D();

        // We update the surface of the window
        Window_Update(&window);

        Clock_End(&clock);
    }

    // Freeing resources and closing SDL
    pfDeleteContext(ctx);
    Window_Destroy(&window);

    return 0;
}