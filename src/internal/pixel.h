#ifndef PF_INTERNAL_PIXEL_H
#define PF_INTERNAL_PIXEL_H

#include "./context/context.h"
#include "./config.h"
#include "../pfm.h"

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


/* Internal pixel setter functions */

static inline void pfInternal_PixelSet_Luminance(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Luminance equivalent color
    PFMvec3 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };
    PFubyte gray = (PFubyte)((nCol[0]*0.299f + nCol[1]*0.587f + nCol[2]*0.114f)*255.0f);

    ((PFubyte*)pixels)[offset] = gray;
}

static inline void pfInternal_PixelSet_Luminance_Alpha(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Luminance equivalent color
    PFMvec3 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };
    PFubyte gray = (PFubyte)((nCol[0]*0.299f + nCol[1]*0.587f + nCol[2]*0.114f)*255.0f);

    PFubyte *pixel = (PFubyte*)pixels + offset*2;
    pixel[0] = gray, pixel[1] = color.a;
}

static inline void pfInternal_PixelSet_RGB_5_6_5(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_5_6_5 equivalent color
    PFMvec3 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };

    PFubyte r = (PFubyte)(roundf(nCol[0]*31.0f));
    PFubyte g = (PFubyte)(roundf(nCol[1]*63.0f));
    PFubyte b = (PFubyte)(roundf(nCol[2]*31.0f));

    ((PFushort*)pixels)[offset] = (PFushort)r << 11 | (PFushort)g << 5 | (PFushort)b;
}

static inline void pfInternal_PixelSet_BGR_5_6_5(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_5_6_5 equivalent color
    PFMvec3 nCol = { (PFfloat)color.b*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.r*INV_255 };

    PFubyte b = (PFubyte)(roundf(nCol[0]*31.0f));
    PFubyte g = (PFubyte)(roundf(nCol[1]*63.0f));
    PFubyte r = (PFubyte)(roundf(nCol[2]*31.0f));

    ((PFushort*)pixels)[offset] = (PFushort)b << 11 | (PFushort)g << 5 | (PFushort)r;
}

static inline void pfInternal_PixelSet_RGB_8_8_8(void* pixels, PFsizei offset, PFcolor color)
{
    PFubyte* pixel = (PFubyte*)pixels + offset*3;
    pixel[0] = color.r;
    pixel[1] = color.g;
    pixel[2] = color.b;
}

static inline void pfInternal_PixelSet_BGR_8_8_8(void* pixels, PFsizei offset, PFcolor color)
{
    PFubyte* pixel = (PFubyte*)pixels + offset*3;
    pixel[0] = color.b;
    pixel[1] = color.g;
    pixel[2] = color.r;
}

static inline void pfInternal_PixelSet_RGBA_5_5_5_1(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_5_5_5_1 equivalent color
    PFMvec4 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255, (PFfloat)color.a*INV_255 };

    PFubyte r = (PFubyte)(roundf(nCol[0]*31.0f));
    PFubyte g = (PFubyte)(roundf(nCol[1]*31.0f));
    PFubyte b = (PFubyte)(roundf(nCol[2]*31.0f));
    PFubyte a = (nCol[3] > ((PFfloat)PF_RGBA_5_5_5_1_ALPHA_THRESHOLD*INV_255)) ? 1 : 0;

    ((PFushort*)pixels)[offset] = (PFushort)r << 11 | (PFushort)g << 6 | (PFushort)b << 1 | (PFushort)a;
}

static inline void pfInternal_PixelSet_BGRA_5_5_5_1(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_5_5_5_1 equivalent color
    PFMvec4 nCol = { (PFfloat)color.b*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.r*INV_255, (PFfloat)color.a*INV_255 };

    PFubyte b = (PFubyte)(roundf(nCol[0]*31.0f));
    PFubyte g = (PFubyte)(roundf(nCol[1]*31.0f));
    PFubyte r = (PFubyte)(roundf(nCol[2]*31.0f));
    PFubyte a = (nCol[3] > ((PFfloat)PF_RGBA_5_5_5_1_ALPHA_THRESHOLD*INV_255)) ? 1 : 0;

    ((PFushort*)pixels)[offset] = (PFushort)b << 11 | (PFushort)g << 6 | (PFushort)r << 1 | (PFushort)a;
}

static inline void pfInternal_PixelSet_RGBA_4_4_4_4(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_5_5_5_1 equivalent color
    PFMvec4 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255, (PFfloat)color.a*INV_255 };

    PFubyte r = (PFubyte)(roundf(nCol[0]*15.0f));
    PFubyte g = (PFubyte)(roundf(nCol[1]*15.0f));
    PFubyte b = (PFubyte)(roundf(nCol[2]*15.0f));
    PFubyte a = (PFubyte)(roundf(nCol[3]*15.0f));

    ((PFushort*)pixels)[offset] = (PFushort)r << 12 | (PFushort)g << 8 | (PFushort)b << 4 | (PFushort)a;
}

static inline void pfInternal_PixelSet_BGRA_4_4_4_4(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_5_5_5_1 equivalent color
    PFMvec4 nCol = { (PFfloat)color.b*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.r*INV_255, (PFfloat)color.a*INV_255 };

    PFubyte b = (PFubyte)(roundf(nCol[0]*15.0f));
    PFubyte g = (PFubyte)(roundf(nCol[1]*15.0f));
    PFubyte r = (PFubyte)(roundf(nCol[2]*15.0f));
    PFubyte a = (PFubyte)(roundf(nCol[3]*15.0f));

    ((PFushort*)pixels)[offset] = (PFushort)b << 12 | (PFushort)g << 8 | (PFushort)r << 4 | (PFushort)a;
}

static inline void pfInternal_PixelSet_RGBA_8_8_8_8(void* pixels, PFsizei offset, PFcolor color)
{
    ((PFuint*)pixels)[offset] = *(PFuint*)(&color);
}

static inline void pfInternal_PixelSet_BGRA_8_8_8_8(void* pixels, PFsizei offset, PFcolor color)
{
    PFubyte *ptr = (PFubyte*)((PFuint*)pixels + offset);
    ptr[0] = color.b;
    ptr[1] = color.g;
    ptr[2] = color.r;
    ptr[3] = color.a;
}

static inline void pfInternal_PixelSet_R_32(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Luminance equivalent color (normalized to 32bit)
    PFMvec3 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };
    ((PFfloat*)pixels)[offset] = nCol[0]*0.299f + nCol[1]*0.587f + nCol[2]*0.114f;
}

static inline void pfInternal_PixelSet_B_32(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Luminance equivalent color (normalized to 32bit)
    PFMvec3 nCol = { (PFfloat)color.b*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.r*INV_255 };
    ((PFfloat*)pixels)[offset] = nCol[0]*0.299f + nCol[1]*0.587f + nCol[2]*0.114f;
}

static inline void pfInternal_PixelSet_RGB_32_32_32(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_32_32_32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = {
        (PFfloat)color.r*INV_255,
        (PFfloat)color.g*INV_255,
        (PFfloat)color.b*INV_255
    };

    memcpy((PFMvec3*)pixels + offset, nCol, sizeof(PFMvec3));
}

static inline void pfInternal_PixelSet_BGR_32_32_32(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate BGR_32_32_32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = {
        (PFfloat)color.b*INV_255,
        (PFfloat)color.g*INV_255,
        (PFfloat)color.r*INV_255
    };

    memcpy((PFMvec3*)pixels + offset, nCol, sizeof(PFMvec3));
}

static inline void pfInternal_PixelSet_RGBA_32_32_32_32(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_32_32_32_32 equivalent color (normalized to 32bit)
    PFMvec4 nCol = {
        (PFfloat)color.r*INV_255,
        (PFfloat)color.g*INV_255,
        (PFfloat)color.b*INV_255,
        (PFfloat)color.a*INV_255
    };

    memcpy((PFMvec4*)pixels + offset, nCol, sizeof(PFMvec4));
}

static inline void pfInternal_PixelSet_BGRA_32_32_32_32(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate BGRA_32_32_32_32 equivalent color (normalized to 32bit)
    PFMvec4 nCol = {
        (PFfloat)color.b*INV_255,
        (PFfloat)color.g*INV_255,
        (PFfloat)color.r*INV_255,
        (PFfloat)color.a*INV_255
    };

    memcpy((PFMvec4*)pixels + offset, nCol, sizeof(PFMvec4));
}

static inline void pfInternal_PixelSet_R_16(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Luminance equivalent color (normalized to 32bit)
    PFMvec3 nCol = { (PFfloat)color.r*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.b*INV_255 };
    ((PFushort*)pixels)[offset] = pfInternal_FloatToHalf(nCol[0]*0.299f + nCol[1]*0.587f + nCol[2]*0.114f);
}

static inline void pfInternal_PixelSet_B_16(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate Luminance equivalent color (normalized to 32bit)
    PFMvec3 nCol = { (PFfloat)color.b*INV_255, (PFfloat)color.g*INV_255, (PFfloat)color.r*INV_255 };
    ((PFushort*)pixels)[offset] = pfInternal_FloatToHalf(nCol[0]*0.299f + nCol[1]*0.587f + nCol[2]*0.114f);
}

static inline void pfInternal_PixelSet_RGB_16_16_16(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_32_32_32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = {
        (PFfloat)color.r*INV_255,
        (PFfloat)color.g*INV_255,
        (PFfloat)color.b*INV_255
    };

    PFushort *pixel = (PFushort*)pixels + offset*3;
    pixel[0] = pfInternal_FloatToHalf(nCol[0]);
    pixel[1] = pfInternal_FloatToHalf(nCol[1]);
    pixel[2] = pfInternal_FloatToHalf(nCol[2]);
}

static inline void pfInternal_PixelSet_BGR_16_16_16(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGB_32_32_32 equivalent color (normalized to 32bit)
    PFMvec3 nCol = {
        (PFfloat)color.b*INV_255,
        (PFfloat)color.g*INV_255,
        (PFfloat)color.r*INV_255
    };

    PFushort *pixel = (PFushort*)pixels + offset*3;
    pixel[0] = pfInternal_FloatToHalf(nCol[0]);
    pixel[1] = pfInternal_FloatToHalf(nCol[1]);
    pixel[2] = pfInternal_FloatToHalf(nCol[2]);
}

static inline void pfInternal_PixelSet_RGBA_16_16_16_16(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_32_32_32_32 equivalent color (normalized to 32bit)
    PFMvec4 nCol = {
        (PFfloat)color.r*INV_255,
        (PFfloat)color.g*INV_255,
        (PFfloat)color.b*INV_255,
        (PFfloat)color.a*INV_255
    };

    PFushort *pixel = (PFushort*)pixels + offset*4;
    pixel[0] = pfInternal_FloatToHalf(nCol[0]);
    pixel[1] = pfInternal_FloatToHalf(nCol[1]);
    pixel[2] = pfInternal_FloatToHalf(nCol[2]);
    pixel[3] = pfInternal_FloatToHalf(nCol[3]);
}

static inline void pfInternal_PixelSet_BGRA_16_16_16_16(void* pixels, PFsizei offset, PFcolor color)
{
    // NOTE: Calculate RGBA_32_32_32_32 equivalent color (normalized to 32bit)
    PFMvec4 nCol = {
        (PFfloat)color.b*INV_255,
        (PFfloat)color.g*INV_255,
        (PFfloat)color.r*INV_255,
        (PFfloat)color.a*INV_255
    };

    PFushort *pixel = (PFushort*)pixels + offset*4;
    pixel[0] = pfInternal_FloatToHalf(nCol[0]);
    pixel[1] = pfInternal_FloatToHalf(nCol[1]);
    pixel[2] = pfInternal_FloatToHalf(nCol[2]);
    pixel[3] = pfInternal_FloatToHalf(nCol[3]);
}


/* Internal pixel getter functions */

static inline PFcolor pfInternal_PixelGet_Luminance(const void* pixels, PFsizei offset)
{
    PFubyte gray = ((PFubyte*)pixels)[offset];
    return (PFcolor) { gray, gray, gray, 255 };
}

static inline PFcolor pfInternal_PixelGet_Luminance_Alpha(const void* pixels, PFsizei offset)
{
    PFubyte *pixel = (PFubyte*)pixels + offset*2;
    return (PFcolor) { *pixel, *pixel, *pixel, pixel[1] };
}

static inline PFcolor pfInternal_PixelGet_RGB_5_6_5(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0xF800) >> 11)*(255.0f/31)),               // 0b1111100000000000
        (PFubyte)((PFfloat)((pixel & 0x7E0) >> 5)*(255.0f/63)),                 // 0b0000011111100000
        (PFubyte)((PFfloat)(pixel & 0x1F)*(255.0f/31)),                         // 0b0000000000011111
        255
    };
}

static inline PFcolor pfInternal_PixelGet_BGR_5_6_5(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)(pixel & 0x1F)*(255.0f/31)),                         // 0b0000000000011111
        (PFubyte)((PFfloat)((pixel & 0x7E0) >> 5)*(255.0f/63)),                 // 0b0000011111100000
        (PFubyte)((PFfloat)((pixel & 0xF800) >> 11)*(255.0f/31)),               // 0b1111100000000000
        255
    };
}

static inline PFcolor pfInternal_PixelGet_RGB_8_8_8(const void* pixels, PFsizei offset)
{
    const PFubyte* pixel = (PFubyte*)pixels + offset*3;
    return (PFcolor) { pixel[0], pixel[1], pixel[2], 255 };
}

static inline PFcolor pfInternal_PixelGet_BGR_8_8_8(const void* pixels, PFsizei offset)
{
    const PFubyte* pixel = (PFubyte*)pixels + offset*3;
    return (PFcolor) { pixel[2], pixel[1], pixel[0], 255 };
}

static inline PFcolor pfInternal_PixelGet_RGBA_5_5_5_1(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0xF800) >> 11)*(255.0f/31)),               // 0b1111100000000000
        (PFubyte)((PFfloat)((pixel & 0x7C0) >> 6)*(255.0f/31)),                 // 0b0000011111000000
        (PFubyte)((PFfloat)((pixel & 0x3E) >> 1)*(255.0f/31)),                  // 0b0000000000111110
        (PFubyte)((pixel & 0x1)*255)                                            // 0b0000000000000001
    };
}

static inline PFcolor pfInternal_PixelGet_BGRA_5_5_5_1(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0x3E) >> 1)*(255.0f/31)),                  // 0b0000000000111110
        (PFubyte)((PFfloat)((pixel & 0x7C0) >> 6)*(255.0f/31)),                 // 0b0000011111000000
        (PFubyte)((PFfloat)((pixel & 0xF800) >> 11)*(255.0f/31)),               // 0b1111100000000000
        (PFubyte)((pixel & 0x1)*255)                                            // 0b0000000000000001
    };
}

static inline PFcolor pfInternal_PixelGet_RGBA_4_4_4_4(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0xF000) >> 12)*(255.0f/15)),               // 0b1111000000000000
        (PFubyte)((PFfloat)((pixel & 0xF00) >> 8)*(255.0f/15)),                 // 0b0000111100000000
        (PFubyte)((PFfloat)((pixel & 0xF0) >> 4)*(255.0f/15)),                  // 0b0000000011110000
        (PFubyte)((PFfloat)(pixel & 0xF)*(255.0f/15))                           // 0b0000000000001111
    };
}

static inline PFcolor pfInternal_PixelGet_BGRA_4_4_4_4(const void* pixels, PFsizei offset)
{
    PFushort pixel = ((PFushort*)pixels)[offset];

    return (PFcolor) {
        (PFubyte)((PFfloat)((pixel & 0xF0) >> 4)*(255.0f/15)),                  // 0b0000000011110000
        (PFubyte)((PFfloat)((pixel & 0xF00) >> 8)*(255.0f/15)),                 // 0b0000111100000000
        (PFubyte)((PFfloat)((pixel & 0xF000) >> 12)*(255.0f/15)),               // 0b1111000000000000
        (PFubyte)((PFfloat)(pixel & 0xF)*(255.0f/15))                           // 0b0000000000001111
    };
}

static inline PFcolor pfInternal_PixelGet_RGBA_8_8_8_8(const void* pixels, PFsizei offset)
{
    return *(PFcolor*)((PFuint*)pixels + offset);
}

static inline PFcolor pfInternal_PixelGet_BGRA_8_8_8_8(const void* pixels, PFsizei offset)
{
    PFubyte *ptr = (PFubyte*)((PFuint*)pixels + offset);
    return (PFcolor) { ptr[2], ptr[1], ptr[0], ptr[3] };
}

static inline PFcolor pfInternal_PixelGet_R_32(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        (PFubyte)(((PFfloat*)pixels)[offset]*255.0f),
        0, 0, 255
    };
}

static inline PFcolor pfInternal_PixelGet_B_32(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        0, 0,
        (PFubyte)(((PFfloat*)pixels)[offset]*255.0f),
        255
    };
}

static inline PFcolor pfInternal_PixelGet_RGB_32_32_32(const void* pixels, PFsizei offset)
{
    const PFfloat *pixel = (PFfloat*)pixels + offset*3;

    return (PFcolor) {
        (PFubyte)(pixel[0]*255.0f),
        (PFubyte)(pixel[1]*255.0f),
        (PFubyte)(pixel[2]*255.0f),
        255
    };
}

static inline PFcolor pfInternal_PixelGet_BGR_32_32_32(const void* pixels, PFsizei offset)
{
    const PFfloat *pixel = (PFfloat*)pixels + offset*3;

    return (PFcolor) {
        (PFubyte)(pixel[2]*255.0f),
        (PFubyte)(pixel[1]*255.0f),
        (PFubyte)(pixel[0]*255.0f),
        255
    };
}

static inline PFcolor pfInternal_PixelGet_RGBA_32_32_32_32(const void* pixels, PFsizei offset)
{
    const PFfloat *pixel = (PFfloat*)pixels + offset*4;

    return (PFcolor) {
        (PFubyte)(pixel[0]*255.0f),
        (PFubyte)(pixel[1]*255.0f),
        (PFubyte)(pixel[2]*255.0f),
        (PFubyte)(pixel[3]*255.0f)
    };
}

static inline PFcolor pfInternal_PixelGet_BGRA_32_32_32_32(const void* pixels, PFsizei offset)
{
    const PFfloat *pixel = (PFfloat*)pixels + offset*4;

    return (PFcolor) {
        (PFubyte)(pixel[2]*255.0f),
        (PFubyte)(pixel[1]*255.0f),
        (PFubyte)(pixel[0]*255.0f),
        (PFubyte)(pixel[3]*255.0f)
    };
}

static inline PFcolor pfInternal_PixelGet_R_16(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        (PFubyte)(pfInternal_HalfToFloat(((PFushort*)pixels)[offset])*255.0f),
        0, 0, 255
    };
}

static inline PFcolor pfInternal_PixelGet_B_16(const void* pixels, PFsizei offset)
{
    return (PFcolor) {
        0, 0,
        (PFubyte)(pfInternal_HalfToFloat(((PFushort*)pixels)[offset])*255.0f),
        255
    };
}

static inline PFcolor pfInternal_PixelGet_RGB_16_16_16(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*3;

    return (PFcolor) {
        (PFubyte)(pfInternal_HalfToFloat(pixel[0]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[1]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[2]*255.0f)),
        255
    };
}

static inline PFcolor pfInternal_PixelGet_BGR_16_16_16(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*3;

    return (PFcolor) {
        (PFubyte)(pfInternal_HalfToFloat(pixel[2]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[1]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[0]*255.0f)),
        255
    };
}

static inline PFcolor pfInternal_PixelGet_RGBA_16_16_16_16(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*4;

    return (PFcolor) {
        (PFubyte)(pfInternal_HalfToFloat(pixel[0]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[1]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[2]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[3]*255.0f))
    };
}

static inline PFcolor pfInternal_PixelGet_BGRA_16_16_16_16(const void* pixels, PFsizei offset)
{
    const PFushort *pixel = (PFushort*)pixels + offset*4;

    return (PFcolor) {
        (PFubyte)(pfInternal_HalfToFloat(pixel[2]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[1]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[0]*255.0f)),
        (PFubyte)(pfInternal_HalfToFloat(pixel[3]*255.0f))
    };
}

/* Internal helper functions */

static inline void pfInternal_GetPixelGetterSetter(PFpixelgetter* getter, PFpixelsetter* setter, PFpixelformat format)
{
    switch (format)
    {
        case PF_LUMINANCE:
            if (getter) *getter = pfInternal_PixelGet_Luminance;
            if (setter) *setter = pfInternal_PixelSet_Luminance;
            break;

        case PF_LUMINANCE_ALPHA:
            if (getter) *getter = pfInternal_PixelGet_Luminance_Alpha;
            if (setter) *setter = pfInternal_PixelSet_Luminance_Alpha;
            break;

        case PF_RGB_5_6_5:
            if (getter) *getter = pfInternal_PixelGet_RGB_5_6_5;
            if (setter) *setter = pfInternal_PixelSet_RGB_5_6_5;
            break;

        case PF_BGR_5_6_5:
            if (getter) *getter = pfInternal_PixelGet_BGR_5_6_5;
            if (setter) *setter = pfInternal_PixelSet_BGR_5_6_5;
            break;

        case PF_RGBA_5_5_5_1:
            if (getter) *getter = pfInternal_PixelGet_RGBA_5_5_5_1;
            if (setter) *setter = pfInternal_PixelSet_RGBA_5_5_5_1;
            break;

        case PF_BGRA_5_5_5_1:
            if (getter) *getter = pfInternal_PixelGet_BGRA_5_5_5_1;
            if (setter) *setter = pfInternal_PixelSet_BGRA_5_5_5_1;
            break;

        case PF_RGBA_4_4_4_4:
            if (getter) *getter = pfInternal_PixelGet_RGBA_4_4_4_4;
            if (setter) *setter = pfInternal_PixelSet_RGBA_4_4_4_4;
            break;

        case PF_BGRA_4_4_4_4:
            if (getter) *getter = pfInternal_PixelGet_BGRA_4_4_4_4;
            if (setter) *setter = pfInternal_PixelSet_BGRA_4_4_4_4;
            break;

        case PF_RGB_8_8_8:
            if (getter) *getter = pfInternal_PixelGet_RGB_8_8_8;
            if (setter) *setter = pfInternal_PixelSet_RGB_8_8_8;
            break;

        case PF_BGR_8_8_8:
            if (getter) *getter = pfInternal_PixelGet_BGR_8_8_8;
            if (setter) *setter = pfInternal_PixelSet_BGR_8_8_8;
            break;

        case PF_RGBA_8_8_8_8:
            if (getter) *getter = pfInternal_PixelGet_RGBA_8_8_8_8;
            if (setter) *setter = pfInternal_PixelSet_RGBA_8_8_8_8;
            break;

        case PF_BGRA_8_8_8_8:
            if (getter) *getter = pfInternal_PixelGet_BGRA_8_8_8_8;
            if (setter) *setter = pfInternal_PixelSet_BGRA_8_8_8_8;
            break;

        case PF_R_32:
            if (getter) *getter = pfInternal_PixelGet_R_32;
            if (setter) *setter = pfInternal_PixelSet_R_32;
            break;

        case PF_B_32:
            if (getter) *getter = pfInternal_PixelGet_B_32;
            if (setter) *setter = pfInternal_PixelSet_B_32;
            break;

        case PF_RGB_32_32_32:
            if (getter) *getter = pfInternal_PixelGet_RGB_32_32_32;
            if (setter) *setter = pfInternal_PixelSet_RGB_32_32_32;
            break;

        case PF_BGR_32_32_32:
            if (getter) *getter = pfInternal_PixelGet_BGR_32_32_32;
            if (setter) *setter = pfInternal_PixelSet_BGR_32_32_32;
            break;

        case PF_RGBA_32_32_32_32 :
            if (getter) *getter = pfInternal_PixelGet_RGBA_32_32_32_32;
            if (setter) *setter = pfInternal_PixelSet_RGBA_32_32_32_32;
            break;

        case PF_BGRA_32_32_32_32:
            if (getter) *getter = pfInternal_PixelGet_BGRA_32_32_32_32;
            if (setter) *setter = pfInternal_PixelSet_BGRA_32_32_32_32;
            break;

        case PF_R_16:
            if (getter) *getter = pfInternal_PixelGet_R_16;
            if (setter) *setter = pfInternal_PixelSet_R_16;
            break;

        case PF_B_16:
            if (getter) *getter = pfInternal_PixelGet_B_16;
            if (setter) *setter = pfInternal_PixelSet_B_16;
            break;

        case PF_RGB_16_16_16:
            if (getter) *getter = pfInternal_PixelGet_RGB_16_16_16;
            if (setter) *setter = pfInternal_PixelSet_RGB_16_16_16;
            break;

        case PF_BGR_16_16_16:
            if (getter) *getter = pfInternal_PixelGet_BGR_16_16_16;
            if (setter) *setter = pfInternal_PixelSet_BGR_16_16_16;
            break;

        case PF_RGBA_16_16_16_16:
            if (getter) *getter = pfInternal_PixelGet_RGBA_16_16_16_16;
            if (setter) *setter = pfInternal_PixelSet_RGBA_16_16_16_16;
            break;

        case PF_BGRA_16_16_16_16:
            if (getter) *getter = pfInternal_PixelGet_BGRA_16_16_16_16;
            if (setter) *setter = pfInternal_PixelSet_BGRA_16_16_16_16;
            break;

        case PF_UNKNOWN_PIXELFORMAT:
            break;

        default:
            if (currentCtx)
            {
                currentCtx->errCode = PF_INVALID_ENUM;
            }
            break;
    }
}

// TODO: Review in order of probability
static inline PFsizei pfInternal_GetPixelBytes(PFpixelformat format)
{
    switch (format)
    {
        case PF_LUMINANCE:          return 1;

        case PF_B_16:
        case PF_R_16:
        case PF_BGR_5_6_5:
        case PF_RGB_5_6_5:
        case PF_BGRA_5_5_5_1:
        case PF_RGBA_5_5_5_1:
        case PF_BGRA_4_4_4_4:
        case PF_RGBA_4_4_4_4:
        case PF_LUMINANCE_ALPHA:    return 2;

        case PF_BGR_8_8_8:
        case PF_RGB_8_8_8:          return 3;

        case PF_B_32:
        case PF_R_32:
        case PF_BGRA_8_8_8_8:
        case PF_RGBA_8_8_8_8:       return 4;

        case PF_BGR_32_32_32:
        case PF_RGB_32_32_32:       return 4*3;

        case PF_BGRA_32_32_32_32:
        case PF_RGBA_32_32_32_32 :  return 4*4;

        case PF_BGR_16_16_16:
        case PF_RGB_16_16_16:       return 2*3;

        case PF_BGRA_16_16_16_16:
        case PF_RGBA_16_16_16_16:   return 2*4;

        default:
            if (currentCtx)
            {
                currentCtx->errCode = PF_INVALID_ENUM;
            }
            break;
    }

    return 0;
}

#endif //PF_INTERNAL_PIXEL_H
