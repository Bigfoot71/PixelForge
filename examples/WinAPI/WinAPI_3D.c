#define PF_WIN_COMMON_IMPL
#include "WinAPI_common.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Create Window
    Window win = Window_Create("PixelForge - Basic 3D", SCREEN_WIDTH, SCREEN_HEIGHT, hInstance, nCmdShow);

    // Create a timer to update the window periodically
    SetTimer(win.hwnd, 1, 16, NULL);

    // Creating the PixelForge context
    PFcontext ctx = PF_InitFromWindow(&win);

    // Define the camera position and a phase for the rotation
    PFMvec3 camPos = { -2.0f, 1.5f, -2.0f };
    float timer = 0;

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
        
        // Update camera position
        camPos[0] = 2.0f * cosf(timer);
        camPos[2] = 2.0f * sinf(timer);
        timer += 2 * 0.016f;

        // Clear the destination buffer
        pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

        // Draw something on each iteration of the main loop
        PF_Begin3D(win.w, win.h, 60.0);
            PF_Update3D(camPos[0], camPos[1], camPos[2], 0, 0, 0);
            PF_DrawCube(1.0f);
        PF_End3D();

        // We update the surface of the window
        Window_Update(&win);
    }

    pfDeleteContext(ctx);
    Window_Destroy(&win);

    return msg.wParam;
}