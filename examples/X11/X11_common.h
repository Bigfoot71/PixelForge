#ifndef PF_X11_COMMON_H
#define PF_X11_COMMON_H

#ifdef PF_X11_COMMON_H
#   define PF_COMMON_IMPL
#endif //PF_X11_COMMON_H

#include "../common.h"
#include <X11/Xlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

/* Pixel getter/setter */

PFcolor PF_GetPixel(const void* pixels, PFsizei offset);
void PF_SetPixel(void* pixels, PFsizei offset, PFcolor color);

/* X11 App management */

typedef struct {
    Display *dpy;
    Window root;
    Window win;
    GC gc;
    XEvent e;
    Atom wmDeleteMessage;
    int screen;
    PFuint *destBuffer;
    XImage *destImage;
} X11_App;

X11_App X11_InitApp(int width, int height);
void X11_CloseApp(X11_App* app);

void X11_UpdateWindow(X11_App* app);

/* Clock management */

typedef struct {
    struct timeval tvLastFrame;
    float deltaTime;
    unsigned int maxFPS;
} Clock;

Clock Clock_Create(unsigned int maxFPS);
void Clock_Begin(Clock* clock);
void Clock_End(Clock* clock);

/* PixelForge context management */

PFctx* PF_InitFromX11App(X11_App* app);


/* Implementation */

#ifdef PF_X11_COMMON_IMPL

/* Pixel getter/setter */

PFcolor PF_GetPixel(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        ((PFubyte*)pixels)[offset*4 + 2],
        ((PFubyte*)pixels)[offset*4 + 1],
        ((PFubyte*)pixels)[offset*4],
        ((PFubyte*)pixels)[offset*4 + 3]
    };
}

void PF_SetPixel(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFubyte*)pixels)[offset*4] = color.b;
    ((PFubyte*)pixels)[offset*4 + 1] = color.g;
    ((PFubyte*)pixels)[offset*4 + 2] = color.r;
    ((PFubyte*)pixels)[offset*4 + 3] = color.a;
}

/* X11 App management */

X11_App X11_InitApp(int width, int height)
{
    X11_App app;

    // Open a connection with the X server
    app.dpy = XOpenDisplay(NULL);
    if (!app.dpy) exit(1);

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

    // Create destination buffer
    app.destBuffer = (PFuint*)malloc(width*height*sizeof(PFuint));
    app.destImage = XCreateImage(app.dpy, DefaultVisual(app.dpy, app.screen), DefaultDepth(app.dpy, app.screen),
                                    ZPixmap, 0, (char*)app.destBuffer, width, height, 32, 0);

    return app;
}

void X11_CloseApp(X11_App* app)
{
    XFree(app->destImage);
    free(app->destBuffer);

    XDestroyWindow(app->dpy, app->win);
    XCloseDisplay(app->dpy);
}

void X11_UpdateWindow(X11_App* app)
{
    XPutImage(app->dpy, app->win, app->gc, app->destImage,
        0, 0, 0, 0, app->destImage->width, app->destImage->height);
}

/* Clock management */

Clock Clock_Create(unsigned int maxFPS)
{
    Clock clock;
    gettimeofday(&clock.tvLastFrame, NULL);
    clock.deltaTime = 0.0f;
    clock.maxFPS = maxFPS;
    return clock;
}

void Clock_Begin(Clock* clock)
{
    gettimeofday(&clock->tvLastFrame, NULL);
}

void Clock_End(Clock* clock)
{
    struct timeval tvThisFrame;
    gettimeofday(&tvThisFrame, NULL);
    unsigned int ticksAtThisFrame = (tvThisFrame.tv_sec * 1000) + (tvThisFrame.tv_usec / 1000);
    unsigned int ticksSinceLastFrame = (ticksAtThisFrame - (clock->tvLastFrame.tv_sec * 1000) - (clock->tvLastFrame.tv_usec / 1000));
    float targetDeltaTime = 1000.0f / clock->maxFPS;

    if (ticksSinceLastFrame < targetDeltaTime)
    {
        unsigned int delayMilliseconds = (unsigned int)(targetDeltaTime - ticksSinceLastFrame);
        usleep(delayMilliseconds * 1000);
    }

    gettimeofday(&clock->tvLastFrame, NULL);
    unsigned int ticksAtLastFrame = (clock->tvLastFrame.tv_sec * 1000) + (clock->tvLastFrame.tv_usec / 1000);
    clock->deltaTime = (float)(ticksAtLastFrame - ticksAtThisFrame) / 1000.0f;
}

/* PixelForge context management */

PFctx* PF_InitFromX11App(X11_App* app)
{
    PFctx *ctx = PF_Init(app->destBuffer, app->destImage->width, app->destImage->height);
    pfSetDefaultPixelGetter(PF_GetPixel);
    pfSetDefaultPixelSetter(PF_SetPixel);
    return ctx;
}

#endif //PF_X11_COMMON_IMPL
#endif //PF_X11_COMMON_H