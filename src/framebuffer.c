#include "pixelforge.h"
#include <stdlib.h>
#include <float.h>

PFframebuffer pfFramebufferGenBuffer(PFuint width, PFuint height, PFpixelformat format)
{
    PFtexture texture = pfTextureGenBuffer(width, height, format);

    PFsizei size = width*height;
    PFfloat *zbuffer = (PFfloat*)PF_MALLOC(size*sizeof(PFfloat));
    for (PFsizei i = 0; i < size; i++) zbuffer[i] = FLT_MAX;

    return (PFframebuffer) { texture, zbuffer };
}

void pfFramebufferDestroy(PFframebuffer* framebuffer)
{
    if (framebuffer)
    {
        pfTextureDestroy(&framebuffer->texture);

        if (framebuffer->zbuffer)
        {
            PF_FREE(framebuffer->zbuffer);
            framebuffer->zbuffer = NULL;
        }
    }
}

void pfFramebufferClear(PFframebuffer* framebuffer, PFcolor color)
{
    PFsizei size = framebuffer->texture.width*framebuffer->texture.height;

    for (PFsizei i = 0; i < size; i++)
    {
        framebuffer->texture.pixelSetter(framebuffer->texture.pixels, i, color);
        framebuffer->zbuffer[i] = FLT_MAX;
    }
}

void pfFramebufferSetPixelDepth(PFframebuffer* framebuffer, PFuint x, PFuint y, PFfloat z, PFcolor color)
{
    PFsizei offset = y*framebuffer->texture.width + x;
    PFfloat *zp = framebuffer->zbuffer + offset;

    if (z < *zp)
    {
        framebuffer->texture.pixelSetter(framebuffer->texture.pixels, offset, color);
        *zp = z;
    }
}

PFfloat pfFramebufferGetDepth(const PFframebuffer* framebuffer, PFuint x, PFuint y)
{
    return framebuffer->zbuffer[y*framebuffer->texture.width + x];
}

void pfFramebufferSetPixel(PFframebuffer* framebuffer, PFuint x, PFuint y, PFcolor color)
{
    framebuffer->texture.pixelSetter(framebuffer->texture.pixels, y*framebuffer->texture.width + x, color);
}

PFcolor pfFrambufferGetPixel(const PFframebuffer* framebuffer, PFuint x, PFuint y)
{
    return framebuffer->texture.pixelGetter(framebuffer->texture.pixels, y*framebuffer->texture.width + x);;
}
