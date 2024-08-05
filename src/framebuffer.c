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

#include "internal/context/context.h"
#include "internal/pixel.h"
#include "pixelforge.h"
#include <stdlib.h>
#include <float.h>

/* Framebuffer functions */

PFframebuffer pfGenFramebuffer(PFsizei width, PFsizei height, PFpixelformat format, PFdatatype type)
{
    PFframebuffer framebuffer = (PFframebuffer) { 0 };
    PFsizei size = width*height;

    void* pixels = PF_CALLOC(size, pfInternal_GetPixelBytes(format, type));
    if (!pixels) return framebuffer;

    PFtexture texture = pfGenTexture(pixels, width, height, format, type);

    if (texture == NULL)
    {
        if (currentCtx)
        {
            currentCtx->errCode = PF_INVALID_ENUM;
        }
        return framebuffer;
    }

    PFfloat *zbuffer = (PFfloat*)PF_MALLOC(size*sizeof(PFfloat));

    if (!zbuffer)
    {
        if (currentCtx)
        {
            currentCtx->errCode = PF_ERROR_OUT_OF_MEMORY;
        }
        pfDeleteTexture(&texture, true);
        return framebuffer;
    }

    for (PFsizei i = 0; i < size; i++)
    {
        zbuffer[i] = FLT_MAX;
    }

    framebuffer.texture = texture;
    framebuffer.zbuffer = zbuffer;

    return framebuffer;
}

void pfDeleteFramebuffer(PFframebuffer* framebuffer)
{
    if (framebuffer)
    {
        pfDeleteTexture(&framebuffer->texture, true);

        if (framebuffer->zbuffer)
        {
            PF_FREE(framebuffer->zbuffer);
            framebuffer->zbuffer = NULL;
        }

        *framebuffer = (PFframebuffer) { 0 };
    }
}

PFboolean pfIsValidFramebuffer(PFframebuffer* framebuffer)
{
    struct PFtex* tex = framebuffer->texture;
    PFboolean result = PF_FALSE;
    if (tex)
    {
        result = framebuffer->zbuffer && tex->width > 0 && tex->height > 0
            && pfIsValidTexture(framebuffer->texture);
    }
    return result;
}

void pfClearFramebuffer(PFframebuffer* framebuffer, PFcolor color, PFfloat depth)
{
    struct PFtex* tex = framebuffer->texture;
    PFsizei size = tex->width*tex->height;

#   ifdef _OPENMP
#       pragma omp parallel for \
            if(size >= PF_OPENMP_CLEAR_BUFFER_SIZE_THRESHOLD)
#   endif
    for (PFsizei i = 0; i < size; i++)
    {
        tex->pixelSetter(tex->pixels, i, color);
        framebuffer->zbuffer[i] = depth;
    }
}

PFcolor pfGetFramebufferPixel(const PFframebuffer* framebuffer, PFsizei x, PFsizei y)
{
    const struct PFtex* tex = framebuffer->texture;
    return tex->pixelGetter(tex->pixels, y*tex->width + x);
}

PFfloat pfGetFramebufferDepth(const PFframebuffer* framebuffer, PFsizei x, PFsizei y)
{
    const struct PFtex* tex = framebuffer->texture;
    return framebuffer->zbuffer[y*tex->width + x];
}

void pfSetFramebufferPixelDepthTest(PFframebuffer* framebuffer, PFsizei x, PFsizei y, PFfloat z, PFcolor color, PFdepthfunc depthFunc)
{
    struct PFtex* tex = framebuffer->texture;
    PFsizei offset = y*tex->width + x;

    PFfloat *zp = framebuffer->zbuffer + offset;

    if (depthFunc(z, *zp))
    {
        tex->pixelSetter(tex->pixels, offset, color);
        *zp = z;
    }
}

void pfSetFramebufferPixelDepth(PFframebuffer* framebuffer, PFsizei x, PFsizei y, PFfloat z, PFcolor color)
{
    struct PFtex* tex = framebuffer->texture;
    PFsizei offset = y*tex->width + x;

    tex->pixelSetter(tex->pixels, offset, color);
    framebuffer->zbuffer[offset] = z;
}

void pfSetFramebufferPixel(PFframebuffer* framebuffer, PFsizei x, PFsizei y, PFcolor color)
{
    struct PFtex* tex = framebuffer->texture;
    tex->pixelSetter(tex->pixels, y*tex->width + x, color);
}
