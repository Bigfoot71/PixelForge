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

static void PixelSetter(void *framebuffer, PFsizei index, PFcolor color)
{
    ((uint32_t*)framebuffer)[index] = (color.r << 16) | (color.g << 8) | color.b;
}

static PFcolor PixelGetter(const void *framebuffer, PFsizei index)
{
    uint32_t pixel = ((const uint32_t *)framebuffer)[index];

    return (PFcolor) {
        .r = (pixel >> 16) & 0xFF,
        .g = (pixel >> 8) & 0xFF,
        .b = pixel & 0xFF,
        .a = 0xFF
    };
}

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

    // Allocate two frame buffers
    char* frontBuffer = (char*)malloc(screensize);
    if (frontBuffer == NULL)
    {
        perror("Error allocating front buffer");
        munmap(fbMem, screensize);
        close(fbFd);
        exit(1);
    }

    char* backBuffer = (char*)malloc(screensize);
    if (backBuffer == NULL)
    {
        perror("Error allocating back buffer");
        free(frontBuffer);
        munmap(fbMem, screensize);
        close(fbFd);
        exit(1);
    }

    // Initialize frame buffers with framebuffer data
    memcpy(frontBuffer, fbMem, screensize);
    memcpy(backBuffer, fbMem, screensize);

    // Creating the PixelForge rendering context
    PFctx *ctx = pfCreateContext(backBuffer, vinfo.xres_virtual, vinfo.yres_virtual, PF_PIXELFORMAT_R8G8B8);
    pfMakeCurrent(ctx);

    // Definition of custom pixel get/set function
    pfSetDefaultPixelGetter(PixelGetter);
    pfSetDefaultPixelSetter(PixelSetter);

    // Set the backBuffer as auxiliary buffer
    pfSetAuxBuffer(frontBuffer);

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

        // Swap buffers (frontBuffer becomes backBuffer and vice versa)
        pfSwapBuffers();

        // Copy the content of backBuffer into the framebuffer
        memcpy(fbMem, backBuffer, screensize);

        // Wait for a short while (about 16 ms)
        usleep(16000);
    }

    // Free allocated memory
    free(frontBuffer);
    free(backBuffer);

    // Close framebuffer
    munmap(fbMem, screensize);
    close(fbFd);

    return 0;
}
