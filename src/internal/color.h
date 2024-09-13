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
#include "./simd.h"
#include <stdint.h>

/* SISD Implementation */

static inline PFcolor
pfiColorLerpSmooth(PFcolor a, PFcolor b, PFfloat t)
{
    return (PFcolor) {
        a.r + t*(b.r - a.r),
        a.g + t*(b.g - a.g),
        a.b + t*(b.b - a.b),
        a.a + t*(b.a - a.a)
    };
}

static inline PFcolor
pfiColorLerpFlat(PFcolor v1, PFcolor v2, PFfloat t)
{
    return (t < 0.5f) ? v1 : v2;
}

static inline PFcolor
pfiColorBarySmooth(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3)
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
pfiColorBaryFlat(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    return ((w1 > w2) & (w1 > w3)) ? v1 : (w2 >= w3) ? v2 : v3;
}

/* SIMD Implementation */

#if PF_SIMD_SUPPORT

typedef PFsimdvi PFsimd_color[4];

static inline void
pfiColorLoadUnpacked_simd(PFsimd_color dst, PFcolor src)
{
    for (int_fast8_t i = 0; i < 4; ++i) {
        dst[i] = pfiSimdSet1_I32(((PFubyte*)&src)[i]);
    }
}

static inline void
pfiColorUnpack_simd(PFsimd_color out, const PFsimdvi packed)
{
    out[0] = pfiSimdAnd_I32(packed, *(PFsimdvi*)GC_simd_i32_255);
    out[1] = pfiSimdAnd_I32(pfiSimdShr_I32(packed, 8), *(PFsimdvi*)GC_simd_i32_255);
    out[2] = pfiSimdAnd_I32(pfiSimdShr_I32(packed, 16), *(PFsimdvi*)GC_simd_i32_255);
    out[3] = pfiSimdAnd_I32(pfiSimdShr_I32(packed, 24), *(PFsimdvi*)GC_simd_i32_255);
}

static inline PFsimdvi
pfiColorPack_simd(const PFsimd_color unpacked)
{
    // Combine into a single vector PFsimdvi
    return pfiSimdOr_I32(
        pfiSimdOr_I32(
            pfiSimdShl_I32(unpacked[3], 24), 
            pfiSimdShl_I32(unpacked[2], 16)),
        pfiSimdOr_I32(
            pfiSimdShl_I32(unpacked[1], 8), 
            unpacked[0]));
}

static inline void
pfiColorLerpSmooth_simd(PFsimd_color out, const PFsimd_color a, const PFsimd_color b, PFsimdvf t)
{
    PFsimdvi uT = pfiSimdConvert_F32_I32(pfiSimdMul_F32(t, *(PFsimdvf*)GC_simd_f32_255));
    out[0] = pfiSimdAdd_I32(a[0], pfiSimdShr_I32(pfiSimdMullo_I32(uT, pfiSimdSub_I32(b[0], a[0])), 8));
    out[1] = pfiSimdAdd_I32(a[1], pfiSimdShr_I32(pfiSimdMullo_I32(uT, pfiSimdSub_I32(b[1], a[1])), 8));
    out[2] = pfiSimdAdd_I32(a[2], pfiSimdShr_I32(pfiSimdMullo_I32(uT, pfiSimdSub_I32(b[2], a[2])), 8));
    out[3] = pfiSimdAdd_I32(a[3], pfiSimdShr_I32(pfiSimdMullo_I32(uT, pfiSimdSub_I32(b[3], a[3])), 8));
}

static inline void
pfiColorLerpFlat_simd(PFsimd_color out, const PFsimd_color a, const PFsimd_color b, PFsimdvf t)
{
    PFsimdvi mask = pfiSimdCast_F32_I32(
        pfiSimdCmpLT_F32(t, *(PFsimdvf*)GC_simd_f32_0p5));
    
    out[0] = pfiSimdBlendV_I8(b[0], a[0], mask);
    out[1] = pfiSimdBlendV_I8(b[1], a[1], mask);
    out[2] = pfiSimdBlendV_I8(b[2], a[2], mask);
    out[3] = pfiSimdBlendV_I8(b[3], a[3], mask);
}

static inline void
pfiColorBarySmooth_simd(PFsimd_color out,
                                const PFsimd_color c1,
                                const PFsimd_color c2,
                                const PFsimd_color c3,
                                PFsimdvf w1,
                                PFsimdvf w2,
                                PFsimdvf w3)
{
    // Multiply weights by 255 and convert them to integers
    PFsimdvi uW1 = pfiSimdConvert_F32_I32(pfiSimdMul_F32(w1, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi uW2 = pfiSimdConvert_F32_I32(pfiSimdMul_F32(w2, *(PFsimdvf*)GC_simd_f32_255));
    PFsimdvi uW3 = pfiSimdConvert_F32_I32(pfiSimdMul_F32(w3, *(PFsimdvf*)GC_simd_f32_255));

    // Perform multiplications and additions for each channel
    // Then approximate division by 255
    for (int_fast8_t i = 0; i < 4; ++i) {
        out[i] = pfiSimdAdd_I32(
            pfiSimdAdd_I32(
                pfiSimdMullo_I32(uW1, c1[i]), 
                pfiSimdMullo_I32(uW2, c2[i])),
                pfiSimdMullo_I32(uW3, c3[i]));
        out[i] = pfiSimdShr_I32(
            pfiSimdMullo_I32(out[i], *(PFsimdvi*)GC_simd_i32_257), 16);
    }
}

static inline void
pfiColorBaryFlat_simd(PFsimd_color out,
                             const PFsimd_color c1,
                             const PFsimd_color c2,
                             const PFsimd_color c3,
                             PFsimdvf w1,
                             PFsimdvf w2,
                             PFsimdvf w3)
{
    // Compare the weights to find the index of the maximum weight
    PFsimdvf maxWeight = pfiSimdMax_F32(w1, pfiSimdMax_F32(w2, w3));
    
    // Compare maxWeight to each weight to find which one is the max
    PFsimdvi mask1 = pfiSimdCast_F32_I32(pfiSimdCmpEQ_F32(maxWeight, w1));
    PFsimdvi mask2 = pfiSimdCast_F32_I32(pfiSimdCmpEQ_F32(maxWeight, w2));
    PFsimdvi mask3 = pfiSimdCast_F32_I32(pfiSimdCmpEQ_F32(maxWeight, w3));

    // Use masks to select the corresponding color
    for (int_fast8_t i = 0; i < 4; ++i) {
        out[i] = pfiSimdOr_I32(
            pfiSimdAnd_I32(mask1, c1[i]),
            pfiSimdOr_I32(
                pfiSimdAnd_I32(mask2, c2[i]),
                pfiSimdAnd_I32(mask3, c3[i])
            )
        );
    }
}

/* SIMD Conversion helpers */

static inline void
pfiColorSisdToVec_simd(PFsimdvf* out, PFcolor in, int vecSize)
{
    for (int i = 0; i < vecSize; ++i) {
        out[i] = pfiSimdDiv_F32(
            pfiSimdSet1_F32(((PFubyte*)&in)[i]),
            *(PFsimdvf*)GC_simd_f32_255);
    }
}

static inline void
pfiColorUnpackToVec_simd(PFsimdvf* out, PFsimd_color in, int vecSize)
{
    for (int i = 0; i < vecSize; ++i) {
        out[i] = pfiSimdDiv_F32(
            pfiSimdConvert_I32_F32(in[i]),
            *(PFsimdvf*)GC_simd_f32_255);
    }
}

static inline void
pfiColorUnpackFromVec_simd(PFsimd_color out, PFsimdvf* in, int vecSize)
{
    out[0] = *(PFsimdvi*)GC_simd_i32_0;
    out[1] = *(PFsimdvi*)GC_simd_i32_0;
    out[2] = *(PFsimdvi*)GC_simd_i32_0;
    out[3] = *(PFsimdvi*)GC_simd_i32_255;

    for (int i = 0; i < vecSize; ++i) {
        out[i] = pfiSimdConvert_F32_I32(pfiSimdMul_F32(in[i], *(PFsimdvf*)GC_simd_f32_255));
        out[i] = pfiSimdClamp_I32(out[i], pfiSimdSetZero_I32(), *(PFsimdvi*)GC_simd_i32_255);
    }
}

static inline PFsimdvi
pfiColorPackGrayscale_simd(PFsimdvi colors)
{
    // Masks to extract the R, G, B, and Alpha channels
    const PFsimdvi maskA = pfiSimdSet1_I32(0xFF000000);

    // Extract the R, G, B channels
    PFsimdvi r = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), pfiSimdSet1_I32(0xFF));
    PFsimdvi g = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), pfiSimdSet1_I32(0xFF));
    PFsimdvi b = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0xFF));

    // Integer coefficients approximating 0.299, 0.587, and 0.114 (multiplied by 256)
    const PFsimdvi coeffR = pfiSimdSet1_I32(77);   // 0.299 * 256 = 76.8
    const PFsimdvi coeffG = pfiSimdSet1_I32(150);  // 0.587 * 256 = 150.272
    const PFsimdvi coeffB = pfiSimdSet1_I32(29);   // 0.114 * 256 = 29.184

    // Calculate the luminance using the integer coefficients
    PFsimdvi gray = pfiSimdAdd_I32(
        pfiSimdAdd_I32(
            pfiSimdMullo_I32(r, coeffR),
            pfiSimdMullo_I32(g, coeffG)
        ),
        pfiSimdMullo_I32(b, coeffB)
    );

    // Divide by 256 (equivalent to a right shift by 8 bits)
    gray = pfiSimdShr_I32(gray, 8);

    // Repeat the luminance in the R, G, B channels
    PFsimdvi grayRGB = pfiSimdOr_I32(
        pfiSimdOr_I32(pfiSimdShl_I32(gray, 16), pfiSimdShl_I32(gray, 8)),
        gray
    );

    // Extract the alpha from the original most significant 8 bits
    PFsimdvi alpha = pfiSimdAnd_I32(colors, maskA);

    // Combine the luminance (repeated in RGB) with the original alpha
    return pfiSimdOr_I32(grayRGB, alpha);
}

#endif //PF_SIMD_SUPPORT
#endif //PF_INTERNAL_COLOR_H
