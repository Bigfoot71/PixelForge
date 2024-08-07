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

#ifndef PF_HELPER_SIMD_H
#define PF_HELPER_SIMD_H

#include "../../pixelforge.h"
#include "../../pfm.h"

typedef PFMsimd_i
    PFsimd_color[4];

static inline void
pfInternal_SimdColorLoadUnpacked(PFsimd_color dst, PFcolor src)
{
    for (int_fast8_t i = 0; i < 4; ++i) {
        dst[i] = pfmSimdSet1_I32(((PFubyte*)&src)[i]);
    }
}

static inline void pfInternal_SimdColorUnpack(PFsimd_color out, const PFMsimd_i packed)
{
    const PFMsimd_i mask = pfmSimdSet1_I32(0xFF);

    out[0] = pfmSimdAnd_I32(packed, mask);
    out[1] = pfmSimdAnd_I32(pfmSimdShr_I32(packed, 8), mask);
    out[2] = pfmSimdAnd_I32(pfmSimdShr_I32(packed, 16), mask);
    out[3] = pfmSimdAnd_I32(pfmSimdShr_I32(packed, 24), mask);
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
pfInternal_SimdColorBary_SMOOTH(PFsimd_color out,
                                const PFsimd_color c1,
                                const PFsimd_color c2,
                                const PFsimd_color c3,
                                PFMsimd_f w1,
                                PFMsimd_f w2,
                                PFMsimd_f w3)
{
    // Multiply weights by 255 and convert them to integers
    PFMsimd_f scale = pfmSimdSet1_F32(255.0f);
    PFMsimd_i uW1 = pfmSimdConvert_F32_I32(pfmSimdMul_F32(w1, scale));
    PFMsimd_i uW2 = pfmSimdConvert_F32_I32(pfmSimdMul_F32(w2, scale));
    PFMsimd_i uW3 = pfmSimdConvert_F32_I32(pfmSimdMul_F32(w3, scale));

    // Perform multiplications and additions for each channel
    // Then approximate division by 255
    PFMsimd_i factor = pfmSimdSet1_I32(257);
    for (int_fast8_t i = 0; i < 4; ++i) {
        out[i] = pfmSimdAdd_I32(
            pfmSimdAdd_I32(
                pfmSimdMullo_I32(uW1, c1[i]), 
                pfmSimdMullo_I32(uW2, c2[i])),
                pfmSimdMullo_I32(uW3, c3[i]));
        out[i] = pfmSimdShr_I32(
            pfmSimdMullo_I32(out[i], factor), 16);
    }
}

static inline void
pfInternal_SimdColorBary_FLAT(PFsimd_color out,
                              const PFsimd_color c1,
                              const PFsimd_color c2,
                              const PFsimd_color c3,
                              PFMsimd_f w1,
                              PFMsimd_f w2,
                              PFMsimd_f w3)
{
    // Multiply weights by 255 and convert them to integers
    PFMsimd_f scale = pfmSimdSet1_F32(255.0f);
    PFMsimd_i uW1 = pfmSimdConvert_F32_I32(pfmSimdMul_F32(w1, scale));
    PFMsimd_i uW2 = pfmSimdConvert_F32_I32(pfmSimdMul_F32(w2, scale));
    PFMsimd_i uW3 = pfmSimdConvert_F32_I32(pfmSimdMul_F32(w3, scale));

    // Perform multiplications and additions for each channel
    // Then approximate division by 255
    PFMsimd_i factor = pfmSimdSet1_I32(257);
    for (int_fast8_t i = 0; i < 4; ++i) {
        out[i] = pfmSimdAdd_I32(
            pfmSimdAdd_I32(
                pfmSimdMullo_I32(uW1, c1[i]), 
                pfmSimdMullo_I32(uW2, c2[i])),
                pfmSimdMullo_I32(uW3, c3[i]));
        out[i] = pfmSimdShr_I32(
            pfmSimdMullo_I32(out[i], factor), 16);
    }
}

#endif //PF_HELPER_SIMD_H
