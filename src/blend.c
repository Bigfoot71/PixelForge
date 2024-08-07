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

#include "internal/simd/helper_simd.h"
#include "pixelforge.h"
#include "pfm.h"

/* Blending API functions */

PFcolor pfBlend(PFcolor src, PFcolor dst)
{
    PFcolor result;
    result.r = (PFubyte)((src.r + dst.r) >> 1);
    result.g = (PFubyte)((src.g + dst.g) >> 1);
    result.b = (PFubyte)((src.b + dst.b) >> 1);
    result.a = (PFubyte)((src.a + dst.a) >> 1);
    return result;
}

PFcolor pfBlendAlpha(PFcolor src, PFcolor dst)
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

PFcolor pfBlendAdditive(PFcolor src, PFcolor dst)
{
    PFcolor result;
    result.r = (PFubyte)MIN_255((PFint)(dst.r + src.r));
    result.g = (PFubyte)MIN_255((PFint)(dst.g + src.g));
    result.b = (PFubyte)MIN_255((PFint)(dst.b + src.b));
    result.a = (PFubyte)MIN_255((PFint)(dst.a + src.a));
    return result;
}

PFcolor pfBlendSubtractive(PFcolor src, PFcolor dst)
{
    PFcolor result;
    result.r = (PFubyte)MAX_0((PFint)(dst.r - src.r));
    result.g = (PFubyte)MAX_0((PFint)(dst.g - src.g));
    result.b = (PFubyte)MAX_0((PFint)(dst.b - src.b));
    result.a = (PFubyte)MAX_0((PFint)(dst.a - src.a));
    return result;
}

PFcolor pfBlendMultiplicative(PFcolor src, PFcolor dst)
{
    PFcolor result;
    result.r = (PFubyte)((src.r*dst.r)/255);
    result.g = (PFubyte)((src.g*dst.g)/255);
    result.b = (PFubyte)((src.b*dst.b)/255);
    result.a = (PFubyte)((src.a*dst.a)/255);
    return result;
}

PFcolor pfBlendScreen(PFcolor src, PFcolor dst)
{
    PFcolor result;
    result.r = (PFubyte)MIN_255((PFint)((dst.r*(255 - src.r) >> 8) + src.r));
    result.g = (PFubyte)MIN_255((PFint)((dst.g*(255 - src.g) >> 8) + src.g));
    result.b = (PFubyte)MIN_255((PFint)((dst.b*(255 - src.b) >> 8) + src.b));
    result.a = (PFubyte)MIN_255((PFint)((dst.a*(255 - src.a) >> 8) + src.a));
    return result;
}

PFcolor pfBlendLighten(PFcolor src, PFcolor dst)
{
    PFcolor result;
    result.r = (PFubyte)(MAX(src.r, dst.r));
    result.g = (PFubyte)(MAX(src.g, dst.g));
    result.b = (PFubyte)(MAX(src.b, dst.b));
    result.a = (PFubyte)(MAX(src.a, dst.a));
    return result;
}

PFcolor pfBlendDarken(PFcolor src, PFcolor dst)
{
    PFcolor result;
    result.r = (PFubyte)(MIN(src.r, dst.r));
    result.g = (PFubyte)(MIN(src.g, dst.g));
    result.b = (PFubyte)(MIN(src.b, dst.b));
    result.a = (PFubyte)(MIN(src.a, dst.a));
    return result;
}

/* SIMD Blending API functions */

void pfBlendSimd(PFsimd_color out, const PFsimd_color src, const PFsimd_color dst)
{
    out[0] = pfmSimdShr_I32(pfmSimdAdd_I32(src[0], dst[0]), 1);
    out[1] = pfmSimdShr_I32(pfmSimdAdd_I32(src[1], dst[1]), 1);
    out[2] = pfmSimdShr_I32(pfmSimdAdd_I32(src[2], dst[2]), 1);
    out[3] = pfmSimdShr_I32(pfmSimdAdd_I32(src[3], dst[3]), 1);
}

void pfBlendAlphaSimd(PFsimd_color out, const PFsimd_color src, const PFsimd_color dst)
{
    PFMsimd_i alpha = pfmSimdAdd_I32(src[0], pfmSimdSet1_I32(1));
    PFMsimd_i invAlpha = pfmSimdSub_I32(pfmSimdSet1_I32(256), alpha);
    out[0] = pfmSimdShr_I32(pfmSimdAdd_I32(pfmSimdMullo_I32(src[0], alpha), pfmSimdMullo_I32(dst[0], invAlpha)), 8);
    out[1] = pfmSimdShr_I32(pfmSimdAdd_I32(pfmSimdMullo_I32(src[1], alpha), pfmSimdMullo_I32(dst[1], invAlpha)), 8);
    out[2] = pfmSimdShr_I32(pfmSimdAdd_I32(pfmSimdMullo_I32(src[2], alpha), pfmSimdMullo_I32(dst[2], invAlpha)), 8);
    out[3] = pfmSimdShr_I32(pfmSimdAdd_I32(pfmSimdMullo_I32(pfmSimdSet1_I32(255), alpha), pfmSimdMullo_I32(dst[3], invAlpha)), 8);
}

void pfBlendAdditiveSimd(PFsimd_color out, const PFsimd_color src, const PFsimd_color dst)
{
    const PFMsimd_i max = pfmSimdSet1_I32(255);
    out[0] = pfmSimdMin_I32(pfmSimdAdd_I32(src[0], dst[0]), max);
    out[1] = pfmSimdMin_I32(pfmSimdAdd_I32(src[1], dst[1]), max);
    out[2] = pfmSimdMin_I32(pfmSimdAdd_I32(src[2], dst[2]), max);
    out[3] = pfmSimdMin_I32(pfmSimdAdd_I32(src[3], dst[3]), max);
}

void pfBlendSubtractiveSimd(PFsimd_color out, const PFsimd_color src, const PFsimd_color dst)
{
    const PFMsimd_i min = pfmSimdSetZero_I32();
    out[0] = pfmSimdMax_I32(pfmSimdAdd_I32(src[0], dst[0]), min);
    out[1] = pfmSimdMax_I32(pfmSimdAdd_I32(src[1], dst[1]), min);
    out[2] = pfmSimdMax_I32(pfmSimdAdd_I32(src[2], dst[2]), min);
    out[3] = pfmSimdMax_I32(pfmSimdAdd_I32(src[3], dst[3]), min);
}

void pfBlendMultiplicativeSimd(PFsimd_color out, const PFsimd_color src, const PFsimd_color dst)
{
    out[0] = pfmSimdShr_I32(pfmSimdMullo_I32(src[0], dst[0]), 8);
    out[1] = pfmSimdShr_I32(pfmSimdMullo_I32(src[1], dst[1]), 8);
    out[2] = pfmSimdShr_I32(pfmSimdMullo_I32(src[2], dst[2]), 8);
    out[3] = pfmSimdShr_I32(pfmSimdMullo_I32(src[3], dst[3]), 8);
}

void pfBlendScreenSimd(PFsimd_color out, const PFsimd_color src, const PFsimd_color dst)
{
    const PFMsimd_i max_val = pfmSimdSet1_I32(255);
    PFMsimd_i inv_src_r = pfmSimdSub_I32(max_val, src[0]);
    PFMsimd_i inv_src_g = pfmSimdSub_I32(max_val, src[1]);
    PFMsimd_i inv_src_b = pfmSimdSub_I32(max_val, src[2]);
    PFMsimd_i inv_src_a = pfmSimdSub_I32(max_val, src[3]);

    out[0] = pfmSimdMin_I32(pfmSimdAdd_I32(pfmSimdShr_I32(pfmSimdMullo_I32(dst[0], inv_src_r), 8), src[0]), max_val);
    out[1] = pfmSimdMin_I32(pfmSimdAdd_I32(pfmSimdShr_I32(pfmSimdMullo_I32(dst[1], inv_src_g), 8), src[1]), max_val);
    out[2] = pfmSimdMin_I32(pfmSimdAdd_I32(pfmSimdShr_I32(pfmSimdMullo_I32(dst[2], inv_src_b), 8), src[2]), max_val);
    out[3] = pfmSimdMin_I32(pfmSimdAdd_I32(pfmSimdShr_I32(pfmSimdMullo_I32(dst[3], inv_src_a), 8), src[3]), max_val);
}

void pfBlendLightenSimd(PFsimd_color out, const PFsimd_color src, const PFsimd_color dst)
{
    out[0] = pfmSimdMax_I32(src[0], dst[0]);
    out[1] = pfmSimdMax_I32(src[1], dst[1]);
    out[2] = pfmSimdMax_I32(src[2], dst[2]);
    out[3] = pfmSimdMax_I32(src[3], dst[3]);
}

void pfBlendDarkenSimd(PFsimd_color out, const PFsimd_color src, const PFsimd_color dst)
{
    out[0] = pfmSimdMin_I32(src[0], dst[0]);
    out[1] = pfmSimdMin_I32(src[1], dst[1]);
    out[2] = pfmSimdMin_I32(src[2], dst[2]);
    out[3] = pfmSimdMin_I32(src[3], dst[3]);
}
