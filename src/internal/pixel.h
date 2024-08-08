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

#ifndef PF_INTERNAL_PIXEL_H
#define PF_INTERNAL_PIXEL_H

#include "./context/context.h"
#include "./config.h"
#include "../pfm.h"

/*
 * NOTE: At first glance, this method seems quite heavy, and we might be tempted to simplify by using only two functions,
 *       GetPixel / SetPixel, handling cases via switches and other operations. However, after many tests and attempts,
 *       it turns out that this method, using predefined function pointers, is by far the most efficient in terms of
 *       performance. If a solution as efficient, or even more efficient, that allows handling all currently supported
 *       pixel format types exists, it will obviously be welcome.
 */

/* Helper Macros */

#define PF_COLOR_RGB_NORMALIZE(color)       \
    {                                       \
        (PFfloat)color.r*INV_255,           \
        (PFfloat)color.g*INV_255,           \
        (PFfloat)color.b*INV_255            \
    }

#define PF_COLOR_BGR_NORMALIZE(color)       \
    {                                       \
        (PFfloat)color.b*INV_255,           \
        (PFfloat)color.g*INV_255,           \
        (PFfloat)color.r*INV_255            \
    }

#define PF_COLOR_RGBA_NORMALIZE(color)      \
    {                                       \
        (PFfloat)color.r*INV_255,           \
        (PFfloat)color.g*INV_255,           \
        (PFfloat)color.b*INV_255,           \
        (PFfloat)color.a*INV_255            \
    }

#define PF_COLOR_BGRA_NORMALIZE(color)      \
    {                                       \
        (PFfloat)color.r*INV_255,           \
        (PFfloat)color.g*INV_255,           \
        (PFfloat)color.b*INV_255,           \
        (PFfloat)color.a*INV_255            \
    }

#define PF_COLOR_GARYSCALE(color)           \
    (                                       \
        (PFfloat)color.r*INV_255*0.299f +   \
        (PFfloat)color.g*INV_255*0.587f +   \
        (PFfloat)color.b*INV_255*0.114f     \
    )

/* Internal convert functions */

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize ("no-strict-aliasing")
#endif

static inline PFushort pfInternal_FloatToHalf(PFfloat x)
{
    const PFuint b = (*(PFuint*)&x)+0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
    const PFuint e = (b&0x7F800000)>>23; // exponent
    const PFuint m = b&0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
    return (b&0x80000000)>>16 | (e>112)*((((e-112)<<10)&0x7C00)|m>>13) | ((e<113)&(e>101))*((((0x007FF000+m)>>(125-e))+1)>>1) | (e>143)*0x7FFF; // sign : normalized : denormalized : saturate
}

static inline PFfloat pfInternal_HalfToFloat(PFushort x)
{
    const PFuint e = (x&0x7C00)>>10; // exponent
    const PFuint m = (x&0x03FF)<<13; // mantissa
    const PFfloat fm = (PFfloat)m;
    const PFuint v = (*(PFuint*)&fm)>>23; // evil log2 bit hack to count leading zeros in denormalized format
    const PFuint r = (x&0x8000)<<16 | (e!=0)*((e+112)<<23|m) | ((e==0)&(m!=0))*((v-37)<<23|((m<<(150-v))&0x007FE000)); // sign : normalized : denormalized
    return *(PFfloat*)&r;
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif


/* SET LUMINANCE */

static inline void
pfInternal_PixelSet_Luminance_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Grayscale equivalent color
    ((PFubyte*)pixels)[offset] = (PFubyte)(255.0f*PF_COLOR_GARYSCALE(color));
}

static inline void
pfInternal_PixelSet_Luminance_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Grayscale equivalent color
    ((PFushort*)pixels)[offset] = pfInternal_FloatToHalf(PF_COLOR_GARYSCALE(color));
}

static inline void
pfInternal_PixelSet_Luminance_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Grayscale equivalent color
    ((PFfloat*)pixels)[offset] = PF_COLOR_GARYSCALE(color);
}


/* SET LUMINANCE ALPHA */

static inline void
pfInternal_PixelSet_Luminance_Alpha_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Grayscale equivalent color
    PFubyte *pixel = (PFubyte*)pixels + 2*offset;
    pixel[0] = 255*PF_COLOR_GARYSCALE(color);
    pixel[1] = color.a;
}

static inline void
pfInternal_PixelSet_Luminance_Alpha_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Grayscale equivalent color
    PFushort *pixel = (PFushort*)pixels + 2*offset;
    pixel[0] = pfInternal_FloatToHalf(PF_COLOR_GARYSCALE(color));
    pixel[1] = color.a;
}

static inline void
pfInternal_PixelSet_Luminance_Alpha_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Grayscale equivalent color
    PFfloat *pixel = (PFfloat*)pixels + 2*offset;
    pixel[0] = PF_COLOR_GARYSCALE(color);
    pixel[1] = color.a;
}


/* SET RGB/BGR */

static inline void
pfInternal_PixelSet_RGB_USHORT_5_6_5(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_5_6_5 equivalent color
    PFMvec3 nCol = PF_COLOR_RGB_NORMALIZE(color);

    PFubyte r = (PFubyte)(roundf(nCol[0]*31.0f));
    PFubyte g = (PFubyte)(roundf(nCol[1]*63.0f));
    PFubyte b = (PFubyte)(roundf(nCol[2]*31.0f));

    ((PFushort*)pixels)[offset] = (PFushort)r << 11 | (PFushort)g << 5 | (PFushort)b;
}

static inline void
pfInternal_PixelSet_BGR_USHORT_5_6_5(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_5_6_5 equivalent color
    PFMvec3 nCol = PF_COLOR_BGR_NORMALIZE(color);

    PFubyte b = (PFubyte)(roundf(nCol[0]*31.0f));
    PFubyte g = (PFubyte)(roundf(nCol[1]*63.0f));
    PFubyte r = (PFubyte)(roundf(nCol[2]*31.0f));

    ((PFushort*)pixels)[offset] = (PFushort)b << 11 | (PFushort)g << 5 | (PFushort)r;
}

static inline void
pfInternal_PixelSet_RGB_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    PFubyte* pixel = (PFubyte*)pixels + offset*3;
    pixel[0] = color.r;
    pixel[1] = color.g;
    pixel[2] = color.b;
}

static inline void
pfInternal_PixelSet_BGR_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    PFubyte* pixel = (PFubyte*)pixels + offset*3;
    pixel[0] = color.b;
    pixel[1] = color.g;
    pixel[2] = color.r;
}

static inline void
pfInternal_PixelSet_RGB_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_32_32_32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = PF_COLOR_RGB_NORMALIZE(color);

    PFushort *pixel = (PFushort*)pixels + offset*3;
    pixel[0] = pfInternal_FloatToHalf(nCol[0]);
    pixel[1] = pfInternal_FloatToHalf(nCol[1]);
    pixel[2] = pfInternal_FloatToHalf(nCol[2]);
}

static inline void
pfInternal_PixelSet_BGR_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_32_32_32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = PF_COLOR_BGR_NORMALIZE(color);

    PFushort *pixel = (PFushort*)pixels + offset*3;
    pixel[0] = pfInternal_FloatToHalf(nCol[0]);
    pixel[1] = pfInternal_FloatToHalf(nCol[1]);
    pixel[2] = pfInternal_FloatToHalf(nCol[2]);
}

static inline void
pfInternal_PixelSet_RGB_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_32_32_32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = PF_COLOR_RGB_NORMALIZE(color);
    memcpy((PFMvec3*)pixels + offset, nCol, sizeof(PFMvec3));
}

static inline void
pfInternal_PixelSet_BGR_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate BGR_32_32_32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = PF_COLOR_BGR_NORMALIZE(color);
    memcpy((PFMvec3*)pixels + offset, nCol, sizeof(PFMvec3));
}


/* SET RGBA/BGRA */

static inline void
pfInternal_PixelSet_RGBA_USHORT_5_5_5_1(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_5_5_5_1 equivalent color
    PFMvec4 nCol = PF_COLOR_RGBA_NORMALIZE(color);

    PFubyte r = (PFubyte)(roundf(nCol[0]*31.0f));
    PFubyte g = (PFubyte)(roundf(nCol[1]*31.0f));
    PFubyte b = (PFubyte)(roundf(nCol[2]*31.0f));
    PFubyte a = (nCol[3] > ((PFfloat)PF_RGBA_5_5_5_1_ALPHA_THRESHOLD*INV_255)) ? 1 : 0;

    ((PFushort*)pixels)[offset] = (PFushort)r << 11 | (PFushort)g << 6 | (PFushort)b << 1 | (PFushort)a;
}

static inline void
pfInternal_PixelSet_BGRA_USHORT_5_5_5_1(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_5_5_5_1 equivalent color
    PFMvec4 nCol = PF_COLOR_BGRA_NORMALIZE(color);

    PFubyte b = (PFubyte)(roundf(nCol[0]*31.0f));
    PFubyte g = (PFubyte)(roundf(nCol[1]*31.0f));
    PFubyte r = (PFubyte)(roundf(nCol[2]*31.0f));
    PFubyte a = (nCol[3] > ((PFfloat)PF_RGBA_5_5_5_1_ALPHA_THRESHOLD*INV_255)) ? 1 : 0;

    ((PFushort*)pixels)[offset] = (PFushort)b << 11 | (PFushort)g << 6 | (PFushort)r << 1 | (PFushort)a;
}

static inline void
pfInternal_PixelSet_RGBA_USHORT_4_4_4_4(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_5_5_5_1 equivalent color
    PFMvec4 nCol = PF_COLOR_RGBA_NORMALIZE(color);

    PFubyte r = (PFubyte)(roundf(nCol[0]*15.0f));
    PFubyte g = (PFubyte)(roundf(nCol[1]*15.0f));
    PFubyte b = (PFubyte)(roundf(nCol[2]*15.0f));
    PFubyte a = (PFubyte)(roundf(nCol[3]*15.0f));

    ((PFushort*)pixels)[offset] = (PFushort)r << 12 | (PFushort)g << 8 | (PFushort)b << 4 | (PFushort)a;
}

static inline void
pfInternal_PixelSet_BGRA_USHORT_4_4_4_4(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_5_5_5_1 equivalent color
    PFMvec4 nCol = PF_COLOR_BGRA_NORMALIZE(color);

    PFubyte b = (PFubyte)(roundf(nCol[0]*15.0f));
    PFubyte g = (PFubyte)(roundf(nCol[1]*15.0f));
    PFubyte r = (PFubyte)(roundf(nCol[2]*15.0f));
    PFubyte a = (PFubyte)(roundf(nCol[3]*15.0f));

    ((PFushort*)pixels)[offset] = (PFushort)b << 12 | (PFushort)g << 8 | (PFushort)r << 4 | (PFushort)a;
}

static inline void
pfInternal_PixelSet_RGBA_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFuint*)pixels)[offset] = *(PFuint*)(&color);
}

static inline void
pfInternal_PixelSet_BGRA_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    PFubyte *ptr = (PFubyte*)((PFuint*)pixels + offset);
    ptr[0] = color.b;
    ptr[1] = color.g;
    ptr[2] = color.r;
    ptr[3] = color.a;
}

static inline void
pfInternal_PixelSet_RGBA_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_32_32_32_32 equivalent color (normalized to 32bit)
    PFMvec4 nCol = PF_COLOR_RGBA_NORMALIZE(color);

    PFushort *pixel = (PFushort*)pixels + offset*4;
    pixel[0] = pfInternal_FloatToHalf(nCol[0]);
    pixel[1] = pfInternal_FloatToHalf(nCol[1]);
    pixel[2] = pfInternal_FloatToHalf(nCol[2]);
    pixel[3] = pfInternal_FloatToHalf(nCol[3]);
}

static inline void
pfInternal_PixelSet_BGRA_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_32_32_32_32 equivalent color (normalized to 32bit)
    PFMvec4 nCol = PF_COLOR_BGRA_NORMALIZE(color);

    PFushort *pixel = (PFushort*)pixels + offset*4;
    pixel[0] = pfInternal_FloatToHalf(nCol[0]);
    pixel[1] = pfInternal_FloatToHalf(nCol[1]);
    pixel[2] = pfInternal_FloatToHalf(nCol[2]);
    pixel[3] = pfInternal_FloatToHalf(nCol[3]);
}

static inline void
pfInternal_PixelSet_RGBA_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_32_32_32_32 equivalent color (normalized to 32bit)
    PFMvec4 nCol = PF_COLOR_RGBA_NORMALIZE(color);
    memcpy((PFMvec4*)pixels + offset, nCol, sizeof(PFMvec4));
}

static inline void
pfInternal_PixelSet_BGRA_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate BGRA_32_32_32_32 equivalent color (normalized to 32bit)
    PFMvec4 nCol = PF_COLOR_BGRA_NORMALIZE(color);
    memcpy((PFMvec4*)pixels + offset, nCol, sizeof(PFMvec4));
}


/* GET LUMINANCE */

static inline PFcolor
pfInternal_PixelGet_Luminance_UBYTE(const void* pixels, PFsizei offset)
{
    PFubyte gray = ((PFubyte*)pixels)[offset];
    return (PFcolor) { gray, gray, gray, 255 };
}

static inline PFcolor
pfInternal_PixelGet_Luminance_HALF(const void* pixels, PFsizei offset)
{
    PFubyte gray = 255*pfInternal_HalfToFloat(((PFushort*)pixels)[offset]);
    return (PFcolor) { gray, gray, gray, 255 };
}

static inline PFcolor
pfInternal_PixelGet_Luminance_FLOAT(const void* pixels, PFsizei offset)
{
    PFubyte gray = 255*((PFfloat*)pixels)[offset];
    return (PFcolor) { gray, gray, gray, 255 };
}


/* GET LUMINANCE ALPHA */

static inline PFcolor
pfInternal_PixelGet_Luminance_Alpha_UBYTE(const void* pixels, PFsizei offset)
{
    const PFubyte *pixel = (PFubyte*)pixels + offset*2;
    return (PFcolor) { *pixel, *pixel, *pixel, pixel[1] };
}

static inline PFcolor
pfInternal_PixelGet_Luminance_Alpha_HALF(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*2;
    PFubyte gray = 255*pfInternal_HalfToFloat(pixel[0]);
    PFubyte alpha = 255*pfInternal_HalfToFloat(pixel[1]);
    return (PFcolor) { gray, gray, gray, alpha };
}

static inline PFcolor
pfInternal_PixelGet_Luminance_Alpha_FLOAT(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*2;
    PFubyte gray = 255*pixel[0], alpha = 255*pixel[1];
    return (PFcolor) { gray, gray, gray, alpha };
}


/* GET RED/GREEN/BLUE/ALPHA */

static inline PFcolor
pfInternal_PixelGet_RED_UBYTE(const void* pixels, PFsizei offset)
{
    PFubyte r = ((PFubyte*)pixels)[offset];
    return (PFcolor) { r, 0, 0, 255 };
}

static inline PFcolor
pfInternal_PixelGet_GREEN_UBYTE(const void* pixels, PFsizei offset)
{
    PFubyte g = ((PFubyte*)pixels)[offset];
    return (PFcolor) { 0, g, 0, 255 };
}

static inline PFcolor
pfInternal_PixelGet_BLUE_UBYTE(const void* pixels, PFsizei offset)
{
    PFubyte b = ((PFubyte*)pixels)[offset];
    return (PFcolor) { 0, 0, b, 255 };
}

static inline PFcolor
pfInternal_PixelGet_ALPHA_UBYTE(const void* pixels, PFsizei offset)
{
    PFubyte a = ((PFubyte*)pixels)[offset];
    return (PFcolor) { 255, 255, 255, a };
}

static inline PFcolor
pfInternal_PixelGet_RED_HALF(const void* pixels, PFsizei offset)
{
    PFubyte r = (PFubyte)(255*pfInternal_HalfToFloat(((PFushort*)pixels)[offset]));
    return (PFcolor) { r, 0, 0, 255 };
}

static inline PFcolor
pfInternal_PixelGet_GREEN_HALF(const void* pixels, PFsizei offset)
{
    PFubyte g = (PFubyte)(255*pfInternal_HalfToFloat(((PFushort*)pixels)[offset]));
    return (PFcolor) { 0, g, 0, 255 };
}

static inline PFcolor
pfInternal_PixelGet_BLUE_HALF(const void* pixels, PFsizei offset)
{
    PFubyte b = (PFubyte)(255*pfInternal_HalfToFloat(((PFushort*)pixels)[offset]));
    return (PFcolor) { 0, 0, b, 255 };
}

static inline PFcolor
pfInternal_PixelGet_ALPHA_HALF(const void* pixels, PFsizei offset)
{
    PFubyte a = (PFubyte)(255*pfInternal_HalfToFloat(((PFushort*)pixels)[offset]));
    return (PFcolor) { 255, 255, 255, a };
}

static inline PFcolor
pfInternal_PixelGet_RED_FLOAT(const void* pixels, PFsizei offset)
{
    PFubyte r = (PFubyte)(255*((PFfloat*)pixels)[offset]);
    return (PFcolor) { r, 0, 0, 255 };
}

static inline PFcolor
pfInternal_PixelGet_GREEN_FLOAT(const void* pixels, PFsizei offset)
{
    PFubyte g = (PFubyte)(255*((PFfloat*)pixels)[offset]);
    return (PFcolor) { 0, g, 0, 255 };
}

static inline PFcolor
pfInternal_PixelGet_BLUE_FLOAT(const void* pixels, PFsizei offset)
{
    PFubyte b = (PFubyte)(255*((PFfloat*)pixels)[offset]);
    return (PFcolor) { 0, 0, b, 255 };
}

static inline PFcolor
pfInternal_PixelGet_ALPHA_FLOAT(const void* pixels, PFsizei offset)
{
    PFubyte a = (PFubyte)(255*((PFfloat*)pixels)[offset]);
    return (PFcolor) { 255, 255, 255, a };
}


/* GET RGB/BGR */

static inline PFcolor
pfInternal_PixelGet_RGB_USHORT_5_6_5(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0xF800) >> 11)*(255.0f/31)),               // 0b1111100000000000
        (PFubyte)((PFfloat)((pixel & 0x7E0) >> 5)*(255.0f/63)),                 // 0b0000011111100000
        (PFubyte)((PFfloat)(pixel & 0x1F)*(255.0f/31)),                         // 0b0000000000011111
        255
    };
}

static inline PFcolor
pfInternal_PixelGet_BGR_USHORT_5_6_5(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)(pixel & 0x1F)*(255.0f/31)),                         // 0b0000000000011111
        (PFubyte)((PFfloat)((pixel & 0x7E0) >> 5)*(255.0f/63)),                 // 0b0000011111100000
        (PFubyte)((PFfloat)((pixel & 0xF800) >> 11)*(255.0f/31)),               // 0b1111100000000000
        255
    };
}

static inline PFcolor
pfInternal_PixelGet_RGB_UBYTE(const void* pixels, PFsizei offset)
{
    const PFubyte* pixel = (PFubyte*)pixels + offset*3;
    return (PFcolor) { pixel[0], pixel[1], pixel[2], 255 };
}

static inline PFcolor
pfInternal_PixelGet_BGR_UBYTE(const void* pixels, PFsizei offset)
{
    const PFubyte* pixel = (PFubyte*)pixels + offset*3;
    return (PFcolor) { pixel[2], pixel[1], pixel[0], 255 };
}

static inline PFcolor
pfInternal_PixelGet_RGB_HALF(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*3;

    return (PFcolor) {
        (PFubyte)(pfInternal_HalfToFloat(pixel[0]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[1]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[2]*255.0f)),
        255
    };
}

static inline PFcolor
pfInternal_PixelGet_BGR_HALF(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*3;

    return (PFcolor) {
        (PFubyte)(pfInternal_HalfToFloat(pixel[2]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[1]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[0]*255.0f)),
        255
    };
}

static inline PFcolor
pfInternal_PixelGet_RGB_FLOAT(const void* pixels, PFsizei offset)
{
    const PFfloat *pixel = (PFfloat*)pixels + offset*3;

    return (PFcolor) {
        (PFubyte)(pixel[0]*255.0f),
        (PFubyte)(pixel[1]*255.0f),
        (PFubyte)(pixel[2]*255.0f),
        255
    };
}

static inline PFcolor
pfInternal_PixelGet_BGR_FLOAT(const void* pixels, PFsizei offset)
{
    const PFfloat *pixel = (PFfloat*)pixels + offset*3;

    return (PFcolor) {
        (PFubyte)(pixel[2]*255.0f),
        (PFubyte)(pixel[1]*255.0f),
        (PFubyte)(pixel[0]*255.0f),
        255
    };
}


/* GET RGBA/BGRA */

static inline PFcolor
pfInternal_PixelGet_RGBA_USHORT_5_5_5_1(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0xF800) >> 11)*(255.0f/31)),               // 0b1111100000000000
        (PFubyte)((PFfloat)((pixel & 0x7C0) >> 6)*(255.0f/31)),                 // 0b0000011111000000
        (PFubyte)((PFfloat)((pixel & 0x3E) >> 1)*(255.0f/31)),                  // 0b0000000000111110
        (PFubyte)((pixel & 0x1)*255)                                            // 0b0000000000000001
    };
}

static inline PFcolor
pfInternal_PixelGet_BGRA_USHORT_5_5_5_1(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0x3E) >> 1)*(255.0f/31)),                  // 0b0000000000111110
        (PFubyte)((PFfloat)((pixel & 0x7C0) >> 6)*(255.0f/31)),                 // 0b0000011111000000
        (PFubyte)((PFfloat)((pixel & 0xF800) >> 11)*(255.0f/31)),               // 0b1111100000000000
        (PFubyte)((pixel & 0x1)*255)                                            // 0b0000000000000001
    };
}

static inline PFcolor
pfInternal_PixelGet_RGBA_USHORT_4_4_4_4(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0xF000) >> 12)*(255.0f/15)),               // 0b1111000000000000
        (PFubyte)((PFfloat)((pixel & 0xF00) >> 8)*(255.0f/15)),                 // 0b0000111100000000
        (PFubyte)((PFfloat)((pixel & 0xF0) >> 4)*(255.0f/15)),                  // 0b0000000011110000
        (PFubyte)((PFfloat)(pixel & 0xF)*(255.0f/15))                           // 0b0000000000001111
    };
}

static inline PFcolor
pfInternal_PixelGet_BGRA_USHORT_4_4_4_4(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0xF0) >> 4)*(255.0f/15)),                  // 0b0000000011110000
        (PFubyte)((PFfloat)((pixel & 0xF00) >> 8)*(255.0f/15)),                 // 0b0000111100000000
        (PFubyte)((PFfloat)((pixel & 0xF000) >> 12)*(255.0f/15)),               // 0b1111000000000000
        (PFubyte)((PFfloat)(pixel & 0xF)*(255.0f/15))                           // 0b0000000000001111
    };
}

static inline PFcolor
pfInternal_PixelGet_RGBA_UBYTE(const void* pixels, PFsizei offset)
{
    return *(PFcolor*)((PFuint*)pixels + offset);
}

static inline PFcolor
pfInternal_PixelGet_BGRA_UBYTE(const void* pixels, PFsizei offset)
{
    PFubyte *ptr = (PFubyte*)((PFuint*)pixels + offset);
    return (PFcolor) { ptr[2], ptr[1], ptr[0], ptr[3] };
}

static inline PFcolor
pfInternal_PixelGet_RGBA_HALF(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*4;

    return (PFcolor) {
        (PFubyte)(pfInternal_HalfToFloat(pixel[0]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[1]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[2]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[3]*255.0f))
    };
}

static inline PFcolor
pfInternal_PixelGet_BGRA_HALF(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*4;

    return (PFcolor) {
        (PFubyte)(pfInternal_HalfToFloat(pixel[2]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[1]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[0]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[3]*255.0f))
    };
}

static inline PFcolor
pfInternal_PixelGet_RGBA_FLOAT(const void* pixels, PFsizei offset)
{
    const PFfloat *pixel = (PFfloat*)pixels + offset*4;

    return (PFcolor) {
        (PFubyte)(pixel[0]*255.0f),
        (PFubyte)(pixel[1]*255.0f),
        (PFubyte)(pixel[2]*255.0f),
        (PFubyte)(pixel[3]*255.0f)
    };
}

static inline PFcolor
pfInternal_PixelGet_BGRA_FLOAT(const void* pixels, PFsizei offset)
{
    const PFfloat *pixel = (PFfloat*)pixels + offset*4;

    return (PFcolor) {
        (PFubyte)(pixel[2]*255.0f),
        (PFubyte)(pixel[1]*255.0f),
        (PFubyte)(pixel[0]*255.0f),
        (PFubyte)(pixel[3]*255.0f)
    };
}


/* Internal helper functions */

static inline void
pfInternal_GetPixelGetterSetter(PFpixelgetter* getter, PFpixelsetter* setter, PFpixelformat format, PFdatatype type)
{
#   define ENTRY(FORMAT, TYPE, FUNC) [FORMAT][TYPE] = FUNC

    static const PFpixelgetter getters[10][12] = {
        ENTRY(PF_RED, PF_UNSIGNED_BYTE, pfInternal_PixelGet_RED_UBYTE),
        ENTRY(PF_RED, PF_HALF_FLOAT, pfInternal_PixelGet_RED_HALF),
        ENTRY(PF_RED, PF_FLOAT, pfInternal_PixelGet_RED_FLOAT),

        ENTRY(PF_GREEN, PF_UNSIGNED_BYTE, pfInternal_PixelGet_GREEN_UBYTE),
        ENTRY(PF_GREEN, PF_HALF_FLOAT, pfInternal_PixelGet_GREEN_HALF),
        ENTRY(PF_GREEN, PF_FLOAT, pfInternal_PixelGet_GREEN_FLOAT),

        ENTRY(PF_BLUE, PF_UNSIGNED_BYTE, pfInternal_PixelGet_BLUE_UBYTE),
        ENTRY(PF_BLUE, PF_HALF_FLOAT, pfInternal_PixelGet_BLUE_HALF),
        ENTRY(PF_BLUE, PF_FLOAT, pfInternal_PixelGet_BLUE_FLOAT),

        ENTRY(PF_ALPHA, PF_UNSIGNED_BYTE, pfInternal_PixelGet_ALPHA_UBYTE),
        ENTRY(PF_ALPHA, PF_HALF_FLOAT, pfInternal_PixelGet_ALPHA_HALF),
        ENTRY(PF_ALPHA, PF_FLOAT, pfInternal_PixelGet_ALPHA_FLOAT),

        ENTRY(PF_LUMINANCE, PF_UNSIGNED_BYTE, pfInternal_PixelGet_Luminance_UBYTE),
        ENTRY(PF_LUMINANCE, PF_HALF_FLOAT, pfInternal_PixelGet_Luminance_HALF),
        ENTRY(PF_LUMINANCE, PF_FLOAT, pfInternal_PixelGet_Luminance_FLOAT),

        ENTRY(PF_LUMINANCE_ALPHA, PF_UNSIGNED_BYTE, pfInternal_PixelGet_Luminance_Alpha_UBYTE),
        ENTRY(PF_LUMINANCE_ALPHA, PF_HALF_FLOAT, pfInternal_PixelGet_Luminance_Alpha_HALF),
        ENTRY(PF_LUMINANCE_ALPHA, PF_FLOAT, pfInternal_PixelGet_Luminance_Alpha_FLOAT),

        ENTRY(PF_RGB, PF_UNSIGNED_BYTE, pfInternal_PixelGet_RGB_UBYTE),
        ENTRY(PF_RGB, PF_UNSIGNED_SHORT_5_6_5, pfInternal_PixelGet_RGB_USHORT_5_6_5),
        ENTRY(PF_RGB, PF_HALF_FLOAT, pfInternal_PixelGet_RGB_HALF),
        ENTRY(PF_RGB, PF_FLOAT, pfInternal_PixelGet_RGB_FLOAT),

        ENTRY(PF_RGBA, PF_UNSIGNED_BYTE, pfInternal_PixelGet_RGBA_UBYTE),
        ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_5_5_5_1, pfInternal_PixelGet_RGBA_USHORT_5_5_5_1),
        ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_4_4_4_4, pfInternal_PixelGet_RGBA_USHORT_4_4_4_4),
        ENTRY(PF_RGBA, PF_HALF_FLOAT, pfInternal_PixelGet_RGBA_HALF),
        ENTRY(PF_RGBA, PF_FLOAT, pfInternal_PixelGet_RGBA_FLOAT),

        ENTRY(PF_BGR, PF_UNSIGNED_BYTE, pfInternal_PixelGet_BGR_UBYTE),
        ENTRY(PF_BGR, PF_UNSIGNED_SHORT_5_6_5, pfInternal_PixelGet_BGR_USHORT_5_6_5),
        ENTRY(PF_BGR, PF_HALF_FLOAT, pfInternal_PixelGet_BGR_HALF),
        ENTRY(PF_BGR, PF_FLOAT, pfInternal_PixelGet_BGR_FLOAT),

        ENTRY(PF_BGRA, PF_UNSIGNED_BYTE, pfInternal_PixelGet_BGRA_UBYTE),
        ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_5_5_5_1, pfInternal_PixelGet_BGRA_USHORT_5_5_5_1),
        ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_4_4_4_4, pfInternal_PixelGet_BGRA_USHORT_4_4_4_4),
        ENTRY(PF_BGRA, PF_HALF_FLOAT, pfInternal_PixelGet_BGRA_HALF),
        ENTRY(PF_BGRA, PF_FLOAT, pfInternal_PixelGet_BGRA_FLOAT),
    };

    static const PFpixelsetter setters[10][12] = {
        ENTRY(PF_RED, PF_UNSIGNED_BYTE, pfInternal_PixelSet_Luminance_UBYTE),
        ENTRY(PF_RED, PF_HALF_FLOAT, pfInternal_PixelSet_Luminance_HALF),
        ENTRY(PF_RED, PF_FLOAT, pfInternal_PixelSet_Luminance_FLOAT),

        ENTRY(PF_GREEN, PF_UNSIGNED_BYTE, pfInternal_PixelSet_Luminance_UBYTE),
        ENTRY(PF_GREEN, PF_HALF_FLOAT, pfInternal_PixelSet_Luminance_HALF),
        ENTRY(PF_GREEN, PF_FLOAT, pfInternal_PixelSet_Luminance_FLOAT),

        ENTRY(PF_BLUE, PF_UNSIGNED_BYTE, pfInternal_PixelSet_Luminance_UBYTE),
        ENTRY(PF_BLUE, PF_HALF_FLOAT, pfInternal_PixelSet_Luminance_HALF),
        ENTRY(PF_BLUE, PF_FLOAT, pfInternal_PixelSet_Luminance_FLOAT),

        ENTRY(PF_ALPHA, PF_UNSIGNED_BYTE, pfInternal_PixelSet_Luminance_UBYTE),
        ENTRY(PF_ALPHA, PF_HALF_FLOAT, pfInternal_PixelSet_Luminance_HALF),
        ENTRY(PF_ALPHA, PF_FLOAT, pfInternal_PixelSet_Luminance_FLOAT),

        ENTRY(PF_LUMINANCE, PF_UNSIGNED_BYTE, pfInternal_PixelSet_Luminance_UBYTE),
        ENTRY(PF_LUMINANCE, PF_HALF_FLOAT, pfInternal_PixelSet_Luminance_HALF),
        ENTRY(PF_LUMINANCE, PF_FLOAT, pfInternal_PixelSet_Luminance_FLOAT),

        ENTRY(PF_LUMINANCE_ALPHA, PF_UNSIGNED_BYTE, pfInternal_PixelSet_Luminance_Alpha_UBYTE),
        ENTRY(PF_LUMINANCE_ALPHA, PF_HALF_FLOAT, pfInternal_PixelSet_Luminance_Alpha_HALF),
        ENTRY(PF_LUMINANCE_ALPHA, PF_FLOAT, pfInternal_PixelSet_Luminance_Alpha_FLOAT),

        ENTRY(PF_RGB, PF_UNSIGNED_BYTE, pfInternal_PixelSet_RGB_UBYTE),
        ENTRY(PF_RGB, PF_UNSIGNED_SHORT_5_6_5, pfInternal_PixelSet_RGB_USHORT_5_6_5),
        ENTRY(PF_RGB, PF_HALF_FLOAT, pfInternal_PixelSet_RGB_HALF),
        ENTRY(PF_RGB, PF_FLOAT, pfInternal_PixelSet_RGB_FLOAT),

        ENTRY(PF_RGBA, PF_UNSIGNED_BYTE, pfInternal_PixelSet_RGBA_UBYTE),
        ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_5_5_5_1, pfInternal_PixelSet_RGBA_USHORT_5_5_5_1),
        ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_4_4_4_4, pfInternal_PixelSet_RGBA_USHORT_4_4_4_4),
        ENTRY(PF_RGBA, PF_HALF_FLOAT, pfInternal_PixelSet_RGBA_HALF),
        ENTRY(PF_RGBA, PF_FLOAT, pfInternal_PixelSet_RGBA_FLOAT),

        ENTRY(PF_BGR, PF_UNSIGNED_BYTE, pfInternal_PixelSet_BGR_UBYTE),
        ENTRY(PF_BGR, PF_UNSIGNED_SHORT_5_6_5, pfInternal_PixelSet_BGR_USHORT_5_6_5),
        ENTRY(PF_BGR, PF_HALF_FLOAT, pfInternal_PixelSet_BGR_HALF),
        ENTRY(PF_BGR, PF_FLOAT, pfInternal_PixelSet_BGR_FLOAT),

        ENTRY(PF_BGRA, PF_UNSIGNED_BYTE, pfInternal_PixelSet_BGRA_UBYTE),
        ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_5_5_5_1, pfInternal_PixelSet_BGRA_USHORT_5_5_5_1),
        ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_4_4_4_4, pfInternal_PixelSet_BGRA_USHORT_4_4_4_4),
        ENTRY(PF_BGRA, PF_HALF_FLOAT, pfInternal_PixelSet_BGRA_HALF),
        ENTRY(PF_BGRA, PF_FLOAT, pfInternal_PixelSet_BGRA_FLOAT),
    };

    if (getter) *getter = getters[format][type];
    if (setter) *setter = setters[format][type];

#   undef ENTRY
}

static inline PFsizei
pfInternal_GetPixelBytes(PFpixelformat format, PFdatatype type)
{
    int components = 0;
    PFsizei typeSize = 0;

    switch (format)
    {
        case PF_RED:
        case PF_GREEN:
        case PF_BLUE:
        case PF_ALPHA:
        case PF_LUMINANCE:
            components = 1;
            break;
        case PF_LUMINANCE_ALPHA:
            components = 2;
            break;
        case PF_RGB:
        case PF_BGR:
            components = 3;
            break;
        case PF_RGBA:
        case PF_BGRA:
            components = 4;
            break;
    }

    switch (type)
    {
        case PF_UNSIGNED_BYTE:
        case PF_BYTE:
            typeSize = 1;
            break;
        case PF_UNSIGNED_SHORT:
        case PF_SHORT:
        case PF_UNSIGNED_SHORT_5_6_5:
        case PF_UNSIGNED_SHORT_5_5_5_1:
        case PF_UNSIGNED_SHORT_4_4_4_4:
            typeSize = 2;
            break;
        case PF_UNSIGNED_INT:
        case PF_INT:
            typeSize = 4;
            break;
        case PF_HALF_FLOAT:
            typeSize = 2;
            break;
        case PF_FLOAT:
            typeSize = 4;
            break;
        case PF_DOUBLE:
            typeSize = 8;
            break;
    }

    return components*typeSize;
}



/* -- SIMD IMPLEMENTATION -- */



/* Internal convert functions */

#define pfInternal_FloatToHalf_simd(x) pfmSimdConvert_F32_F16(x, _MM_FROUND_TO_NEAREST_INT)
#define pfInternal_HalfToFloat_simd(x) pfmSimdConvert_F16_F32(x)

/* SET LUMINANCE */

static inline void
pfInternal_PixelSet_Luminance_UBYTE_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_Luminance_HALF_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_Luminance_FLOAT_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}


/* SET LUMINANCE ALPHA */

static inline void
pfInternal_PixelSet_Luminance_Alpha_UBYTE_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_Luminance_Alpha_HALF_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_Luminance_Alpha_FLOAT_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}


/* SET RGB/BGR */

static inline void
pfInternal_PixelSet_RGB_USHORT_5_6_5_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_BGR_USHORT_5_6_5_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_RGB_UBYTE_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_BGR_UBYTE_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_RGB_HALF_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_BGR_HALF_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_RGB_FLOAT_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_BGR_FLOAT_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}


/* SET RGBA/BGRA */

static inline void
pfInternal_PixelSet_RGBA_USHORT_5_5_5_1_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
/*
    // Define the conversion constant from 255 to 31 (5 bits)
    const PFMsimd_i scale = pfmSimdSet1_F32(31.0f / 255.0f);

    // Extract the R, G, B, A components
    PFMsimd_i ri = pfmSimdShuffle_I8(colors, pfmSimdSetR_x4_I8(0, 0, 0, 0)); // Extract R channels
    PFMsimd_i gi = pfmSimdShuffle_I8(colors, pfmSimdSetR_x4_I8(1, 1, 1, 1)); // Extract G channels
    PFMsimd_i bi = pfmSimdShuffle_I8(colors, pfmSimdSetR_x4_I8(2, 2, 2, 2)); // Extract B channels
    PFMsimd_i ai = pfmSimdShuffle_I8(colors, pfmSimdSetR_x4_I8(3, 3, 3, 3)); // Extract A channels

    // Convert values from [0, 255] to [0, 31] for R, G, B
    PFMsimd_f rf = pfmSimdMul_F32(pfmSimdConvert_I32_F32(ri), scale);
    PFMsimd_f gf = pfmSimdMul_F32(pfmSimdConvert_I32_F32(gi), scale);
    PFMsimd_f bf = pfmSimdMul_F32(pfmSimdConvert_I32_F32(bi), scale);

    // Convert A values to bit (0 or 1)
    PFMsimd_f threshold = pfmSimdSet1_F32(PF_RGBA_5_5_5_1_ALPHA_THRESHOLD / 255.0f);
    PFMsimd_f af = pfmSimdCmpGT_F32(pfmSimdConvert_I32_F32(ai), threshold); // a = 1 if > threshold, otherwise 0

    // Convert float values to int
    ri = pfmSimdConvert_F32_I32(pfmSimdRound_F32(rf, _MM_FROUND_TO_NEAREST_INT));
    gi = pfmSimdConvert_F32_I32(pfmSimdRound_F32(gf, _MM_FROUND_TO_NEAREST_INT));
    bi = pfmSimdConvert_F32_I32(pfmSimdRound_F32(bf, _MM_FROUND_TO_NEAREST_INT));
    ai = pfmSimdConvert_F32_I32(pfmSimdRound_F32(af, _MM_FROUND_TO_NEAREST_INT));

    // Combine components into a 5-5-5-1 format
    ri = pfmSimdShl_I32(ri, 11);                     // Shift R by 11 bits
    gi = pfmSimdShl_I32(gi, 6);                      // Shift G by 6 bits
    bi = pfmSimdShl_I32(bi, 1);                      // Shift B by 1 bit

    PFMsimd_i combined = pfmSimdOr_I32(ri, gi);        // Combine R and G
    combined = pfmSimdOr_I32(combined, bi);          // Add B
    combined = pfmSimdOr_I32(combined, ai);          // Add A

    // Store the result in memory
    pfmSimdStore_I16((PFMsimd_i*)(pixels + offset), combined);
*/
}

static inline void
pfInternal_PixelSet_BGRA_USHORT_5_5_5_1_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
/*
    // Define the conversion constant from 255 to 31 (5 bits)
    const PFMsimd_i scale = pfmSimdSet1_F32(31.0f / 255.0f);

    // Extract the R, G, B, A components
    PFMsimd_i bi = pfmSimdShuffle_I8(colors, pfmSimdSetR_x4_I8(2, 2, 2, 2)); // Extract B channels
    PFMsimd_i gi = pfmSimdShuffle_I8(colors, pfmSimdSetR_x4_I8(1, 1, 1, 1)); // Extract G channels
    PFMsimd_i ri = pfmSimdShuffle_I8(colors, pfmSimdSetR_x4_I8(0, 0, 0, 0)); // Extract R channels
    PFMsimd_i ai = pfmSimdShuffle_I8(colors, pfmSimdSetR_x4_I8(3, 3, 3, 3)); // Extract A channels

    // Convert values from [0, 255] to [0, 31] for R, G, B
    PFMsimd_f bf = pfmSimdMul_F32(pfmSimdConvert_I32_F32(bi), scale);
    PFMsimd_f gf = pfmSimdMul_F32(pfmSimdConvert_I32_F32(gi), scale);
    PFMsimd_f rf = pfmSimdMul_F32(pfmSimdConvert_I32_F32(ri), scale);

    // Convert A values to bit (0 or 1)
    PFMsimd_f threshold = pfmSimdSet1_F32(PF_RGBA_5_5_5_1_ALPHA_THRESHOLD / 255.0f);
    PFMsimd_f af = pfmSimdCmpGT_F32(pfmSimdConvert_I32_F32(ai), threshold); // a = 1 if > threshold, otherwise 0

    // Convert float values to int
    bi = pfmSimdConvert_F32_I32(pfmSimdRound_F32(bf, _MM_FROUND_TO_NEAREST_INT));
    gi = pfmSimdConvert_F32_I32(pfmSimdRound_F32(gf, _MM_FROUND_TO_NEAREST_INT));
    ri = pfmSimdConvert_F32_I32(pfmSimdRound_F32(rf, _MM_FROUND_TO_NEAREST_INT));
    ai = pfmSimdConvert_F32_I32(pfmSimdRound_F32(af, _MM_FROUND_TO_NEAREST_INT));

    // Combine components into a 5-5-5-1 format
    bi = pfmSimdShl_I32(bi, 11);                     // Shift B by 11 bits
    gi = pfmSimdShl_I32(gi, 6);                      // Shift G by 6 bits
    ri = pfmSimdShl_I32(ri, 1);                      // Shift R by 1 bit

    PFMsimd_i combined = pfmSimdOr_I32(bi, gi);        // Combine B and G
    combined = pfmSimdOr_I32(combined, ri);          // Add R
    combined = pfmSimdOr_I32(combined, ai);          // Add A

    // Store the result in memory
    pfmSimdStore_I16((PFMsimd_i*)(pixels + offset), combined);
*/
}

static inline void
pfInternal_PixelSet_RGBA_USHORT_4_4_4_4_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_BGRA_USHORT_4_4_4_4_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_RGBA_UBYTE_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    PFMsimd_i pixelsColors = pfmSimdLoad_I32((PFuint*)pixels + offset);
    PFMsimd_i maskedColors = pfmSimdBlendV_I8(pixelsColors, colors, mask);

    pfmSimdStore_I32((PFuint*)pixels + offset, maskedColors);
}

static inline void
pfInternal_PixelSet_BGRA_UBYTE_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    PFMsimd_i bgraColors = pfmSimdShuffle_I8(colors, pfmSimdSetR_x4_I8(2, 1, 0, 3));

    PFMsimd_i pixelsColors = pfmSimdLoad_I32((PFuint*)pixels + offset);
    PFMsimd_i maskedColors = pfmSimdBlendV_I8(pixelsColors, bgraColors, mask);

    pfmSimdStore_I32((PFuint*)pixels + offset, maskedColors);
}

static inline void
pfInternal_PixelSet_RGBA_HALF_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_BGRA_HALF_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_RGBA_FLOAT_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_BGRA_FLOAT_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}


/* GET LUMINANCE */

static inline PFMsimd_i
pfInternal_PixelGet_Luminance_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_Luminance_HALF_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_Luminance_FLOAT_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}


/* GET LUMINANCE ALPHA */

static inline PFMsimd_i
pfInternal_PixelGet_Luminance_Alpha_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_Luminance_Alpha_HALF_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_Luminance_Alpha_FLOAT_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}


/* GET RED/GREEN/BLUE/ALPHA */

static inline PFMsimd_i
pfInternal_PixelGet_RED_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_GREEN_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_BLUE_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_ALPHA_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_RED_HALF_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_GREEN_HALF_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_BLUE_HALF_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_ALPHA_HALF_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_RED_FLOAT_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_GREEN_FLOAT_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_BLUE_FLOAT_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_ALPHA_FLOAT_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}


/* GET RGB/BGR */

static inline PFMsimd_i
pfInternal_PixelGet_RGB_USHORT_5_6_5_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_BGR_USHORT_5_6_5_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_RGB_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_BGR_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_RGB_HALF_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_BGR_HALF_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_RGB_FLOAT_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_BGR_FLOAT_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}



/* GET RGBA/BGRA */

static inline PFMsimd_i
pfInternal_PixelGet_RGBA_USHORT_5_5_5_1_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_BGRA_USHORT_5_5_5_1_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_RGBA_USHORT_4_4_4_4_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_BGRA_USHORT_4_4_4_4_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_RGBA_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    // REVIEW: Sometimes leads to segfaults, called in the triangle rasterization func
    return pfmSimdGather_I32(pixels, offsets);
}

static inline PFMsimd_i
pfInternal_PixelGet_BGRA_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = pfmSimdGather_I32(pixels, offsets);
    return pfmSimdShuffle_I8(result, pfmSimdSetR_x4_I8(2, 1, 0, 3));
}

static inline PFMsimd_i
pfInternal_PixelGet_RGBA_HALF_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_BGRA_HALF_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_RGBA_FLOAT_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

static inline PFMsimd_i
pfInternal_PixelGet_BGRA_FLOAT_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = { 0 };

    return result;
}

/* Internal helper functions */

static inline void
pfInternal_GetPixelGetterSetter_simd(PFpixelgetter_simd* getter, PFpixelsetter_simd* setter, PFpixelformat format, PFdatatype type)
{
#   define ENTRY(FORMAT, TYPE, FUNC) [FORMAT][TYPE] = FUNC

    static const PFpixelgetter_simd getters[10][12] = {
        ENTRY(PF_RED, PF_UNSIGNED_BYTE, pfInternal_PixelGet_RED_UBYTE_simd),
        ENTRY(PF_RED, PF_HALF_FLOAT, pfInternal_PixelGet_RED_HALF_simd),
        ENTRY(PF_RED, PF_FLOAT, pfInternal_PixelGet_RED_FLOAT_simd),

        ENTRY(PF_GREEN, PF_UNSIGNED_BYTE, pfInternal_PixelGet_GREEN_UBYTE_simd),
        ENTRY(PF_GREEN, PF_HALF_FLOAT, pfInternal_PixelGet_GREEN_HALF_simd),
        ENTRY(PF_GREEN, PF_FLOAT, pfInternal_PixelGet_GREEN_FLOAT_simd),

        ENTRY(PF_BLUE, PF_UNSIGNED_BYTE, pfInternal_PixelGet_BLUE_UBYTE_simd),
        ENTRY(PF_BLUE, PF_HALF_FLOAT, pfInternal_PixelGet_BLUE_HALF_simd),
        ENTRY(PF_BLUE, PF_FLOAT, pfInternal_PixelGet_BLUE_FLOAT_simd),

        ENTRY(PF_ALPHA, PF_UNSIGNED_BYTE, pfInternal_PixelGet_ALPHA_UBYTE_simd),
        ENTRY(PF_ALPHA, PF_HALF_FLOAT, pfInternal_PixelGet_ALPHA_HALF_simd),
        ENTRY(PF_ALPHA, PF_FLOAT, pfInternal_PixelGet_ALPHA_FLOAT_simd),

        ENTRY(PF_LUMINANCE, PF_UNSIGNED_BYTE, pfInternal_PixelGet_Luminance_UBYTE_simd),
        ENTRY(PF_LUMINANCE, PF_HALF_FLOAT, pfInternal_PixelGet_Luminance_HALF_simd),
        ENTRY(PF_LUMINANCE, PF_FLOAT, pfInternal_PixelGet_Luminance_FLOAT_simd),

        ENTRY(PF_LUMINANCE_ALPHA, PF_UNSIGNED_BYTE, pfInternal_PixelGet_Luminance_Alpha_UBYTE_simd),
        ENTRY(PF_LUMINANCE_ALPHA, PF_HALF_FLOAT, pfInternal_PixelGet_Luminance_Alpha_HALF_simd),
        ENTRY(PF_LUMINANCE_ALPHA, PF_FLOAT, pfInternal_PixelGet_Luminance_Alpha_FLOAT_simd),

        ENTRY(PF_RGB, PF_UNSIGNED_BYTE, pfInternal_PixelGet_RGB_UBYTE_simd),
        ENTRY(PF_RGB, PF_UNSIGNED_SHORT_5_6_5, pfInternal_PixelGet_RGB_USHORT_5_6_5_simd),
        ENTRY(PF_RGB, PF_HALF_FLOAT, pfInternal_PixelGet_RGB_HALF_simd),
        ENTRY(PF_RGB, PF_FLOAT, pfInternal_PixelGet_RGB_FLOAT_simd),

        ENTRY(PF_RGBA, PF_UNSIGNED_BYTE, pfInternal_PixelGet_RGBA_UBYTE_simd),
        ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_5_5_5_1, pfInternal_PixelGet_RGBA_USHORT_5_5_5_1_simd),
        ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_4_4_4_4, pfInternal_PixelGet_RGBA_USHORT_4_4_4_4_simd),
        ENTRY(PF_RGBA, PF_HALF_FLOAT, pfInternal_PixelGet_RGBA_HALF_simd),
        ENTRY(PF_RGBA, PF_FLOAT, pfInternal_PixelGet_RGBA_FLOAT_simd),

        ENTRY(PF_BGR, PF_UNSIGNED_BYTE, pfInternal_PixelGet_BGR_UBYTE_simd),
        ENTRY(PF_BGR, PF_UNSIGNED_SHORT_5_6_5, pfInternal_PixelGet_BGR_USHORT_5_6_5_simd),
        ENTRY(PF_BGR, PF_HALF_FLOAT, pfInternal_PixelGet_BGR_HALF_simd),
        ENTRY(PF_BGR, PF_FLOAT, pfInternal_PixelGet_BGR_FLOAT_simd),

        ENTRY(PF_BGRA, PF_UNSIGNED_BYTE, pfInternal_PixelGet_BGRA_UBYTE_simd),
        ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_5_5_5_1, pfInternal_PixelGet_BGRA_USHORT_5_5_5_1_simd),
        ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_4_4_4_4, pfInternal_PixelGet_BGRA_USHORT_4_4_4_4_simd),
        ENTRY(PF_BGRA, PF_HALF_FLOAT, pfInternal_PixelGet_BGRA_HALF_simd),
        ENTRY(PF_BGRA, PF_FLOAT, pfInternal_PixelGet_BGRA_FLOAT_simd),
    };

    static const PFpixelsetter_simd setters[10][12] = {
        ENTRY(PF_RED, PF_UNSIGNED_BYTE, pfInternal_PixelSet_Luminance_UBYTE_simd),
        ENTRY(PF_RED, PF_HALF_FLOAT, pfInternal_PixelSet_Luminance_HALF_simd),
        ENTRY(PF_RED, PF_FLOAT, pfInternal_PixelSet_Luminance_FLOAT_simd),

        ENTRY(PF_GREEN, PF_UNSIGNED_BYTE, pfInternal_PixelSet_Luminance_UBYTE_simd),
        ENTRY(PF_GREEN, PF_HALF_FLOAT, pfInternal_PixelSet_Luminance_HALF_simd),
        ENTRY(PF_GREEN, PF_FLOAT, pfInternal_PixelSet_Luminance_FLOAT_simd),

        ENTRY(PF_BLUE, PF_UNSIGNED_BYTE, pfInternal_PixelSet_Luminance_UBYTE_simd),
        ENTRY(PF_BLUE, PF_HALF_FLOAT, pfInternal_PixelSet_Luminance_HALF_simd),
        ENTRY(PF_BLUE, PF_FLOAT, pfInternal_PixelSet_Luminance_FLOAT_simd),

        ENTRY(PF_ALPHA, PF_UNSIGNED_BYTE, pfInternal_PixelSet_Luminance_UBYTE_simd),
        ENTRY(PF_ALPHA, PF_HALF_FLOAT, pfInternal_PixelSet_Luminance_HALF_simd),
        ENTRY(PF_ALPHA, PF_FLOAT, pfInternal_PixelSet_Luminance_FLOAT_simd),

        ENTRY(PF_LUMINANCE, PF_UNSIGNED_BYTE, pfInternal_PixelSet_Luminance_UBYTE_simd),
        ENTRY(PF_LUMINANCE, PF_HALF_FLOAT, pfInternal_PixelSet_Luminance_HALF_simd),
        ENTRY(PF_LUMINANCE, PF_FLOAT, pfInternal_PixelSet_Luminance_FLOAT_simd),

        ENTRY(PF_LUMINANCE_ALPHA, PF_UNSIGNED_BYTE, pfInternal_PixelSet_Luminance_Alpha_UBYTE_simd),
        ENTRY(PF_LUMINANCE_ALPHA, PF_HALF_FLOAT, pfInternal_PixelSet_Luminance_Alpha_HALF_simd),
        ENTRY(PF_LUMINANCE_ALPHA, PF_FLOAT, pfInternal_PixelSet_Luminance_Alpha_FLOAT_simd),

        ENTRY(PF_RGB, PF_UNSIGNED_BYTE, pfInternal_PixelSet_RGB_UBYTE_simd),
        ENTRY(PF_RGB, PF_UNSIGNED_SHORT_5_6_5, pfInternal_PixelSet_RGB_USHORT_5_6_5_simd),
        ENTRY(PF_RGB, PF_HALF_FLOAT, pfInternal_PixelSet_RGB_HALF_simd),
        ENTRY(PF_RGB, PF_FLOAT, pfInternal_PixelSet_RGB_FLOAT_simd),

        ENTRY(PF_RGBA, PF_UNSIGNED_BYTE, pfInternal_PixelSet_RGBA_UBYTE_simd),
        ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_5_5_5_1, pfInternal_PixelSet_RGBA_USHORT_5_5_5_1_simd),
        ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_4_4_4_4, pfInternal_PixelSet_RGBA_USHORT_4_4_4_4_simd),
        ENTRY(PF_RGBA, PF_HALF_FLOAT, pfInternal_PixelSet_RGBA_HALF_simd),
        ENTRY(PF_RGBA, PF_FLOAT, pfInternal_PixelSet_RGBA_FLOAT_simd),

        ENTRY(PF_BGR, PF_UNSIGNED_BYTE, pfInternal_PixelSet_BGR_UBYTE_simd),
        ENTRY(PF_BGR, PF_UNSIGNED_SHORT_5_6_5, pfInternal_PixelSet_BGR_USHORT_5_6_5_simd),
        ENTRY(PF_BGR, PF_HALF_FLOAT, pfInternal_PixelSet_BGR_HALF_simd),
        ENTRY(PF_BGR, PF_FLOAT, pfInternal_PixelSet_BGR_FLOAT_simd),

        ENTRY(PF_BGRA, PF_UNSIGNED_BYTE, pfInternal_PixelSet_BGRA_UBYTE_simd),
        ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_5_5_5_1, pfInternal_PixelSet_BGRA_USHORT_5_5_5_1_simd),
        ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_4_4_4_4, pfInternal_PixelSet_BGRA_USHORT_4_4_4_4_simd),
        ENTRY(PF_BGRA, PF_HALF_FLOAT, pfInternal_PixelSet_BGRA_HALF_simd),
        ENTRY(PF_BGRA, PF_FLOAT, pfInternal_PixelSet_BGRA_FLOAT_simd),
    };

    if (getter) *getter = getters[format][type];
    if (setter) *setter = setters[format][type];

#   undef ENTRY
}

#endif //PF_INTERNAL_PIXEL_H
