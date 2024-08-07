#include "pixelforge.h"
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
    PFcontext ctx = pfCreateContext(buffer, vinfo.xres_virtual, vinfo.yres_virtual, PF_BGR, PF_UNSIGNED_BYTE);
    pfMakeCurrent(ctx);

    while (1)
    {
        // Clear the destination buffer (frontBuffer)
        pfClear(PF_COLOR_BUFFER_BIT | PF_DEPTH_BUFFER_BIT);

        // Draw something on each iteration of the main loop
        pfBegin(PF_TRIANGLES);
            pfColor3f(1.0f, 0.0f, 0.0f); // Red
            pfVertex2f(-0.5f, -0.5f);
            pfColor3f(0.0f, 1.0f, 0.0f); // Green
            pfVertex2f(0.5f, -0.5f);
            pfColor3f(0.0f, 0.0f, 1.0f); // Blue
            pfVertex2f(0.0f, 0.5f);
        pfEnd();

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
