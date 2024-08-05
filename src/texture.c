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

/* Texture functions */

PFtexture pfGenTexture(void* pixels, PFsizei width, PFsizei height, PFpixelformat format, PFdatatype type)
{
    struct PFtex *texture = NULL;
    PFpixelgetter getter = NULL;
    PFpixelsetter setter = NULL;

    pfInternal_GetPixelGetterSetter(&getter, &setter, format, type);

    if (!getter || !setter)
    {
        if (currentCtx)
        {
            currentCtx->errCode = PF_INVALID_ENUM;
        }

        return texture;
    }

    texture = PF_MALLOC(sizeof(struct PFtex));

    if (texture == NULL)
    {
        if (currentCtx)
        {
            currentCtx->errCode = PF_ERROR_OUT_OF_MEMORY;
        }

        return texture;
    }

    texture->pixels = pixels;
    texture->format = format;
    texture->type = type;

    texture->width = width;
    texture->height = height;

    texture->pixelGetter = getter;
    texture->pixelSetter = setter;

    return texture;
}

void pfDeleteTexture(PFtexture* texture, PFboolean freeBuffer)
{
    struct PFtex* tex = *texture;
    if (tex)
    {
        if (freeBuffer && tex->pixels)
        {
            PF_FREE(tex->pixels);
        }
        PF_FREE(tex);
        *texture = NULL;
    }
}

PFboolean pfIsValidTexture(const PFtexture texture)
{
    const struct PFtex* tex = texture;
    PFboolean result = PF_FALSE;
    if (tex)
    {
        result = tex->pixels &&
                 tex->width > 0 && tex->height > 0 &&
                 tex->pixelGetter && tex->pixelSetter;
    }
    return result;
}

void* pfGetTexturePixels(const PFtexture texture, PFsizei* width, PFsizei* height, PFpixelformat* format, PFdatatype* type)
{
    const struct PFtex* tex = texture;
    void* pixels = NULL;
    if (tex)
    {
        if (width) *width = tex->width;
        if (height) *height = tex->height;
        if (format) *format = tex->format;
        if (type) *type = tex->type;
        pixels = tex->pixels;
    }
    return pixels;
}

void pfSetTexturePixel(PFtexture texture, PFsizei x, PFsizei y, PFcolor color)
{
    struct PFtex* tex = texture;
    tex->pixelSetter(tex->pixels, y*tex->width + x, color);
}

PFcolor pfGetTexturePixel(const PFtexture texture, PFsizei x, PFsizei y)
{
    struct PFtex* tex = texture;
    return tex->pixelGetter(tex->pixels, y*tex->width + x);
}

void pfSetTextureSample(PFtexture texture, PFfloat u, PFfloat v, PFcolor color)
{
    struct PFtex* tex = texture;

#ifdef PF_SUPPORT_NO_POT_TEXTURE
    PFsizei x = (PFsizei)((u - (PFint)u)*(tex->width - 1)) % tex->width;
    PFsizei y = (PFsizei)((v - (PFint)v)*(tex->height - 1)) % tex->height;
#else
    PFsizei x = (PFsizei)((u - (PFint)u)*(tex->width - 1)) & (tex->width - 1);
    PFsizei y = (PFsizei)((v - (PFint)v)*(tex->height - 1)) & (tex->height - 1);
#endif //PF_SUPPORT_NO_POT_TEXTURE

    tex->pixelSetter(tex->pixels, y*tex->width + x, color);
}

PFcolor pfGetTextureSample(const PFtexture texture, PFfloat u, PFfloat v)
{
    struct PFtex* tex = texture;

#ifdef PF_SUPPORT_NO_POT_TEXTURE
    PFsizei x = (PFsizei)((u - (PFint)u)*(tex->width - 1)) % tex->width;
    PFsizei y = (PFsizei)((v - (PFint)v)*(tex->height - 1)) % tex->height;
#else
    PFsizei x = (PFsizei)((u - (PFint)u)*(tex->width - 1)) & (tex->width - 1);
    PFsizei y = (PFsizei)((v - (PFint)v)*(tex->height - 1)) & (tex->height - 1);
#endif //PF_SUPPORT_NO_POT_TEXTURE

    return tex->pixelGetter(tex->pixels, y*tex->width + x);
}
