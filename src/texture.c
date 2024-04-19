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
#include "pfm.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

/* Including internal function prototypes */

PFerrcode* pfInternal_GetErrorPtr(void);


/* Internal convert functions */

static PFushort FloatToHalf(PFfloat x)
{
    const PFuint b = (*(PFuint*)&x)+0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
    const PFuint e = (b&0x7F800000)>>23; // exponent
    const PFuint m = b&0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
    return (b&0x80000000)>>16 | (e>112)*((((e-112)<<10)&0x7C00)|m>>13) | ((e<113)&(e>101))*((((0x007FF000+m)>>(125-e))+1)>>1) | (e>143)*0x7FFF; // sign : normalized : denormalized : saturate
}

static PFfloat HalfToFloat(PFushort x)
{
    const PFuint e = (x&0x7C00)>>10; // exponent
    const PFuint m = (x&0x03FF)<<13; // mantissa
    const PFfloat fm = (PFfloat)m;
    const PFuint v = (*(PFuint*)&fm)>>23; // evil log2 bit hack to count leading zeros in denormalized format
    const PFuint r = (x&0x8000)<<16 | (e!=0)*((e+112)<<23|m) | ((e==0)&(m!=0))*((v-37)<<23|((m<<(150-v))&0x007FE000)); // sign : normalized : denormalized
    return *(PFfloat*)&r;
}


/* Internal pixel setter functions */

static void SetGrayscale(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate grayscale equivalent color
    PFMvec3 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };
    PFubyte gray = (PFubyte)((nCol[0]*0.299f + nCol[1]*0.587f + nCol[2]*0.114f)*255.0f);

    ((PFubyte*)pixels)[offset] = gray;
}

static void SetGrayAlpha(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate grayscale equivalent color
    PFMvec3 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };
    PFubyte gray = (PFubyte)((nCol[0]*0.299f + nCol[1]*0.587f + nCol[2]*0.114f)*255.0f);

    PFubyte *pixel = (PFubyte*)pixels + offset*2;
    pixel[0] = gray, pixel[1] = color.a;
}

static void SetR5G6B5(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate R5G6B5 equivalent color
    PFMvec3 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };

    PFubyte r = (PFubyte)(round(nCol[0]*31.0f));
    PFubyte g = (PFubyte)(round(nCol[1]*63.0f));
    PFubyte b = (PFubyte)(round(nCol[2]*31.0f));

    ((PFushort*)pixels)[offset] = (PFushort)r << 11 | (PFushort)g << 5 | (PFushort)b;
}

static void SetR8G8B8(void* pixels, PFsizei offset, PFcolor color)
{
    PFubyte* pixel = (PFubyte*)pixels + offset*3;
    pixel[0] = color.r, pixel[1] = color.g, pixel[2] = color.b;
}

static void SetR5G5B5A1(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate R5G5B5A1 equivalent color
    PFMvec4 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255, (PFfloat)color.a*INV_255 };

    PFubyte r = (PFubyte)(round(nCol[0]*31.0f));
    PFubyte g = (PFubyte)(round(nCol[1]*31.0f));
    PFubyte b = (PFubyte)(round(nCol[2]*31.0f));
    PFubyte a = (nCol[3] > ((PFfloat)PF_PIXELFORMAT_R5G5B5A1_ALPHA_THRESHOLD*INV_255))? 1 : 0;

    ((PFushort*)pixels)[offset] = (PFushort)r << 11 | (PFushort)g << 6 | (PFushort)b << 1 | (PFushort)a;
}

static void SetR4G4B4A4(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate R5G5B5A1 equivalent color
    PFMvec4 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255, (PFfloat)color.a*INV_255 };

    PFubyte r = (PFubyte)(round(nCol[0]*15.0f));
    PFubyte g = (PFubyte)(round(nCol[1]*15.0f));
    PFubyte b = (PFubyte)(round(nCol[2]*15.0f));
    PFubyte a = (PFubyte)(round(nCol[3]*15.0f));

    ((PFushort*)pixels)[offset] = (PFushort)r << 12 | (PFushort)g << 8 | (PFushort)b << 4 | (PFushort)a;
}

static void SetR8G8B8A8(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFuint*)pixels)[offset] = *(PFuint*)(&color);
}

static void SetR32(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate grayscale equivalent color (normalized to 32bit)
    PFMvec3 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };

    ((PFfloat*)pixels)[offset] = nCol[0]*0.299f + nCol[1]*0.587f + nCol[2]*0.114f;
}

static void SetR32G32B32(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate R32G32B32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };

    memcpy((PFMvec3*)pixels + offset, nCol, sizeof(PFMvec3));
}

static void SetR32G32B32A32(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate R32G32B32A32 equivalent color (normalized to 32bit)
    PFMvec4 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255, (PFfloat)color.a*INV_255 };

    memcpy((PFMvec4*)pixels + offset, nCol, sizeof(PFMvec4));
}

static void SetR16(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate grayscale equivalent color (normalized to 32bit)
    PFMvec3 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };

    ((PFushort*)pixels)[offset] = FloatToHalf(nCol[0]*0.299f + nCol[1]*0.587f + nCol[2]*0.114f);
}

static void SetR16G16B16(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate R32G32B32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };

    PFushort *pixel = (PFushort*)pixels + offset*3;
    pixel[0] = FloatToHalf(nCol[0]);
    pixel[1] = FloatToHalf(nCol[1]);
    pixel[2] = FloatToHalf(nCol[2]);
}

static void SetR16G16B16A16(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate R32G32B32A32 equivalent color (normalized to 32bit)
    PFMvec4 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255, (PFfloat)color.a*INV_255 };

    PFushort *pixel = (PFushort*)pixels + offset*4;
    pixel[0] = FloatToHalf(nCol[0]);
    pixel[1] = FloatToHalf(nCol[1]);
    pixel[2] = FloatToHalf(nCol[2]);
    pixel[3] = FloatToHalf(nCol[3]);
}


/* Internal pixel getter functions */

static PFcolor GetGrayscale(const void* pixels, PFsizei offset)
{
    PFubyte gray = ((PFubyte*)pixels)[offset];
    return (PFcolor) { gray, gray, gray, 255 };
}

static PFcolor GetGrayAlpha(const void* pixels, PFsizei offset)
{
    PFubyte *pixel = (PFubyte*)pixels + offset*2;
    return (PFcolor) { *pixel, *pixel, *pixel, pixel[1] };
}

static PFcolor GetR5G6B5(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0xF800) >> 11)*(255/31)),              // 0b1111100000000000
        (PFubyte)((PFfloat)((pixel & 0x7E0) >> 5)*(255/63)),                // 0b0000011111100000
        (PFubyte)((PFfloat)(pixel & 0x1F)*(255/31)),                        // 0b0000000000011111
        255
    };
}

static PFcolor GetR8G8B8(const void* pixels, PFsizei offset)
{
    const PFubyte* pixel = (PFubyte*)pixels + offset*3;
    return (PFcolor) { pixel[0], pixel[1], pixel[2], 255 };
}

static PFcolor GetR5G5B5A1(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0xF800) >> 11)*(255/31)),              // 0b1111100000000000
        (PFubyte)((PFfloat)((pixel & 0x7C0) >> 6)*(255/31)),                // 0b0000011111000000
        (PFubyte)((PFfloat)((pixel & 0x3E) >> 1)*(255/31)),                 // 0b0000000000111110
        (PFubyte)((pixel & 0x1)*255)                                        // 0b0000000000000001
    };
}

static PFcolor GetR4G4B4A4(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0xF000) >> 12)*(255/15)),              // 0b1111000000000000
        (PFubyte)((PFfloat)((pixel & 0xF00) >> 8)*(255/15)),                // 0b0000111100000000
        (PFubyte)((PFfloat)((pixel & 0xF0) >> 4)*(255/15)),                 // 0b0000000011110000
        (PFubyte)((PFfloat)(pixel & 0xF)*(255/15))                          // 0b0000000000001111
    };
}

static PFcolor GetR8G8B8A8(const void* pixels, PFsizei offset)
{
    return ((PFcolor*)pixels)[offset];
}

static PFcolor GetR32(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        (PFubyte)(((PFfloat*)pixels)[offset]*255.0f),
        0, 0, 255
    };
}

static PFcolor GetR32G32B32(const void* pixels, PFsizei offset)
{
    const PFfloat *pixel = (PFfloat*)pixels + offset*3;

    return (PFcolor) {
        (PFubyte)(pixel[0]*255.0f),
        (PFubyte)(pixel[1]*255.0f),
        (PFubyte)(pixel[2]*255.0f),
        255
    };
}

static PFcolor GetR32G32B32A32(const void* pixels, PFsizei offset)
{
    const PFfloat *pixel = (PFfloat*)pixels + offset*4;

    return (PFcolor) {
        (PFubyte)(pixel[0]*255.0f),
        (PFubyte)(pixel[1]*255.0f),
        (PFubyte)(pixel[2]*255.0f),
        (PFubyte)(pixel[3]*255.0f)
    };
}

static PFcolor GetR16(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        (PFubyte)(HalfToFloat(((PFushort*)pixels)[offset])*255.0f),
        0, 0, 255
    };
}

static PFcolor GetR16G16B16(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*3;

    return (PFcolor) {
        (PFubyte)(HalfToFloat(pixel[0]*255.0f)),
        (PFubyte)(HalfToFloat(pixel[1]*255.0f)),
        (PFubyte)(HalfToFloat(pixel[2]*255.0f)),
        255
    };
}

static PFcolor GetR16G16B16A16(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*4;

    return (PFcolor) {
        (PFubyte)(HalfToFloat(pixel[0]*255.0f)),
        (PFubyte)(HalfToFloat(pixel[1]*255.0f)),
        (PFubyte)(HalfToFloat(pixel[2]*255.0f)),
        (PFubyte)(HalfToFloat(pixel[3]*255.0f))
    };
}


/* Internal helper functions */

void pfInternal_DefineGetterSetter(PFpixelgetter* getter, PFpixelsetter* setter, PFpixelformat format)
{
    switch (format)
    {
        case PF_PIXELFORMAT_GRAYSCALE:
            if (getter) *getter = GetGrayscale;
            if (setter) *setter = SetGrayscale;
            break;

        case PF_PIXELFORMAT_GRAY_ALPHA:
            if (getter) *getter = GetGrayAlpha;
            if (setter) *setter = SetGrayAlpha;
            break;

        case PF_PIXELFORMAT_R5G6B5:
            if (getter) *getter = GetR5G6B5;
            if (setter) *setter = SetR5G6B5;
            break;

        case PF_PIXELFORMAT_R5G5B5A1:
            if (getter) *getter = GetR5G5B5A1;
            if (setter) *setter = SetR5G5B5A1;
            break;

        case PF_PIXELFORMAT_R4G4B4A4:
            if (getter) *getter = GetR4G4B4A4;
            if (setter) *setter = SetR4G4B4A4;
            break;

        case PF_PIXELFORMAT_R8G8B8:
            if (getter) *getter = GetR8G8B8;
            if (setter) *setter = SetR8G8B8;
            break;

        case PF_PIXELFORMAT_R8G8B8A8:
            if (getter) *getter = GetR8G8B8A8;
            if (setter) *setter = SetR8G8B8A8;
            break;

        case PF_PIXELFORMAT_R32:
            if (getter) *getter = GetR32;
            if (setter) *setter = SetR32;
            break;

        case PF_PIXELFORMAT_R32G32B32:
            if (getter) *getter = GetR32G32B32;
            if (setter) *setter = SetR32G32B32;
            break;

        case PF_PIXELFORMAT_R32G32B32A32:
            if (getter) *getter = GetR32G32B32A32;
            if (setter) *setter = SetR32G32B32A32;
            break;

        case PF_PIXELFORMAT_R16:
            if (getter) *getter = GetR16;
            if (setter) *setter = SetR16;
            break;

        case PF_PIXELFORMAT_R16G16B16:
            if (getter) *getter = GetR16G16B16;
            if (setter) *setter = SetR16G16B16;
            break;

        case PF_PIXELFORMAT_R16G16B16A16:
            if (getter) *getter = GetR16G16B16A16;
            if (setter) *setter = SetR16G16B16A16;
            break;

        default:
            *pfInternal_GetErrorPtr() = PF_INVALID_ENUM;
            break;
    }
}

static PFsizei GetPixelByte(PFpixelformat format)
{
    switch (format)
    {
        case PF_PIXELFORMAT_GRAYSCALE:      return 1;
        case PF_PIXELFORMAT_GRAY_ALPHA:
        case PF_PIXELFORMAT_R5G6B5:
        case PF_PIXELFORMAT_R5G5B5A1:
        case PF_PIXELFORMAT_R4G4B4A4:       return 2;
        case PF_PIXELFORMAT_R8G8B8A8:       return 4;
        case PF_PIXELFORMAT_R8G8B8:         return 3;
        case PF_PIXELFORMAT_R32:            return 4;
        case PF_PIXELFORMAT_R32G32B32:      return 4*3;
        case PF_PIXELFORMAT_R32G32B32A32:   return 4*4;
        case PF_PIXELFORMAT_R16:            return 2;
        case PF_PIXELFORMAT_R16G16B16:      return 2*3;
        case PF_PIXELFORMAT_R16G16B16A16:   return 2*4;
        default: break;
    }

    return 0;
}


/* Texture functions */

PFtexture pfGenTexture(void* pixels, PFuint width, PFuint height, PFpixelformat format)
{
    PFtexture texture = { NULL, NULL, pixels, width, height, format };
    if (format == PF_PIXELFORMAT_UNKNOWN) return texture;

    pfInternal_DefineGetterSetter(&texture.pixelGetter, &texture.pixelSetter, format);

    return texture;
}

PFtexture pfGenTextureBuffer(PFuint width, PFuint height, PFpixelformat format)
{
    PFtexture texture = { NULL, NULL, NULL, width, height, format };
    if (format == PF_PIXELFORMAT_UNKNOWN) return texture;

    pfInternal_DefineGetterSetter(&texture.pixelGetter, &texture.pixelSetter, format);

    PFsizei size = width*height;
    texture.pixels = PF_MALLOC(size*GetPixelByte(format));

    if (!texture.pixels)
    {
        *pfInternal_GetErrorPtr() = PF_ERROR_OUT_OF_MEMORY;
        return texture;
    }

    for (PFsizei i = 0; i < size; i++)
    {
        texture.pixelSetter(texture.pixels, i, (PFcolor) { 0 });
    }

    return texture;
}

PFtexture pfGenTextureBufferColor(PFuint width, PFuint height, PFcolor color, PFpixelformat format)
{
    PFtexture texture = { NULL, NULL, NULL, width, height, format };
    if (format == PF_PIXELFORMAT_UNKNOWN) return texture;

    pfInternal_DefineGetterSetter(&texture.pixelGetter, &texture.pixelSetter, format);

    PFsizei size = width*height;
    texture.pixels = PF_MALLOC(size*GetPixelByte(format));

    if (!texture.pixels)
    {
        *pfInternal_GetErrorPtr() = PF_ERROR_OUT_OF_MEMORY;
        return texture;
    }

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

        texture->pixels = NULL;
        texture->width = texture->height = 0;
        texture->format = PF_PIXELFORMAT_UNKNOWN;
    }
}

void pfSetTexturePixel(PFtexture* texture, PFuint x, PFuint y, PFcolor color)
{
    texture->pixelSetter(texture->pixels, y*texture->width + x, color);
}

PFcolor pfGetTexturePixel(const PFtexture* texture, PFuint x, PFuint y)
{
    return texture->pixelGetter(texture->pixels, y*texture->width + x);
}

void pfSetTextureSample(PFtexture* texture, PFfloat u, PFfloat v, PFcolor color)
{
    PFuint x = (PFuint)((u - (PFint)u)*(texture->width - 1)) & (texture->width - 1);
    PFuint y = (PFuint)((v - (PFint)v)*(texture->height - 1)) & (texture->height - 1);
    texture->pixelSetter(texture->pixels, y*texture->width + x, color);
}

PFcolor pfGetTextureSample(const PFtexture* texture, PFfloat u, PFfloat v)
{
    PFuint x = (PFuint)((u - (PFint)u)*(texture->width - 1)) & (texture->width - 1);
    PFuint y = (PFuint)((v - (PFint)v)*(texture->height - 1)) & (texture->height - 1);
    return texture->pixelGetter(texture->pixels, y*texture->width + x);
}
