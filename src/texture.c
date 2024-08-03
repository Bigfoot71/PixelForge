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
#include <stddef.h>
#include <string.h>

/* Texture functions */

PFtexture pfGenTexture(void* pixels, PFsizei width, PFsizei height, PFpixelformat format)
{
    PFtexture texture = { NULL, NULL, pixels, width, height, format };
    if (format == PF_PIXELFORMAT_UNKNOWN) return texture;

    pfInternal_GetPixelGetterSetter(&texture.pixelGetter, &texture.pixelSetter, format);

    return texture;
}

PFtexture pfGenTextureBuffer(PFsizei width, PFsizei height, PFpixelformat format)
{
    PFtexture texture = { NULL, NULL, NULL, width, height, format };
    if (format == PF_PIXELFORMAT_UNKNOWN) return texture;

    pfInternal_GetPixelGetterSetter(&texture.pixelGetter, &texture.pixelSetter, format);

    PFsizei size = width*height;
    texture.pixels = PF_CALLOC(size, pfInternal_GetPixelBytes(format));

    if (!texture.pixels)
    {
        if (currentCtx->errCode)
        {
            currentCtx->errCode = PF_ERROR_OUT_OF_MEMORY;
        }
        return texture;
    }

    return texture;
}

PFtexture pfGenTextureBufferColor(PFsizei width, PFsizei height, PFcolor color, PFpixelformat format)
{
    PFtexture texture = { NULL, NULL, NULL, width, height, format };
    if (format == PF_PIXELFORMAT_UNKNOWN) return texture;

    pfInternal_GetPixelGetterSetter(&texture.pixelGetter, &texture.pixelSetter, format);

    PFsizei size = width*height;
    texture.pixels = PF_MALLOC(size*pfInternal_GetPixelBytes(format));

    if (!texture.pixels)
    {
        if (currentCtx->errCode)
        {
            currentCtx->errCode = PF_ERROR_OUT_OF_MEMORY;
        }
        return texture;
    }

#   ifdef _OPENMP
#       pragma omp parallel for \
            if(size >= PF_OPENMP_CLEAR_BUFFER_SIZE_THRESHOLD)
#   endif
    for (PFsizei i = 0; i < size; i++)
    {
        texture.pixelSetter(texture.pixels, i, color);
    }

    return texture;
}

void pfDeleteTexture(PFtexture* texture)
{
    if (texture)
    {
        if (texture->pixels)
        {
            PF_FREE(texture->pixels);
        }

        *texture = (PFtexture) { 0 };
    }
}

PFboolean pfIsValidTexture(PFtexture* texture)
{
    return texture->pixels &&
           texture->width > 0 && texture->height > 0 &&
           texture->pixelGetter && texture->pixelSetter;
}

void pfSetTexturePixel(PFtexture* texture, PFsizei x, PFsizei y, PFcolor color)
{
    texture->pixelSetter(texture->pixels, y*texture->width + x, color);
}

PFcolor pfGetTexturePixel(const PFtexture* texture, PFsizei x, PFsizei y)
{
    return texture->pixelGetter(texture->pixels, y*texture->width + x);
}

void pfSetTextureSample(PFtexture* texture, PFfloat u, PFfloat v, PFcolor color)
{
#ifdef PF_SUPPORT_NO_POT_TEXTURE
    PFsizei x = (PFsizei)((u - (PFint)u)*(texture->width - 1)) % texture->width;
    PFsizei y = (PFsizei)((v - (PFint)v)*(texture->height - 1)) % texture->height;
#else
    PFsizei x = (PFsizei)((u - (PFint)u)*(texture->width - 1)) & (texture->width - 1);
    PFsizei y = (PFsizei)((v - (PFint)v)*(texture->height - 1)) & (texture->height - 1);
#endif //PF_SUPPORT_NO_POT_TEXTURE

    texture->pixelSetter(texture->pixels, y*texture->width + x, color);
}

PFcolor pfGetTextureSample(const PFtexture* texture, PFfloat u, PFfloat v)
{
#ifdef PF_SUPPORT_NO_POT_TEXTURE
    PFsizei x = (PFsizei)((u - (PFint)u)*(texture->width - 1)) % texture->width;
    PFsizei y = (PFsizei)((v - (PFint)v)*(texture->height - 1)) % texture->height;
#else
    PFsizei x = (PFsizei)((u - (PFint)u)*(texture->width - 1)) & (texture->width - 1);
    PFsizei y = (PFsizei)((v - (PFint)v)*(texture->height - 1)) & (texture->height - 1);
#endif //PF_SUPPORT_NO_POT_TEXTURE

    return texture->pixelGetter(texture->pixels, y*texture->width + x);
}
