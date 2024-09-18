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

#ifndef PF_INTERNAL_BLEND_H
#define PF_INTERNAL_BLEND_H

#include "./context/context.h"
#include "../pixelforge.h"
#include "./color.h"

/* SISD Blending functions */

static inline PFcolor
pfiBlendAverage(PFcolor src, PFcolor dst)
{
    PFcolor result;
    result.r = (PFubyte)((src.r + dst.r) >> 1);
    result.g = (PFubyte)((src.g + dst.g) >> 1);
    result.b = (PFubyte)((src.b + dst.b) >> 1);
    result.a = (PFubyte)((src.a + dst.a) >> 1);
    return result;
}

static inline PFcolor
pfiBlendAlpha(PFcolor src, PFcolor dst)
{
    PFuint alpha = src.a + 1;
    PFuint invAlpha = 256 - alpha;

    PFcolor result;
    result.r = (PFubyte)((alpha*src.r + invAlpha*dst.r) >> 8);
    result.g = (PFubyte)((alpha*src.g + invAlpha*dst.g) >> 8);
    result.b = (PFubyte)((alpha*src.b + invAlpha*dst.b) >> 8);
    result.a = (PFubyte)((alpha*255 + invAlpha*dst.a) >> 8);
    return result;
}

static inline PFcolor
pfiBlendAdditive(PFcolor src, PFcolor dst)
{
    PFcolor result;
    result.r = (PFubyte)PF_MIN_255((PFint)(dst.r + src.r));
    result.g = (PFubyte)PF_MIN_255((PFint)(dst.g + src.g));
    result.b = (PFubyte)PF_MIN_255((PFint)(dst.b + src.b));
    result.a = (PFubyte)PF_MIN_255((PFint)(dst.a + src.a));
    return result;
}

static inline PFcolor
pfiBlendSubtractive(PFcolor src, PFcolor dst)
{
    PFcolor result;
    result.r = (PFubyte)PF_MAX_0((PFint)(dst.r - src.r));
    result.g = (PFubyte)PF_MAX_0((PFint)(dst.g - src.g));
    result.b = (PFubyte)PF_MAX_0((PFint)(dst.b - src.b));
    result.a = (PFubyte)PF_MAX_0((PFint)(dst.a - src.a));
    return result;
}

static inline PFcolor
pfiBlendMultiplicative(PFcolor src, PFcolor dst)
{
    PFcolor result;
    result.r = (PFubyte)((src.r*dst.r)/255);
    result.g = (PFubyte)((src.g*dst.g)/255);
    result.b = (PFubyte)((src.b*dst.b)/255);
    result.a = (PFubyte)((src.a*dst.a)/255);
    return result;
}

static inline PFcolor
pfiBlendScreen(PFcolor src, PFcolor dst)
{
    PFcolor result;
    result.r = (PFubyte)PF_MIN_255((PFint)((dst.r*(255 - src.r) >> 8) + src.r));
    result.g = (PFubyte)PF_MIN_255((PFint)((dst.g*(255 - src.g) >> 8) + src.g));
    result.b = (PFubyte)PF_MIN_255((PFint)((dst.b*(255 - src.b) >> 8) + src.b));
    result.a = (PFubyte)PF_MIN_255((PFint)((dst.a*(255 - src.a) >> 8) + src.a));
    return result;
}

static inline PFcolor
pfiBlendLighten(PFcolor src, PFcolor dst)
{
    PFcolor result;
    result.r = (PFubyte)(PF_MAX(src.r, dst.r));
    result.g = (PFubyte)(PF_MAX(src.g, dst.g));
    result.b = (PFubyte)(PF_MAX(src.b, dst.b));
    result.a = (PFubyte)(PF_MAX(src.a, dst.a));
    return result;
}

static inline PFcolor
pfiBlendDarken(PFcolor src, PFcolor dst)
{
    PFcolor result;
    result.r = (PFubyte)(PF_MIN(src.r, dst.r));
    result.g = (PFubyte)(PF_MIN(src.g, dst.g));
    result.b = (PFubyte)(PF_MIN(src.b, dst.b));
    result.a = (PFubyte)(PF_MIN(src.a, dst.a));
    return result;
}

#define ENTRY(MODE, FUNC) [MODE] = FUNC
static const PFIblendfunc GC_blendFuncs[8] = {
    ENTRY(PF_BLEND_AVERAGE, pfiBlendAverage),
    ENTRY(PF_BLEND_ALPHA, pfiBlendAlpha),
    ENTRY(PF_BLEND_ADD, pfiBlendAdditive),
    ENTRY(PF_BLEND_SUB, pfiBlendSubtractive),
    ENTRY(PF_BLEND_MUL, pfiBlendMultiplicative),
    ENTRY(PF_BLEND_SCREEN, pfiBlendScreen),
    ENTRY(PF_BLEND_LIGHTEN, pfiBlendLighten),
    ENTRY(PF_BLEND_DARKEN, pfiBlendDarken)
};
#undef ENTRY

/* SIMD Blending functions */

#if PF_SIMD_SUPPORT

static inline PFIsimdvi
pfiBlendAverage_simd(const PFIsimdvi src, const PFIsimdvi dst)
{
    PFIsimdvi srcV[4], dstV[4], outV[4];
    pfiColorSIMDToVecI_simd(srcV, src, 4);
    pfiColorSIMDToVecI_simd(dstV, dst, 4);

    outV[0] = pfiSimdShr_I32(pfiSimdAdd_I32(srcV[0], dstV[0]), 1);
    outV[1] = pfiSimdShr_I32(pfiSimdAdd_I32(srcV[1], dstV[1]), 1);
    outV[2] = pfiSimdShr_I32(pfiSimdAdd_I32(srcV[2], dstV[2]), 1);
    outV[3] = pfiSimdShr_I32(pfiSimdAdd_I32(srcV[3], dstV[3]), 1);

    return pfiColorSIMDFromVecI_simd(outV, 4);
}

static inline PFIsimdvi
pfiBlendAlpha_simd(const PFIsimdvi src, const PFIsimdvi dst)
{
    PFIsimdvi srcV[4], dstV[4], outV[4];
    pfiColorSIMDToVecI_simd(srcV, src, 4);
    pfiColorSIMDToVecI_simd(dstV, dst, 4);

    PFIsimdvi alpha = pfiSimdAdd_I32(srcV[3], *(PFIsimdvi*)GC_simd_i32_1);
    PFIsimdvi invAlpha = pfiSimdSub_I32(*(PFIsimdvi*)GC_simd_i32_256, alpha);
    outV[0] = pfiSimdShr_I32(pfiSimdAdd_I32(pfiSimdMullo_I32(srcV[0], alpha), pfiSimdMullo_I32(dstV[0], invAlpha)), 8);
    outV[1] = pfiSimdShr_I32(pfiSimdAdd_I32(pfiSimdMullo_I32(srcV[1], alpha), pfiSimdMullo_I32(dstV[1], invAlpha)), 8);
    outV[2] = pfiSimdShr_I32(pfiSimdAdd_I32(pfiSimdMullo_I32(srcV[2], alpha), pfiSimdMullo_I32(dstV[2], invAlpha)), 8);
    outV[3] = pfiSimdShr_I32(pfiSimdAdd_I32(pfiSimdMullo_I32(*(PFIsimdvi*)GC_simd_i32_255, alpha), pfiSimdMullo_I32(dstV[3], invAlpha)), 8);

    return pfiColorSIMDFromVecI_simd(outV, 4);
}

static inline PFIsimdvi
pfiBlendAdditive_simd(const PFIsimdvi src, const PFIsimdvi dst)
{
    PFIsimdvi srcV[4], dstV[4], outV[4];
    pfiColorSIMDToVecI_simd(srcV, src, 4);
    pfiColorSIMDToVecI_simd(dstV, dst, 4);

    outV[0] = pfiSimdMin_I32(pfiSimdAdd_I32(srcV[0], dstV[0]), *(PFIsimdvi*)GC_simd_i32_255);
    outV[1] = pfiSimdMin_I32(pfiSimdAdd_I32(srcV[1], dstV[1]), *(PFIsimdvi*)GC_simd_i32_255);
    outV[2] = pfiSimdMin_I32(pfiSimdAdd_I32(srcV[2], dstV[2]), *(PFIsimdvi*)GC_simd_i32_255);
    outV[3] = pfiSimdMin_I32(pfiSimdAdd_I32(srcV[3], dstV[3]), *(PFIsimdvi*)GC_simd_i32_255);

    return pfiColorSIMDFromVecI_simd(outV, 4);
}

static inline PFIsimdvi
pfiBlendSubtractive_simd(const PFIsimdvi src, const PFIsimdvi dst)
{
    PFIsimdvi srcV[4], dstV[4], outV[4];
    pfiColorSIMDToVecI_simd(srcV, src, 4);
    pfiColorSIMDToVecI_simd(dstV, dst, 4);

    outV[0] = pfiSimdMax_I32(pfiSimdAdd_I32(srcV[0], dstV[0]), *(PFIsimdvi*)GC_simd_i32_0);
    outV[1] = pfiSimdMax_I32(pfiSimdAdd_I32(srcV[1], dstV[1]), *(PFIsimdvi*)GC_simd_i32_0);
    outV[2] = pfiSimdMax_I32(pfiSimdAdd_I32(srcV[2], dstV[2]), *(PFIsimdvi*)GC_simd_i32_0);
    outV[3] = pfiSimdMax_I32(pfiSimdAdd_I32(srcV[3], dstV[3]), *(PFIsimdvi*)GC_simd_i32_0);

    return pfiColorSIMDFromVecI_simd(outV, 4);
}

static inline PFIsimdvi
pfiBlendMultiplicative_simd(const PFIsimdvi src, const PFIsimdvi dst)
{
    PFIsimdvi srcV[4], dstV[4], outV[4];
    pfiColorSIMDToVecI_simd(srcV, src, 4);
    pfiColorSIMDToVecI_simd(dstV, dst, 4);

    outV[0] = pfiSimdShr_I32(pfiSimdMullo_I32(srcV[0], dstV[0]), 8);
    outV[1] = pfiSimdShr_I32(pfiSimdMullo_I32(srcV[1], dstV[1]), 8);
    outV[2] = pfiSimdShr_I32(pfiSimdMullo_I32(srcV[2], dstV[2]), 8);
    outV[3] = pfiSimdShr_I32(pfiSimdMullo_I32(srcV[3], dstV[3]), 8);

    return pfiColorSIMDFromVecI_simd(outV, 4);
}

static inline PFIsimdvi
pfiBlendScreen_simd(const PFIsimdvi src, const PFIsimdvi dst)
{
    PFIsimdvi srcV[4], dstV[4], outV[4];
    pfiColorSIMDToVecI_simd(srcV, src, 4);
    pfiColorSIMDToVecI_simd(dstV, dst, 4);

    PFIsimdvi inv_src_r = pfiSimdSub_I32(*(PFIsimdvi*)GC_simd_i32_255, srcV[0]);
    PFIsimdvi inv_src_g = pfiSimdSub_I32(*(PFIsimdvi*)GC_simd_i32_255, srcV[1]);
    PFIsimdvi inv_src_b = pfiSimdSub_I32(*(PFIsimdvi*)GC_simd_i32_255, srcV[2]);
    PFIsimdvi inv_src_a = pfiSimdSub_I32(*(PFIsimdvi*)GC_simd_i32_255, srcV[3]);

    outV[0] = pfiSimdMin_I32(pfiSimdAdd_I32(pfiSimdShr_I32(pfiSimdMullo_I32(dstV[0], inv_src_r), 8), srcV[0]), *(PFIsimdvi*)GC_simd_i32_255);
    outV[1] = pfiSimdMin_I32(pfiSimdAdd_I32(pfiSimdShr_I32(pfiSimdMullo_I32(dstV[1], inv_src_g), 8), srcV[1]), *(PFIsimdvi*)GC_simd_i32_255);
    outV[2] = pfiSimdMin_I32(pfiSimdAdd_I32(pfiSimdShr_I32(pfiSimdMullo_I32(dstV[2], inv_src_b), 8), srcV[2]), *(PFIsimdvi*)GC_simd_i32_255);
    outV[3] = pfiSimdMin_I32(pfiSimdAdd_I32(pfiSimdShr_I32(pfiSimdMullo_I32(dstV[3], inv_src_a), 8), srcV[3]), *(PFIsimdvi*)GC_simd_i32_255);

    return pfiColorSIMDFromVecI_simd(outV, 4);
}

static inline PFIsimdvi
pfiBlendLighten_simd(const PFIsimdvi src, const PFIsimdvi dst)
{
    PFIsimdvi srcV[4], dstV[4], outV[4];
    pfiColorSIMDToVecI_simd(srcV, src, 4);
    pfiColorSIMDToVecI_simd(dstV, dst, 4);

    outV[0] = pfiSimdMax_I32(srcV[0], dstV[0]);
    outV[1] = pfiSimdMax_I32(srcV[1], dstV[1]);
    outV[2] = pfiSimdMax_I32(srcV[2], dstV[2]);
    outV[3] = pfiSimdMax_I32(srcV[3], dstV[3]);

    return pfiColorSIMDFromVecI_simd(outV, 4);
}

static inline PFIsimdvi
pfiBlendDarken_simd(const PFIsimdvi src, const PFIsimdvi dst)
{
    PFIsimdvi srcV[4], dstV[4], outV[4];
    pfiColorSIMDToVecI_simd(srcV, src, 4);
    pfiColorSIMDToVecI_simd(dstV, dst, 4);

    outV[0] = pfiSimdMin_I32(srcV[0], dstV[0]);
    outV[1] = pfiSimdMin_I32(srcV[1], dstV[1]);
    outV[2] = pfiSimdMin_I32(srcV[2], dstV[2]);
    outV[3] = pfiSimdMin_I32(srcV[3], dstV[3]);

    return pfiColorSIMDFromVecI_simd(outV, 4);
}

#define ENTRY(MODE, FUNC) [MODE] = FUNC
static const PFIblendfunc_simd GC_blendFuncs_simd[8] = {
    ENTRY(PF_BLEND_AVERAGE, pfiBlendAverage_simd),
    ENTRY(PF_BLEND_ALPHA, pfiBlendAlpha_simd),
    ENTRY(PF_BLEND_ADD, pfiBlendAdditive_simd),
    ENTRY(PF_BLEND_SUB, pfiBlendSubtractive_simd),
    ENTRY(PF_BLEND_MUL, pfiBlendMultiplicative_simd),
    ENTRY(PF_BLEND_SCREEN, pfiBlendScreen_simd),
    ENTRY(PF_BLEND_LIGHTEN, pfiBlendLighten_simd),
    ENTRY(PF_BLEND_DARKEN, pfiBlendDarken_simd)
};
#undef ENTRY

#endif //PF_SIMD_SUPPORT


/* Helper Functions */

static inline PFboolean
pfiIsBlendModeValid(PFblendmode mode)
{
    return (mode >= PF_BLEND_AVERAGE && mode <= PF_BLEND_DARKEN);
}


#endif //PF_INTERNAL_BLEND_H
