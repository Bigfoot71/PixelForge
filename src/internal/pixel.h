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
pfiPixelSet_Luminance_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Grayscale equivalent color
    ((PFubyte*)pixels)[offset] = (PFubyte)(255.0f*PF_COLOR_GARYSCALE(color));
}

static inline void
pfiPixelSet_Luminance_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Grayscale equivalent color
    ((PFushort*)pixels)[offset] = pfmFloatToHalf(PF_COLOR_GARYSCALE(color));
}

static inline void
pfiPixelSet_Luminance_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Grayscale equivalent color
    ((PFfloat*)pixels)[offset] = PF_COLOR_GARYSCALE(color);
}


/* SET LUMINANCE ALPHA */

static inline void
pfiPixelSet_Luminance_Alpha_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Grayscale equivalent color
    PFubyte *pixel = (PFubyte*)pixels + 2*offset;
    pixel[0] = 255*PF_COLOR_GARYSCALE(color);
    pixel[1] = color.a;
}

static inline void
pfiPixelSet_Luminance_Alpha_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Grayscale equivalent color
    PFushort *pixel = (PFushort*)pixels + 2*offset;
    pixel[0] = pfmFloatToHalf(PF_COLOR_GARYSCALE(color));
    pixel[1] = color.a;
}

static inline void
pfiPixelSet_Luminance_Alpha_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Grayscale equivalent color
    PFfloat *pixel = (PFfloat*)pixels + 2*offset;
    pixel[0] = PF_COLOR_GARYSCALE(color);
    pixel[1] = color.a;
}

/* SET RED/GREEN/BLUE/ALPHA */

static inline void
pfiPixelSet_RED_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFubyte*)pixels)[offset] = color.r;
}

static inline void
pfiPixelSet_GREEN_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFubyte*)pixels)[offset] = color.g;
}

static inline void
pfiPixelSet_BLUE_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFubyte*)pixels)[offset] = color.b;
}

static inline void
pfiPixelSet_ALPHA_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFubyte*)pixels)[offset] = color.a;
}

static inline void
pfiPixelSet_RED_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFushort*)pixels)[offset] = pfmFloatToHalf(color.r/255.0f);
}

static inline void
pfiPixelSet_GREEN_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFushort*)pixels)[offset] = pfmFloatToHalf(color.g/255.0f);
}

static inline void
pfiPixelSet_BLUE_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFushort*)pixels)[offset] = pfmFloatToHalf(color.b/255.0f);
}

static inline void
pfiPixelSet_ALPHA_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFushort*)pixels)[offset] = pfmFloatToHalf(color.a/255.0f);
}

static inline void
pfiPixelSet_RED_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFfloat*)pixels)[offset] = color.r/255.0f;
}

static inline void
pfiPixelSet_GREEN_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFfloat*)pixels)[offset] = color.g/255.0f;
}

static inline void
pfiPixelSet_BLUE_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFfloat*)pixels)[offset] = color.b/255.0f;
}

static inline void
pfiPixelSet_ALPHA_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFfloat*)pixels)[offset] = color.a/255.0f;
}


/* SET RGB/BGR */

static inline void
pfiPixelSet_RGB_USHORT_5_6_5(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_5_6_5 equivalent color
    PFMvec3 nCol = PF_COLOR_RGB_NORMALIZE(color);

    PFubyte r = (PFubyte)(roundf(nCol[0]*31.0f));
    PFubyte g = (PFubyte)(roundf(nCol[1]*63.0f));
    PFubyte b = (PFubyte)(roundf(nCol[2]*31.0f));

    ((PFushort*)pixels)[offset] = (PFushort)r << 11 | (PFushort)g << 5 | (PFushort)b;
}

static inline void
pfiPixelSet_BGR_USHORT_5_6_5(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_5_6_5 equivalent color
    PFMvec3 nCol = PF_COLOR_BGR_NORMALIZE(color);

    PFubyte b = (PFubyte)(roundf(nCol[0]*31.0f));
    PFubyte g = (PFubyte)(roundf(nCol[1]*63.0f));
    PFubyte r = (PFubyte)(roundf(nCol[2]*31.0f));

    ((PFushort*)pixels)[offset] = (PFushort)b << 11 | (PFushort)g << 5 | (PFushort)r;
}

static inline void
pfiPixelSet_RGB_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    PFubyte* pixel = (PFubyte*)pixels + offset*3;
    pixel[0] = color.r;
    pixel[1] = color.g;
    pixel[2] = color.b;
}

static inline void
pfiPixelSet_BGR_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    PFubyte* pixel = (PFubyte*)pixels + offset*3;
    pixel[0] = color.b;
    pixel[1] = color.g;
    pixel[2] = color.r;
}

static inline void
pfiPixelSet_RGB_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_32_32_32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = PF_COLOR_RGB_NORMALIZE(color);

    PFushort *pixel = (PFushort*)pixels + offset*3;
    pixel[0] = pfmFloatToHalf(nCol[0]);
    pixel[1] = pfmFloatToHalf(nCol[1]);
    pixel[2] = pfmFloatToHalf(nCol[2]);
}

static inline void
pfiPixelSet_BGR_HALF(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_32_32_32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = PF_COLOR_BGR_NORMALIZE(color);

    PFushort *pixel = (PFushort*)pixels + offset*3;
    pixel[0] = pfmFloatToHalf(nCol[0]);
    pixel[1] = pfmFloatToHalf(nCol[1]);
    pixel[2] = pfmFloatToHalf(nCol[2]);
}

static inline void
pfiPixelSet_RGB_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_32_32_32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = PF_COLOR_RGB_NORMALIZE(color);
    memcpy((PFMvec3*)pixels + offset, nCol, sizeof(PFMvec3));
}

static inline void
pfiPixelSet_BGR_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate BGR_32_32_32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = PF_COLOR_BGR_NORMALIZE(color);
    memcpy((PFMvec3*)pixels + offset, nCol, sizeof(PFMvec3));
}


/* SET RGBA/BGRA */

static inline void
pfiPixelSet_RGBA_USHORT_5_5_5_1(void* pixels, PFsizei offset, PFcolor color)
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
pfiPixelSet_BGRA_USHORT_5_5_5_1(void* pixels, PFsizei offset, PFcolor color)
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
pfiPixelSet_RGBA_USHORT_4_4_4_4(void* pixels, PFsizei offset, PFcolor color)
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
pfiPixelSet_BGRA_USHORT_4_4_4_4(void* pixels, PFsizei offset, PFcolor color)
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
pfiPixelSet_RGBA_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFuint*)pixels)[offset] = *(PFuint*)(&color);
}

static inline void
pfiPixelSet_BGRA_UBYTE(void* pixels, PFsizei offset, PFcolor color)
{
    PFubyte *ptr = (PFubyte*)((PFuint*)pixels + offset);
    ptr[0] = color.b;
    ptr[1] = color.g;
    ptr[2] = color.r;
    ptr[3] = color.a;
}

static inline void
pfiPixelSet_RGBA_HALF(void* pixels, PFsizei offset, PFcolor color)
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
pfiPixelSet_BGRA_HALF(void* pixels, PFsizei offset, PFcolor color)
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
pfiPixelSet_RGBA_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_32_32_32_32 equivalent color (normalized to 32bit)
    PFMvec4 nCol = PF_COLOR_RGBA_NORMALIZE(color);
    memcpy((PFMvec4*)pixels + offset, nCol, sizeof(PFMvec4));
}

static inline void
pfiPixelSet_BGRA_FLOAT(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate BGRA_32_32_32_32 equivalent color (normalized to 32bit)
    PFMvec4 nCol = PF_COLOR_BGRA_NORMALIZE(color);
    memcpy((PFMvec4*)pixels + offset, nCol, sizeof(PFMvec4));
}


/* GET LUMINANCE */

static inline PFcolor
pfiPixelGet_Luminance_UBYTE(const void* pixels, PFsizei offset)
{
    PFubyte gray = ((PFubyte*)pixels)[offset];
    return (PFcolor) { gray, gray, gray, 255 };
}

static inline PFcolor
pfiPixelGet_Luminance_HALF(const void* pixels, PFsizei offset)
{
    PFubyte gray = 255*pfmHalfToFloat(((PFushort*)pixels)[offset]);
    return (PFcolor) { gray, gray, gray, 255 };
}

static inline PFcolor
pfiPixelGet_Luminance_FLOAT(const void* pixels, PFsizei offset)
{
    PFubyte gray = 255*((PFfloat*)pixels)[offset];
    return (PFcolor) { gray, gray, gray, 255 };
}


/* GET LUMINANCE ALPHA */

static inline PFcolor
pfiPixelGet_Luminance_Alpha_UBYTE(const void* pixels, PFsizei offset)
{
    const PFubyte *pixel = (PFubyte*)pixels + offset*2;
    return (PFcolor) { pixel[0], pixel[0], pixel[0], pixel[1] };
}

static inline PFcolor
pfiPixelGet_Luminance_Alpha_HALF(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*2;
    PFubyte gray = 255*pfmHalfToFloat(pixel[0]);
    PFubyte alpha = 255*pfmHalfToFloat(pixel[1]);
    return (PFcolor) { gray, gray, gray, alpha };
}

static inline PFcolor
pfiPixelGet_Luminance_Alpha_FLOAT(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*2;
    PFubyte gray = 255*pixel[0], alpha = 255*pixel[1];
    return (PFcolor) { gray, gray, gray, alpha };
}


/* GET RED/GREEN/BLUE/ALPHA */

static inline PFcolor
pfiPixelGet_RED_UBYTE(const void* pixels, PFsizei offset)
{
    PFubyte r = ((PFubyte*)pixels)[offset];
    return (PFcolor) { r, 0, 0, 255 };
}

static inline PFcolor
pfiPixelGet_GREEN_UBYTE(const void* pixels, PFsizei offset)
{
    PFubyte g = ((PFubyte*)pixels)[offset];
    return (PFcolor) { 0, g, 0, 255 };
}

static inline PFcolor
pfiPixelGet_BLUE_UBYTE(const void* pixels, PFsizei offset)
{
    PFubyte b = ((PFubyte*)pixels)[offset];
    return (PFcolor) { 0, 0, b, 255 };
}

static inline PFcolor
pfiPixelGet_ALPHA_UBYTE(const void* pixels, PFsizei offset)
{
    PFubyte a = ((PFubyte*)pixels)[offset];
    return (PFcolor) { 255, 255, 255, a };
}

static inline PFcolor
pfiPixelGet_RED_HALF(const void* pixels, PFsizei offset)
{
    PFubyte r = (PFubyte)(255*pfmHalfToFloat(((PFushort*)pixels)[offset]));
    return (PFcolor) { r, 0, 0, 255 };
}

static inline PFcolor
pfiPixelGet_GREEN_HALF(const void* pixels, PFsizei offset)
{
    PFubyte g = (PFubyte)(255*pfmHalfToFloat(((PFushort*)pixels)[offset]));
    return (PFcolor) { 0, g, 0, 255 };
}

static inline PFcolor
pfiPixelGet_BLUE_HALF(const void* pixels, PFsizei offset)
{
    PFubyte b = (PFubyte)(255*pfmHalfToFloat(((PFushort*)pixels)[offset]));
    return (PFcolor) { 0, 0, b, 255 };
}

static inline PFcolor
pfiPixelGet_ALPHA_HALF(const void* pixels, PFsizei offset)
{
    PFubyte a = (PFubyte)(255*pfmHalfToFloat(((PFushort*)pixels)[offset]));
    return (PFcolor) { 255, 255, 255, a };
}

static inline PFcolor
pfiPixelGet_RED_FLOAT(const void* pixels, PFsizei offset)
{
    PFubyte r = (PFubyte)(255*((PFfloat*)pixels)[offset]);
    return (PFcolor) { r, 0, 0, 255 };
}

static inline PFcolor
pfiPixelGet_GREEN_FLOAT(const void* pixels, PFsizei offset)
{
    PFubyte g = (PFubyte)(255*((PFfloat*)pixels)[offset]);
    return (PFcolor) { 0, g, 0, 255 };
}

static inline PFcolor
pfiPixelGet_BLUE_FLOAT(const void* pixels, PFsizei offset)
{
    PFubyte b = (PFubyte)(255*((PFfloat*)pixels)[offset]);
    return (PFcolor) { 0, 0, b, 255 };
}

static inline PFcolor
pfiPixelGet_ALPHA_FLOAT(const void* pixels, PFsizei offset)
{
    PFubyte a = (PFubyte)(255*((PFfloat*)pixels)[offset]);
    return (PFcolor) { 255, 255, 255, a };
}


/* GET RGB/BGR */

static inline PFcolor
pfiPixelGet_RGB_USHORT_5_6_5(const void* pixels, PFsizei offset)
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
pfiPixelGet_BGR_USHORT_5_6_5(const void* pixels, PFsizei offset)
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
pfiPixelGet_RGB_UBYTE(const void* pixels, PFsizei offset)
{
    const PFubyte* pixel = (PFubyte*)pixels + offset*3;
    return (PFcolor) { pixel[0], pixel[1], pixel[2], 255 };
}

static inline PFcolor
pfiPixelGet_BGR_UBYTE(const void* pixels, PFsizei offset)
{
    const PFubyte* pixel = (PFubyte*)pixels + offset*3;
    return (PFcolor) { pixel[2], pixel[1], pixel[0], 255 };
}

static inline PFcolor
pfiPixelGet_RGB_HALF(const void* pixels, PFsizei offset)
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
pfiPixelGet_BGR_HALF(const void* pixels, PFsizei offset)
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
pfiPixelGet_RGB_FLOAT(const void* pixels, PFsizei offset)
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
pfiPixelGet_BGR_FLOAT(const void* pixels, PFsizei offset)
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
pfiPixelGet_RGBA_USHORT_5_5_5_1(const void* pixels, PFsizei offset)
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
pfiPixelGet_BGRA_USHORT_5_5_5_1(const void* pixels, PFsizei offset)
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
pfiPixelGet_RGBA_USHORT_4_4_4_4(const void* pixels, PFsizei offset)
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
pfiPixelGet_BGRA_USHORT_4_4_4_4(const void* pixels, PFsizei offset)
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
pfiPixelGet_RGBA_UBYTE(const void* pixels, PFsizei offset)
{
    return *(PFcolor*)((PFuint*)pixels + offset);
}

static inline PFcolor
pfiPixelGet_BGRA_UBYTE(const void* pixels, PFsizei offset)
{
    PFubyte *ptr = (PFubyte*)((PFuint*)pixels + offset);
    return (PFcolor) { ptr[2], ptr[1], ptr[0], ptr[3] };
}

static inline PFcolor
pfiPixelGet_RGBA_HALF(const void* pixels, PFsizei offset)
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
pfiPixelGet_BGRA_HALF(const void* pixels, PFsizei offset)
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
pfiPixelGet_RGBA_FLOAT(const void* pixels, PFsizei offset)
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
pfiPixelGet_BGRA_FLOAT(const void* pixels, PFsizei offset)
{
    const PFfloat *pixel = (PFfloat*)pixels + offset*4;

    return (PFcolor) {
        (PFubyte)(pixel[2]*255.0f),
        (PFubyte)(pixel[1]*255.0f),
        (PFubyte)(pixel[0]*255.0f),
        (PFubyte)(pixel[3]*255.0f)
    };
}

/* Internal helper constant array */

#define ENTRY(FORMAT, TYPE, FUNC) [FORMAT][TYPE] = FUNC

static const PFpixelgetter GC_pixelGetters[10][12] = {
    ENTRY(PF_RED, PF_UNSIGNED_BYTE, pfiPixelGet_RED_UBYTE),
    ENTRY(PF_RED, PF_HALF_FLOAT, pfiPixelGet_RED_HALF),
    ENTRY(PF_RED, PF_FLOAT, pfiPixelGet_RED_FLOAT),

    ENTRY(PF_GREEN, PF_UNSIGNED_BYTE, pfiPixelGet_GREEN_UBYTE),
    ENTRY(PF_GREEN, PF_HALF_FLOAT, pfiPixelGet_GREEN_HALF),
    ENTRY(PF_GREEN, PF_FLOAT, pfiPixelGet_GREEN_FLOAT),

    ENTRY(PF_BLUE, PF_UNSIGNED_BYTE, pfiPixelGet_BLUE_UBYTE),
    ENTRY(PF_BLUE, PF_HALF_FLOAT, pfiPixelGet_BLUE_HALF),
    ENTRY(PF_BLUE, PF_FLOAT, pfiPixelGet_BLUE_FLOAT),

    ENTRY(PF_ALPHA, PF_UNSIGNED_BYTE, pfiPixelGet_ALPHA_UBYTE),
    ENTRY(PF_ALPHA, PF_HALF_FLOAT, pfiPixelGet_ALPHA_HALF),
    ENTRY(PF_ALPHA, PF_FLOAT, pfiPixelGet_ALPHA_FLOAT),

    ENTRY(PF_LUMINANCE, PF_UNSIGNED_BYTE, pfiPixelGet_Luminance_UBYTE),
    ENTRY(PF_LUMINANCE, PF_HALF_FLOAT, pfiPixelGet_Luminance_HALF),
    ENTRY(PF_LUMINANCE, PF_FLOAT, pfiPixelGet_Luminance_FLOAT),

    ENTRY(PF_LUMINANCE_ALPHA, PF_UNSIGNED_BYTE, pfiPixelGet_Luminance_Alpha_UBYTE),
    ENTRY(PF_LUMINANCE_ALPHA, PF_HALF_FLOAT, pfiPixelGet_Luminance_Alpha_HALF),
    ENTRY(PF_LUMINANCE_ALPHA, PF_FLOAT, pfiPixelGet_Luminance_Alpha_FLOAT),

    ENTRY(PF_RGB, PF_UNSIGNED_BYTE, pfiPixelGet_RGB_UBYTE),
    ENTRY(PF_RGB, PF_UNSIGNED_SHORT_5_6_5, pfiPixelGet_RGB_USHORT_5_6_5),
    ENTRY(PF_RGB, PF_HALF_FLOAT, pfiPixelGet_RGB_HALF),
    ENTRY(PF_RGB, PF_FLOAT, pfiPixelGet_RGB_FLOAT),

    ENTRY(PF_RGBA, PF_UNSIGNED_BYTE, pfiPixelGet_RGBA_UBYTE),
    ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_5_5_5_1, pfiPixelGet_RGBA_USHORT_5_5_5_1),
    ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_4_4_4_4, pfiPixelGet_RGBA_USHORT_4_4_4_4),
    ENTRY(PF_RGBA, PF_HALF_FLOAT, pfiPixelGet_RGBA_HALF),
    ENTRY(PF_RGBA, PF_FLOAT, pfiPixelGet_RGBA_FLOAT),

    ENTRY(PF_BGR, PF_UNSIGNED_BYTE, pfiPixelGet_BGR_UBYTE),
    ENTRY(PF_BGR, PF_UNSIGNED_SHORT_5_6_5, pfiPixelGet_BGR_USHORT_5_6_5),
    ENTRY(PF_BGR, PF_HALF_FLOAT, pfiPixelGet_BGR_HALF),
    ENTRY(PF_BGR, PF_FLOAT, pfiPixelGet_BGR_FLOAT),

    ENTRY(PF_BGRA, PF_UNSIGNED_BYTE, pfiPixelGet_BGRA_UBYTE),
    ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_5_5_5_1, pfiPixelGet_BGRA_USHORT_5_5_5_1),
    ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_4_4_4_4, pfiPixelGet_BGRA_USHORT_4_4_4_4),
    ENTRY(PF_BGRA, PF_HALF_FLOAT, pfiPixelGet_BGRA_HALF),
    ENTRY(PF_BGRA, PF_FLOAT, pfiPixelGet_BGRA_FLOAT),
};

static const PFpixelsetter GC_pixelSetters[10][12] = {
    ENTRY(PF_RED, PF_UNSIGNED_BYTE, pfiPixelSet_RED_UBYTE),
    ENTRY(PF_RED, PF_HALF_FLOAT, pfiPixelSet_RED_HALF),
    ENTRY(PF_RED, PF_FLOAT, pfiPixelSet_RED_FLOAT),

    ENTRY(PF_GREEN, PF_UNSIGNED_BYTE, pfiPixelSet_GREEN_UBYTE),
    ENTRY(PF_GREEN, PF_HALF_FLOAT, pfiPixelSet_GREEN_HALF),
    ENTRY(PF_GREEN, PF_FLOAT, pfiPixelSet_GREEN_FLOAT),

    ENTRY(PF_BLUE, PF_UNSIGNED_BYTE, pfiPixelSet_BLUE_UBYTE),
    ENTRY(PF_BLUE, PF_HALF_FLOAT, pfiPixelSet_BLUE_HALF),
    ENTRY(PF_BLUE, PF_FLOAT, pfiPixelSet_BLUE_FLOAT),

    ENTRY(PF_ALPHA, PF_UNSIGNED_BYTE, pfiPixelSet_ALPHA_UBYTE),
    ENTRY(PF_ALPHA, PF_HALF_FLOAT, pfiPixelSet_ALPHA_HALF),
    ENTRY(PF_ALPHA, PF_FLOAT, pfiPixelSet_ALPHA_FLOAT),

    ENTRY(PF_LUMINANCE, PF_UNSIGNED_BYTE, pfiPixelSet_Luminance_UBYTE),
    ENTRY(PF_LUMINANCE, PF_HALF_FLOAT, pfiPixelSet_Luminance_HALF),
    ENTRY(PF_LUMINANCE, PF_FLOAT, pfiPixelSet_Luminance_FLOAT),

    ENTRY(PF_LUMINANCE_ALPHA, PF_UNSIGNED_BYTE, pfiPixelSet_Luminance_Alpha_UBYTE),
    ENTRY(PF_LUMINANCE_ALPHA, PF_HALF_FLOAT, pfiPixelSet_Luminance_Alpha_HALF),
    ENTRY(PF_LUMINANCE_ALPHA, PF_FLOAT, pfiPixelSet_Luminance_Alpha_FLOAT),

    ENTRY(PF_RGB, PF_UNSIGNED_BYTE, pfiPixelSet_RGB_UBYTE),
    ENTRY(PF_RGB, PF_UNSIGNED_SHORT_5_6_5, pfiPixelSet_RGB_USHORT_5_6_5),
    ENTRY(PF_RGB, PF_HALF_FLOAT, pfiPixelSet_RGB_HALF),
    ENTRY(PF_RGB, PF_FLOAT, pfiPixelSet_RGB_FLOAT),

    ENTRY(PF_RGBA, PF_UNSIGNED_BYTE, pfiPixelSet_RGBA_UBYTE),
    ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_5_5_5_1, pfiPixelSet_RGBA_USHORT_5_5_5_1),
    ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_4_4_4_4, pfiPixelSet_RGBA_USHORT_4_4_4_4),
    ENTRY(PF_RGBA, PF_HALF_FLOAT, pfiPixelSet_RGBA_HALF),
    ENTRY(PF_RGBA, PF_FLOAT, pfiPixelSet_RGBA_FLOAT),

    ENTRY(PF_BGR, PF_UNSIGNED_BYTE, pfiPixelSet_BGR_UBYTE),
    ENTRY(PF_BGR, PF_UNSIGNED_SHORT_5_6_5, pfiPixelSet_BGR_USHORT_5_6_5),
    ENTRY(PF_BGR, PF_HALF_FLOAT, pfiPixelSet_BGR_HALF),
    ENTRY(PF_BGR, PF_FLOAT, pfiPixelSet_BGR_FLOAT),

    ENTRY(PF_BGRA, PF_UNSIGNED_BYTE, pfiPixelSet_BGRA_UBYTE),
    ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_5_5_5_1, pfiPixelSet_BGRA_USHORT_5_5_5_1),
    ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_4_4_4_4, pfiPixelSet_BGRA_USHORT_4_4_4_4),
    ENTRY(PF_BGRA, PF_HALF_FLOAT, pfiPixelSet_BGRA_HALF),
    ENTRY(PF_BGRA, PF_FLOAT, pfiPixelSet_BGRA_FLOAT),
};

#undef ENTRY




/* -- SIMD IMPLEMENTATION -- */

#if PF_SIMD_SUPPORT

/* SET LUMINANCE */

static inline void
pfiPixelSet_Luminance_UBYTE_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    colors = pfiColorPackedGrayscale_simd(colors);
    PFsimdvi luminance = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x000000FF));

#   define WRITE_LUMINANCE_PIXEL(index) \
    { \
        PFuint lumValue = pfiSimdExtract_I32(luminance, index) & 0xFFFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFFFF; \
        PFubyte* targetPixel = (PFubyte*)pixels + offset + index; \
        *targetPixel = ((*targetPixel) & ~maskValue) | (lumValue & maskValue); \
    }

    WRITE_LUMINANCE_PIXEL(0);
    WRITE_LUMINANCE_PIXEL(1);

#ifdef __SSE2__
    WRITE_LUMINANCE_PIXEL(2);
    WRITE_LUMINANCE_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_LUMINANCE_PIXEL(4);
    WRITE_LUMINANCE_PIXEL(5);
    WRITE_LUMINANCE_PIXEL(6);
    WRITE_LUMINANCE_PIXEL(7);
#endif //__AVX2__

#undef WRITE_LUMINANCE_PIXEL
}

static inline void
pfiPixelSet_Luminance_HALF_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    colors = pfiColorPackedGrayscale_simd(colors);
    PFsimdvi luminance = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x000000FF));

#   define WRITE_LUMINANCE_PIXEL(index) \
    { \
        PFuint lumValue = pfiSimdExtract_I32(luminance, index) & 0xFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFF; \
        PFushort* targetPixel = (PFushort*)pixels + offset + index; \
        PFubyte currentPixelValue = pfmHalfToFloat(*targetPixel)*255.0f; \
        PFubyte finalColor = (currentPixelValue & ~maskValue) | (lumValue & maskValue); \
        *targetPixel = pfmFloatToHalf((PFfloat)finalColor/255.0f); \
    }

    WRITE_LUMINANCE_PIXEL(0);
    WRITE_LUMINANCE_PIXEL(1);

#ifdef __SSE2__
    WRITE_LUMINANCE_PIXEL(2);
    WRITE_LUMINANCE_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_LUMINANCE_PIXEL(4);
    WRITE_LUMINANCE_PIXEL(5);
    WRITE_LUMINANCE_PIXEL(6);
    WRITE_LUMINANCE_PIXEL(7);
#endif //__AVX2__

#undef WRITE_LUMINANCE_PIXEL
}

static inline void
pfiPixelSet_Luminance_FLOAT_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    colors = pfiColorPackedGrayscale_simd(colors);
    PFsimdvi luminance = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x000000FF));

#   define WRITE_LUMINANCE_PIXEL(index) \
    { \
        PFuint lumValue = pfiSimdExtract_I32(luminance, index) & 0xFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFF; \
        PFfloat* targetPixel = (PFfloat*)pixels + offset + index; \
        PFubyte currentPixelValue = (*targetPixel)*255.0f; \
        PFubyte finalColor = (currentPixelValue & ~maskValue) | (lumValue & maskValue); \
        *targetPixel = (PFfloat)finalColor/255.0f; \
    }

    WRITE_LUMINANCE_PIXEL(0);
    WRITE_LUMINANCE_PIXEL(1);

#ifdef __SSE2__
    WRITE_LUMINANCE_PIXEL(2);
    WRITE_LUMINANCE_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_LUMINANCE_PIXEL(4);
    WRITE_LUMINANCE_PIXEL(5);
    WRITE_LUMINANCE_PIXEL(6);
    WRITE_LUMINANCE_PIXEL(7);
#endif //__AVX2__

#undef WRITE_LUMINANCE_PIXEL
}


/* SET LUMINANCE ALPHA */

static inline void
pfiPixelSet_Luminance_Alpha_UBYTE_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    colors = pfiColorPackedGrayscale_simd(colors);

    PFsimdvi luminance = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x000000FF));
    PFsimdvi alpha = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0xFF000000));
    alpha = pfiSimdShr_I32(alpha, 16);

    PFsimdvi lumAlpha = pfiSimdOr_I32(luminance, alpha);

#   define WRITE_LUMINANCE_ALPHA_PIXEL(index) \
    { \
        PFuint lumAlphaValue = pfiSimdExtract_I32(lumAlpha, index) & 0xFFFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFFFF; \
        PFushort* targetPixel = (PFushort*)pixels + offset + index; \
        *targetPixel = ((*targetPixel) & ~maskValue) | (lumAlphaValue & maskValue); \
    }

    WRITE_LUMINANCE_ALPHA_PIXEL(0);
    WRITE_LUMINANCE_ALPHA_PIXEL(1);

#ifdef __SSE2__
    WRITE_LUMINANCE_ALPHA_PIXEL(2);
    WRITE_LUMINANCE_ALPHA_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_LUMINANCE_ALPHA_PIXEL(4);
    WRITE_LUMINANCE_ALPHA_PIXEL(5);
    WRITE_LUMINANCE_ALPHA_PIXEL(6);
    WRITE_LUMINANCE_ALPHA_PIXEL(7);
#endif //__AVX2__

#undef WRITE_LUMINANCE_ALPHA_PIXEL
}

static inline void
pfiPixelSet_Luminance_Alpha_HALF_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{

}

static inline void
pfiPixelSet_Luminance_Alpha_FLOAT_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{

}


/* SET RED/GREEN/BLUE/ALPHA */

static inline void
pfiPixelSet_RED_UBYTE_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvi red = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x000000FF));

#   define WRITE_RED_PIXEL(index) \
    { \
        PFuint redValue = pfiSimdExtract_I32(red, index) & 0xFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFF; \
        PFubyte* targetPixel = (PFubyte*)pixels + offset + index; \
        *targetPixel = ((*targetPixel) & ~maskValue) | (redValue & maskValue); \
    }

    WRITE_RED_PIXEL(0);
    WRITE_RED_PIXEL(1);

#ifdef __SSE2__
    WRITE_RED_PIXEL(2);
    WRITE_RED_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_RED_PIXEL(4);
    WRITE_RED_PIXEL(5);
    WRITE_RED_PIXEL(6);
    WRITE_RED_PIXEL(7);
#endif //__AVX2__

#undef WRITE_RED_PIXEL
}

static inline void
pfiPixelSet_GREEN_UBYTE_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvi green = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x0000FF00));
    green = pfiSimdShr_I32(green, 8);

#   define WRITE_GREEN_PIXEL(index) \
    { \
        PFuint greenValue = pfiSimdExtract_I32(green, index) & 0xFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFF; \
        PFubyte* targetPixel = (PFubyte*)pixels + offset + index; \
        *targetPixel = (PFubyte)((greenValue & maskValue) | ((*targetPixel) & ~maskValue)); \
    }

    WRITE_GREEN_PIXEL(0);
    WRITE_GREEN_PIXEL(1);

#ifdef __SSE2__
    WRITE_GREEN_PIXEL(2);
    WRITE_GREEN_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_GREEN_PIXEL(4);
    WRITE_GREEN_PIXEL(5);
    WRITE_GREEN_PIXEL(6);
    WRITE_GREEN_PIXEL(7);
#endif //__AVX2__

#undef WRITE_GREEN_PIXEL
}

static inline void
pfiPixelSet_BLUE_UBYTE_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvi blue = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x00FF0000));
    blue = pfiSimdShr_I32(blue, 16);

#   define WRITE_BLUE_PIXEL(index) \
    { \
        PFuint blueValue = pfiSimdExtract_I32(blue, index) & 0xFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFF; \
        PFubyte* targetPixel = (PFubyte*)pixels + offset + index; \
        *targetPixel = (PFubyte)((blueValue & maskValue) | ((*targetPixel) & ~maskValue)); \
    }

    WRITE_BLUE_PIXEL(0);
    WRITE_BLUE_PIXEL(1);

#ifdef __SSE2__
    WRITE_BLUE_PIXEL(2);
    WRITE_BLUE_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_BLUE_PIXEL(4);
    WRITE_BLUE_PIXEL(5);
    WRITE_BLUE_PIXEL(6);
    WRITE_BLUE_PIXEL(7);
#endif //__AVX2__

#undef WRITE_BLUE_PIXEL
}

static inline void
pfiPixelSet_ALPHA_UBYTE_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvi alpha = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0xFF000000));
    alpha = pfiSimdShr_I32(alpha, 24);

#   define WRITE_ALPHA_PIXEL(index) \
    { \
        PFuint alphaValue = pfiSimdExtract_I32(alpha, index) & 0xFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFF; \
        PFubyte* targetPixel = (PFubyte*)pixels + offset + index; \
        *targetPixel = (PFubyte)((alphaValue & maskValue) | ((*targetPixel) & ~maskValue)); \
    }

    WRITE_ALPHA_PIXEL(0);
    WRITE_ALPHA_PIXEL(1);

#ifdef __SSE2__
    WRITE_ALPHA_PIXEL(2);
    WRITE_ALPHA_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_ALPHA_PIXEL(4);
    WRITE_ALPHA_PIXEL(5);
    WRITE_ALPHA_PIXEL(6);
    WRITE_ALPHA_PIXEL(7);
#endif //__AVX2__

#undef WRITE_ALPHA_PIXEL
}

static inline void
pfiPixelSet_RED_HALF_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvi red = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x000000FF));

#   define WRITE_RED_PIXEL(index) \
    { \
        PFuint redValue = pfiSimdExtract_I32(red, index) & 0xFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFF; \
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
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_RED_PIXEL(4);
    WRITE_RED_PIXEL(5);
    WRITE_RED_PIXEL(6);
    WRITE_RED_PIXEL(7);
#endif //__AVX2__

#undef WRITE_RED_PIXEL
}

static inline void
pfiPixelSet_GREEN_HALF_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvi green = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x0000FF00));
    green = pfiSimdShr_I32(green, 8);

#   define WRITE_GREEN_PIXEL(index) \
    { \
        PFuint greenValue = pfiSimdExtract_I32(green, index) & 0xFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFF; \
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
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_GREEN_PIXEL(4);
    WRITE_GREEN_PIXEL(5);
    WRITE_GREEN_PIXEL(6);
    WRITE_GREEN_PIXEL(7);
#endif //__AVX2__

#undef WRITE_GREEN_PIXEL
}

static inline void
pfiPixelSet_BLUE_HALF_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvi blue = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x00FF0000));
    blue = pfiSimdShr_I32(blue, 16);

#   define WRITE_BLUE_PIXEL(index) \
    { \
        PFuint blueValue = pfiSimdExtract_I32(blue, index) & 0xFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFF; \
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
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_BLUE_PIXEL(4);
    WRITE_BLUE_PIXEL(5);
    WRITE_BLUE_PIXEL(6);
    WRITE_BLUE_PIXEL(7);
#endif //__AVX2__

#undef WRITE_BLUE_PIXEL
}

static inline void
pfiPixelSet_ALPHA_HALF_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvi alpha = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0xFF000000));
    alpha = pfiSimdShr_I32(alpha, 24);

#   define WRITE_ALPHA_PIXEL(index) \
    { \
        PFuint alphaValue = pfiSimdExtract_I32(alpha, index) & 0xFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFF; \
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
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_ALPHA_PIXEL(4);
    WRITE_ALPHA_PIXEL(5);
    WRITE_ALPHA_PIXEL(6);
    WRITE_ALPHA_PIXEL(7);
#endif //__AVX2__

#undef WRITE_ALPHA_PIXEL
}

static inline void
pfiPixelSet_RED_FLOAT_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvi red = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x000000FF));

#   define WRITE_RED_PIXEL(index) \
    { \
        PFuint redValue = pfiSimdExtract_I32(red, index) & 0xFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFF; \
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
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_RED_PIXEL(4);
    WRITE_RED_PIXEL(5);
    WRITE_RED_PIXEL(6);
    WRITE_RED_PIXEL(7);
#endif //__AVX2__

#undef WRITE_RED_PIXEL
}

static inline void
pfiPixelSet_GREEN_FLOAT_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvi green = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x0000FF00));
    green = pfiSimdShr_I32(green, 8);

#   define WRITE_GREEN_PIXEL(index) \
    { \
        PFuint greenValue = pfiSimdExtract_I32(green, index) & 0xFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFF; \
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
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_GREEN_PIXEL(4);
    WRITE_GREEN_PIXEL(5);
    WRITE_GREEN_PIXEL(6);
    WRITE_GREEN_PIXEL(7);
#endif //__AVX2__

#undef WRITE_GREEN_PIXEL
}

static inline void
pfiPixelSet_BLUE_FLOAT_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvi blue = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x00FF0000));
    blue = pfiSimdShr_I32(blue, 16);

#   define WRITE_BLUE_PIXEL(index) \
    { \
        PFuint blueValue = pfiSimdExtract_I32(blue, index) & 0xFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFF; \
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
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_BLUE_PIXEL(4);
    WRITE_BLUE_PIXEL(5);
    WRITE_BLUE_PIXEL(6);
    WRITE_BLUE_PIXEL(7);
#endif //__AVX2__

#undef WRITE_BLUE_PIXEL
}

static inline void
pfiPixelSet_ALPHA_FLOAT_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvi alpha = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0xFF000000));
    alpha = pfiSimdShr_I32(alpha, 24);

#   define WRITE_ALPHA_PIXEL(index) \
    { \
        PFuint alphaValue = pfiSimdExtract_I32(alpha, index) & 0xFF; \
        PFuint maskValue = pfiSimdExtract_I32(mask, index) & 0xFF; \
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
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_ALPHA_PIXEL(4);
    WRITE_ALPHA_PIXEL(5);
    WRITE_ALPHA_PIXEL(6);
    WRITE_ALPHA_PIXEL(7);
#endif //__AVX2__

#undef WRITE_ALPHA_PIXEL
}


/* SET RGB/BGR */

static inline void
pfiPixelSet_RGB_USHORT_5_6_5_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    const PFsimdvi scale5 = pfiSimdSet1_I32(31);       // To convert to 5 bits (2^5 - 1)
    const PFsimdvi scale6 = pfiSimdSet1_I32(63);       // To convert to 6 bits (2^6 - 1)

    PFsimdvi r = pfiSimdAnd_I32(colors, *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi g = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi b = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFsimdvi*)GC_simd_i32_255);

    PFsimdvi r5 = pfiSimdShr_I32(pfiSimdMullo_I32(r, scale5), 8);
    PFsimdvi g6 = pfiSimdShr_I32(pfiSimdMullo_I32(g, scale6), 8);
    PFsimdvi b5 = pfiSimdShr_I32(pfiSimdMullo_I32(b, scale5), 8);

    PFsimdvi rgb565 = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdShl_I32(r5, 11),
            pfiSimdShl_I32(g6, 5)
        ),
        b5
    );

#   define WRITE_RGB565_PIXEL(index) \
    { \
        uint32_t rgbValue = pfiSimdExtract_I32(rgb565, index); \
        uint32_t maskValue = pfiSimdExtract_I32(mask, index); \
        PFushort* targetPixel = (PFushort*)pixels + offset + index; \
        uint16_t currentPixelValue = *targetPixel; \
        *targetPixel = (uint16_t)((currentPixelValue & ~maskValue) | (rgbValue & maskValue)); \
    }

    WRITE_RGB565_PIXEL(0);
    WRITE_RGB565_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGB565_PIXEL(2);
    WRITE_RGB565_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_RGB565_PIXEL(4);
    WRITE_RGB565_PIXEL(5);
    WRITE_RGB565_PIXEL(6);
    WRITE_RGB565_PIXEL(7);
#endif //__AVX2__

#undef WRITE_RGB565_PIXEL
}

static inline void
pfiPixelSet_BGR_USHORT_5_6_5_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    const PFsimdvi scale5 = pfiSimdSet1_I32(31);       // To convert to 5 bits (2^5 - 1)
    const PFsimdvi scale6 = pfiSimdSet1_I32(63);       // To convert to 6 bits (2^6 - 1)

    PFsimdvi b = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi g = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi r = pfiSimdAnd_I32(colors, *(PFsimdvi*)GC_simd_i32_255);

    PFsimdvi b5 = pfiSimdShr_I32(pfiSimdMullo_I32(b, scale5), 8);
    PFsimdvi g6 = pfiSimdShr_I32(pfiSimdMullo_I32(g, scale6), 8);
    PFsimdvi r5 = pfiSimdShr_I32(pfiSimdMullo_I32(r, scale5), 8);

    PFsimdvi rgb565 = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdShl_I32(b5, 11),
            pfiSimdShl_I32(g6, 5)
        ),
        r5
    );

#   define WRITE_RGB565_PIXEL(index) \
    { \
        uint32_t rgbValue = pfiSimdExtract_I32(rgb565, index); \
        uint32_t maskValue = pfiSimdExtract_I32(mask, index); \
        PFushort* targetPixel = (PFushort*)pixels + offset + index; \
        uint16_t currentPixelValue = *targetPixel; \
        *targetPixel = (uint16_t)((currentPixelValue & ~maskValue) | (rgbValue & maskValue)); \
    }

    WRITE_RGB565_PIXEL(0);
    WRITE_RGB565_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGB565_PIXEL(2);
    WRITE_RGB565_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_RGB565_PIXEL(4);
    WRITE_RGB565_PIXEL(5);
    WRITE_RGB565_PIXEL(6);
    WRITE_RGB565_PIXEL(7);
#endif //__AVX2__

#undef WRITE_RGB565_PIXEL
}

static inline void
pfiPixelSet_RGB_UBYTE_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    // Extract the RGB components from the RGBA colors (ignoring the alpha)
    PFsimdvi red = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x00FF0000));
    PFsimdvi green = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x0000FF00));
    PFsimdvi blue = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x000000FF));

    // Reverse the shifts to get the RGB values in the correct order
    red = pfiSimdShr_I32(red, 16);        // Red should remain in the least significant 8 bits
    green = pfiSimdShr_I32(green, 8);     // Shift green so it is in the middle bits

    // Combine the components into a 24-bit RGB format
    PFsimdvi rgb24 = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdShl_I32(blue, 16),     // Blue in the most significant 8 bits
            pfiSimdShl_I32(green, 8)      // Green in the middle 8 bits
        ),
        red  // Red in the least significant 8 bits
    );

#define WRITE_RGB_PIXEL(index) \
    { \
        PFuint pixelColor = pfiSimdExtract_I32(rgb24, index); \
        PFuint maskValue = pfiSimdExtract_I32(mask, index); \
        PFubyte* targetPixel = (PFubyte*)pixels + 3 * (offset + index); \
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
pfiPixelSet_BGR_UBYTE_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    // Extract the BGR components from the RGBA colors (ignoring the alpha)
    PFsimdvi blue = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x00FF0000));
    PFsimdvi green = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x0000FF00));
    PFsimdvi red = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0x000000FF));

    // Shift the components to obtain the BGR values
    blue = pfiSimdShr_I32(blue, 16);      // Shift blue to the correct 8 bits
    green = pfiSimdShr_I32(green, 8);     // Shift green to the correct 8 bits

    // Combine the components into a 24-bit BGR format
    PFsimdvi bgr24 = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdShl_I32(blue, 16),    // Position blue
            pfiSimdShl_I32(green, 8)     // Position green
        ),
        red // Position red
    );

#   define WRITE_RGB_PIXEL(index) \
    { \
        PFuint pixelColor = pfiSimdExtract_I32(bgr24, index); \
        PFuint maskValue = pfiSimdExtract_I32(mask, index); \
        PFubyte* targetPixel = (PFubyte*)pixels + 3 * (offset + index); \
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
pfiPixelSet_RGB_HALF_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvf r = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(colors, *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf g = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf b = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFsimdvi*)GC_simd_i32_255));

    r = pfiSimdMul_F32(r, *(PFsimdvf*)GC_simd_f32_inv255);
    g = pfiSimdMul_F32(g, *(PFsimdvf*)GC_simd_f32_inv255);
    b = pfiSimdMul_F32(b, *(PFsimdvf*)GC_simd_f32_inv255);

#define WRITE_RGB_FLOAT_PIXEL(index) \
    { \
        uint16_t* targetPixel = (uint16_t*)pixels + 3 * (offset + index); \
        if (pfiSimdExtract_I32(mask, index) != 0) { \
            targetPixel[0] = pfmFloatToHalf(pfiSimdExtract_F32(r, index)); \
            targetPixel[1] = pfmFloatToHalf(pfiSimdExtract_F32(g, index)); \
            targetPixel[2] = pfmFloatToHalf(pfiSimdExtract_F32(b, index)); \
        } \
    }

    WRITE_RGB_FLOAT_PIXEL(0);
    WRITE_RGB_FLOAT_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGB_FLOAT_PIXEL(2);
    WRITE_RGB_FLOAT_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_RGB_FLOAT_PIXEL(4);
    WRITE_RGB_FLOAT_PIXEL(5);
    WRITE_RGB_FLOAT_PIXEL(6);
    WRITE_RGB_FLOAT_PIXEL(7);
#endif //__AVX2__

#undef WRITE_RGB_FLOAT_PIXEL
}

static inline void
pfiPixelSet_BGR_HALF_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvf b = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(colors, *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf g = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf r = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFsimdvi*)GC_simd_i32_255));

    b = pfiSimdMul_F32(b, *(PFsimdvf*)GC_simd_f32_inv255);
    g = pfiSimdMul_F32(g, *(PFsimdvf*)GC_simd_f32_inv255);
    r = pfiSimdMul_F32(r, *(PFsimdvf*)GC_simd_f32_inv255);

#define WRITE_RGB_FLOAT_PIXEL(index) \
    { \
        uint16_t* targetPixel = (uint16_t*)pixels + 3 * (offset + index); \
        if (pfiSimdExtract_I32(mask, index) != 0) { \
            targetPixel[0] = pfmFloatToHalf(pfiSimdExtract_F32(b, index)); \
            targetPixel[1] = pfmFloatToHalf(pfiSimdExtract_F32(g, index)); \
            targetPixel[2] = pfmFloatToHalf(pfiSimdExtract_F32(r, index)); \
        } \
    }

    WRITE_RGB_FLOAT_PIXEL(0);
    WRITE_RGB_FLOAT_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGB_FLOAT_PIXEL(2);
    WRITE_RGB_FLOAT_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_RGB_FLOAT_PIXEL(4);
    WRITE_RGB_FLOAT_PIXEL(5);
    WRITE_RGB_FLOAT_PIXEL(6);
    WRITE_RGB_FLOAT_PIXEL(7);
#endif //__AVX2__

#undef WRITE_RGB_FLOAT_PIXEL
}

static inline void
pfiPixelSet_RGB_FLOAT_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvf r = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(colors, *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf g = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf b = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFsimdvi*)GC_simd_i32_255));

    r = pfiSimdMul_F32(r, *(PFsimdvf*)GC_simd_f32_inv255);
    g = pfiSimdMul_F32(g, *(PFsimdvf*)GC_simd_f32_inv255);
    b = pfiSimdMul_F32(b, *(PFsimdvf*)GC_simd_f32_inv255);

#define WRITE_RGB_FLOAT_PIXEL(index) \
    { \
        float* targetPixel = (float*)pixels + 3 * (offset + index); \
        if (pfiSimdExtract_I32(mask, index) != 0) { \
            targetPixel[0] = pfiSimdExtract_F32(r, index); \
            targetPixel[1] = pfiSimdExtract_F32(g, index); \
            targetPixel[2] = pfiSimdExtract_F32(b, index); \
        } \
    }

    WRITE_RGB_FLOAT_PIXEL(0);
    WRITE_RGB_FLOAT_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGB_FLOAT_PIXEL(2);
    WRITE_RGB_FLOAT_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_RGB_FLOAT_PIXEL(4);
    WRITE_RGB_FLOAT_PIXEL(5);
    WRITE_RGB_FLOAT_PIXEL(6);
    WRITE_RGB_FLOAT_PIXEL(7);
#endif //__AVX2__

#undef WRITE_RGB_FLOAT_PIXEL
}

static inline void
pfiPixelSet_BGR_FLOAT_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvf b = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(colors, *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf g = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf r = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFsimdvi*)GC_simd_i32_255));

    b = pfiSimdMul_F32(b, *(PFsimdvf*)GC_simd_f32_inv255);
    g = pfiSimdMul_F32(g, *(PFsimdvf*)GC_simd_f32_inv255);
    r = pfiSimdMul_F32(r, *(PFsimdvf*)GC_simd_f32_inv255);

#define WRITE_RGB_FLOAT_PIXEL(index) \
    { \
        float* targetPixel = (float*)pixels + 3 * (offset + index); \
        if (pfiSimdExtract_I32(mask, index) != 0) { \
            targetPixel[0] = pfiSimdExtract_F32(b, index); \
            targetPixel[1] = pfiSimdExtract_F32(g, index); \
            targetPixel[2] = pfiSimdExtract_F32(r, index); \
        } \
    }

    WRITE_RGB_FLOAT_PIXEL(0);
    WRITE_RGB_FLOAT_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGB_FLOAT_PIXEL(2);
    WRITE_RGB_FLOAT_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_RGB_FLOAT_PIXEL(4);
    WRITE_RGB_FLOAT_PIXEL(5);
    WRITE_RGB_FLOAT_PIXEL(6);
    WRITE_RGB_FLOAT_PIXEL(7);
#endif //__AVX2__

#undef WRITE_RGB_FLOAT_PIXEL
}


/* SET RGBA/BGRA */

static inline void
pfiPixelSet_RGBA_USHORT_5_5_5_1_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    const PFsimdvi scale5 = pfiSimdSet1_I32(31);    // To convert to 5 bits (2^5 - 1)
    const PFsimdvi scale1 = pfiSimdSet1_I32(1);     // To convert alpha to 1 bit

    PFsimdvi r = pfiSimdAnd_I32(colors, *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi g = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi b = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi a = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 24), *(PFsimdvi*)GC_simd_i32_255);

    PFsimdvi r5 = pfiSimdShr_I32(pfiSimdMullo_I32(r, scale5), 8);
    PFsimdvi g5 = pfiSimdShr_I32(pfiSimdMullo_I32(g, scale5), 8);
    PFsimdvi b5 = pfiSimdShr_I32(pfiSimdMullo_I32(b, scale5), 8);
    PFsimdvi a1 = pfiSimdShr_I32(pfiSimdCmpGT_I32(a, pfiSimdSet1_I32(127)), 31);

    PFsimdvi rgba5551 = pfiSimdOr_I32(
        pfiSimdShl_I32(r5, 11),
        pfiSimdOr_I32(
            pfiSimdShl_I32(g5, 6),
            pfiSimdOr_I32(
                pfiSimdShl_I32(b5, 1),
                a1
            )
        )
    );

#   define WRITE_RGBA5551_PIXEL(index) \
    { \
        uint32_t rgbaValue = pfiSimdExtract_I32(rgba5551, index); \
        uint32_t maskValue = pfiSimdExtract_I32(mask, index); \
        PFushort* targetPixel = (PFushort*)pixels + offset + index; \
        *targetPixel = (uint16_t)(((*targetPixel) & ~maskValue) | (rgbaValue & maskValue)); \
    }

    WRITE_RGBA5551_PIXEL(0);
    WRITE_RGBA5551_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGBA5551_PIXEL(2);
    WRITE_RGBA5551_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_RGBA5551_PIXEL(4);
    WRITE_RGBA5551_PIXEL(5);
    WRITE_RGBA5551_PIXEL(6);
    WRITE_RGBA5551_PIXEL(7);
#endif //__AVX2__

#undef WRITE_RGBA5551_PIXEL
}

static inline void
pfiPixelSet_BGRA_USHORT_5_5_5_1_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    const PFsimdvi scale5 = pfiSimdSet1_I32(31);    // To convert to 5 bits (2^5 - 1)
    const PFsimdvi scale1 = pfiSimdSet1_I32(1);     // To convert alpha to 1 bit

    PFsimdvi b = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi g = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi r = pfiSimdAnd_I32(colors, *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi a = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 24), *(PFsimdvi*)GC_simd_i32_255);

    PFsimdvi b5 = pfiSimdShr_I32(pfiSimdMullo_I32(b, scale5), 8);
    PFsimdvi g5 = pfiSimdShr_I32(pfiSimdMullo_I32(g, scale5), 8);
    PFsimdvi r5 = pfiSimdShr_I32(pfiSimdMullo_I32(r, scale5), 8);
    PFsimdvi a1 = pfiSimdShr_I32(pfiSimdCmpGT_I32(a, pfiSimdSet1_I32(127)), 31);

    PFsimdvi rgba5551 = pfiSimdOr_I32(
        pfiSimdShl_I32(b5, 11),
        pfiSimdOr_I32(
            pfiSimdShl_I32(g5, 6),
            pfiSimdOr_I32(
                pfiSimdShl_I32(r5, 1),
                a1
            )
        )
    );

#   define WRITE_RGBA5551_PIXEL(index) \
    { \
        uint32_t rgbaValue = pfiSimdExtract_I32(rgba5551, index); \
        uint32_t maskValue = pfiSimdExtract_I32(mask, index); \
        PFushort* targetPixel = (PFushort*)pixels + offset + index; \
        *targetPixel = (uint16_t)(((*targetPixel) & ~maskValue) | (rgbaValue & maskValue)); \
    }

    WRITE_RGBA5551_PIXEL(0);
    WRITE_RGBA5551_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGBA5551_PIXEL(2);
    WRITE_RGBA5551_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_RGBA5551_PIXEL(4);
    WRITE_RGBA5551_PIXEL(5);
    WRITE_RGBA5551_PIXEL(6);
    WRITE_RGBA5551_PIXEL(7);
#endif //__AVX2__

#undef WRITE_RGBA5551_PIXEL
}

static inline void
pfiPixelSet_RGBA_USHORT_4_4_4_4_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    const PFsimdvi scale4 = pfiSimdSet1_I32(15);    // To convert to 4 bits (2^4 - 1)

    PFsimdvi r = pfiSimdAnd_I32(colors, *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi g = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi b = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi a = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 24), *(PFsimdvi*)GC_simd_i32_255);

    PFsimdvi r4 = pfiSimdShr_I32(pfiSimdMullo_I32(r, scale4), 8);
    PFsimdvi g4 = pfiSimdShr_I32(pfiSimdMullo_I32(g, scale4), 8);
    PFsimdvi b4 = pfiSimdShr_I32(pfiSimdMullo_I32(b, scale4), 8);
    PFsimdvi a4 = pfiSimdShr_I32(pfiSimdMullo_I32(a, scale4), 8);

    PFsimdvi rgba4444 = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdOr_I32(
                pfiSimdShl_I32(r4, 12),
                pfiSimdShl_I32(g4, 8)
            ),
            pfiSimdShl_I32(b4, 4)
        ),
        a4
    );

#   define WRITE_RGBA4444_PIXEL(index) \
    { \
        uint32_t rgbaValue = pfiSimdExtract_I32(rgba4444, index); \
        uint32_t maskValue = pfiSimdExtract_I32(mask, index); \
        PFushort* targetPixel = (PFushort*)pixels + offset + index; \
        uint16_t currentPixelValue = *targetPixel; \
        *targetPixel = (uint16_t)((currentPixelValue & ~maskValue) | (rgbaValue & maskValue)); \
    }

    WRITE_RGBA4444_PIXEL(0);
    WRITE_RGBA4444_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGBA4444_PIXEL(2);
    WRITE_RGBA4444_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_RGBA4444_PIXEL(4);
    WRITE_RGBA4444_PIXEL(5);
    WRITE_RGBA4444_PIXEL(6);
    WRITE_RGBA4444_PIXEL(7);
#endif //__AVX2__

#undef WRITE_RGBA4444_PIXEL
}

static inline void
pfiPixelSet_BGRA_USHORT_4_4_4_4_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    const PFsimdvi scale4 = pfiSimdSet1_I32(15);    // To convert to 4 bits (2^4 - 1)

    PFsimdvi b = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi g = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi r = pfiSimdAnd_I32(colors, *(PFsimdvi*)GC_simd_i32_255);
    PFsimdvi a = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 24), *(PFsimdvi*)GC_simd_i32_255);

    PFsimdvi b4 = pfiSimdShr_I32(pfiSimdMullo_I32(b, scale4), 8);
    PFsimdvi g4 = pfiSimdShr_I32(pfiSimdMullo_I32(g, scale4), 8);
    PFsimdvi r4 = pfiSimdShr_I32(pfiSimdMullo_I32(r, scale4), 8);
    PFsimdvi a4 = pfiSimdShr_I32(pfiSimdMullo_I32(a, scale4), 8);

    PFsimdvi rgba4444 = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdOr_I32(
                pfiSimdShl_I32(b4, 12),
                pfiSimdShl_I32(g4, 8)
            ),
            pfiSimdShl_I32(r4, 4)
        ),
        a4
    );

#   define WRITE_RGBA4444_PIXEL(index) \
    { \
        uint32_t rgbaValue = pfiSimdExtract_I32(rgba4444, index); \
        uint32_t maskValue = pfiSimdExtract_I32(mask, index); \
        PFushort* targetPixel = (PFushort*)pixels + offset + index; \
        uint16_t currentPixelValue = *targetPixel; \
        *targetPixel = (uint16_t)((currentPixelValue & ~maskValue) | (rgbaValue & maskValue)); \
    }

    WRITE_RGBA4444_PIXEL(0);
    WRITE_RGBA4444_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGBA4444_PIXEL(2);
    WRITE_RGBA4444_PIXEL(3);
#endif //__SSE2__

#ifdef __AVX2__
    WRITE_RGBA4444_PIXEL(4);
    WRITE_RGBA4444_PIXEL(5);
    WRITE_RGBA4444_PIXEL(6);
    WRITE_RGBA4444_PIXEL(7);
#endif //__AVX2__

#undef WRITE_RGBA4444_PIXEL
}

static inline void
pfiPixelSet_RGBA_UBYTE_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvi pixelsColors = pfiSimdLoad_I32((PFuint*)pixels + offset);
    PFsimdvi maskedColors = pfiSimdBlendV_I8(pixelsColors, colors, mask);

    pfiSimdStore_I32((PFuint*)pixels + offset, maskedColors);
}

static inline void
pfiPixelSet_BGRA_UBYTE_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvi bgraColors = pfiSimdShuffle_I8(colors, pfiSimdSetR_x4_I8(2, 1, 0, 3));

    PFsimdvi pixelsColors = pfiSimdLoad_I32((PFuint*)pixels + offset);
    PFsimdvi maskedColors = pfiSimdBlendV_I8(pixelsColors, bgraColors, mask);

    pfiSimdStore_I32((PFuint*)pixels + offset, maskedColors);
}

static inline void
pfiPixelSet_RGBA_HALF_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvf r = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(colors, *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf g = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf b = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf a = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 24), *(PFsimdvi*)GC_simd_i32_255));

    r = pfiSimdMul_F32(r, *(PFsimdvf*)GC_simd_f32_inv255);
    g = pfiSimdMul_F32(g, *(PFsimdvf*)GC_simd_f32_inv255);
    b = pfiSimdMul_F32(b, *(PFsimdvf*)GC_simd_f32_inv255);
    a = pfiSimdMul_F32(a, *(PFsimdvf*)GC_simd_f32_inv255);

#   define WRITE_RGBA_FLOAT_PIXEL(index) \
    { \
        if (pfiSimdExtract_I32(mask, index) & 0xFFFFFFFF) { \
            uint16_t* targetPixel = (uint16_t*)pixels + 4 * (offset + index); \
            targetPixel[0] = pfmFloatToHalf(pfiSimdExtract_F32(r, index)); \
            targetPixel[1] = pfmFloatToHalf(pfiSimdExtract_F32(g, index)); \
            targetPixel[2] = pfmFloatToHalf(pfiSimdExtract_F32(b, index)); \
            targetPixel[3] = pfmFloatToHalf(pfiSimdExtract_F32(a, index)); \
        } \
    }

    WRITE_RGBA_FLOAT_PIXEL(0);
    WRITE_RGBA_FLOAT_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGBA_FLOAT_PIXEL(2);
    WRITE_RGBA_FLOAT_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_RGBA_FLOAT_PIXEL(4);
    WRITE_RGBA_FLOAT_PIXEL(5);
    WRITE_RGBA_FLOAT_PIXEL(6);
    WRITE_RGBA_FLOAT_PIXEL(7);
#endif // __AVX2__

#undef WRITE_RGBA_FLOAT_PIXEL
}

static inline void
pfiPixelSet_BGRA_HALF_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvf b = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf g = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf r = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(colors, *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf a = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 24), *(PFsimdvi*)GC_simd_i32_255));

    b = pfiSimdMul_F32(b, *(PFsimdvf*)GC_simd_f32_inv255);
    g = pfiSimdMul_F32(g, *(PFsimdvf*)GC_simd_f32_inv255);
    r = pfiSimdMul_F32(r, *(PFsimdvf*)GC_simd_f32_inv255);
    a = pfiSimdMul_F32(a, *(PFsimdvf*)GC_simd_f32_inv255);

#   define WRITE_RGBA_FLOAT_PIXEL(index) \
    { \
        if (pfiSimdExtract_I32(mask, index) & 0xFFFFFFFF) { \
            uint16_t* targetPixel = (uint16_t*)pixels + 4 * (offset + index); \
            targetPixel[0] = pfmFloatToHalf(pfiSimdExtract_F32(b, index)); \
            targetPixel[1] = pfmFloatToHalf(pfiSimdExtract_F32(g, index)); \
            targetPixel[2] = pfmFloatToHalf(pfiSimdExtract_F32(r, index)); \
            targetPixel[3] = pfmFloatToHalf(pfiSimdExtract_F32(a, index)); \
        } \
    }

    WRITE_RGBA_FLOAT_PIXEL(0);
    WRITE_RGBA_FLOAT_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGBA_FLOAT_PIXEL(2);
    WRITE_RGBA_FLOAT_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_RGBA_FLOAT_PIXEL(4);
    WRITE_RGBA_FLOAT_PIXEL(5);
    WRITE_RGBA_FLOAT_PIXEL(6);
    WRITE_RGBA_FLOAT_PIXEL(7);
#endif // __AVX2__

#undef WRITE_RGBA_FLOAT_PIXEL
}

static inline void
pfiPixelSet_RGBA_FLOAT_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvf r = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(colors, *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf g = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf b = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf a = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 24), *(PFsimdvi*)GC_simd_i32_255));

    r = pfiSimdMul_F32(r, *(PFsimdvf*)GC_simd_f32_inv255);
    g = pfiSimdMul_F32(g, *(PFsimdvf*)GC_simd_f32_inv255);
    b = pfiSimdMul_F32(b, *(PFsimdvf*)GC_simd_f32_inv255);
    a = pfiSimdMul_F32(a, *(PFsimdvf*)GC_simd_f32_inv255);

#   define WRITE_RGBA_FLOAT_PIXEL(index) \
    { \
        if (pfiSimdExtract_I32(mask, index) & 0xFFFFFFFF) { \
            PFfloat* targetPixel = (PFfloat*)pixels + 4 * (offset + index); \
            targetPixel[0] = pfiSimdExtract_F32(r, index); \
            targetPixel[1] = pfiSimdExtract_F32(g, index); \
            targetPixel[2] = pfiSimdExtract_F32(b, index); \
            targetPixel[3] = pfiSimdExtract_F32(a, index); \
        } \
    }

    WRITE_RGBA_FLOAT_PIXEL(0);
    WRITE_RGBA_FLOAT_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGBA_FLOAT_PIXEL(2);
    WRITE_RGBA_FLOAT_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_RGBA_FLOAT_PIXEL(4);
    WRITE_RGBA_FLOAT_PIXEL(5);
    WRITE_RGBA_FLOAT_PIXEL(6);
    WRITE_RGBA_FLOAT_PIXEL(7);
#endif // __AVX2__

#undef WRITE_RGBA_FLOAT_PIXEL
}


static inline void
pfiPixelSet_BGRA_FLOAT_simd(void* pixels, PFsizei offset, PFsimdvi colors, PFsimdvi mask)
{
    PFsimdvf b = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf g = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf r = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(colors, *(PFsimdvi*)GC_simd_i32_255));
    PFsimdvf a = pfiSimdConvert_I32_F32(pfiSimdAnd_I32(pfiSimdShr_I32(colors, 24), *(PFsimdvi*)GC_simd_i32_255));

    b = pfiSimdMul_F32(b, *(PFsimdvf*)GC_simd_f32_inv255);
    g = pfiSimdMul_F32(g, *(PFsimdvf*)GC_simd_f32_inv255);
    r = pfiSimdMul_F32(r, *(PFsimdvf*)GC_simd_f32_inv255);
    a = pfiSimdMul_F32(a, *(PFsimdvf*)GC_simd_f32_inv255);

#   define WRITE_RGBA_FLOAT_PIXEL(index) \
    { \
        if (pfiSimdExtract_I32(mask, index) & 0xFFFFFFFF) { \
            PFfloat* targetPixel = (PFfloat*)pixels + 4 * (offset + index); \
            targetPixel[0] = pfiSimdExtract_F32(b, index); \
            targetPixel[1] = pfiSimdExtract_F32(g, index); \
            targetPixel[2] = pfiSimdExtract_F32(r, index); \
            targetPixel[3] = pfiSimdExtract_F32(a, index); \
        } \
    }

    WRITE_RGBA_FLOAT_PIXEL(0);
    WRITE_RGBA_FLOAT_PIXEL(1);

#ifdef __SSE2__
    WRITE_RGBA_FLOAT_PIXEL(2);
    WRITE_RGBA_FLOAT_PIXEL(3);
#endif // __SSE2__

#ifdef __AVX2__
    WRITE_RGBA_FLOAT_PIXEL(4);
    WRITE_RGBA_FLOAT_PIXEL(5);
    WRITE_RGBA_FLOAT_PIXEL(6);
    WRITE_RGBA_FLOAT_PIXEL(7);
#endif // __AVX2__

#undef WRITE_RGBA_FLOAT_PIXEL
}


/* GET LUMINANCE */

static inline PFsimdvi
pfiPixelGet_Luminance_UBYTE_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi lum = pfiSimdGather_I32(pixels, offsets, sizeof(PFubyte));

    lum = pfiSimdOr_I32(pfiSimdShr_I32(lum, 24), pfiSimdSet1_I32(0xFF000000));
    PFsimdvi lastByte = pfiSimdAnd_I32(lum, *(PFsimdvi*)GC_simd_i32_255);
    lum = pfiSimdOr_I32(lum, pfiSimdShl_I32(lastByte, 16));
    lum = pfiSimdOr_I32(lum, pfiSimdShl_I32(lastByte, 8));

    return lum;
}

static inline PFsimdvi
pfiPixelGet_Luminance_HALF_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi lum = pfiSimdGather_I32(pixels, offsets, sizeof(PFushort));

    lum = pfiSimdConvert_F32_I32(pfiSimdMul_F32(
        pfiSimdConvert_F16_F32(lum),
        *(PFsimdvf*)GC_simd_f32_255));

    lum = pfiSimdOr_I32(pfiSimdShr_I32(lum, 24), pfiSimdSet1_I32(0xFF000000));
    PFsimdvi lastByte = pfiSimdAnd_I32(lum, *(PFsimdvi*)GC_simd_i32_255);
    lum = pfiSimdOr_I32(lum, pfiSimdShl_I32(lastByte, 16));
    lum = pfiSimdOr_I32(lum, pfiSimdShl_I32(lastByte, 8));

    return lum;
}

static inline PFsimdvi
pfiPixelGet_Luminance_FLOAT_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi lum = pfiSimdGather_I32(pixels, offsets, sizeof(PFfloat));

    lum = pfiSimdConvert_F32_I32(
        pfiSimdMul_F32(pfiSimdCast_I32_F32(lum),
        *(PFsimdvf*)GC_simd_f32_255));

    lum = pfiSimdOr_I32(pfiSimdShr_I32(lum, 24), pfiSimdSet1_I32(0xFF000000));
    PFsimdvi lastByte = pfiSimdAnd_I32(lum, *(PFsimdvi*)GC_simd_i32_255);
    lum = pfiSimdOr_I32(lum, pfiSimdShl_I32(lastByte, 16));
    lum = pfiSimdOr_I32(lum, pfiSimdShl_I32(lastByte, 8));

    return lum;
}


/* GET LUMINANCE ALPHA */

static inline PFsimdvi
pfiPixelGet_Luminance_Alpha_UBYTE_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi lumAlpha = pfiSimdGather_I32(
        pixels, offsets, 2 * sizeof(PFubyte));

    PFsimdvi mask = pfiSimdSetR_I8(
         2,  2,  2,  3,
         6,  6,  6,  7,
        10, 10, 10, 11,
        14, 14, 14, 15,
        18, 18, 18, 19,
        22, 22, 22, 23,
        26, 26, 26, 27,
        30, 30, 30, 31);

    return pfiSimdShuffle_I8(lumAlpha, mask);
}

static inline PFsimdvi
pfiPixelGet_Luminance_Alpha_HALF_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi result = { 0 };

    return result;
}

static inline PFsimdvi
pfiPixelGet_Luminance_Alpha_FLOAT_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi result = { 0 };

    return result;
}


/* GET RED/GREEN/BLUE/ALPHA */

static inline PFsimdvi
pfiPixelGet_RED_UBYTE_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi red = pfiSimdGather_I32(pixels, offsets, sizeof(PFubyte));

    return pfiSimdOr_I32(
        pfiSimdShr_I32(red, 24),
        pfiSimdSet1_I32(0xFF000000));
}


static inline PFsimdvi
pfiPixelGet_GREEN_UBYTE_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi green = pfiSimdGather_I32(pixels, offsets, sizeof(PFubyte));

    return pfiSimdOr_I32(
        pfiSimdShl_I32(pfiSimdShr_I32(green, 24), 8),
        pfiSimdSet1_I32(0xFF000000));
}

static inline PFsimdvi
pfiPixelGet_BLUE_UBYTE_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi blue = pfiSimdGather_I32(pixels, offsets, sizeof(PFubyte));

    return pfiSimdOr_I32(
        pfiSimdShl_I32(pfiSimdShr_I32(blue, 24), 16),
        pfiSimdSet1_I32(0xFF000000));
}

static inline PFsimdvi
pfiPixelGet_ALPHA_UBYTE_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi alpha = pfiSimdGather_I32(pixels, offsets, sizeof(PFubyte));
    return pfiSimdShl_I32(alpha, 24);
}

static inline PFsimdvi
pfiPixelGet_RED_HALF_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi red = pfiSimdGather_I32(pixels, offsets, sizeof(PFushort));

    red = pfiSimdConvert_F32_I32(pfiSimdMul_F32(
        pfiSimdConvert_F16_F32(red),
        *(PFsimdvf*)GC_simd_f32_255));

    return pfiSimdOr_I32(red,
        pfiSimdSet1_I32(0xFF000000));
}

static inline PFsimdvi
pfiPixelGet_GREEN_HALF_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi green = pfiSimdGather_I32(pixels, offsets, sizeof(PFushort));

    green = pfiSimdConvert_F32_I32(pfiSimdMul_F32(
        pfiSimdConvert_F16_F32(green),
        *(PFsimdvf*)GC_simd_f32_255));

    return pfiSimdOr_I32(
        pfiSimdShl_I32(green, 8),
        pfiSimdSet1_I32(0xFF000000));
}

static inline PFsimdvi
pfiPixelGet_BLUE_HALF_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi blue = pfiSimdGather_I32(pixels, offsets, sizeof(PFushort));

    blue = pfiSimdConvert_F32_I32(pfiSimdMul_F32(
        pfiSimdConvert_F16_F32(blue),
        *(PFsimdvf*)GC_simd_f32_255));

    return pfiSimdOr_I32(
        pfiSimdShl_I32(blue, 16),
        pfiSimdSet1_I32(0xFF000000));
}

static inline PFsimdvi
pfiPixelGet_ALPHA_HALF_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi alpha = pfiSimdGather_I32(pixels, offsets, sizeof(PFushort));

    alpha = pfiSimdConvert_F32_I32(pfiSimdMul_F32(
        pfiSimdConvert_F16_F32(alpha),
        *(PFsimdvf*)GC_simd_f32_255));

    return pfiSimdShl_I32(alpha, 24);
}

static inline PFsimdvi
pfiPixelGet_RED_FLOAT_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi red = pfiSimdGather_I32(pixels, offsets, sizeof(PFfloat));

    red = pfiSimdConvert_F32_I32(
        pfiSimdMul_F32(pfiSimdCast_I32_F32(red),
        *(PFsimdvf*)GC_simd_f32_255));

    return pfiSimdOr_I32(red,
        pfiSimdSet1_I32(0xFF000000));
}

static inline PFsimdvi
pfiPixelGet_GREEN_FLOAT_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi green = pfiSimdGather_I32(pixels, offsets, sizeof(PFfloat));

    green = pfiSimdConvert_F32_I32(
        pfiSimdMul_F32(pfiSimdCast_I32_F32(green),
        *(PFsimdvf*)GC_simd_f32_255));

    return pfiSimdOr_I32(
        pfiSimdShl_I32(green, 8),
        pfiSimdSet1_I32(0xFF000000));
}

static inline PFsimdvi
pfiPixelGet_BLUE_FLOAT_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi blue = pfiSimdGather_I32(pixels, offsets, sizeof(PFfloat));

    blue = pfiSimdConvert_F32_I32(
        pfiSimdMul_F32(pfiSimdCast_I32_F32(blue),
        *(PFsimdvf*)GC_simd_f32_255));

    return pfiSimdOr_I32(
        pfiSimdShl_I32(blue, 16),
        pfiSimdSet1_I32(0xFF000000));
}

static inline PFsimdvi
pfiPixelGet_ALPHA_FLOAT_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi alpha = pfiSimdGather_I32(pixels, offsets, sizeof(PFfloat));

    alpha = pfiSimdConvert_F32_I32(
        pfiSimdMul_F32(pfiSimdCast_I32_F32(alpha),
        *(PFsimdvf*)GC_simd_f32_255));

    return pfiSimdShl_I32(alpha, 24);
}


/* GET RGB/BGR */

static inline PFsimdvi
pfiPixelGet_RGB_USHORT_5_6_5_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi rgb565 = pfiSimdGather_I32(pixels, offsets, sizeof(PFushort));

    const PFsimdvi maskR = pfiSimdSet1_I32(0xF800);
    const PFsimdvi maskG = pfiSimdSet1_I32(0x07E0);
    const PFsimdvi maskB = pfiSimdSet1_I32(0x001F);

    PFsimdvi r = pfiSimdAnd_I32(rgb565, maskR);
    r = pfiSimdShr_I32(r, 11);
    r = pfiSimdShl_I32(r, 3);

    PFsimdvi g = pfiSimdAnd_I32(rgb565, maskG);
    g = pfiSimdShr_I32(g, 5);
    g = pfiSimdShl_I32(g, 2);

    PFsimdvi b = pfiSimdAnd_I32(rgb565, maskB);
    b = pfiSimdShl_I32(b, 3);

    PFsimdvi a = pfiSimdSet1_I32(0xFF000000);

    PFsimdvi rgba = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdOr_I32(pfiSimdShl_I32(r, 16), pfiSimdShl_I32(g, 8)),
            b),
        a);

    return rgba;
}

static inline PFsimdvi
pfiPixelGet_BGR_USHORT_5_6_5_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi bgr565 = pfiSimdGather_I32(pixels, offsets, sizeof(PFushort));

    const PFsimdvi maskB = pfiSimdSet1_I32(0xF800);
    const PFsimdvi maskG = pfiSimdSet1_I32(0x07E0);
    const PFsimdvi maskR = pfiSimdSet1_I32(0x001F);

    PFsimdvi b = pfiSimdAnd_I32(bgr565, maskB);
    b = pfiSimdShl_I32(b, 8 - 11);

    PFsimdvi g = pfiSimdAnd_I32(bgr565, maskG);
    g = pfiSimdShl_I32(g, 8 - 5);
    g = pfiSimdShr_I32(g, 5);

    PFsimdvi r = pfiSimdAnd_I32(bgr565, maskR);
    r = pfiSimdShl_I32(r, 8 - 0);
    r = pfiSimdShr_I32(r, 11);

    PFsimdvi a = pfiSimdSet1_I32(0xFF000000);

    PFsimdvi rgba = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdOr_I32(b, pfiSimdShl_I32(g, 8)),
            pfiSimdShl_I32(r, 16)),
        a);

    return rgba;
}

static inline PFsimdvi
pfiPixelGet_RGB_UBYTE_simd(const void* pixels, PFsimdvi offsets)
{
    // Read the 24-bit RGB values from the buffer.
    PFsimdvi rgb24 = pfiSimdGather_I32(pixels, pfiSimdMullo_I32(offsets, pfiSimdSet1_I32(3 * sizeof(PFubyte))), 1);

    // Extract the RGB components using the appropriate masks
    PFsimdvi red = pfiSimdAnd_I32(rgb24, pfiSimdSet1_I32(0xFF0000));
    PFsimdvi green = pfiSimdAnd_I32(rgb24, pfiSimdSet1_I32(0x00FF00));
    PFsimdvi blue = pfiSimdAnd_I32(rgb24, pfiSimdSet1_I32(0x0000FF));

    // Shift the values to obtain the 8-bit RGB components
    red = pfiSimdShr_I32(red, 16);   // Shift red to get the 8-bit red component
    green = pfiSimdShr_I32(green, 8); // Shift green to get the 8-bit green component
    // Blue is already in place in the least significant 8 bits

    // Create the alpha component with a value of 0xFF
    PFsimdvi alpha = *(PFsimdvi*)GC_simd_i32_255;

    // Combine the RGB components and alpha into a single RGBA value
    PFsimdvi rgba32 = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdOr_I32(
                pfiSimdShl_I32(red, 16),   // Red in the most significant 8 bits
                pfiSimdShl_I32(green, 8) // Green in the next 8 bits
            ),
            blue // Blue in the next 8 bits
        ),
        pfiSimdShl_I32(alpha, 24) // Alpha in the least significant 8 bits
    );

    return rgba32;
}

static inline PFsimdvi
pfiPixelGet_BGR_UBYTE_simd(const void* pixels, PFsimdvi offsets)
{
    // Read the 24-bit BGR values from the buffer
    PFsimdvi bgr24 = pfiSimdGather_I32(pixels, pfiSimdMullo_I32(offsets, pfiSimdSet1_I32(3 * sizeof(PFubyte))), 1);

    // Extract the BGR components using the appropriate masks
    PFsimdvi blue = pfiSimdAnd_I32(bgr24, pfiSimdSet1_I32(0xFF0000));
    PFsimdvi green = pfiSimdAnd_I32(bgr24, pfiSimdSet1_I32(0x00FF00));
    PFsimdvi red = pfiSimdAnd_I32(bgr24, pfiSimdSet1_I32(0x0000FF));

    // Shift the values to obtain the 8-bit RGB components
    blue = pfiSimdShr_I32(blue, 16);      // Shift blue to get the 8-bit blue component
    green = pfiSimdShr_I32(green, 8);     // Shift green to get the 8-bit green component
    // Red is already in place in the least significant 8 bits

    // Create the alpha component with a value of 0xFF
    PFsimdvi alpha = *(PFsimdvi*)GC_simd_i32_255;

    // Combine the RGB components and alpha into a single RGBA value
    PFsimdvi rgba32 = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdOr_I32(
                pfiSimdShl_I32(red, 16),   // Red in the most significant 8 bits
                pfiSimdShl_I32(green, 8)  // Green in the next 8 bits
            ),
            blue // Blue in the least significant 8 bits
        ),
        pfiSimdShl_I32(alpha, 24) // Alpha in the most significant 8 bits
    );

    return rgba32;
}

static inline PFsimdvi
pfiPixelGet_RGB_HALF_simd(const void* pixels, PFsimdvi offsets)
{
    // Calculate scaled offsets
    PFsimdvi scaledOffset = pfiSimdMullo_I32(offsets, pfiSimdSet1_I32(3));

    // Read the float RGB values from the buffer
    PFsimdvf r = pfiSimdConvert_F16_F32(pfiSimdGather_I32(pixels, scaledOffset, sizeof(uint16_t)));
    PFsimdvf g = pfiSimdConvert_F16_F32(pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(1)), sizeof(uint16_t)));
    PFsimdvf b = pfiSimdConvert_F16_F32(pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(2)), sizeof(uint16_t)));

    // Convert float values to 8-bit integer values (0-255)
    PFsimdvi ri = pfiSimdConvert_F32_I32(pfiSimdMul_F32(r, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi gi = pfiSimdConvert_F32_I32(pfiSimdMul_F32(g, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi bi = pfiSimdConvert_F32_I32(pfiSimdMul_F32(b, *(PFsimdvf*)GC_simd_f32_255));

    // Combine the RGB components and alpha into a single RGBA value
    PFsimdvi rgba = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdSet1_I32(0xFF000000),
            pfiSimdShl_I32(bi, 16)
        ),
        pfiSimdOr_I32(
            pfiSimdShl_I32(gi, 8),
            ri
        )
    );

    return rgba;
}

static inline PFsimdvi
pfiPixelGet_BGR_HALF_simd(const void* pixels, PFsimdvi offsets)
{
    // Calculate scaled offsets
    PFsimdvi scaledOffset = pfiSimdMullo_I32(offsets, pfiSimdSet1_I32(3));

    // Read the float BGR values from the buffer
    PFsimdvf b = pfiSimdConvert_F16_F32(pfiSimdGather_I32(pixels, scaledOffset, sizeof(uint16_t)));
    PFsimdvf g = pfiSimdConvert_F16_F32(pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(1)), sizeof(uint16_t)));
    PFsimdvf r = pfiSimdConvert_F16_F32(pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(2)), sizeof(uint16_t)));

    // Convert float values to 8-bit integer values (0-255)
    PFsimdvi bi = pfiSimdConvert_F32_I32(pfiSimdMul_F32(b, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi gi = pfiSimdConvert_F32_I32(pfiSimdMul_F32(g, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi ri = pfiSimdConvert_F32_I32(pfiSimdMul_F32(r, *(PFsimdvf*)GC_simd_f32_255));

    // Combine the BGR components and alpha into a single BGRA value
    PFsimdvi bgra = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdSet1_I32(0xFF000000),
            pfiSimdShl_I32(ri, 16)
        ),
        pfiSimdOr_I32(
            pfiSimdShl_I32(gi, 8),
            bi
        )
    );

    return bgra;
}

static inline PFsimdvi
pfiPixelGet_RGB_FLOAT_simd(const void* pixels, PFsimdvi offsets)
{
    // Calculate scaled offsets
    PFsimdvi scaledOffset = pfiSimdMullo_I32(offsets, pfiSimdSet1_I32(3));

    // Read the float RGB values from the buffer
    PFsimdvf r = pfiSimdCast_I32_F32(pfiSimdGather_I32(pixels, scaledOffset, sizeof(PFfloat)));
    PFsimdvf g = pfiSimdCast_I32_F32(pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(1)), sizeof(PFfloat)));
    PFsimdvf b = pfiSimdCast_I32_F32(pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(2)), sizeof(PFfloat)));

    // Convert float values to 8-bit integer values (0-255)
    PFsimdvi ri = pfiSimdConvert_F32_I32(pfiSimdMul_F32(r, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi gi = pfiSimdConvert_F32_I32(pfiSimdMul_F32(g, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi bi = pfiSimdConvert_F32_I32(pfiSimdMul_F32(b, *(PFsimdvf*)GC_simd_f32_255));

    // Combine the RGB components and alpha into a single RGBA value
    PFsimdvi rgba = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdSet1_I32(0xFF000000),
            pfiSimdShl_I32(bi, 16)
        ),
        pfiSimdOr_I32(
            pfiSimdShl_I32(gi, 8),
            ri
        )
    );

    return rgba;
}

static inline PFsimdvi
pfiPixelGet_BGR_FLOAT_simd(const void* pixels, PFsimdvi offsets)
{
    // Calculate scaled offsets
    PFsimdvi scaledOffset = pfiSimdMullo_I32(offsets, pfiSimdSet1_I32(3));

    // Read the float BGR values from the buffer
    PFsimdvf b = pfiSimdCast_I32_F32(pfiSimdGather_I32(pixels, scaledOffset, sizeof(PFfloat)));
    PFsimdvf g = pfiSimdCast_I32_F32(pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(1)), sizeof(PFfloat)));
    PFsimdvf r = pfiSimdCast_I32_F32(pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(2)), sizeof(PFfloat)));

    // Convert float values to 8-bit integer values (0-255)
    PFsimdvi bi = pfiSimdConvert_F32_I32(pfiSimdMul_F32(b, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi gi = pfiSimdConvert_F32_I32(pfiSimdMul_F32(g, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi ri = pfiSimdConvert_F32_I32(pfiSimdMul_F32(r, *(PFsimdvf*)GC_simd_f32_255));

    // Combine the BGR components and alpha into a single BGRA value
    PFsimdvi bgra = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdSet1_I32(0xFF000000),
            pfiSimdShl_I32(ri, 16)
        ),
        pfiSimdOr_I32(
            pfiSimdShl_I32(gi, 8),
            bi
        )
    );

    return bgra;
}



/* GET RGBA/BGRA */

static inline PFsimdvi
pfiPixelGet_RGBA_USHORT_5_5_5_1_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi gathered = pfiSimdGather_I32(pixels, offsets, sizeof(PFushort));

    PFsimdvi r = pfiSimdAnd_I32(pfiSimdShr_I32(gathered, 11), pfiSimdSet1_I32(0x1F));
    PFsimdvi g = pfiSimdAnd_I32(pfiSimdShr_I32(gathered, 6), pfiSimdSet1_I32(0x1F));
    PFsimdvi b = pfiSimdAnd_I32(pfiSimdShr_I32(gathered, 1), pfiSimdSet1_I32(0x1F));
    PFsimdvi a = pfiSimdAnd_I32(gathered, pfiSimdSet1_I32(0x1));

    PFsimdvi r8 = pfiSimdMullo_I32(r, pfiSimdSet1_I32(255 / 31));
    PFsimdvi g8 = pfiSimdMullo_I32(g, pfiSimdSet1_I32(255 / 31));
    PFsimdvi b8 = pfiSimdMullo_I32(b, pfiSimdSet1_I32(255 / 31));
    PFsimdvi a8 = pfiSimdMullo_I32(a, pfiSimdSet1_I32(255));

    return pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdOr_I32(pfiSimdShl_I32(r8, 0), pfiSimdShl_I32(g8, 8)),
            pfiSimdShl_I32(b8, 16)
        ),
        pfiSimdShl_I32(a8, 24)
    );
}

static inline PFsimdvi
pfiPixelGet_BGRA_USHORT_5_5_5_1_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi gathered = pfiSimdGather_I32(pixels, offsets, sizeof(PFushort));

    PFsimdvi b = pfiSimdAnd_I32(pfiSimdShr_I32(gathered, 11), pfiSimdSet1_I32(0x1F));
    PFsimdvi g = pfiSimdAnd_I32(pfiSimdShr_I32(gathered, 6), pfiSimdSet1_I32(0x1F));
    PFsimdvi r = pfiSimdAnd_I32(pfiSimdShr_I32(gathered, 1), pfiSimdSet1_I32(0x1F));
    PFsimdvi a = pfiSimdAnd_I32(gathered, pfiSimdSet1_I32(0x1));

    PFsimdvi b8 = pfiSimdMullo_I32(b, pfiSimdSet1_I32(255 / 31));
    PFsimdvi g8 = pfiSimdMullo_I32(g, pfiSimdSet1_I32(255 / 31));
    PFsimdvi r8 = pfiSimdMullo_I32(r, pfiSimdSet1_I32(255 / 31));
    PFsimdvi a8 = pfiSimdMullo_I32(a, pfiSimdSet1_I32(255));

    return pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdOr_I32(pfiSimdShl_I32(r8, 0), pfiSimdShl_I32(g8, 8)),
            pfiSimdShl_I32(b8, 16)
        ),
        pfiSimdShl_I32(a8, 24)
    );
}

static inline PFsimdvi
pfiPixelGet_RGBA_USHORT_4_4_4_4_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi gathered = pfiSimdGather_I32(pixels, offsets, sizeof(PFushort));

    PFsimdvi r = pfiSimdAnd_I32(pfiSimdShr_I32(gathered, 12), pfiSimdSet1_I32(0xF));
    PFsimdvi g = pfiSimdAnd_I32(pfiSimdShr_I32(gathered, 8), pfiSimdSet1_I32(0xF));
    PFsimdvi b = pfiSimdAnd_I32(pfiSimdShr_I32(gathered, 4), pfiSimdSet1_I32(0xF));
    PFsimdvi a = pfiSimdAnd_I32(gathered, pfiSimdSet1_I32(0xF));

    PFsimdvi r8 = pfiSimdMullo_I32(r, pfiSimdSet1_I32(255 / 15));
    PFsimdvi g8 = pfiSimdMullo_I32(g, pfiSimdSet1_I32(255 / 15));
    PFsimdvi b8 = pfiSimdMullo_I32(b, pfiSimdSet1_I32(255 / 15));
    PFsimdvi a8 = pfiSimdMullo_I32(a, pfiSimdSet1_I32(255 / 15));

    return pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdOr_I32(pfiSimdShl_I32(r8, 0), pfiSimdShl_I32(g8, 8)),
            pfiSimdShl_I32(b8, 16)
        ),
        pfiSimdShl_I32(a8, 24)
    );
}

static inline PFsimdvi
pfiPixelGet_BGRA_USHORT_4_4_4_4_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi gathered = pfiSimdGather_I32(pixels, offsets, sizeof(PFushort));

    PFsimdvi b = pfiSimdAnd_I32(pfiSimdShr_I32(gathered, 4), pfiSimdSet1_I32(0xF));
    PFsimdvi g = pfiSimdAnd_I32(pfiSimdShr_I32(gathered, 8), pfiSimdSet1_I32(0xF));
    PFsimdvi r = pfiSimdAnd_I32(pfiSimdShr_I32(gathered, 12), pfiSimdSet1_I32(0xF));
    PFsimdvi a = pfiSimdAnd_I32(gathered, pfiSimdSet1_I32(0xF));

    PFsimdvi b8 = pfiSimdMullo_I32(b, pfiSimdSet1_I32(255 / 15));
    PFsimdvi g8 = pfiSimdMullo_I32(g, pfiSimdSet1_I32(255 / 15));
    PFsimdvi r8 = pfiSimdMullo_I32(r, pfiSimdSet1_I32(255 / 15));
    PFsimdvi a8 = pfiSimdMullo_I32(a, pfiSimdSet1_I32(255 / 15));

    return pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdOr_I32(pfiSimdShl_I32(r8, 0), pfiSimdShl_I32(g8, 8)),
            pfiSimdShl_I32(b8, 16)
        ),
        pfiSimdShl_I32(a8, 24)
    );
}

static inline PFsimdvi
pfiPixelGet_RGBA_UBYTE_simd(const void* pixels, PFsimdvi offsets)
{
    return pfiSimdGather_I32(pixels, offsets, sizeof(PFuint));
}

static inline PFsimdvi
pfiPixelGet_BGRA_UBYTE_simd(const void* pixels, PFsimdvi offsets)
{
    PFsimdvi result = pfiSimdGather_I32(pixels, offsets, sizeof(PFuint));
    return pfiSimdShuffle_I8(result, pfiSimdSetR_x4_I8(2, 1, 0, 3));
}

static inline PFsimdvi
pfiPixelGet_RGBA_HALF_simd(const void* pixels, PFsimdvi offsets)
{
    const PFsimdvi scaledOffset = pfiSimdMullo_I32(
        offsets, pfiSimdSet1_I32(4));

    PFsimdvf r = pfiSimdConvert_F16_F32(pfiSimdGather_I32(pixels, scaledOffset, sizeof(uint16_t)));
    PFsimdvf g = pfiSimdConvert_F16_F32(pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(1)), sizeof(uint16_t)));
    PFsimdvf b = pfiSimdConvert_F16_F32(pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(2)), sizeof(uint16_t)));
    PFsimdvf a = pfiSimdConvert_F16_F32(pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(3)), sizeof(uint16_t)));

    PFsimdvi ri = pfiSimdConvert_F32_I32(pfiSimdMul_F32(r, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi gi = pfiSimdConvert_F32_I32(pfiSimdMul_F32(g, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi bi = pfiSimdConvert_F32_I32(pfiSimdMul_F32(b, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi ai = pfiSimdConvert_F32_I32(pfiSimdMul_F32(a, *(PFsimdvf*)GC_simd_f32_255));

    PFsimdvi rgba = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdShl_I32(ai, 24),
            pfiSimdShl_I32(bi, 16)
        ),
        pfiSimdOr_I32(
            pfiSimdShl_I32(gi, 8),
            ri
        )
    );

    return rgba;
}

static inline PFsimdvi
pfiPixelGet_BGRA_HALF_simd(const void* pixels, PFsimdvi offsets)
{
    const PFsimdvi scaledOffset = pfiSimdMullo_I32(
        offsets, pfiSimdSet1_I32(4));

    PFsimdvf b = pfiSimdConvert_F16_F32(pfiSimdGather_I32(pixels, scaledOffset, sizeof(uint16_t)));
    PFsimdvf g = pfiSimdConvert_F16_F32(pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(1)), sizeof(uint16_t)));
    PFsimdvf r = pfiSimdConvert_F16_F32(pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(2)), sizeof(uint16_t)));
    PFsimdvf a = pfiSimdConvert_F16_F32(pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(3)), sizeof(uint16_t)));

    PFsimdvi bi = pfiSimdConvert_F32_I32(pfiSimdMul_F32(b, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi gi = pfiSimdConvert_F32_I32(pfiSimdMul_F32(g, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi ri = pfiSimdConvert_F32_I32(pfiSimdMul_F32(r, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi ai = pfiSimdConvert_F32_I32(pfiSimdMul_F32(a, *(PFsimdvf*)GC_simd_f32_255));

    PFsimdvi bgra = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdShl_I32(ai, 24),
            pfiSimdShl_I32(bi, 16)
        ),
        pfiSimdOr_I32(
            pfiSimdShl_I32(gi, 8),
            ri
        )
    );

    return bgra;
}

static inline PFsimdvi
pfiPixelGet_RGBA_FLOAT_simd(const void* pixels, PFsimdvi offsets)
{
    const PFsimdvi scaledOffset = pfiSimdMullo_I32(
        offsets, pfiSimdSet1_I32(4));

    PFsimdvi r = pfiSimdGather_I32(pixels, scaledOffset, sizeof(PFfloat));
    PFsimdvi g = pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(1)), sizeof(PFfloat));
    PFsimdvi b = pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(2)), sizeof(PFfloat));
    PFsimdvi a = pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(3)), sizeof(PFfloat));

    r = pfiSimdConvert_F32_I32(pfiSimdMul_F32(pfiSimdCast_I32_F32(r), *(PFsimdvf*)GC_simd_f32_255));
    g = pfiSimdConvert_F32_I32(pfiSimdMul_F32(pfiSimdCast_I32_F32(g), *(PFsimdvf*)GC_simd_f32_255));
    b = pfiSimdConvert_F32_I32(pfiSimdMul_F32(pfiSimdCast_I32_F32(b), *(PFsimdvf*)GC_simd_f32_255));
    a = pfiSimdConvert_F32_I32(pfiSimdMul_F32(pfiSimdCast_I32_F32(a), *(PFsimdvf*)GC_simd_f32_255));

    PFsimdvi rgba = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdShl_I32(a, 24),
            pfiSimdShl_I32(b, 16)
        ),
        pfiSimdOr_I32(
            pfiSimdShl_I32(g, 8),
            r
        )
    );

    return rgba;
}

static inline PFsimdvi
pfiPixelGet_BGRA_FLOAT_simd(const void* pixels, PFsimdvi offsets)
{
    const PFsimdvi scaledOffset = pfiSimdMullo_I32(
        offsets, pfiSimdSet1_I32(4));

    PFsimdvi b = pfiSimdGather_I32(pixels, scaledOffset, sizeof(PFfloat));
    PFsimdvi g = pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(1)), sizeof(PFfloat));
    PFsimdvi r = pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(2)), sizeof(PFfloat));
    PFsimdvi a = pfiSimdGather_I32(pixels, pfiSimdAdd_I32(scaledOffset, pfiSimdSet1_I32(3)), sizeof(PFfloat));

    b = pfiSimdConvert_F32_I32(pfiSimdMul_F32(pfiSimdCast_I32_F32(b), *(PFsimdvf*)GC_simd_f32_255));
    g = pfiSimdConvert_F32_I32(pfiSimdMul_F32(pfiSimdCast_I32_F32(g), *(PFsimdvf*)GC_simd_f32_255));
    r = pfiSimdConvert_F32_I32(pfiSimdMul_F32(pfiSimdCast_I32_F32(r), *(PFsimdvf*)GC_simd_f32_255));
    a = pfiSimdConvert_F32_I32(pfiSimdMul_F32(pfiSimdCast_I32_F32(a), *(PFsimdvf*)GC_simd_f32_255));

    PFsimdvi bgra = pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdShl_I32(a, 24),
            pfiSimdShl_I32(b, 16)
        ),
        pfiSimdOr_I32(
            pfiSimdShl_I32(g, 8),
            r
        )
    );

    return bgra;
}

/* Internal helper constant array */

#define ENTRY(FORMAT, TYPE, FUNC) [FORMAT][TYPE] = FUNC

static const PFpixelgetter_simd GC_pixelGetters_simd[10][12] = {
    ENTRY(PF_RED, PF_UNSIGNED_BYTE, pfiPixelGet_RED_UBYTE_simd),
    ENTRY(PF_RED, PF_HALF_FLOAT, pfiPixelGet_RED_HALF_simd),
    ENTRY(PF_RED, PF_FLOAT, pfiPixelGet_RED_FLOAT_simd),

    ENTRY(PF_GREEN, PF_UNSIGNED_BYTE, pfiPixelGet_GREEN_UBYTE_simd),
    ENTRY(PF_GREEN, PF_HALF_FLOAT, pfiPixelGet_GREEN_HALF_simd),
    ENTRY(PF_GREEN, PF_FLOAT, pfiPixelGet_GREEN_FLOAT_simd),

    ENTRY(PF_BLUE, PF_UNSIGNED_BYTE, pfiPixelGet_BLUE_UBYTE_simd),
    ENTRY(PF_BLUE, PF_HALF_FLOAT, pfiPixelGet_BLUE_HALF_simd),
    ENTRY(PF_BLUE, PF_FLOAT, pfiPixelGet_BLUE_FLOAT_simd),

    ENTRY(PF_ALPHA, PF_UNSIGNED_BYTE, pfiPixelGet_ALPHA_UBYTE_simd),
    ENTRY(PF_ALPHA, PF_HALF_FLOAT, pfiPixelGet_ALPHA_HALF_simd),
    ENTRY(PF_ALPHA, PF_FLOAT, pfiPixelGet_ALPHA_FLOAT_simd),

    ENTRY(PF_LUMINANCE, PF_UNSIGNED_BYTE, pfiPixelGet_Luminance_UBYTE_simd),
    ENTRY(PF_LUMINANCE, PF_HALF_FLOAT, pfiPixelGet_Luminance_HALF_simd),
    ENTRY(PF_LUMINANCE, PF_FLOAT, pfiPixelGet_Luminance_FLOAT_simd),

    ENTRY(PF_LUMINANCE_ALPHA, PF_UNSIGNED_BYTE, pfiPixelGet_Luminance_Alpha_UBYTE_simd),
    ENTRY(PF_LUMINANCE_ALPHA, PF_HALF_FLOAT, pfiPixelGet_Luminance_Alpha_HALF_simd),
    ENTRY(PF_LUMINANCE_ALPHA, PF_FLOAT, pfiPixelGet_Luminance_Alpha_FLOAT_simd),

    ENTRY(PF_RGB, PF_UNSIGNED_BYTE, pfiPixelGet_RGB_UBYTE_simd),
    ENTRY(PF_RGB, PF_UNSIGNED_SHORT_5_6_5, pfiPixelGet_RGB_USHORT_5_6_5_simd),
    ENTRY(PF_RGB, PF_HALF_FLOAT, pfiPixelGet_RGB_HALF_simd),
    ENTRY(PF_RGB, PF_FLOAT, pfiPixelGet_RGB_FLOAT_simd),

    ENTRY(PF_RGBA, PF_UNSIGNED_BYTE, pfiPixelGet_RGBA_UBYTE_simd),
    ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_5_5_5_1, pfiPixelGet_RGBA_USHORT_5_5_5_1_simd),
    ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_4_4_4_4, pfiPixelGet_RGBA_USHORT_4_4_4_4_simd),
    ENTRY(PF_RGBA, PF_HALF_FLOAT, pfiPixelGet_RGBA_HALF_simd),
    ENTRY(PF_RGBA, PF_FLOAT, pfiPixelGet_RGBA_FLOAT_simd),

    ENTRY(PF_BGR, PF_UNSIGNED_BYTE, pfiPixelGet_BGR_UBYTE_simd),
    ENTRY(PF_BGR, PF_UNSIGNED_SHORT_5_6_5, pfiPixelGet_BGR_USHORT_5_6_5_simd),
    ENTRY(PF_BGR, PF_HALF_FLOAT, pfiPixelGet_BGR_HALF_simd),
    ENTRY(PF_BGR, PF_FLOAT, pfiPixelGet_BGR_FLOAT_simd),

    ENTRY(PF_BGRA, PF_UNSIGNED_BYTE, pfiPixelGet_BGRA_UBYTE_simd),
    ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_5_5_5_1, pfiPixelGet_BGRA_USHORT_5_5_5_1_simd),
    ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_4_4_4_4, pfiPixelGet_BGRA_USHORT_4_4_4_4_simd),
    ENTRY(PF_BGRA, PF_HALF_FLOAT, pfiPixelGet_BGRA_HALF_simd),
    ENTRY(PF_BGRA, PF_FLOAT, pfiPixelGet_BGRA_FLOAT_simd),
};

static const PFpixelsetter_simd GC_pixelSetters_simd[10][12] = {
    ENTRY(PF_RED, PF_UNSIGNED_BYTE, pfiPixelSet_RED_UBYTE_simd),
    ENTRY(PF_RED, PF_HALF_FLOAT, pfiPixelSet_RED_HALF_simd),
    ENTRY(PF_RED, PF_FLOAT, pfiPixelSet_RED_FLOAT_simd),

    ENTRY(PF_GREEN, PF_UNSIGNED_BYTE, pfiPixelSet_GREEN_UBYTE_simd),
    ENTRY(PF_GREEN, PF_HALF_FLOAT, pfiPixelSet_GREEN_HALF_simd),
    ENTRY(PF_GREEN, PF_FLOAT, pfiPixelSet_GREEN_FLOAT_simd),

    ENTRY(PF_BLUE, PF_UNSIGNED_BYTE, pfiPixelSet_BLUE_UBYTE_simd),
    ENTRY(PF_BLUE, PF_HALF_FLOAT, pfiPixelSet_BLUE_HALF_simd),
    ENTRY(PF_BLUE, PF_FLOAT, pfiPixelSet_BLUE_FLOAT_simd),

    ENTRY(PF_ALPHA, PF_UNSIGNED_BYTE, pfiPixelSet_ALPHA_UBYTE_simd),
    ENTRY(PF_ALPHA, PF_HALF_FLOAT, pfiPixelSet_ALPHA_HALF_simd),
    ENTRY(PF_ALPHA, PF_FLOAT, pfiPixelSet_ALPHA_FLOAT_simd),

    ENTRY(PF_LUMINANCE, PF_UNSIGNED_BYTE, pfiPixelSet_Luminance_UBYTE_simd),
    ENTRY(PF_LUMINANCE, PF_HALF_FLOAT, pfiPixelSet_Luminance_HALF_simd),
    ENTRY(PF_LUMINANCE, PF_FLOAT, pfiPixelSet_Luminance_FLOAT_simd),

    ENTRY(PF_LUMINANCE_ALPHA, PF_UNSIGNED_BYTE, pfiPixelSet_Luminance_Alpha_UBYTE_simd),
    ENTRY(PF_LUMINANCE_ALPHA, PF_HALF_FLOAT, pfiPixelSet_Luminance_Alpha_HALF_simd),
    ENTRY(PF_LUMINANCE_ALPHA, PF_FLOAT, pfiPixelSet_Luminance_Alpha_FLOAT_simd),

    ENTRY(PF_RGB, PF_UNSIGNED_BYTE, pfiPixelSet_RGB_UBYTE_simd),
    ENTRY(PF_RGB, PF_UNSIGNED_SHORT_5_6_5, pfiPixelSet_RGB_USHORT_5_6_5_simd),
    ENTRY(PF_RGB, PF_HALF_FLOAT, pfiPixelSet_RGB_HALF_simd),
    ENTRY(PF_RGB, PF_FLOAT, pfiPixelSet_RGB_FLOAT_simd),

    ENTRY(PF_RGBA, PF_UNSIGNED_BYTE, pfiPixelSet_RGBA_UBYTE_simd),
    ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_5_5_5_1, pfiPixelSet_RGBA_USHORT_5_5_5_1_simd),
    ENTRY(PF_RGBA, PF_UNSIGNED_SHORT_4_4_4_4, pfiPixelSet_RGBA_USHORT_4_4_4_4_simd),
    ENTRY(PF_RGBA, PF_HALF_FLOAT, pfiPixelSet_RGBA_HALF_simd),
    ENTRY(PF_RGBA, PF_FLOAT, pfiPixelSet_RGBA_FLOAT_simd),

    ENTRY(PF_BGR, PF_UNSIGNED_BYTE, pfiPixelSet_BGR_UBYTE_simd),
    ENTRY(PF_BGR, PF_UNSIGNED_SHORT_5_6_5, pfiPixelSet_BGR_USHORT_5_6_5_simd),
    ENTRY(PF_BGR, PF_HALF_FLOAT, pfiPixelSet_BGR_HALF_simd),
    ENTRY(PF_BGR, PF_FLOAT, pfiPixelSet_BGR_FLOAT_simd),

    ENTRY(PF_BGRA, PF_UNSIGNED_BYTE, pfiPixelSet_BGRA_UBYTE_simd),
    ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_5_5_5_1, pfiPixelSet_BGRA_USHORT_5_5_5_1_simd),
    ENTRY(PF_BGRA, PF_UNSIGNED_SHORT_4_4_4_4, pfiPixelSet_BGRA_USHORT_4_4_4_4_simd),
    ENTRY(PF_BGRA, PF_HALF_FLOAT, pfiPixelSet_BGRA_HALF_simd),
    ENTRY(PF_BGRA, PF_FLOAT, pfiPixelSet_BGRA_FLOAT_simd),
};

#undef ENTRY
#endif //PF_SIMD_SUPPORT


/* Helper Functions */

static inline PFboolean
pfiIsPixelFormatValid(PFpixelformat mode, PFdatatype type)
{
    return (mode >= PF_RED && mode <= PF_BGRA)
        && (type >= PF_UNSIGNED_BYTE && type <= PF_DOUBLE);
}

static inline PFsizei
pfiGetPixelBytes(PFpixelformat format, PFdatatype type)
{
    int components = 0;
    PFsizei typeSize = 0;

    switch (format) {
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

    switch (type) {
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

#endif //PF_INTERNAL_PIXEL_H
