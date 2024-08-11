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

#ifndef PF_INTERNAL_COLOR_H
#define PF_INTERNAL_COLOR_H

#include "../pixelforge.h"
#include "../pfm.h"
#include <stdint.h>

/* SISD Implementation */

static inline PFcolor
pfInternal_ColorLerpSmooth(PFcolor a, PFcolor b, PFfloat t)
{
    return (PFcolor) {
        a.r + t*(b.r - a.r),
        a.g + t*(b.g - a.g),
        a.b + t*(b.b - a.b),
        a.a + t*(b.a - a.a)
    };
}

static inline PFcolor
pfInternal_ColorLerpFlat(PFcolor v1, PFcolor v2, PFfloat t)
{
    return (t < 0.5f) ? v1 : v2;
}

static inline PFcolor
pfInternal_ColorBarySmooth(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    PFubyte uW1 = 255*w1;
    PFubyte uW2 = 255*w2;
    PFubyte uW3 = 255*w3;

    return (PFcolor) {
        ((uW1*v1.r) + (uW2*v2.r) + (uW3*v3.r))/255,
        ((uW1*v1.g) + (uW2*v2.g) + (uW3*v3.g))/255,
        ((uW1*v1.b) + (uW2*v2.b) + (uW3*v3.b))/255,
        ((uW1*v1.a) + (uW2*v2.a) + (uW3*v3.a))/255
    };
}

static inline PFcolor
pfInternal_ColorBaryFlat(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    return ((w1 > w2) & (w1 > w3)) ? v1 : (w2 >= w3) ? v2 : v3;
}

/* SIMD Implementation */

typedef PFMsimd_i PFsimd_color[4];

static inline void
pfInternal_SimdColorLoadUnpacked(PFsimd_color dst, PFcolor src)
{
    for (int_fast8_t i = 0; i < 4; ++i) {
        dst[i] = pfmSimdSet1_I32(((PFubyte*)&src)[i]);
    }
}

static inline void
pfInternal_SimdColorUnpack(PFsimd_color out, const PFMsimd_i packed)
{
    out[0] = pfmSimdAnd_I32(packed, *(PFMsimd_i*)pfm_i32_255);
    out[1] = pfmSimdAnd_I32(pfmSimdShr_I32(packed, 8), *(PFMsimd_i*)pfm_i32_255);
    out[2] = pfmSimdAnd_I32(pfmSimdShr_I32(packed, 16), *(PFMsimd_i*)pfm_i32_255);
    out[3] = pfmSimdAnd_I32(pfmSimdShr_I32(packed, 24), *(PFMsimd_i*)pfm_i32_255);
}

static inline PFMsimd_i
pfInternal_SimdColorPack(const PFsimd_color unpacked)
{
    // Combine into a single vector PFMsimd_i
    return pfmSimdOr_I32(
        pfmSimdOr_I32(
            pfmSimdShl_I32(unpacked[3], 24), 
            pfmSimdShl_I32(unpacked[2], 16)),
        pfmSimdOr_I32(
            pfmSimdShl_I32(unpacked[1], 8), 
            unpacked[0]));
}

static inline void
pfInternal_SimdColorLerpSmooth(PFsimd_color out, const PFsimd_color a, const PFsimd_color b, PFMsimd_f t)
{
    PFMsimd_i uT = pfmSimdConvert_F32_I32(pfmSimdMul_F32(t, *(PFMsimd_f*)pfm_f32_255));
    out[0] = pfmSimdAdd_I32(a[0], pfmSimdShr_I32(pfmSimdMullo_I32(uT, pfmSimdSub_I32(b[0], a[0])), 8));
    out[1] = pfmSimdAdd_I32(a[1], pfmSimdShr_I32(pfmSimdMullo_I32(uT, pfmSimdSub_I32(b[1], a[1])), 8));
    out[2] = pfmSimdAdd_I32(a[2], pfmSimdShr_I32(pfmSimdMullo_I32(uT, pfmSimdSub_I32(b[2], a[2])), 8));
    out[3] = pfmSimdAdd_I32(a[3], pfmSimdShr_I32(pfmSimdMullo_I32(uT, pfmSimdSub_I32(b[3], a[3])), 8));
}

static inline void
pfInternal_SimdColorLerpFlat(PFsimd_color out, const PFsimd_color a, const PFsimd_color b, PFMsimd_f t)
{
    PFMsimd_i mask = pfmSimdCast_F32_I32(
        pfmSimdCmpLT_F32(t, *(PFMsimd_f*)pfm_f32_0p5));
    
    out[0] = pfmSimdBlendV_I8(b[0], a[0], mask);
    out[1] = pfmSimdBlendV_I8(b[1], a[1], mask);
    out[2] = pfmSimdBlendV_I8(b[2], a[2], mask);
    out[3] = pfmSimdBlendV_I8(b[3], a[3], mask);
}

static inline void
pfInternal_SimdColorBarySmooth(PFsimd_color out,
                                const PFsimd_color c1,
                                const PFsimd_color c2,
                                const PFsimd_color c3,
                                PFMsimd_f w1,
                                PFMsimd_f w2,
                                PFMsimd_f w3)
{
    // Multiply weights by 255 and convert them to integers
    PFMsimd_i uW1 = pfmSimdConvert_F32_I32(pfmSimdMul_F32(w1, *(PFMsimd_f*)pfm_f32_255));
    PFMsimd_i uW2 = pfmSimdConvert_F32_I32(pfmSimdMul_F32(w2, *(PFMsimd_f*)pfm_f32_255));
    PFMsimd_i uW3 = pfmSimdConvert_F32_I32(pfmSimdMul_F32(w3, *(PFMsimd_f*)pfm_f32_255));

    // Perform multiplications and additions for each channel
    // Then approximate division by 255
    for (int_fast8_t i = 0; i < 4; ++i) {
        out[i] = pfmSimdAdd_I32(
            pfmSimdAdd_I32(
                pfmSimdMullo_I32(uW1, c1[i]), 
                pfmSimdMullo_I32(uW2, c2[i])),
                pfmSimdMullo_I32(uW3, c3[i]));
        out[i] = pfmSimdShr_I32(
            pfmSimdMullo_I32(out[i], *(PFMsimd_i*)pfm_i32_257), 16);
    }
}

static inline void
pfInternal_SimdColorBaryFlat(PFsimd_color out,
                             const PFsimd_color c1,
                             const PFsimd_color c2,
                             const PFsimd_color c3,
                             PFMsimd_f w1,
                             PFMsimd_f w2,
                             PFMsimd_f w3)
{
    // Compare the weights to find the index of the maximum weight
    PFMsimd_f maxWeight = pfmSimdMax_F32(w1, pfmSimdMax_F32(w2, w3));
    
    // Compare maxWeight to each weight to find which one is the max
    PFMsimd_i mask1 = pfmSimdCast_F32_I32(pfmSimdCmpEQ_F32(maxWeight, w1));
    PFMsimd_i mask2 = pfmSimdCast_F32_I32(pfmSimdCmpEQ_F32(maxWeight, w2));
    PFMsimd_i mask3 = pfmSimdCast_F32_I32(pfmSimdCmpEQ_F32(maxWeight, w3));

    // Use masks to select the corresponding color
    for (int_fast8_t i = 0; i < 4; ++i) {
        out[i] = pfmSimdOr_I32(
            pfmSimdAnd_I32(mask1, c1[i]),
            pfmSimdOr_I32(
                pfmSimdAnd_I32(mask2, c2[i]),
                pfmSimdAnd_I32(mask3, c3[i])
            )
        );
    }
}

/* SIMD Conversion helpers */

static inline void
pfInternal_SimdColorSisdToVec(PFMsimd_f* out, PFcolor in, int vecSize)
{
    for (int i = 0; i < vecSize; ++i)
    {
        out[i] = pfmSimdDiv_F32(
            pfmSimdSet1_F32(((PFubyte*)&in)[i]),
            *(PFMsimd_f*)pfm_f32_255);
    }
}

static inline void
pfInternal_SimdColorUnpackedToVec(PFMsimd_f* out, PFsimd_color in, int vecSize)
{
    for (int i = 0; i < vecSize; ++i)
    {
        out[i] = pfmSimdDiv_F32(
            pfmSimdConvert_I32_F32(in[i]),
            *(PFMsimd_f*)pfm_f32_255);
    }
}

static inline void
pfInternal_SimdColorUnpackedFromVec(PFsimd_color out, PFMsimd_f* in, int vecSize)
{
    out[0] = *(PFMsimd_i*)pfm_i32_0;
    out[1] = *(PFMsimd_i*)pfm_i32_0;
    out[2] = *(PFMsimd_i*)pfm_i32_0;
    out[3] = *(PFMsimd_i*)pfm_i32_255;

    for (int i = 0; i < vecSize; ++i)
    {
        out[i] = pfmSimdConvert_F32_I32(pfmSimdMul_F32(in[i], *(PFMsimd_f*)pfm_f32_255));
        out[i] = pfmSimdClamp_I32(out[i], pfmSimdSetZero_I32(), *(PFMsimd_i*)pfm_i32_255);
    }
}

static inline PFMsimd_i
pfInternal_SimdColorPackedGrayscale(PFMsimd_i colors)
{
    // Masks to extract the R, G, B, and Alpha channels
    const PFMsimd_i maskA = pfmSimdSet1_I32(0xFF000000);

    // Extract the R, G, B channels
    PFMsimd_i r = pfmSimdAnd_I32(pfmSimdShr_I32(colors, 16), pfmSimdSet1_I32(0xFF));
    PFMsimd_i g = pfmSimdAnd_I32(pfmSimdShr_I32(colors, 8), pfmSimdSet1_I32(0xFF));
    PFMsimd_i b = pfmSimdAnd_I32(colors, pfmSimdSet1_I32(0xFF));

    // Integer coefficients approximating 0.299, 0.587, and 0.114 (multiplied by 256)
    const PFMsimd_i coeffR = pfmSimdSet1_I32(77);   // 0.299 * 256 = 76.8
    const PFMsimd_i coeffG = pfmSimdSet1_I32(150);  // 0.587 * 256 = 150.272
    const PFMsimd_i coeffB = pfmSimdSet1_I32(29);   // 0.114 * 256 = 29.184

    // Calculate the luminance using the integer coefficients
    PFMsimd_i gray = pfmSimdAdd_I32(
        pfmSimdAdd_I32(
            pfmSimdMullo_I32(r, coeffR),
            pfmSimdMullo_I32(g, coeffG)
        ),
        pfmSimdMullo_I32(b, coeffB)
    );

    // Divide by 256 (equivalent to a right shift by 8 bits)
    gray = pfmSimdShr_I32(gray, 8);

    // Repeat the luminance in the R, G, B channels
    PFMsimd_i grayRGB = pfmSimdOr_I32(
        pfmSimdOr_I32(pfmSimdShl_I32(gray, 16), pfmSimdShl_I32(gray, 8)),
        gray
    );

    // Extract the alpha from the original most significant 8 bits
    PFMsimd_i alpha = pfmSimdAnd_I32(colors, maskA);

    // Combine the luminance (repeated in RGB) with the original alpha
    return pfmSimdOr_I32(grayRGB, alpha);
}

#endif //PF_INTERNAL_COLOR_H
