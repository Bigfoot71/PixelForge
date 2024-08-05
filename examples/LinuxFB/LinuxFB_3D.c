#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <math.h>

#define PF_COMMON_IMPL
#include "../common.h"

int main(void)
{
    int fbFd = open("/dev/fb0", O_RDWR);
    if (fbFd == -1)
    {
        perror("Error opening framebuffer device");
        exit(1);
    }

    struct fb_var_screeninfo vinfo;
    if (ioctl(fbFd, FBIOGET_VSCREENINFO, &vinfo))
    {
        perror("Error reading variable information");
        close(fbFd);
        exit(1);
    }

    size_t screensize = vinfo.yres_virtual * vinfo.xres_virtual * vinfo.bits_per_pixel / 8;
    char* fbMem = (char*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbFd, 0);
    if ((intptr_t)fbMem == -1)
    {
        perror("Error mapping framebuffer device to memory");
        close(fbFd);
        exit(1);
    }

    // Allocate second buffer
    // NOTE: to avoid tearing, this is not the best solution
    char* buffer = (char*)malloc(screensize);
    if (buffer == NULL)
    {
        perror("Error allocating back buffer");
        free(buffer);
        munmap(fbMem, screensize);
        close(fbFd);
        exit(1);
    }

    // Initialize buffer with framebuffer data
    memcpy(buffer, fbMem, screensize);

    // Creating the PixelForge rendering context
    PFcontext ctx = pfCreateContext(buffer, vinfo.xres_virtual, vinfo.yres_virtual, PF_BGR_8_8_8);
    pfMakeCurrent(ctx);

    // Define the camera position and a phase for the rotation
    PFMvec3 camPos = { -2.0f, 1.5f, -2.0f };
    float timer = 0;

    while (1)
    {
        // Update camera position
        camPos[0] = 2.0f * cosf(timer);
        camPos[2] = 2.0f * sinf(timer);
        timer += 2.0f * 0.032f;

        // Clear the destination buffer (frontBuffer)
        pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

        // Draw something on each iteration of the main loop
        PF_Begin3D(vinfo.xres_virtual, vinfo.yres_virtual, 60.0);
            PF_Update3D(camPos[0], camPos[1], camPos[2], 0, 0, 0);
            PF_DrawCube(1.0f);
        PF_End3D();

        // Copy the content of buffer into the framebuffer
        memcpy(fbMem, buffer, screensize);

        // Wait for a short while (about 16 ms)
        usleep(16000);
    }

    // Free allocated memory
    free(buffer);

    // Close framebuffer
    munmap(fbMem, screensize);
    close(fbFd);

    return 0;
}
