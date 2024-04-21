#ifndef PF_WIN_COMMON_H
#define PF_WIN_COMMON_H

#ifdef PF_WIN_COMMON_IMPL
#   define PF_COMMON_IMPL
#endif //PF_WIN_COMMON_IMPL

#include "../common.h"
#include <windows.h>

/* Pixel getter/setter */

PFcolor PF_GetPixel(const void* pixels, PFsizei offset);
void PF_SetPixel(void* pixels, PFsizei offset, PFcolor color);

/* Window management */

typedef struct {
    WNDCLASS wc;
    HWND hwnd;
    HDC hdc;
    BYTE *pixels;
    BITMAPINFO bmi;
    int w, h;
} Window;

Window Window_Create(const char* title, int w, int h, HINSTANCE hInstance, int nCmdShow);
void Window_Destroy(Window* window);
void Window_Update(Window* window);

/* PixelForge context management */

PFctx* PF_InitFromWindow(Window* window);


/* Implementation */

#ifdef PF_WIN_COMMON_IMPL

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

/* Window management */

Window Window_Create(const char* title, int w, int h, HINSTANCE hInstance, int nCmdShow)
{
    Window win = { 0 };

    const char* className = "WindowClass";

    // Register window class
    win.wc.lpfnWndProc = DefWindowProc;
    win.wc.hInstance = hInstance;
    win.wc.lpszClassName = className;
    RegisterClass(&win.wc);

    // Create window
    win.hwnd = CreateWindow(className, title, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                              CW_USEDEFAULT, CW_USEDEFAULT, w, h,
                              NULL, NULL, hInstance, NULL);
    
    // Show window
    ShowWindow(win.hwnd, nCmdShow);
    
    // Get device context for drawing
    win.hdc = GetDC(win.hwnd);

    // Allocate memory for the pixel buffer
    win.pixels = (BYTE*)malloc(w*h*sizeof(BYTE)*4);
    memset(win.pixels, 0, w*h*sizeof(BYTE)*4);
    
    // Set up BITMAPINFO structure
    win.bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    win.bmi.bmiHeader.biWidth = w;
    win.bmi.bmiHeader.biHeight = -h; // Negative height to specify top-down bitmap
    win.bmi.bmiHeader.biPlanes = 1;
    win.bmi.bmiHeader.biBitCount = 32; // 32 bits per pixel (ARGB)
    win.bmi.bmiHeader.biCompression = BI_RGB;
    
    // Set window dimensions
    win.w = w, win.h = h;
    
    return win;
}

void Window_Destroy(Window* window)
{
    ReleaseDC(window->hwnd, window->hdc);
    free(window->pixels);
}

void Window_Update(Window* window)
{
    SetDIBitsToDevice(window->hdc,
        0, 0, window->w, window->h,
        0, 0, 0, window->h, window->pixels,
        &window->bmi, DIB_RGB_COLORS);
}

/* PixelForge context management */

PFctx* PF_InitFromWindow(Window* window)
{
    PFctx *ctx = PF_Init(window->pixels, window->w, window->h);
    pfSetDefaultPixelGetter(PF_GetPixel);
    pfSetDefaultPixelSetter(PF_SetPixel);
    return ctx;
}

#endif //PF_WIN_COMMON_IMPL
#endif //PF_WIN_COMMON_H