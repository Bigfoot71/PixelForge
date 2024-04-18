#include "pixelforge.h"
#include <X11/Xlib.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>

#define PF_COMMON_IMPL
#include "../common.h"

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

/* Helper functions */

unsigned long micros()
{
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return currentTime.tv_sec * 1000000 + currentTime.tv_usec;
}

/* X11 management */

typedef struct {
    Display *dpy;
    Window root;
    Window win;
    GC gc;
    XEvent e;
    Atom wmDeleteMessage;
    int screen;
} X11_App;

X11_App X11_InitApp(int width, int height)
{
    X11_App app;

    // Open a connection with the X server
    app.dpy = XOpenDisplay(NULL);
    if (!app.dpy) exit(-1);

    // Get screen number and window root
    app.screen = DefaultScreen(app.dpy);
    app.root = RootWindow(app.dpy, app.screen);

    // Create a simple window
    app.win = XCreateSimpleWindow(app.dpy, app.root, 0, 0, width, height,
        0, BlackPixel(app.dpy, app.screen), WhitePixel(app.dpy, app.screen));

    // Select events to monitor
    XSelectInput(app.dpy, app.win, ExposureMask | KeyPressMask | StructureNotifyMask);

    // Create a graphic context
    app.gc = XCreateGC(app.dpy, app.win, 0, NULL);

    // Show window
    XMapWindow(app.dpy, app.win);

    // Get Window Removal Message
    app.wmDeleteMessage = XInternAtom(app.dpy, "WM_DELETE_WINDOW", False);

    // Set Window Suppression Protocol
    XSetWMProtocols(app.dpy, app.win, &app.wmDeleteMessage, 1);

    return app;
}

void X11_CloseApp(X11_App* app)
{
    XDestroyWindow(app->dpy, app->win);
    XCloseDisplay(app->dpy);
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

int main()
{
    // Init X11 application
    X11_App app = X11_InitApp(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Init buffer and XImage destination for PixelForge
    PFuint *pixels = (PFuint*)malloc(SCREEN_WIDTH*SCREEN_HEIGHT*sizeof(PFuint));
    XImage *image = XCreateImage(app.dpy, DefaultVisual(app.dpy, app.screen), DefaultDepth(app.dpy, app.screen),
                                    ZPixmap, 0, (char*)pixels, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0);

    // Allocate destination buffer and init PixelForge context
    PFctx *ctx = PF_Init(pixels, SCREEN_WIDTH, SCREEN_HEIGHT);
    PF_Reshape(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Defining our own pixel getter/setter functions
    pfSetDefaultPixelGetter(GetScreenPixel);
    pfSetDefaultPixelSetter(SetScreenPixel);

    // Set a timer for continuous rendering
    const int frameDelay = 1000000 / 60; // 60 FPS
    unsigned long frameStart;
    int frameTime;

    // Set camera position and phase for rotation
    PFvec3f camPos = { -2.0f, 1.5f, -2.0f };
    float timer = 0;

    while (1)
    {
        // Start of frame timer
        frameStart = micros();

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
        XPutImage(app.dpy, app.win, app.gc, image, 0, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        // Calculate elapsed time during frame
        // And wait until the end of the frameDelay
        frameTime = micros() - frameStart;
        if (frameDelay > frameTime)
        {
            usleep(frameDelay - frameTime);
        }
    }

    // Destroy PixelForge context
    pfDeleteContext(ctx);

    // Free image and pixel buffer
    XFree(image);
    free(pixels);

    // Close connection
    X11_CloseApp(&app);

    return 0;
}