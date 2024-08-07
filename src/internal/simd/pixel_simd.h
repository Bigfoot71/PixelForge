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

#ifndef PF_INTERNAL_PIXEL_SIMD_H
#define PF_INTERNAL_PIXEL_SIMD_H

#include "../context/context.h"
#include "../../pfm.h"

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
    return pfmSimdGather_I32((const PFMsimd_i*)pixels, offsets);
}

static inline PFMsimd_i
pfInternal_PixelGet_BGRA_UBYTE_simd(const void* pixels, PFMsimd_i offsets)
{
    PFMsimd_i result = pfmSimdGather_I32((const PFMsimd_i*)pixels, offsets);
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
}

#endif //PF_INTERNAL_PIXEL_SIMD_H
