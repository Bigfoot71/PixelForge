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
    // Defined as [FORMAT][TYPE]

    static const PFpixelgetter getters[10][12] = {

        // PF_RED
        {
            pfInternal_PixelGet_RED_UBYTE,              // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelGet_RED_HALF,               // PF_HALF_FLOAT
            pfInternal_PixelGet_RED_FLOAT,              // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_GREEN
        {
            pfInternal_PixelGet_GREEN_UBYTE,            // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelGet_GREEN_HALF,             // PF_HALF_FLOAT
            pfInternal_PixelGet_GREEN_FLOAT,            // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_BLUE
        {
            pfInternal_PixelGet_BLUE_UBYTE,             // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelGet_BLUE_HALF,              // PF_HALF_FLOAT
            pfInternal_PixelGet_BLUE_FLOAT,             // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_ALPHA
        {
            pfInternal_PixelGet_ALPHA_UBYTE,            // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelGet_ALPHA_HALF,             // PF_HALF_FLOAT
            pfInternal_PixelGet_ALPHA_FLOAT,            // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_LUMINANCE
        {
            pfInternal_PixelGet_Luminance_UBYTE,        // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelGet_Luminance_HALF,         // PF_HALF_FLOAT
            pfInternal_PixelGet_Luminance_FLOAT,        // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_LUMINANCE_ALPHA
        {
            pfInternal_PixelGet_Luminance_Alpha_UBYTE,  // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelGet_Luminance_Alpha_HALF,   // PF_HALF_FLOAT
            pfInternal_PixelGet_Luminance_Alpha_FLOAT,  // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_RGB
        {
            pfInternal_PixelGet_RGB_UBYTE,              // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            pfInternal_PixelGet_RGB_USHORT_5_6_5,       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelGet_RGB_HALF,               // PF_HALF_FLOAT
            pfInternal_PixelGet_RGB_FLOAT,             // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_RGBA
        {
            pfInternal_PixelGet_RGBA_UBYTE,             // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            pfInternal_PixelGet_RGBA_USHORT_5_5_5_1,    // PF_UNSIGNED_SHORT_5_5_5_1
            pfInternal_PixelGet_RGBA_USHORT_4_4_4_4,    // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelGet_RGBA_HALF,              // PF_HALF_FLOAT
            pfInternal_PixelGet_RGBA_FLOAT,             // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_BGR
        {
            pfInternal_PixelGet_BGR_UBYTE,              // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            pfInternal_PixelGet_BGR_USHORT_5_6_5,       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelGet_BGR_HALF,               // PF_HALF_FLOAT
            pfInternal_PixelGet_BGR_FLOAT,             // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_BGRA
        {
            pfInternal_PixelGet_BGRA_UBYTE,             // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            pfInternal_PixelGet_BGRA_USHORT_5_5_5_1,    // PF_UNSIGNED_SHORT_5_5_5_1
            pfInternal_PixelGet_BGRA_USHORT_4_4_4_4,    // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelGet_BGRA_HALF,              // PF_HALF_FLOAT
            pfInternal_PixelGet_BGRA_FLOAT,             // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

    };

    static const PFpixelsetter setters[10][12] = {


        // PF_RED
        {
            pfInternal_PixelSet_Luminance_UBYTE,        // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelSet_Luminance_HALF,         // PF_HALF_FLOAT
            pfInternal_PixelSet_Luminance_FLOAT,        // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_GREEN
        {
            pfInternal_PixelSet_Luminance_UBYTE,        // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelSet_Luminance_HALF,         // PF_HALF_FLOAT
            pfInternal_PixelSet_Luminance_FLOAT,        // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_BLUE
        {
            pfInternal_PixelSet_Luminance_UBYTE,        // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelSet_Luminance_HALF,         // PF_HALF_FLOAT
            pfInternal_PixelSet_Luminance_FLOAT,        // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_ALPHA
        {
            pfInternal_PixelSet_Luminance_UBYTE,        // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelSet_Luminance_HALF,         // PF_HALF_FLOAT
            pfInternal_PixelSet_Luminance_FLOAT,        // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_LUMINANCE
        {
            pfInternal_PixelSet_Luminance_UBYTE,        // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelSet_Luminance_HALF,         // PF_HALF_FLOAT
            pfInternal_PixelSet_Luminance_FLOAT,        // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_LUMINANCE_ALPHA
        {
            pfInternal_PixelSet_Luminance_Alpha_UBYTE,  // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelSet_Luminance_Alpha_HALF,   // PF_HALF_FLOAT
            pfInternal_PixelSet_Luminance_Alpha_FLOAT,  // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_RGB
        {
            pfInternal_PixelSet_RGB_UBYTE,              // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            pfInternal_PixelSet_RGB_USHORT_5_6_5,       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelSet_RGB_HALF,               // PF_HALF_FLOAT
            pfInternal_PixelSet_RGB_FLOAT,             // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_RGBA
        {
            pfInternal_PixelSet_RGBA_UBYTE,             // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            pfInternal_PixelSet_RGBA_USHORT_5_5_5_1,    // PF_UNSIGNED_SHORT_5_5_5_1
            pfInternal_PixelSet_RGBA_USHORT_4_4_4_4,    // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelSet_RGBA_HALF,              // PF_HALF_FLOAT
            pfInternal_PixelSet_RGBA_FLOAT,             // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_BGR
        {
            pfInternal_PixelSet_BGR_UBYTE,              // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            pfInternal_PixelSet_BGR_USHORT_5_6_5,       // PF_UNSIGNED_SHORT_5_6_5
            NULL,                                       // PF_UNSIGNED_SHORT_5_5_5_1
            NULL,                                       // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelSet_BGR_HALF,               // PF_HALF_FLOAT
            pfInternal_PixelSet_BGR_FLOAT,             // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

        // PF_BGRA
        {
            pfInternal_PixelSet_BGRA_UBYTE,             // PF_UNSIGNED_BYTE
            NULL,                                       // PF_UNSIGNED_SHORT
            NULL,                                       // PF_UNSIGNED_SHORT_5_6_5
            pfInternal_PixelSet_BGRA_USHORT_5_5_5_1,    // PF_UNSIGNED_SHORT_5_5_5_1
            pfInternal_PixelSet_BGRA_USHORT_4_4_4_4,    // PF_UNSIGNED_SHORT_4_4_4_4
            NULL,                                       // PF_UNSIGNED_INT
            NULL,                                       // PF_BYTE
            NULL,                                       // PF_SHORT
            NULL,                                       // PF_INT
            pfInternal_PixelSet_BGRA_HALF,              // PF_HALF_FLOAT
            pfInternal_PixelSet_BGRA_FLOAT,             // PF_FLOAT
            NULL                                        // PF_DOUBLE
        },

    };

    if (getter) *getter = getters[format][type];
    if (setter) *setter = setters[format][type];
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

#endif //PF_INTERNAL_PIXEL_H
