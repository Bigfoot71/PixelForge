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

/* SISD Implementation */

static inline PFcolor
pfiColorLerpSmooth(PFcolor a, PFcolor b, PFfloat t)
{
    return (PFcolor) {
        (PFubyte)(a.r + t * (b.r - a.r)),
        (PFubyte)(a.g + t * (b.g - a.g)),
        (PFubyte)(a.b + t * (b.b - a.b)),
        (PFubyte)(a.a + t * (b.a - a.a))
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
    PFubyte uW1 = (PFubyte)(255 * w1);
    PFubyte uW2 = (PFubyte)(255 * w2);
    PFubyte uW3 = (PFubyte)(255 * w3);

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

static inline PFIsimdvi
pfiColorLoad_simd(PFcolor color)
{
    union { PFcolor s; PFint v; } r = { .s = color };
    return pfiSimdSet1_I32(r.v);
}

static inline void
pfiColorSIMDToVecI_simd(PFIsimdvi* out, const PFIsimdvi packed, int vecSize)
{
    for (int_fast8_t i = 0; i < vecSize; ++i) {
        out[i] = pfiSimdAnd_I32(pfiSimdShr_I32(packed, i * 8), *(PFIsimdvi*)GC_simd_i32_255);
    }
}

static inline void
pfiColorSIMDToVecF_simd(PFIsimdvf* out, const PFIsimdvi packed, int vecSize)
{
    for (int_fast8_t i = 0; i < vecSize; ++i) {
        PFIsimdvi comp = pfiSimdAnd_I32(pfiSimdShr_I32(packed, i * 8), *(PFIsimdvi*)GC_simd_i32_255);
        out[i] = pfiSimdMul_F32(pfiSimdConvert_I32_F32(comp), *(PFIsimdvf*)GC_simd_f32_inv255);
    }
}

static inline void
pfiColorSISDToVecI_simd(PFIsimdvi* out, PFcolor in, int vecSize)
{
    for (int_fast8_t i = 0; i < vecSize; ++i) {
        out[i] = pfiSimdSet1_I32(((PFubyte*)&in)[i]);
    }
}

static inline void
pfiColorSISDToVecF_simd(PFIsimdvf* out, PFcolor in, int vecSize)
{
    for (int_fast8_t i = 0; i < vecSize; ++i) {
        out[i] = pfiSimdMul_F32(
            pfiSimdSet1_F32(((PFubyte*)&in)[i]),
            *(PFIsimdvf*)GC_simd_f32_inv255);
    }
}

static inline PFIsimdvi
pfiColorSIMDFromVecI_simd(const PFIsimdvi* in, int vecSize)
{
    PFIsimdvi packed = pfiSimdSetZero_I32();

    for (int_fast8_t i = 0; i < vecSize; ++i) {
        packed = pfiSimdOr_I32(packed, pfiSimdShl_I32(in[i], i * 8));
    }

    return packed;
}

static inline PFIsimdvi
pfiColorSIMDFromVecF_simd(const PFIsimdvf* in, int vecSize)
{
    PFIsimdvi packed = pfiSimdSetZero_I32();

    for (int_fast8_t i = 0; i < vecSize; ++i) {
        PFIsimdvi comp = pfiSimdMul_F32(pfiSimdClamp_F32(in[i], *(PFIsimdvf*)GC_simd_f32_0, *(PFIsimdvf*)GC_simd_f32_1), *(PFIsimdvf*)GC_simd_f32_255);
        packed = pfiSimdOr_I32(packed, pfiSimdShl_I32(comp, i * 8));
    }

    return packed;
}

static inline PFIsimdvi
pfiColorLerpSmooth_simd(const PFIsimdvi a, const PFIsimdvi b, PFIsimdvf t)
{
    PFIsimdv4f aV4; pfiColorSIMDToVecF_simd(aV4, a, 4);
    PFIsimdv4f bV4; pfiColorSIMDToVecF_simd(aV4, b, 4);
    PFIsimdv4f rV4; pfiVec4LerpR_simd(rV4, aV4, bV4, t);
    return pfiColorSIMDFromVecF_simd(rV4, 4);
}

static inline PFIsimdvi
pfiColorLerpFlat_simd(const PFIsimdvi a, const PFIsimdvi b, PFIsimdvf t)
{
    PFIsimdvi mask = pfiSimdCast_F32_I32(pfiSimdCmpLT_F32(t, *(PFIsimdvf*)GC_simd_f32_0p5));
    return pfiSimdBlendV_I8(b, a, mask);
}

static inline PFIsimdvi
pfiColorBarySmooth_simd(PFIsimdvi c1, PFIsimdvi c2, PFIsimdvi c3,
                        PFIsimdvf w1, PFIsimdvf w2, PFIsimdvf w3)
{
    // Multiply weights by 255 and convert them to integers
    PFIsimdvi uW1 = pfiSimdConvert_F32_I32(pfiSimdMul_F32(w1, *(PFIsimdvf*)GC_simd_f32_255));
    PFIsimdvi uW2 = pfiSimdConvert_F32_I32(pfiSimdMul_F32(w2, *(PFIsimdvf*)GC_simd_f32_255));
    PFIsimdvi uW3 = pfiSimdConvert_F32_I32(pfiSimdMul_F32(w3, *(PFIsimdvf*)GC_simd_f32_255));

    PFIsimdvi c1V[4]; pfiColorSIMDToVecI_simd(c1V, c1, 4);
    PFIsimdvi c2V[4]; pfiColorSIMDToVecI_simd(c2V, c2, 4);
    PFIsimdvi c3V[4]; pfiColorSIMDToVecI_simd(c3V, c3, 4);

    // Perform multiplications and additions for each channel
    // Then approximate division by 255
    PFIsimdvi rV[4];
    for (int_fast8_t i = 0; i < 4; ++i) {
        rV[i] = pfiSimdAdd_I32(
            pfiSimdAdd_I32(
                pfiSimdMullo_I32(uW1, c1V[i]), 
                pfiSimdMullo_I32(uW2, c2V[i])),
                pfiSimdMullo_I32(uW3, c3V[i]));
        rV[i] = pfiSimdShr_I32(
            pfiSimdMullo_I32(rV[i], *(PFIsimdvi*)GC_simd_i32_257), 16);
    }

    // Pack components into 16-bit
    return pfiColorSIMDFromVecI_simd(rV, 4);
}

static inline PFIsimdvi
pfiColorBaryFlat_simd(PFIsimdvi c1, PFIsimdvi c2, PFIsimdvi c3,
                      PFIsimdvf w1, PFIsimdvf w2, PFIsimdvf w3)
{
    // Compare the weights to find the index of the maximum weight
    PFIsimdvf maxWeight = pfiSimdMax_F32(w1, pfiSimdMax_F32(w2, w3));
    
    // Compare maxWeight to each weight to find which one is the max
    PFIsimdvi mask1 = pfiSimdCast_F32_I32(pfiSimdCmpEQ_F32(maxWeight, w1));
    PFIsimdvi mask2 = pfiSimdCast_F32_I32(pfiSimdCmpEQ_F32(maxWeight, w2));
    PFIsimdvi mask3 = pfiSimdCast_F32_I32(pfiSimdCmpEQ_F32(maxWeight, w3));

    // Use masks to select the corresponding color
    return pfiSimdOr_I32(
        pfiSimdAnd_I32(mask1, c1),
        pfiSimdOr_I32(
            pfiSimdAnd_I32(mask2, c2),
            pfiSimdAnd_I32(mask3, c3)
        )
    );
}

static inline PFIsimdvi
pfiColorPackedGrayscale_simd(PFIsimdvi colors)
{
    // Extract the R, G, B channels
    PFIsimdvi r = pfiSimdAnd_I32(colors, *(PFIsimdvi*)GC_simd_i32_255);
    PFIsimdvi g = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 8), *(PFIsimdvi*)GC_simd_i32_255);
    PFIsimdvi b = pfiSimdAnd_I32(pfiSimdShr_I32(colors, 16), *(PFIsimdvi*)GC_simd_i32_255);

    // Integer coefficients approximating 0.299, 0.587, and 0.114 (multiplied by 256)
    const PFIsimdvi coeffR = pfiSimdSet1_I32(77);   // 0.299 * 256 = 76.8
    const PFIsimdvi coeffG = pfiSimdSet1_I32(150);  // 0.587 * 256 = 150.272
    const PFIsimdvi coeffB = pfiSimdSet1_I32(29);   // 0.114 * 256 = 29.184

    // Calculate the luminance using the integer coefficients
    PFIsimdvi gray = pfiSimdAdd_I32(
        pfiSimdAdd_I32(
            pfiSimdMullo_I32(b, coeffB),
            pfiSimdMullo_I32(g, coeffG)
        ),
        pfiSimdMullo_I32(r, coeffR)
    );

    // Divide by 256 (equivalent to a right shift by 8 bits)
    gray = pfiSimdShr_I32(gray, 8);

    // Repeat the luminance in the R, G, B channels
    PFIsimdvi grayRGB = pfiSimdOr_I32(
        pfiSimdOr_I32(pfiSimdShl_I32(gray, 16), pfiSimdShl_I32(gray, 8)),
        gray
    );

    // Extract the alpha from the original color (bits 24-31) and keep it unchanged
    PFIsimdvi alpha = pfiSimdAnd_I32(colors, pfiSimdSet1_I32(0xFF000000));

    // Combine the luminance (repeated in RGB) with the original alpha
    return pfiSimdOr_I32(grayRGB, alpha);
}

#endif //PF_SIMD_SUPPORT
#endif //PF_INTERNAL_COLOR_H
