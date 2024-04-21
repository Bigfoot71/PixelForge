#define PF_WIN_COMMON_IMPL
#include "WinAPI_common.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Create Window
    Window win = Window_Create("PixelForge - Basic 2D", SCREEN_WIDTH, SCREEN_HEIGHT, hInstance, nCmdShow);

    // Creating the PixelForge context
    PFctx *ctx = PF_InitFromWindow(&win);

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
    Window_Update(&win);

    // Main loop
    MSG msg = {0};
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        // Handle the error and possibly exit
        if (bRet == -1)
        {
            continue;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
        
        // Check if the message is WM_QUIT (indicating window close)
        if (msg.message == WM_QUIT)
        {
            break;
        }
    }

    pfDeleteContext(ctx);
    Window_Destroy(&win);

    return msg.wParam;
}