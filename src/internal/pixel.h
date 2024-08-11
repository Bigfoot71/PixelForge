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
#include <stdint.h>

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
    ((PFushort*)pixels)[offset] = pfmFloatToHalf(PF_COLOR_GARYSCALE(color));
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
    pixel[0] = pfmFloatToHalf(PF_COLOR_GARYSCALE(color));
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

/* SET RED/GREEN/BLUE/ALPHA */

static inline void
pfInternal_PixelSet_RED_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFubyte*)pixels)[offset] = color.r;
}

static inline void
pfInternal_PixelSet_GREEN_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFubyte*)pixels)[offset] = color.g;
}

static inline void
pfInternal_PixelSet_BLUE_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFubyte*)pixels)[offset] = color.b;
}

static inline void
pfInternal_PixelSet_ALPHA_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFubyte*)pixels)[offset] = color.a;
}

static inline void
pfInternal_PixelSet_RED_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFushort*)pixels)[offset] = pfmFloatToHalf(color.r/255.0f);
}

static inline void
pfInternal_PixelSet_GREEN_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFushort*)pixels)[offset] = pfmFloatToHalf(color.g/255.0f);
}

static inline void
pfInternal_PixelSet_BLUE_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFushort*)pixels)[offset] = pfmFloatToHalf(color.b/255.0f);
}

static inline void
pfInternal_PixelSet_ALPHA_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFushort*)pixels)[offset] = pfmFloatToHalf(color.a/255.0f);
}

static inline void
pfInternal_PixelSet_RED_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFfloat*)pixels)[offset] = color.r/255.0f;
}

static inline void
pfInternal_PixelSet_GREEN_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFfloat*)pixels)[offset] = color.g/255.0f;
}

static inline void
pfInternal_PixelSet_BLUE_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFfloat*)pixels)[offset] = color.b/255.0f;
}

static inline void
pfInternal_PixelSet_ALPHA_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFfloat*)pixels)[offset] = color.a/255.0f;
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
    pixel[0] = pfmFloatToHalf(nCol[0]);
    pixel[1] = pfmFloatToHalf(nCol[1]);
    pixel[2] = pfmFloatToHalf(nCol[2]);
}

static inline void
pfInternal_PixelSet_BGR_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_32_32_32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = PF_COLOR_BGR_NORMALIZE(color);

    PFushort *pixel = (PFushort*)pixels + offset*3;
    pixel[0] = pfmFloatToHalf(nCol[0]);
    pixel[1] = pfmFloatToHalf(nCol[1]);
    pixel[2] = pfmFloatToHalf(nCol[2]);
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
    pixel[0] = pfmFloatToHalf(nCol[0]);
    pixel[1] = pfmFloatToHalf(nCol[1]);
    pixel[2] = pfmFloatToHalf(nCol[2]);
    pixel[3] = pfmFloatToHalf(nCol[3]);
}

static inline void
pfInternal_PixelSet_BGRA_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_32_32_32_32 equivalent color (normalized to 32bit)
    PFMvec4 nCol = PF_COLOR_BGRA_NORMALIZE(color);

    PFushort *pixel = (PFushort*)pixels + offset*4;
    pixel[0] = pfmFloatToHalf(nCol[0]);
    pixel[1] = pfmFloatToHalf(nCol[1]);
    pixel[2] = pfmFloatToHalf(nCol[2]);
    pixel[3] = pfmFloatToHalf(nCol[3]);
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
    PFubyte gray = 255*pfmHalfToFloat(((PFushort*)pixels)[offset]);
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
    return (PFcolor) { pixel[0], pixel[0], pixel[0], pixel[1] };
}

static inline PFcolor
pfInternal_PixelGet_Luminance_Alpha_HALF(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*2;
    PFubyte gray = 255*pfmHalfToFloat(pixel[0]);
    PFubyte alpha = 255*pfmHalfToFloat(pixel[1]);
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
    PFubyte r = (PFubyte)(255*pfmHalfToFloat(((PFushort*)pixels)[offset]));
    return (PFcolor) { r, 0, 0, 255 };
}

static inline PFcolor
pfInternal_PixelGet_GREEN_HALF(const void* pixels, PFsizei offset)
{
    PFubyte g = (PFubyte)(255*pfmHalfToFloat(((PFushort*)pixels)[offset]));
    return (PFcolor) { 0, g, 0, 255 };
}

static inline PFcolor
pfInternal_PixelGet_BLUE_HALF(const void* pixels, PFsizei offset)
{
    PFubyte b = (PFubyte)(255*pfmHalfToFloat(((PFushort*)pixels)[offset]));
    return (PFcolor) { 0, 0, b, 255 };
}

static inline PFcolor
pfInternal_PixelGet_ALPHA_HALF(const void* pixels, PFsizei offset)
{
    PFubyte a = (PFubyte)(255*pfmHalfToFloat(((PFushort*)pixels)[offset]));
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
        (PFubyte)(pfmHalfToFloat(pixel[0]*255.0f)),
        (PFubyte)(pfmHalfToFloat(pixel[1]*255.0f)),
        (PFubyte)(pfmHalfToFloat(pixel[2]*255.0f)),
        255
    };
}

static inline PFcolor
pfInternal_PixelGet_BGR_HALF(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*3;

    return (PFcolor) {
        (PFubyte)(pfmHalfToFloat(pixel[2]*255.0f)),
        (PFubyte)(pfmHalfToFloat(pixel[1]*255.0f)),
        (PFubyte)(pfmHalfToFloat(pixel[0]*255.0f)),
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
        (PFubyte)(pfmHalfToFloat(pixel[0]*255.0f)),
        (PFubyte)(pfmHalfToFloat(pixel[1]*255.0f)),
        (PFubyte)(pfmHalfToFloat(pixel[2]*255.0f)),
        (PFubyte)(pfmHalfToFloat(pixel[3]*255.0f))
    };
}

static inline PFcolor
pfInternal_PixelGet_BGRA_HALF(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*4;

    return (PFcolor) {
        (PFubyte)(pfmHalfToFloat(pixel[2]*255.0f)),
        (PFubyte)(pfmHalfToFloat(pixel[1]*255.0f)),
        (PFubyte)(pfmHalfToFloat(pixel[0]*255.0f)),
        (PFubyte)(pfmHalfToFloat(pixel[3]*255.0f))
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
        ENTRY(PF_RED, PF_UNSIGNED_BYTE, pfInternal_PixelSet_RED_UBYTE),
        ENTRY(PF_RED, PF_HALF_FLOAT, pfInternal_PixelSet_RED_HALF),
        ENTRY(PF_RED, PF_FLOAT, pfInternal_PixelSet_RED_FLOAT),

        ENTRY(PF_GREEN, PF_UNSIGNED_BYTE, pfInternal_PixelSet_GREEN_UBYTE),
        ENTRY(PF_GREEN, PF_HALF_FLOAT, pfInternal_PixelSet_GREEN_HALF),
        ENTRY(PF_GREEN, PF_FLOAT, pfInternal_PixelSet_GREEN_FLOAT),

        ENTRY(PF_BLUE, PF_UNSIGNED_BYTE, pfInternal_PixelSet_BLUE_UBYTE),
        ENTRY(PF_BLUE, PF_HALF_FLOAT, pfInternal_PixelSet_BLUE_HALF),
        ENTRY(PF_BLUE, PF_FLOAT, pfInternal_PixelSet_BLUE_FLOAT),

        ENTRY(PF_ALPHA, PF_UNSIGNED_BYTE, pfInternal_PixelSet_ALPHA_UBYTE),
        ENTRY(PF_ALPHA, PF_HALF_FLOAT, pfInternal_PixelSet_ALPHA_HALF),
        ENTRY(PF_ALPHA, PF_FLOAT, pfInternal_PixelSet_ALPHA_FLOAT),

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

#define pfmFloatToHalf_simd(x) pfmSimdConvert_F32_F16(x, _MM_FROUND_TO_NEAREST_INT)
#define pfmHalfToFloat_simd(x) pfmSimdConvert_F16_F32(x)

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
    colors = pfInternal_SimdColorPackedGrayscale(colors);

    PFMsimd_i luminance = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x000000FF));
    PFMsimd_i alpha = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0xFF000000));
    alpha = pfmSimdShr_I32(alpha, 16);

    PFMsimd_i lumAlpha = pfmSimdOr_I32(luminance, alpha);

#   define WRITE_LUMINANCE_ALPHA_PIXEL(index) \
    { \
        PFuint lumAlphaValue = pfmSimdExtract_I32(lumAlpha, index) & 0xFFFF; \
        PFuint maskValue = pfmSimdExtract_I32(mask, index) & 0xFFFF; \
        PFushort* targetPixel = (PFushort*)pixels + offset + index; \
        *targetPixel = ((*targetPixel) & ~maskValue) | (lumAlphaValue & maskValue); \
    }

    // Écriture des pixels luminance + alpha modifiés dans le buffer
    WRITE_LUMINANCE_ALPHA_PIXEL(0);
    WRITE_LUMINANCE_ALPHA_PIXEL(1);

#ifdef __SSE2__
    WRITE_LUMINANCE_ALPHA_PIXEL(2);
    WRITE_LUMINANCE_ALPHA_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_LUMINANCE_ALPHA_PIXEL(4);
    WRITE_LUMINANCE_ALPHA_PIXEL(5);
    WRITE_LUMINANCE_ALPHA_PIXEL(6);
    WRITE_LUMINANCE_ALPHA_PIXEL(7);
#endif // __AVX2__

#undef WRITE_LUMINANCE_ALPHA_PIXEL
}

static inline void
pfInternal_PixelSet_Luminance_Alpha_HALF_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}

static inline void
pfInternal_PixelSet_Luminance_Alpha_FLOAT_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

}


/* SET RED/GREEN/BLUE/ALPHA */

static inline void
pfInternal_PixelSet_RED_UBYTE_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    PFMsimd_i red = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x000000FF));

#   define WRITE_RED_PIXEL(index) \
    { \
        PFuint redValue = pfmSimdExtract_I32(red, index) & 0xFF; \
        PFuint maskValue = pfmSimdExtract_I32(mask, index) & 0xFF; \
        PFubyte* targetPixel = (PFubyte*)pixels + offset + index; \
        *targetPixel = ((*targetPixel) & ~maskValue) | (redValue & maskValue); \
    }

    WRITE_RED_PIXEL(0);
    WRITE_RED_PIXEL(1);

#ifdef __SSE2__
    WRITE_RED_PIXEL(2);
    WRITE_RED_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_RED_PIXEL(4);
    WRITE_RED_PIXEL(5);
    WRITE_RED_PIXEL(6);
    WRITE_RED_PIXEL(7);
#endif // __AVX2__

#undef WRITE_RED_PIXEL
}

static inline void
pfInternal_PixelSet_GREEN_UBYTE_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    PFMsimd_i green = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x0000FF00));
    green = pfmSimdShr_I32(green, 8);

#   define WRITE_GREEN_PIXEL(index) \
    { \
        PFuint greenValue = pfmSimdExtract_I32(green, index) & 0xFF; \
        PFuint maskValue = pfmSimdExtract_I32(mask, index) & 0xFF; \
        PFubyte* targetPixel = (PFubyte*)pixels + offset + index; \
        *targetPixel = (PFubyte)((greenValue & maskValue) | ((*targetPixel) & ~maskValue)); \
    }

    WRITE_GREEN_PIXEL(0);
    WRITE_GREEN_PIXEL(1);

#ifdef __SSE2__
    WRITE_GREEN_PIXEL(2);
    WRITE_GREEN_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_GREEN_PIXEL(4);
    WRITE_GREEN_PIXEL(5);
    WRITE_GREEN_PIXEL(6);
    WRITE_GREEN_PIXEL(7);
#endif // __AVX2__

#undef WRITE_GREEN_PIXEL
}

static inline void
pfInternal_PixelSet_BLUE_UBYTE_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    PFMsimd_i blue = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x00FF0000));
    blue = pfmSimdShr_I32(blue, 16);

#   define WRITE_BLUE_PIXEL(index) \
    { \
        PFuint blueValue = pfmSimdExtract_I32(blue, index) & 0xFF; \
        PFuint maskValue = pfmSimdExtract_I32(mask, index) & 0xFF; \
        PFubyte* targetPixel = (PFubyte*)pixels + offset + index; \
        *targetPixel = (PFubyte)((blueValue & maskValue) | ((*targetPixel) & ~maskValue)); \
    }

    WRITE_BLUE_PIXEL(0);
    WRITE_BLUE_PIXEL(1);

#ifdef __SSE2__
    WRITE_BLUE_PIXEL(2);
    WRITE_BLUE_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_BLUE_PIXEL(4);
    WRITE_BLUE_PIXEL(5);
    WRITE_BLUE_PIXEL(6);
    WRITE_BLUE_PIXEL(7);
#endif // __AVX2__

#undef WRITE_BLUE_PIXEL
}

static inline void
pfInternal_PixelSet_ALPHA_UBYTE_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    PFMsimd_i alpha = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0xFF000000));
    alpha = pfmSimdShr_I32(alpha, 24);

#   define WRITE_ALPHA_PIXEL(index) \
    { \
        PFuint alphaValue = pfmSimdExtract_I32(alpha, index) & 0xFF; \
        PFuint maskValue = pfmSimdExtract_I32(mask, index) & 0xFF; \
        PFubyte* targetPixel = (PFubyte*)pixels + offset + index; \
        *targetPixel = (PFubyte)((alphaValue & maskValue) | ((*targetPixel) & ~maskValue)); \
    }

    WRITE_ALPHA_PIXEL(0);
    WRITE_ALPHA_PIXEL(1);

#ifdef __SSE2__
    WRITE_ALPHA_PIXEL(2);
    WRITE_ALPHA_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_ALPHA_PIXEL(4);
    WRITE_ALPHA_PIXEL(5);
    WRITE_ALPHA_PIXEL(6);
    WRITE_ALPHA_PIXEL(7);
#endif // __AVX2__

#undef WRITE_ALPHA_PIXEL
}

static inline void
pfInternal_PixelSet_RED_HALF_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    PFMsimd_i red = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x000000FF));

#   define WRITE_RED_PIXEL(index) \
    { \
        PFuint redValue = pfmSimdExtract_I32(red, index) & 0xFF; \
        PFuint maskValue = pfmSimdExtract_I32(mask, index) & 0xFF; \
        PFushort* targetPixel = (PFushort*)pixels + offset + index; \
        PFubyte currentPixelValue = pfmHalfToFloat(*targetPixel)*255.0f; \
        PFubyte finalColor = (currentPixelValue & ~maskValue) | (redValue & maskValue); \
        *targetPixel = pfmFloatToHalf((PFfloat)finalColor/255.0f); \
    }

    WRITE_RED_PIXEL(0);
    WRITE_RED_PIXEL(1);

#ifdef __SSE2__
    WRITE_RED_PIXEL(2);
    WRITE_RED_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_RED_PIXEL(4);
    WRITE_RED_PIXEL(5);
    WRITE_RED_PIXEL(6);
    WRITE_RED_PIXEL(7);
#endif // __AVX2__

#undef WRITE_RED_PIXEL
}

static inline void
pfInternal_PixelSet_GREEN_HALF_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    PFMsimd_i green = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x0000FF00));
    green = pfmSimdShr_I32(green, 8);

#   define WRITE_GREEN_PIXEL(index) \
    { \
        PFuint greenValue = pfmSimdExtract_I32(green, index) & 0xFF; \
        PFuint maskValue = pfmSimdExtract_I32(mask, index) & 0xFF; \
        PFushort* targetPixel = (PFushort*)pixels + offset + index; \
        PFubyte currentPixelValue = pfmHalfToFloat(*targetPixel)*255.0f; \
        PFubyte finalColor = (currentPixelValue & ~maskValue) | (greenValue & maskValue); \
        *targetPixel = pfmFloatToHalf((PFfloat)finalColor/255.0f); \
    }

    WRITE_GREEN_PIXEL(0);
    WRITE_GREEN_PIXEL(1);

#ifdef __SSE2__
    WRITE_GREEN_PIXEL(2);
    WRITE_GREEN_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_GREEN_PIXEL(4);
    WRITE_GREEN_PIXEL(5);
    WRITE_GREEN_PIXEL(6);
    WRITE_GREEN_PIXEL(7);
#endif // __AVX2__

#undef WRITE_GREEN_PIXEL
}

static inline void
pfInternal_PixelSet_BLUE_HALF_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    PFMsimd_i blue = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x00FF0000));
    blue = pfmSimdShr_I32(blue, 16);

#   define WRITE_BLUE_PIXEL(index) \
    { \
        PFuint blueValue = pfmSimdExtract_I32(blue, index) & 0xFF; \
        PFuint maskValue = pfmSimdExtract_I32(mask, index) & 0xFF; \
        PFushort* targetPixel = (PFushort*)pixels + offset + index; \
        PFubyte currentPixelValue = pfmHalfToFloat(*targetPixel)*255.0f; \
        PFubyte finalColor = (currentPixelValue & ~maskValue) | (blueValue & maskValue); \
        *targetPixel = pfmFloatToHalf((PFfloat)finalColor/255.0f); \
    }

    WRITE_BLUE_PIXEL(0);
    WRITE_BLUE_PIXEL(1);

#ifdef __SSE2__
    WRITE_BLUE_PIXEL(2);
    WRITE_BLUE_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_BLUE_PIXEL(4);
    WRITE_BLUE_PIXEL(5);
    WRITE_BLUE_PIXEL(6);
    WRITE_BLUE_PIXEL(7);
#endif // __AVX2__

#undef WRITE_BLUE_PIXEL
}

static inline void
pfInternal_PixelSet_ALPHA_HALF_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    PFMsimd_i alpha = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0xFF000000));
    alpha = pfmSimdShr_I32(alpha, 24);

#   define WRITE_ALPHA_PIXEL(index) \
    { \
        PFuint alphaValue = pfmSimdExtract_I32(alpha, index) & 0xFF; \
        PFuint maskValue = pfmSimdExtract_I32(mask, index) & 0xFF; \
        PFushort* targetPixel = (PFushort*)pixels + offset + index; \
        PFubyte currentPixelValue = pfmHalfToFloat(*targetPixel)*255.0f; \
        PFubyte finalColor = (currentPixelValue & ~maskValue) | (alphaValue & maskValue); \
        *targetPixel = pfmFloatToHalf((PFfloat)finalColor/255.0f); \
    }

    WRITE_ALPHA_PIXEL(0);
    WRITE_ALPHA_PIXEL(1);

#ifdef __SSE2__
    WRITE_ALPHA_PIXEL(2);
    WRITE_ALPHA_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_ALPHA_PIXEL(4);
    WRITE_ALPHA_PIXEL(5);
    WRITE_ALPHA_PIXEL(6);
    WRITE_ALPHA_PIXEL(7);
#endif // __AVX2__

#undef WRITE_ALPHA_PIXEL
}

static inline void
pfInternal_PixelSet_RED_FLOAT_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    PFMsimd_i red = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x000000FF));

#   define WRITE_RED_PIXEL(index) \
    { \
        PFuint redValue = pfmSimdExtract_I32(red, index) & 0xFF; \
        PFuint maskValue = pfmSimdExtract_I32(mask, index) & 0xFF; \
        PFfloat* targetPixel = (PFfloat*)pixels + offset + index; \
        PFubyte currentPixelValue = (*targetPixel)*255.0f; \
        PFubyte finalColor = (currentPixelValue & ~maskValue) | (redValue & maskValue); \
        *targetPixel = finalColor/255.0f; \
    }

    WRITE_RED_PIXEL(0);
    WRITE_RED_PIXEL(1);

#ifdef __SSE2__
    WRITE_RED_PIXEL(2);
    WRITE_RED_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_RED_PIXEL(4);
    WRITE_RED_PIXEL(5);
    WRITE_RED_PIXEL(6);
    WRITE_RED_PIXEL(7);
#endif // __AVX2__

#undef WRITE_RED_PIXEL
}

static inline void
pfInternal_PixelSet_GREEN_FLOAT_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    PFMsimd_i green = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x0000FF00));
    green = pfmSimdShr_I32(green, 8);

#   define WRITE_GREEN_PIXEL(index) \
    { \
        PFuint greenValue = pfmSimdExtract_I32(green, index) & 0xFF; \
        PFuint maskValue = pfmSimdExtract_I32(mask, index) & 0xFF; \
        PFfloat* targetPixel = (PFfloat*)pixels + offset + index; \
        PFubyte currentPixelValue = (*targetPixel)*255.0f; \
        PFubyte finalColor = (currentPixelValue & ~maskValue) | (greenValue & maskValue); \
        *targetPixel = finalColor/255.0f; \
    }

    WRITE_GREEN_PIXEL(0);
    WRITE_GREEN_PIXEL(1);

#ifdef __SSE2__
    WRITE_GREEN_PIXEL(2);
    WRITE_GREEN_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_GREEN_PIXEL(4);
    WRITE_GREEN_PIXEL(5);
    WRITE_GREEN_PIXEL(6);
    WRITE_GREEN_PIXEL(7);
#endif // __AVX2__

#undef WRITE_GREEN_PIXEL
}

static inline void
pfInternal_PixelSet_BLUE_FLOAT_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    PFMsimd_i blue = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x00FF0000));
    blue = pfmSimdShr_I32(blue, 16);

#   define WRITE_BLUE_PIXEL(index) \
    { \
        PFuint blueValue = pfmSimdExtract_I32(blue, index) & 0xFF; \
        PFuint maskValue = pfmSimdExtract_I32(mask, index) & 0xFF; \
        PFfloat* targetPixel = (PFfloat*)pixels + offset + index; \
        PFubyte currentPixelValue = (*targetPixel)*255.0f; \
        PFubyte finalColor = (currentPixelValue & ~maskValue) | (blueValue & maskValue); \
        *targetPixel = finalColor/255.0f; \
    }

    WRITE_BLUE_PIXEL(0);
    WRITE_BLUE_PIXEL(1);

#ifdef __SSE2__
    WRITE_BLUE_PIXEL(2);
    WRITE_BLUE_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_BLUE_PIXEL(4);
    WRITE_BLUE_PIXEL(5);
    WRITE_BLUE_PIXEL(6);
    WRITE_BLUE_PIXEL(7);
#endif // __AVX2__

#undef WRITE_BLUE_PIXEL
}

static inline void
pfInternal_PixelSet_ALPHA_FLOAT_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    PFMsimd_i alpha = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0xFF000000));
    alpha = pfmSimdShr_I32(alpha, 24);

#   define WRITE_ALPHA_PIXEL(index) \
    { \
        PFuint alphaValue = pfmSimdExtract_I32(alpha, index) & 0xFF; \
        PFuint maskValue = pfmSimdExtract_I32(mask, index) & 0xFF; \
        PFfloat* targetPixel = (PFfloat*)pixels + offset + index; \
        PFubyte currentPixelValue = (*targetPixel)*255.0f; \
        PFubyte finalColor = (currentPixelValue & ~maskValue) | (alphaValue & maskValue); \
        *targetPixel = finalColor/255.0f; \
    }

    WRITE_ALPHA_PIXEL(0);
    WRITE_ALPHA_PIXEL(1);

#ifdef __SSE2__
    WRITE_ALPHA_PIXEL(2);
    WRITE_ALPHA_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_ALPHA_PIXEL(4);
    WRITE_ALPHA_PIXEL(5);
    WRITE_ALPHA_PIXEL(6);
    WRITE_ALPHA_PIXEL(7);
#endif // __AVX2__

#undef WRITE_ALPHA_PIXEL
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
    // Calculate the address in the buffer, taking into account the 24-bit RGB format
    PFubyte* pixelPtr = (PFubyte*)pixels + 3 * offset;

    // Extract the RGB components from the RGBA colors (ignoring the alpha)
    PFMsimd_i red = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x00FF0000));
    PFMsimd_i green = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x0000FF00));
    PFMsimd_i blue = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x000000FF));

    // Reverse the shifts to get the RGB values in the correct order
    red = pfmSimdShr_I32(red, 16);        // Red should remain in the least significant 8 bits
    green = pfmSimdShr_I32(green, 8);     // Shift green so it is in the middle bits

    // Combine the components into a 24-bit RGB format
    PFMsimd_i rgb24 = pfmSimdOr_I32(
        pfmSimdOr_I32(
            pfmSimdShl_I32(blue, 16),     // Blue in the most significant 8 bits
            pfmSimdShl_I32(green, 8)      // Green in the middle 8 bits
        ),
        red  // Red in the least significant 8 bits
    );

#define WRITE_RGB_PIXEL(index) \
    { \
        PFuint pixelColor = pfmSimdExtract_I32(rgb24, index); \
        PFuint maskValue = pfmSimdExtract_I32(mask, index); \
        PFubyte* targetPixel = pixelPtr + index * 3; \
        PFuint oldPixelColor = (targetPixel[0] << 16) | (targetPixel[1] << 8) | targetPixel[2]; \
        PFuint newPixelColor = (pixelColor & maskValue) | (oldPixelColor & ~maskValue); \
        targetPixel[0] = (newPixelColor >> 16) & 0xFF; \
        targetPixel[1] = (newPixelColor >> 8) & 0xFF; \
        targetPixel[2] = newPixelColor & 0xFF; \
    }

    WRITE_RGB_PIXEL(0);
    WRITE_RGB_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGB_PIXEL(2);
    WRITE_RGB_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_RGB_PIXEL(4);
    WRITE_RGB_PIXEL(5);
    WRITE_RGB_PIXEL(6);
    WRITE_RGB_PIXEL(7);
#endif // __AVX2__

#undef WRITE_RGB_PIXEL
}

static inline void
pfInternal_PixelSet_BGR_UBYTE_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{
    // Calculate the address in the buffer, taking into account the 24-bit BGR format
    PFubyte* pixelPtr = (PFubyte*)pixels + 3 * offset;

    // Extract the BGR components from the RGBA colors (ignoring the alpha)
    PFMsimd_i blue = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x00FF0000));
    PFMsimd_i green = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x0000FF00));
    PFMsimd_i red = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0x000000FF));

    // Shift the components to obtain the BGR values
    blue = pfmSimdShr_I32(blue, 16);      // Shift blue to the correct 8 bits
    green = pfmSimdShr_I32(green, 8);     // Shift green to the correct 8 bits

    // Combine the components into a 24-bit BGR format
    PFMsimd_i bgr24 = pfmSimdOr_I32(
        pfmSimdOr_I32(
            pfmSimdShl_I32(blue, 16),    // Position blue
            pfmSimdShl_I32(green, 8)     // Position green
        ),
        red // Position red
    );

#   define WRITE_RGB_PIXEL(index) \
    { \
        PFuint pixelColor = pfmSimdExtract_I32(bgr24, index); \
        PFuint maskValue = pfmSimdExtract_I32(mask, index); \
        PFubyte* targetPixel = pixelPtr + index * 3; \
        PFuint oldPixelColor = (targetPixel[0] << 16) | (targetPixel[1] << 8) | targetPixel[2]; \
        PFuint newPixelColor = (pixelColor & maskValue) | (oldPixelColor & ~maskValue); \
        targetPixel[0] = (newPixelColor >> 16) & 0xFF; \
        targetPixel[1] = (newPixelColor >> 8) & 0xFF; \
        targetPixel[2] = newPixelColor & 0xFF; \
    }

    WRITE_RGB_PIXEL(0);
    WRITE_RGB_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGB_PIXEL(2);
    WRITE_RGB_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_RGB_PIXEL(4);
    WRITE_RGB_PIXEL(5);
    WRITE_RGB_PIXEL(6);
    WRITE_RGB_PIXEL(7);
#endif //__AVX2__

#undef WRITE_RGB_PIXEL
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

}

static inline void
pfInternal_PixelSet_BGRA_USHORT_5_5_5_1_simd(void* pixels, PFsizei offset, PFMsimd_i colors, PFMsimd_i mask)
{

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
    PFMsimd_i lumAlpha = pfmSimdGather_I32(
        pixels, offsets, 2 * sizeof(PFubyte));

    PFMsimd_i mask = pfmSimdSetR_I8(
         2,  2,  2,  3,
         6,  6,  6,  7,
        10, 10, 10, 11,
        14, 14, 14, 15,
        18, 18, 18, 19,
        22, 22, 22, 23,
        26, 26, 26, 27,
        30, 30, 30, 31);

    return pfmSimdShuffle_I8(lumAlpha, mask);
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
    PFMsimd_i red = pfmSimdGather_I32(pixels, offsets, sizeof(PFubyte));

    return pfmSimdOr_I32(
        pfmSimdShr_I32(red, 24),
        pfmSimdSet1_I32(0xFF000000));
}


static inline PFMsimd_i
pfInternal_PixelGet_GREEN_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i green = pfmSimdGather_I32(pixels, offsets, sizeof(PFubyte));

    return pfmSimdOr_I32(
        pfmSimdShl_I32(pfmSimdShr_I32(green, 24), 8),
        pfmSimdSet1_I32(0xFF000000));
}

static inline PFMsimd_i
pfInternal_PixelGet_BLUE_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i blue = pfmSimdGather_I32(pixels, offsets, sizeof(PFubyte));

    return pfmSimdOr_I32(
        pfmSimdShl_I32(pfmSimdShr_I32(blue, 24), 16),
        pfmSimdSet1_I32(0xFF000000));
}

static inline PFMsimd_i
pfInternal_PixelGet_ALPHA_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i alpha = pfmSimdGather_I32(pixels, offsets, sizeof(PFubyte));
    return pfmSimdShl_I32(alpha, 24);
}

static inline PFMsimd_i
pfInternal_PixelGet_RED_HALF_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i red = pfmSimdGather_I32(pixels, offsets, sizeof(PFushort));

    red = pfmSimdConvert_F32_I32(pfmSimdMul_F32(
        pfmSimdConvert_F16_F32(red),
        *(PFMsimd_f*)pfm_f32_255));

    return pfmSimdOr_I32(red,
        pfmSimdSet1_I32(0xFF000000));
}

static inline PFMsimd_i
pfInternal_PixelGet_GREEN_HALF_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i green = pfmSimdGather_I32(pixels, offsets, sizeof(PFushort));

    green = pfmSimdConvert_F32_I32(pfmSimdMul_F32(
        pfmSimdConvert_F16_F32(green),
        *(PFMsimd_f*)pfm_f32_255));

    return pfmSimdOr_I32(
        pfmSimdShl_I32(green, 8),
        pfmSimdSet1_I32(0xFF000000));
}

static inline PFMsimd_i
pfInternal_PixelGet_BLUE_HALF_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i blue = pfmSimdGather_I32(pixels, offsets, sizeof(PFushort));

    blue = pfmSimdConvert_F32_I32(pfmSimdMul_F32(
        pfmSimdConvert_F16_F32(blue),
        *(PFMsimd_f*)pfm_f32_255));

    return pfmSimdOr_I32(
        pfmSimdShl_I32(blue, 16),
        pfmSimdSet1_I32(0xFF000000));
}

static inline PFMsimd_i
pfInternal_PixelGet_ALPHA_HALF_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i alpha = pfmSimdGather_I32(pixels, offsets, sizeof(PFushort));

    alpha = pfmSimdConvert_F32_I32(pfmSimdMul_F32(
        pfmSimdConvert_F16_F32(alpha),
        *(PFMsimd_f*)pfm_f32_255));

    return pfmSimdShl_I32(alpha, 24);
}

static inline PFMsimd_i
pfInternal_PixelGet_RED_FLOAT_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i red = pfmSimdGather_I32(pixels, offsets, sizeof(PFfloat));

    red = pfmSimdConvert_F32_I32(
        pfmSimdMul_F32(pfmSimdCast_I32_F32(red),
        *(PFMsimd_f*)pfm_f32_255));

    return pfmSimdOr_I32(red,
        pfmSimdSet1_I32(0xFF000000));
}

static inline PFMsimd_i
pfInternal_PixelGet_GREEN_FLOAT_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i green = pfmSimdGather_I32(pixels, offsets, sizeof(PFfloat));

    green = pfmSimdConvert_F32_I32(
        pfmSimdMul_F32(pfmSimdCast_I32_F32(green),
        *(PFMsimd_f*)pfm_f32_255));

    return pfmSimdOr_I32(
        pfmSimdShl_I32(green, 8),
        pfmSimdSet1_I32(0xFF000000));
}

static inline PFMsimd_i
pfInternal_PixelGet_BLUE_FLOAT_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i blue = pfmSimdGather_I32(pixels, offsets, sizeof(PFfloat));

    blue = pfmSimdConvert_F32_I32(
        pfmSimdMul_F32(pfmSimdCast_I32_F32(blue),
        *(PFMsimd_f*)pfm_f32_255));

    return pfmSimdOr_I32(
        pfmSimdShl_I32(blue, 16),
        pfmSimdSet1_I32(0xFF000000));
}

static inline PFMsimd_i
pfInternal_PixelGet_ALPHA_FLOAT_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i alpha = pfmSimdGather_I32(pixels, offsets, sizeof(PFfloat));

    alpha = pfmSimdConvert_F32_I32(
        pfmSimdMul_F32(pfmSimdCast_I32_F32(alpha),
        *(PFMsimd_f*)pfm_f32_255));

    return pfmSimdShl_I32(alpha, 24);
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
    // Read the 24-bit RGB values from the buffer.
    PFMsimd_i rgb24 = pfmSimdGather_I32(pixels, pfmSimdMullo_I32(offsets, pfmSimdSet1_I32(3 * sizeof(PFubyte))), 1);

    // Extract the RGB components using the appropriate masks
    PFMsimd_i red = pfmSimdAnd_I32(rgb24, pfmSimdSet1_I32(0xFF0000));
    PFMsimd_i green = pfmSimdAnd_I32(rgb24, pfmSimdSet1_I32(0x00FF00));
    PFMsimd_i blue = pfmSimdAnd_I32(rgb24, pfmSimdSet1_I32(0x0000FF));

    // Shift the values to obtain the 8-bit RGB components
    red = pfmSimdShr_I32(red, 16);   // Shift red to get the 8-bit red component
    green = pfmSimdShr_I32(green, 8); // Shift green to get the 8-bit green component
    // Blue is already in place in the least significant 8 bits

    // Create the alpha component with a value of 0xFF
    PFMsimd_i alpha = pfmSimdSet1_I32(0xFF);

    // Combine the RGB components and alpha into a single RGBA value
    PFMsimd_i rgba32 = pfmSimdOr_I32(
        pfmSimdOr_I32(
            pfmSimdOr_I32(
                pfmSimdShl_I32(red, 16),   // Red in the most significant 8 bits
                pfmSimdShl_I32(green, 8) // Green in the next 8 bits
            ),
            blue // Blue in the next 8 bits
        ),
        pfmSimdShl_I32(alpha, 24) // Alpha in the least significant 8 bits
    );

    return rgba32;
}

static inline PFMsimd_i
pfInternal_PixelGet_BGR_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    // Read the 24-bit BGR values from the buffer
    PFMsimd_i bgr24 = pfmSimdGather_I32(pixels, pfmSimdMullo_I32(offsets, pfmSimdSet1_I32(3 * sizeof(PFubyte))), 1);

    // Extract the BGR components using the appropriate masks
    PFMsimd_i blue = pfmSimdAnd_I32(bgr24, pfmSimdSet1_I32(0xFF0000));
    PFMsimd_i green = pfmSimdAnd_I32(bgr24, pfmSimdSet1_I32(0x00FF00));
    PFMsimd_i red = pfmSimdAnd_I32(bgr24, pfmSimdSet1_I32(0x0000FF));

    // Shift the values to obtain the 8-bit RGB components
    blue = pfmSimdShr_I32(blue, 16);      // Shift blue to get the 8-bit blue component
    green = pfmSimdShr_I32(green, 8);     // Shift green to get the 8-bit green component
    // Red is already in place in the least significant 8 bits

    // Create the alpha component with a value of 0xFF
    PFMsimd_i alpha = pfmSimdSet1_I32(0xFF);

    // Combine the RGB components and alpha into a single RGBA value
    PFMsimd_i rgba32 = pfmSimdOr_I32(
        pfmSimdOr_I32(
            pfmSimdOr_I32(
                pfmSimdShl_I32(red, 16),   // Red in the most significant 8 bits
                pfmSimdShl_I32(green, 8)  // Green in the next 8 bits
            ),
            blue // Blue in the least significant 8 bits
        ),
        pfmSimdShl_I32(alpha, 24) // Alpha in the most significant 8 bits
    );

    return rgba32;
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
    return pfmSimdGather_I32(pixels, offsets, sizeof(PFuint));
}

static inline PFMsimd_i
pfInternal_PixelGet_BGRA_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = pfmSimdGather_I32(pixels, offsets, sizeof(PFuint));
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
        ENTRY(PF_RED, PF_UNSIGNED_BYTE, pfInternal_PixelSet_RED_UBYTE_simd),
        ENTRY(PF_RED, PF_HALF_FLOAT, pfInternal_PixelSet_RED_HALF_simd),
        ENTRY(PF_RED, PF_FLOAT, pfInternal_PixelSet_RED_FLOAT_simd),

        ENTRY(PF_GREEN, PF_UNSIGNED_BYTE, pfInternal_PixelSet_GREEN_UBYTE_simd),
        ENTRY(PF_GREEN, PF_HALF_FLOAT, pfInternal_PixelSet_GREEN_HALF_simd),
        ENTRY(PF_GREEN, PF_FLOAT, pfInternal_PixelSet_GREEN_FLOAT_simd),

        ENTRY(PF_BLUE, PF_UNSIGNED_BYTE, pfInternal_PixelSet_BLUE_UBYTE_simd),
        ENTRY(PF_BLUE, PF_HALF_FLOAT, pfInternal_PixelSet_BLUE_HALF_simd),
        ENTRY(PF_BLUE, PF_FLOAT, pfInternal_PixelSet_BLUE_FLOAT_simd),

        ENTRY(PF_ALPHA, PF_UNSIGNED_BYTE, pfInternal_PixelSet_ALPHA_UBYTE_simd),
        ENTRY(PF_ALPHA, PF_HALF_FLOAT, pfInternal_PixelSet_ALPHA_HALF_simd),
        ENTRY(PF_ALPHA, PF_FLOAT, pfInternal_PixelSet_ALPHA_FLOAT_simd),

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
