#define PF_X11_COMMON_IMPL
#include "X11_common.h"

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

int main(void)
{
    // Init X11 application
    X11_App app = X11_InitApp(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Allocate destination buffer and init PixelForge context
    PFcontext ctx = PF_InitFromX11App(&app);

    // Init clock system
    Clock clock = Clock_Create(60);

    // Set camera position and phase for rotation
    PFMvec3 camPos = { -2.0f, 1.5f, -2.0f };
    float timer = 0;

    // Main loop
    while (1)
    {
        Clock_Begin(&clock);

        // Check X11 window events
        int windowClosed = 0;
        while (XPending(app.dpy))
        {
            XNextEvent(app.dpy, &app.e);
            if (app.e.type == ClientMessage && app.e.xclient.data.l[0] == (int)app.wmDeleteMessage)
            {
                windowClosed = 1;
                break;
            }
        }

        // If the window is closed we break the main loop
        if (windowClosed) break;

        // Update camera position
        camPos[0] = 2.0f * cosf(timer);
        camPos[2] = 2.0f * sinf(timer);
        timer += 0.01f;

        // Clear the destination buffer
        pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

        // Draw something on each iteration of the main loop
        PF_Begin3D(SCREEN_WIDTH, SCREEN_HEIGHT, 60.0);
            PF_Update3D(camPos[0], camPos[1], camPos[2], 0, 0, 0);
            PF_DrawCube(1);
        PF_End3D();

        // Draw the contents of the pixel buffer on the window
        X11_UpdateWindow(&app);

        Clock_End(&clock);
    }

    // Freeing resources and closing X11_App
    pfDeleteContext(ctx);
    X11_CloseApp(&app);

    return 0;
}