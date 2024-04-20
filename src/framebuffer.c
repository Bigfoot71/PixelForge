/**
 *  Copyright (c) 2024 Le Juez Victor
 *
 *  This software is provided "as-is", without any express or implied warranty. In no event 
 *  will the authors be held liable for any damages arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose, including commercial 
 *  applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not claim that you 
 *  wrote the original software. If you use this software in a product, an acknowledgment 
 *  in the product documentation would be appreciated but is not required.
 *
 *  2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *  as being the original software.
 *
 *   3. This notice may not be removed or altered from any source distribution.
 */

#include "pixelforge.h"
#include <stdlib.h>
#include <float.h>

/* Including internal function prototypes */

PFerrcode* pfInternal_GetErrorPtr(void);


/* Framebuffer functions */

PFframebuffer pfGenFramebuffer(PFuint width, PFuint height, PFpixelformat format)
{
    PFtexture texture = pfGenTextureBuffer(width, height, format);
    if (!texture.pixels) return (PFframebuffer) { 0 };

    PFsizei size = width*height;
    PFfloat *zbuffer = (PFfloat*)PF_MALLOC(size*sizeof(PFfloat));

    if (!zbuffer)
    {
        *pfInternal_GetErrorPtr() = PF_ERROR_OUT_OF_MEMORY;
        pfDeleteTexture(&texture);

        return (PFframebuffer) { 0 };
    }

    for (PFsizei i = 0; i < size; i++)
    {
        zbuffer[i] = FLT_MAX;
    }

    return (PFframebuffer) { texture, zbuffer };
}

void pfDeleteFramebuffer(PFframebuffer* framebuffer)
{
    if (framebuffer)
    {
        pfDeleteTexture(&framebuffer->texture);

        if (framebuffer->zbuffer)
        {
            PF_FREE(framebuffer->zbuffer);
            framebuffer->zbuffer = NULL;
        }
    }
}

PFboolean pfIsValidFramebuffer(PFframebuffer* framebuffer)
{
    return framebuffer->zbuffer &&
        pfIsValidTexture(&framebuffer->texture);
}

void pfClearFramebuffer(PFframebuffer* framebuffer, PFcolor color)
{
    PFsizei size = framebuffer->texture.width*framebuffer->texture.height;

    for (PFsizei i = 0; i < size; i++)
    {
        framebuffer->texture.pixelSetter(framebuffer->texture.pixels, i, color);
        framebuffer->zbuffer[i] = FLT_MAX;
    }
}

void pfSetFramebufferPixelDepth(PFframebuffer* framebuffer, PFuint x, PFuint y, PFfloat z, PFcolor color)
{
    PFsizei offset = y*framebuffer->texture.width + x;
    PFfloat *zp = framebuffer->zbuffer + offset;

    if (z < *zp)
    {
        framebuffer->texture.pixelSetter(framebuffer->texture.pixels, offset, color);
        *zp = z;
    }
}

PFfloat pfGetFramebufferDepth(const PFframebuffer* framebuffer, PFuint x, PFuint y)
{
    return framebuffer->zbuffer[y*framebuffer->texture.width + x];
}

void pfSetFramebufferPixel(PFframebuffer* framebuffer, PFuint x, PFuint y, PFcolor color)
{
    framebuffer->texture.pixelSetter(framebuffer->texture.pixels, y*framebuffer->texture.width + x, color);
}

PFcolor pfGetFramebufferPixel(const PFframebuffer* framebuffer, PFuint x, PFuint y)
{
    return framebuffer->texture.pixelGetter(framebuffer->texture.pixels, y*framebuffer->texture.width + x);;
}
