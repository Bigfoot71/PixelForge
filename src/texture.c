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
#include "internal/sampler.h"
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

    PFpixelgetter_simd getterSimd = NULL;
    PFpixelsetter_simd setterSimd = NULL;

    pfInternal_GetPixelGetterSetter(&getter, &setter, format, type);
    pfInternal_GetPixelGetterSetter_simd(&getterSimd, &setterSimd, format, type);

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

    texture->w = width;
    texture->h = height;

    texture->tx = 1.0f/width;
    texture->ty = 1.0f/height;

    texture->getter = getter;
    texture->setter = setter;

    texture->getterSimd = getterSimd;
    texture->setterSimd = setterSimd;

    texture->sampler = pfInternal_Texture2DSampler_NEAREST_REPEAT;
    texture->samplerSimd = pfInternal_SimdTexture2DSampler_NEAREST_REPEAT;

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
                 tex->w > 0 && tex->h > 0 &&
                 tex->getter && tex->setter;
    }
    return result;
}

void pfTextureParameter(PFtexture texture, PFtexturewrap wrapMode, PFtexturefilter filterMode)
{
    struct PFtex* tex = texture;

    if (!pfInternal_IsTextureParameterValid(wrapMode, filterMode))
    {
        if (currentCtx) currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    pfInternal_GetTexture2DSampler(
        &tex->sampler, &tex->samplerSimd,
        wrapMode, filterMode);
}

void* pfGetTexturePixels(const PFtexture texture, PFsizei* width, PFsizei* height, PFpixelformat* format, PFdatatype* type)
{
    const struct PFtex* tex = texture;
    void* pixels = NULL;
    if (tex)
    {
        if (width) *width = tex->w;
        if (height) *height = tex->h;
        if (format) *format = tex->format;
        if (type) *type = tex->type;
        pixels = tex->pixels;
    }
    return pixels;
}
