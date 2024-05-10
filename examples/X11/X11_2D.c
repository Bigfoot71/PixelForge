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
    Clock clock = Clock_Create(10);

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

        // Draw something on each iteration of the main loop
        pfBegin(PF_TRIANGLES);
            pfColor3f(1.0f, 0.0f, 0.0f); // Red
            pfVertex2f(-0.5f, -0.5f);
            pfColor3f(0.0f, 1.0f, 0.0f); // Green
            pfVertex2f(0.5f, -0.5f);
            pfColor3f(0.0f, 0.0f, 1.0f); // Blue
            pfVertex2f(0.0f, 0.5f);
        pfEnd();

        // We update the content of the window
        X11_UpdateWindow(&app);

        Clock_End(&clock);
    }

    // Freeing resources and closing X11_App
    pfDeleteContext(ctx);
    X11_CloseApp(&app);

    return 0;
}