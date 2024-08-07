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
#include "internal/simd/pixel_simd.h"
#include "internal/pixel.h"
#include "pfm.h"

#include <stdlib.h>
#include <stddef.h>
#include <xmmintrin.h>

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

    texture->width = width;
    texture->height = height;

    texture->getter = getter;
    texture->setter = setter;

    texture->getterSimd = getterSimd;
    texture->setterSimd = setterSimd;

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
                 tex->getter && tex->setter;
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
    tex->setter(tex->pixels, y*tex->width + x, color);
}

void pfSetTexturePixelSimd(PFtexture texture, PFsizei xFrom, PFsizei yRow, PFMsimd_i color, PFMsimd_i mask)
{
    struct PFtex* tex = texture;
    PFsizei offset = yRow*tex->width + xFrom;
    tex->setterSimd(tex->pixels, offset, color, mask);
}

PFcolor pfGetTexturePixel(const PFtexture texture, PFsizei x, PFsizei y)
{
    struct PFtex* tex = texture;
    return tex->getter(tex->pixels, y*tex->width + x);
}

PFMsimd_i pfGetTexturePixelSimd(const PFtexture texture, PFMsimd_i positions[2])
{
    struct PFtex* tex = texture;

    PFMsimd_i offsets = pfmSimdAdd_I32(
        pfmSimdMullo_I32(positions[1], pfmSimdSet1_I32(tex->width)),
        positions[0]);

    return tex->getterSimd(tex->pixels, offsets);
}

PFcolor pfTextureSampleNearestWrap(const PFtexture texture, PFfloat u, PFfloat v)
{
    struct PFtex* tex = texture;
    PFsizei x = abs((PFint)roundf((u - (PFint)u)*(tex->width - 1)));
    PFsizei y = abs((PFint)roundf((v - (PFint)v)*(tex->height - 1)));
    return tex->getter(tex->pixels, y*tex->width + x);
}

PFMsimd_i pfTextureSampleNearestWrapSimd(const PFtexture texture, const PFMsimd_vec2 texcoords)
{
    struct PFtex* tex = texture;

    PFMsimd_f u = texcoords[0];
    PFMsimd_f v = texcoords[1];

    u = pfmSimdMul_F32(
        pfmSimdSub_F32(u, pfmSimdRound_F32(u, _MM_FROUND_TO_ZERO)),
        pfmSimdSet1_F32((float)(tex->width - 1)));

    v = pfmSimdMul_F32(
        pfmSimdSub_F32(v, pfmSimdRound_F32(v, _MM_FROUND_TO_ZERO)),
        pfmSimdSet1_F32((float)(tex->height - 1)));

    PFMsimd_i x = pfmSimdAbs_I32(pfmSimdConvert_F32_I32(u));
    PFMsimd_i y = pfmSimdAbs_I32(pfmSimdConvert_F32_I32(v));

    PFMsimd_i offsets = pfmSimdAdd_I32(
        pfmSimdMullo_I32(y, pfmSimdSet1_I32(tex->width)), x);

    return tex->getterSimd(tex->pixels, offsets);
}
