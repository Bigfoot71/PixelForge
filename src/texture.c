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
    if (!pfiIsPixelFormatValid(format, type)) {
        if (G_currentCtx) {
            G_currentCtx->errCode = PF_INVALID_ENUM;
        }
        return NULL;
    }

    struct PFItex * texture = PF_MALLOC(sizeof(struct PFItex));

    if (texture == NULL) {
        if (G_currentCtx) {
            G_currentCtx->errCode = PF_ERROR_OUT_OF_MEMORY;
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

    texture->getter = GC_pixelGetters[format][type];
    texture->setter = GC_pixelSetters[format][type];
    texture->sampler = pfiTexture2DSampler_NEAREST_REPEAT;

#if PF_SIMD_SUPPORT
    texture->getterSimd = GC_pixelGetters_simd[format][type];
    texture->setterSimd = GC_pixelSetters_simd[format][type];
    texture->samplerSimd = pfiTexture2DSampler_NEAREST_REPEAT_simd;
#endif //PF_SIMD_SUPPORT

    return texture;
}

void pfDeleteTexture(PFtexture* texture, PFboolean freeBuffer)
{
    struct PFItex* tex = *texture;
    if (tex) {
        if (freeBuffer && tex->pixels) {
            PF_FREE(tex->pixels);
        }
        PF_FREE(tex);
        *texture = NULL;
    }
}

PFboolean pfIsValidTexture(const PFtexture texture)
{
    const struct PFItex* tex = texture;
    PFboolean result = PF_FALSE;
    if (tex) {
        result = tex->pixels &&
                 tex->w > 0 && tex->h > 0 &&
                 tex->getter && tex->setter;
    }
    return result;
}

void pfTextureParameter(PFtexture texture, PFtexturewrap wrapMode, PFtexturefilter filterMode)
{
    struct PFItex* tex = texture;

    if (!pfiIsTextureParameterValid(wrapMode, filterMode)) {
        if (G_currentCtx) G_currentCtx->errCode = PF_INVALID_ENUM;
        return;
    }

    tex->sampler = GC_textureSamplers[filterMode][wrapMode];

#   if PF_SIMD_SUPPORT
        tex->samplerSimd = GC_textureSamplers_simd[filterMode][wrapMode];
#   endif //PF_SIMD_SUPPORT
}

void* pfGetTexturePixels(const PFtexture texture, PFsizei* width, PFsizei* height, PFpixelformat* format, PFdatatype* type)
{
    const struct PFItex* tex = texture;
    void* pixels = NULL;
    if (tex) {
        if (width) *width = tex->w;
        if (height) *height = tex->h;
        if (format) *format = tex->format;
        if (type) *type = tex->type;
        pixels = tex->pixels;
    }
    return pixels;
}
