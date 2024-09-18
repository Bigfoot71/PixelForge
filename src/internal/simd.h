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

#ifndef PF_INTERNAL_SIMD_H
#define PF_INTERNAL_SIMD_H

//#define __STDC_WANT_IEC_60559_TYPES_EXT__   ///< To check if FC16 (_Float16) is supported

#include "../pfm.h"

#include <immintrin.h>
#include <float.h>

#if defined(__AVX2__)
#   define PF_SIMD_SIZE 8
#elif defined(__SSE2__)
#   define PF_SIMD_SIZE 4
#else
#   define PF_SIMD_SIZE 0
#endif

#define PF_SIMD_SUPPORT\
    (PF_SIMD_SIZE > 0)

#if PF_SIMD_SUPPORT

/* SIMD types definitions */

#if defined(__AVX2__)
typedef __m256 PFIsimdvf;
typedef __m256i PFIsimdvi;
#elif defined(__SSE2__)
typedef __m128 PFIsimdvf;
typedef __m128i PFIsimdvi;
#endif

typedef PFIsimdvf PFIsimdv2f[2];
typedef PFIsimdvf PFIsimdv3f[3];
typedef PFIsimdvf PFIsimdv4f[4];

typedef PFIsimdvf* PFIsimdvf_ptr;

/* SIMD constants  */

#if defined(__AVX2__)
#   ifdef _MSC_VER
#       define ALIGN32_BEG __declspec(align(32))
#       define ALIGN32_END 
#   else /* gcc or icc */
#       define ALIGN32_BEG
#       define ALIGN32_END __attribute__((aligned(32)))
#   endif
#   define GC_SIMD_F32(Name, Val) static const ALIGN32_BEG float GC_simd_f32_##Name[8] ALIGN32_END = { Val, Val, Val, Val, Val, Val, Val, Val }
#   define GC_SIMD_I32(Name, Val) static const ALIGN32_BEG int GC_simd_i32_##Name[8] ALIGN32_END = { Val, Val, Val, Val, Val, Val, Val, Val }
#   define GC_SIMD_F32_TYPE(Name, Type, Val) static const ALIGN32_BEG Type GC_simd_f32_##Name[8] ALIGN32_END = { Val, Val, Val, Val, Val, Val, Val, Val }
#elif defined(__SSE2__)
#   ifdef _MSC_VER
#       define ALIGN16_BEG __declspec(align(16))
#       define ALIGN16_END 
#   else /* gcc or icc */
#       define ALIGN16_BEG
#       define ALIGN16_END __attribute__((aligned(16)))
#   endif
#   define GC_SIMD_F32(Name, Val) static const ALIGN16_BEG float GC_simd_f32_##Name[4] ALIGN16_END = { Val, Val, Val, Val }
#   define GC_SIMD_I32(Name, Val) static const ALIGN16_BEG int GC_simd_i32_##Name[4] ALIGN16_END = { Val, Val, Val, Val }
#   define GC_SIMD_F32_TYPE(Name, Type, Val) static const ALIGN16_BEG Type GC_simd_f32_##Name[4] ALIGN16_END = { Val, Val, Val, Val }
#endif

GC_SIMD_F32(epsilon, 1e-5f);
GC_SIMD_F32(0, 0.0f);
GC_SIMD_F32(0p5, 0.5f);
GC_SIMD_F32(inv255, 1.0f/255.0f);
GC_SIMD_F32(1, 1.0f);
GC_SIMD_F32(2, 2.0f);
GC_SIMD_F32(255, 255.0f);

// the smallest non denormalized float number
GC_SIMD_F32_TYPE(min_norm_pos, int, 0x00800000);
GC_SIMD_F32_TYPE(mant_mask, int, 0x7f800000);
GC_SIMD_F32_TYPE(inv_mant_mask, int, ~0x7f800000);

GC_SIMD_F32_TYPE(sign_mask, int, 0x80000000);
GC_SIMD_F32_TYPE(inv_sign_mask, int, ~0x80000000);

GC_SIMD_I32(0, 0);
GC_SIMD_I32(1, 1);
GC_SIMD_I32(inv1, ~1);
GC_SIMD_I32(2, 2);
GC_SIMD_I32(3, 3);
GC_SIMD_I32(4, 4);
GC_SIMD_I32(255, 255);
GC_SIMD_I32(256, 256);
GC_SIMD_I32(257, 257);
GC_SIMD_I32(0x7f, 0x7f);
GC_SIMD_I32(0xffffffff, 0xffffffff);

GC_SIMD_F32(cephes_SQRTHF, 0.707106781186547524);
GC_SIMD_F32(cephes_log_p0, 7.0376836292E-2);
GC_SIMD_F32(cephes_log_p1, - 1.1514610310E-1);
GC_SIMD_F32(cephes_log_p2, 1.1676998740E-1);
GC_SIMD_F32(cephes_log_p3, - 1.2420140846E-1);
GC_SIMD_F32(cephes_log_p4, + 1.4249322787E-1);
GC_SIMD_F32(cephes_log_p5, - 1.6668057665E-1);
GC_SIMD_F32(cephes_log_p6, + 2.0000714765E-1);
GC_SIMD_F32(cephes_log_p7, - 2.4999993993E-1);
GC_SIMD_F32(cephes_log_p8, + 3.3333331174E-1);
GC_SIMD_F32(cephes_log_q1, -2.12194440e-4);
GC_SIMD_F32(cephes_log_q2, 0.693359375);

GC_SIMD_F32(exp_hi,	88.3762626647949f);
GC_SIMD_F32(exp_lo,	-88.3762626647949f);

GC_SIMD_F32(cephes_LOG2EF, 1.44269504088896341);
GC_SIMD_F32(cephes_exp_C1, 0.693359375);
GC_SIMD_F32(cephes_exp_C2, -2.12194440e-4);

GC_SIMD_F32(cephes_exp_p0, 1.9875691500E-4);
GC_SIMD_F32(cephes_exp_p1, 1.3981999507E-3);
GC_SIMD_F32(cephes_exp_p2, 8.3334519073E-3);
GC_SIMD_F32(cephes_exp_p3, 4.1665795894E-2);
GC_SIMD_F32(cephes_exp_p4, 1.6666665459E-1);
GC_SIMD_F32(cephes_exp_p5, 5.0000001201E-1);

/* SIMD helper functions */

#if defined(__AVX2__)

/**
 *  NOTE: This extract allows you to perform the 'log' and 'exp' operations for AVX2.
 *        This is an AVX2 adaptation of Julien Pommier's SSE2 implementation by Giovanni Garberoglio.
 *
 * SOURCE: http://web.archive.org/web/20200216175033/http://software-lisc.fbk.eu/avx_mathfun/
 *
 * LICENSE:
 *  Based on "sse_mathfun.h", by Julien Pommier
 *  http://gruntthepeon.free.fr/ssemath/
 *
 *  Copyright (C) 2012 Giovanni Garberoglio
 *  Interdisciplinary Laboratory for Computational Science (LISC)
 *  Fondazione Bruno Kessler and University of Trento
 *  via Sommarive, 18
 *  I-38123 Trento (Italy)
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty.  In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *      claim that you wrote the original software. If you use this software
 *      in a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 *
 *  (this is the zlib license)
 */

/**
 * natural logarithm computed for 8 simultaneous float 
 * return NaN for x <= 0
 */
static inline __m256
_mm256_log_ps(__m256 x)
{
    __m256i imm0;
    __m256 one = *(__m256*)GC_simd_f32_1;

    __m256 invalid_mask = _mm256_cmp_ps(x, _mm256_setzero_ps(), _CMP_LE_OS);

    x = _mm256_max_ps(x, *(__m256*)GC_simd_f32_min_norm_pos);  /* cut off denormalized stuff */

    imm0 = _mm256_srli_epi32(_mm256_castps_si256(x), 23);

    // keep only the fractional part
    x = _mm256_and_ps(x, *(__m256*)GC_simd_f32_inv_mant_mask);
    x = _mm256_or_ps(x, *(__m256*)GC_simd_f32_0p5);

    imm0 = _mm256_sub_epi32(imm0, *(__m256i*)GC_simd_i32_0x7f);
    __m256 e = _mm256_cvtepi32_ps(imm0);

    e = _mm256_add_ps(e, one);

    /* part2: 
       if( x < SQRTHF )
       {
            e -= 1;
            x = x + x - 1.0;
       }    else { x = x - 1.0; }
    */
    __m256 mask = _mm256_cmp_ps(x, *(__m256*)GC_simd_f32_cephes_SQRTHF, _CMP_LT_OS);
    __m256 tmp = _mm256_and_ps(x, mask);
    x = _mm256_sub_ps(x, one);
    e = _mm256_sub_ps(e, _mm256_and_ps(one, mask));
    x = _mm256_add_ps(x, tmp);

    __m256 z = _mm256_mul_ps(x,x);

    __m256 y = *(__m256*)GC_simd_f32_cephes_log_p0;
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)GC_simd_f32_cephes_log_p1);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)GC_simd_f32_cephes_log_p2);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)GC_simd_f32_cephes_log_p3);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)GC_simd_f32_cephes_log_p4);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)GC_simd_f32_cephes_log_p5);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)GC_simd_f32_cephes_log_p6);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)GC_simd_f32_cephes_log_p7);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)GC_simd_f32_cephes_log_p8);
    y = _mm256_mul_ps(y, x);

    y = _mm256_mul_ps(y, z);
  
    tmp = _mm256_mul_ps(e, *(__m256*)GC_simd_f32_cephes_log_q1);
    y = _mm256_add_ps(y, tmp);

    tmp = _mm256_mul_ps(z, *(__m256*)GC_simd_f32_0p5);
    y = _mm256_sub_ps(y, tmp);

    tmp = _mm256_mul_ps(e, *(__m256*)GC_simd_f32_cephes_log_q2);
    x = _mm256_add_ps(x, y);
    x = _mm256_add_ps(x, tmp);
    x = _mm256_or_ps(x, invalid_mask); // negative arg will be NAN

    return x;
}

static inline __m256
_mm256_exp_ps(__m256 x)
{
  __m256 tmp = _mm256_setzero_ps(), fx;
  __m256i imm0;
  __m256 one = *(__m256*)GC_simd_f32_1;

  x = _mm256_min_ps(x, *(__m256*)GC_simd_f32_exp_hi);
  x = _mm256_max_ps(x, *(__m256*)GC_simd_f32_exp_lo);

  // express exp(x) as exp(g + n*log(2))
  fx = _mm256_mul_ps(x, *(__m256*)GC_simd_f32_cephes_LOG2EF);
  fx = _mm256_add_ps(fx, *(__m256*)GC_simd_f32_0p5);

  tmp = _mm256_floor_ps(fx);

  // if greater, substract 1
  __m256 mask = _mm256_cmp_ps(tmp, fx, _CMP_GT_OS);    
  mask = _mm256_and_ps(mask, one);
  fx = _mm256_sub_ps(tmp, mask);

  tmp = _mm256_mul_ps(fx, *(__m256*)GC_simd_f32_cephes_exp_C1);
  __m256 z = _mm256_mul_ps(fx, *(__m256*)GC_simd_f32_cephes_exp_C2);
  x = _mm256_sub_ps(x, tmp);
  x = _mm256_sub_ps(x, z);

  z = _mm256_mul_ps(x,x);
  
  __m256 y = *(__m256*)GC_simd_f32_cephes_exp_p0;
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(__m256*)GC_simd_f32_cephes_exp_p1);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(__m256*)GC_simd_f32_cephes_exp_p2);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(__m256*)GC_simd_f32_cephes_exp_p3);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(__m256*)GC_simd_f32_cephes_exp_p4);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(__m256*)GC_simd_f32_cephes_exp_p5);
  y = _mm256_mul_ps(y, z);
  y = _mm256_add_ps(y, x);
  y = _mm256_add_ps(y, one);

  // build 2^n
  imm0 = _mm256_cvttps_epi32(fx);
  imm0 = _mm256_add_epi32(imm0, *(__m256i*)GC_simd_i32_0x7f);
  imm0 = _mm256_slli_epi32(imm0, 23);
  __m256 pow2n = _mm256_castsi256_ps(imm0);
  y = _mm256_mul_ps(y, pow2n);
  return y;
}

#elif defined(__SSE2__)

static inline __m128i
_mm_mullo_epi32_sse2(__m128i x, __m128i y)
{
    // From Agner Fog's Vector Class Library
    // SEE: https://github.com/vectorclass/version2/blob/master/vectori128.h#L3108

    __m128i x13 = _mm_shuffle_epi32(x, 0xF5);              // (-,x3,-,x1)
    __m128i y13 = _mm_shuffle_epi32(y, 0xF5);              // (-,y3,-,y1)
    __m128i prod02 = _mm_mul_epu32(x, y);                  // (-,x2*y2,-,x0*y0)
    __m128i prod13 = _mm_mul_epu32(x13, y13);              // (-,x3*y3,-,x1*y1)
    __m128i prod01 = _mm_unpacklo_epi32(prod02, prod13);   // (-,-,x1*y1,x0*y0)
    __m128i prod23 = _mm_unpackhi_epi32(prod02, prod13);   // (-,-,x3*y3,x2*y2)
    return           _mm_unpacklo_epi64(prod01, prod23);   // (xy3,xy2,xy1,xy0)
}

static inline __m128i
_mm_shuffle_epi8_sse2(__m128i x, __m128i y)
{
    // From Agner Fog's Vector Class Library
    // SEE: https://github.com/vectorclass/version2/blob/master/vectori128.h#L5516

    uint8_t yy[16];
    int8_t  xx[16], rr[16];
    _mm_storeu_si128((__m128i*)xx, x);
    _mm_storeu_si128((__m128i*)yy, y);
    for (int j = 0; j < 16; j++) rr[j] = xx[yy[j] & 0x0F];
    return _mm_loadu_si128((__m128i const*)rr);
}

__m128i
_mm_blendv_epi8_sse2(__m128i x, __m128i y, __m128i mask)
{
    __m128i not_mask = _mm_andnot_si128(mask, x);   // _mm_andnot_si128(mask, x) : bits of x where mask is 0
    __m128i masked_y = _mm_and_si128(mask, y);      // _mm_and_si128(mask, y) : bits of y where mask is 1
    return _mm_or_si128(not_mask, masked_y);        // Combine the two results to get the final result
}

/**
 *  NOTE: This extract allows you to perform the 'log' and 'exp' operations for SSE2.
 *        This implementation was written by Julien Pommier.
 *
 * SOURCE: http://gruntthepeon.free.fr/ssemath/
 *
 * LICENSE:
 *  Copyright (C) 2007  Julien Pommier
 *
 *  This software is provided 'as-is', without any express or implied
 *  warranty.  In no event will the authors be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *      claim that you wrote the original software. If you use this software
 *      in a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *  2. Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software.
 *  3. This notice may not be removed or altered from any source distribution.
 *
 *  (this is the zlib license)
 */

/**
 * natural logarithm computed for 4 simultaneous float 
 * return NaN for x <= 0
 */
static inline __m128
_mm_log_ps(__m128 x)
{
  __m128i emm0;
  __m128 one = *(__m128*)GC_simd_f32_1;

  __m128 invalid_mask = _mm_cmple_ps(x, _mm_setzero_ps());

  x = _mm_max_ps(x, *(__m128*)GC_simd_f32_min_norm_pos); // cut off denormalized stuff
  emm0 = _mm_srli_epi32(_mm_castps_si128(x), 23);

  // keep only the fractional part
  x = _mm_and_ps(x, *(__m128*)GC_simd_f32_inv_mant_mask);
  x = _mm_or_ps(x, *(__m128*)GC_simd_f32_0p5);

  emm0 = _mm_sub_epi32(emm0, *(__m128i*)GC_simd_i32_0x7f);
  __m128 e = _mm_cvtepi32_ps(emm0);

  e = _mm_add_ps(e, one);

    /* part2: 
       if( x < SQRTHF )
       {
            e -= 1;
            x = x + x - 1.0;
       }    else { x = x - 1.0; }
    */
    __m128 mask = _mm_cmplt_ps(x, *(__m128*)GC_simd_f32_cephes_SQRTHF);
    __m128 tmp = _mm_and_ps(x, mask);
    x = _mm_sub_ps(x, one);
    e = _mm_sub_ps(e, _mm_and_ps(one, mask));
    x = _mm_add_ps(x, tmp);

    __m128 z = _mm_mul_ps(x,x);

    __m128 y = *(__m128*)GC_simd_f32_cephes_log_p0;
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)GC_simd_f32_cephes_log_p1);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)GC_simd_f32_cephes_log_p2);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)GC_simd_f32_cephes_log_p3);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)GC_simd_f32_cephes_log_p4);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)GC_simd_f32_cephes_log_p5);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)GC_simd_f32_cephes_log_p6);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)GC_simd_f32_cephes_log_p7);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)GC_simd_f32_cephes_log_p8);
    y = _mm_mul_ps(y, x);

    y = _mm_mul_ps(y, z);

    tmp = _mm_mul_ps(e, *(__m128*)GC_simd_f32_cephes_log_q1);
    y = _mm_add_ps(y, tmp);

    tmp = _mm_mul_ps(z, *(__m128*)GC_simd_f32_0p5);
    y = _mm_sub_ps(y, tmp);

    tmp = _mm_mul_ps(e, *(__m128*)GC_simd_f32_cephes_log_q2);
    x = _mm_add_ps(x, y);
    x = _mm_add_ps(x, tmp);
    x = _mm_or_ps(x, invalid_mask); // negative arg will be NAN
    return x;
}

static inline __m128
_mm_exp_ps(__m128 x)
{
    __m128 tmp = _mm_setzero_ps(), fx;
    __m128i emm0;
    __m128 one = *(__m128*)GC_simd_f32_1;

    x = _mm_min_ps(x, *(__m128*)GC_simd_f32_exp_hi);
    x = _mm_max_ps(x, *(__m128*)GC_simd_f32_exp_lo);

    // express exp(x) as exp(g + n*log(2))
    fx = _mm_mul_ps(x, *(__m128*)GC_simd_f32_cephes_LOG2EF);
    fx = _mm_add_ps(fx, *(__m128*)GC_simd_f32_0p5);

    // how to perform a floorf with SSE: just below
    emm0 = _mm_cvttps_epi32(fx);
    tmp  = _mm_cvtepi32_ps(emm0);

    // if greater, substract 1
    __m128 mask = _mm_cmpgt_ps(tmp, fx);    
    mask = _mm_and_ps(mask, one);
    fx = _mm_sub_ps(tmp, mask);

    tmp = _mm_mul_ps(fx, *(__m128*)GC_simd_f32_cephes_exp_C1);
    __m128 z = _mm_mul_ps(fx, *(__m128*)GC_simd_f32_cephes_exp_C2);
    x = _mm_sub_ps(x, tmp);
    x = _mm_sub_ps(x, z);

    z = _mm_mul_ps(x,x);
  
    __m128 y = *(__m128*)GC_simd_f32_cephes_exp_p0;
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)GC_simd_f32_cephes_exp_p1);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)GC_simd_f32_cephes_exp_p2);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)GC_simd_f32_cephes_exp_p3);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)GC_simd_f32_cephes_exp_p4);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)GC_simd_f32_cephes_exp_p5);
    y = _mm_mul_ps(y, z);
    y = _mm_add_ps(y, x);
    y = _mm_add_ps(y, one);

    // build 2^n
    emm0 = _mm_cvttps_epi32(fx);
    emm0 = _mm_add_epi32(emm0, *(__m128i*)GC_simd_i32_0x7f);
    emm0 = _mm_slli_epi32(emm0, 23);
    __m128 pow2n = _mm_castsi128_ps(emm0);

    y = _mm_mul_ps(y, pow2n);
    return y;
}

#endif // (__AVX2__) || (__SSE2__)


/* Main Module Functions */


static inline PFIsimdvf
pfiSimdSet1_F32(float x)
{
#if defined(__AVX2__)
    return _mm256_set1_ps(x);
#elif defined(__SSE2__)
    return _mm_set1_ps(x);
#endif
}

static inline PFIsimdvi
pfiSimdSet1_I32(int32_t x)
{
#if defined(__AVX2__)
    return _mm256_set1_epi32(x);
#elif defined(__SSE2__)
    return _mm_set1_epi32(x);
#endif
}

static inline PFIsimdvi
pfiSimdSetR_I8(int8_t i0,  int8_t  i1, int8_t  i2, int8_t  i3,
               int8_t i4,  int8_t  i5, int8_t  i6, int8_t  i7,
               int8_t i8,  int8_t  i9, int8_t i10, int8_t i11,
               int8_t i12, int8_t i13, int8_t i14, int8_t i15,
               int8_t i16, int8_t i17, int8_t i18, int8_t i19,
               int8_t i20, int8_t i21, int8_t i22, int8_t i23,
               int8_t i24, int8_t i25, int8_t i26, int8_t i27,
               int8_t i28, int8_t i29, int8_t i30, int8_t i31)
{
#if defined(__AVX2__)

    return _mm256_setr_epi8(i0,  i1,  i2,  i3,
                            i4,  i5,  i6,  i7,
                            i8,  i9,  i10, i11,
                            i12, i13, i14, i15,
                            i16, i17, i18, i19,
                            i20, i21, i22, i23,
                            i24, i25, i26, i27,
                            i28, i29, i30, i31);

#elif defined(__SSE2__)

    (void)i16; (void)i17; (void)i18; (void)i19;
    (void)i20; (void)i21; (void)i22; (void)i23;
    (void)i24; (void)i25; (void)i26; (void)i27;
    (void)i28; (void)i29; (void)i30; (void)i31;

    return _mm_setr_epi8(i0,  i1,  i2,  i3,
                         i4,  i5,  i6,  i7,
                         i8,  i9,  i10, i11,
                         i12, i13, i14, i15);

#endif
}

static inline PFIsimdvi
pfiSimdSetR_x4_I8(int8_t i0, int8_t i1, int8_t i2, int8_t i3)
{
#if defined(__AVX2__)
    return _mm256_setr_epi8(i0, i1, i2, i3,
                            i0, i1, i2, i3,
                            i0, i1, i2, i3,
                            i0, i1, i2, i3,
                            i0, i1, i2, i3,
                            i0, i1, i2, i3,
                            i0, i1, i2, i3,
                            i0, i1, i2, i3);
#elif defined(__SSE2__)
    return _mm_setr_epi8(i0, i1, i2, i3,
                         i0, i1, i2, i3,
                         i0, i1, i2, i3,
                         i0, i1, i2, i3);
#endif
}

static inline PFIsimdvi
pfiSimdSetR_I32(int32_t i0, int32_t i1, int32_t i2, int32_t i3, int32_t i4, int32_t i5, int32_t i6, int32_t i7)
{
#if defined(__AVX2__)
    return _mm256_setr_epi32(i0, i1, i2, i3, i4, i5, i6, i7);
#elif defined(__SSE2__)
    (void)i4, (void)i5, (void)i6, (void)i7;
    return _mm_setr_epi32(i0, i1, i2, i3);
#endif
}

static inline PFIsimdvi
pfiSimdSetZero_I32(void)
{
#if defined(__AVX2__)
    return _mm256_setzero_si256();
#elif defined(__SSE2__)
    return _mm_setzero_si128();
#endif
}

static inline PFIsimdvf
pfiSimdSetZero_F32(void)
{
#if defined(__AVX2__)
    return _mm256_setzero_ps();
#elif defined(__SSE2__)
    return _mm_setzero_ps();
#endif
}

#if defined(__AVX2__)
#   define pfiSimdRound_F32(x, imm) \
        _mm256_round_ps(x, imm)
#elif defined(__SSE2__)
#   define pfiSimdRound_F32(x, imm) \
        _mm_round_ps(x, imm)
#endif

static inline PFIsimdvi
pfiSimdAbs_I32(PFIsimdvi x)
{
#if defined(__AVX2__)
    return _mm256_abs_epi32(x);
#elif defined(__SSE2__)
    return _mm_abs_epi32(x);
#endif
}

static inline PFIsimdvf
pfiSimdAbs_F32(PFIsimdvf x)
{
#if defined(__AVX2__)
    return _mm256_andnot_ps(
        _mm256_set1_ps(-0.0f), x);
#elif defined(__SSE2__)
    return _mm_andnot_ps(
        _mm_set1_ps(-0.0f), x);
#endif
}

static inline PFIsimdvi
pfiSimdUnpackLo_I8(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_unpacklo_epi8(x, y);
#elif defined(__SSE2__)
    return _mm_unpacklo_epi8(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdUnpackLo_I16(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_unpacklo_epi16(x, y);
#elif defined(__SSE2__)
    return _mm_unpacklo_epi16(x, y);
#endif
}

static inline void
pfiSimdStore_I8(void* p, PFIsimdvi x)
{
#if defined(__AVX2__)
    __m128i lower = _mm256_castsi256_si128(x);
    _mm_storeu_si64((__m128i*)p, lower);
#elif defined(__SSE2__)
    _mm_storeu_si32((__m128i*)p, x);
#endif
}

static inline void
pfiSimdStore_I16(void* p, PFIsimdvi x)
{
#if defined(__AVX2__)
    __m128i lower = _mm256_castsi256_si128(x);
    _mm_storeu_si128((__m128i*)p, lower);
#elif defined(__SSE2__)
    _mm_storeu_si64((__m128i*)p, x);
#endif
}

static inline void
pfiSimdStore_I32(void* p, PFIsimdvi x)
{
#if defined(__AVX2__)
    _mm256_storeu_si256((__m256i*)p, x);
#elif defined(__SSE2__)
    _mm_storeu_si128((__m128i*)p, x);
#endif
}

static inline void
pfiSimdStore_F32(void* p, PFIsimdvf x)
{
#if defined(__AVX2__)
    _mm256_storeu_ps((float*)p, x);
#elif defined(__SSE2__)
    _mm_storeu_ps((float*)p, x);
#endif
}

static inline PFIsimdvi
pfiSimdLoad_I8(const void* p)
{
#if defined(__AVX2__)
    __m128i lower = _mm_loadu_si64((const __m128i*)p);
    return _mm256_castsi128_si256(lower);
#elif defined(__SSE2__)
    return _mm_loadu_si32((const __m128i*)p);
#endif
}

static inline PFIsimdvi
pfiSimdLoad_I16(const void* p)
{
#if defined(__AVX2__)
    __m128i lower = _mm_loadu_si128((const __m128i*)p);
    return _mm256_castsi128_si256(lower);
#elif defined(__SSE2__)
    return _mm_loadu_si64((const __m128i*)p);
#endif
}

static inline PFIsimdvi
pfiSimdLoad_I32(const void* p)
{
#if defined(__AVX2__)
    return _mm256_loadu_si256((const __m256i*)p);
#elif defined(__SSE2__)
    return _mm_loadu_si128((const __m128i*)p);
#endif
}

static inline PFIsimdvf
pfiSimdLoad_F32(const void* p)
{
#if defined(__AVX2__)
    return _mm256_loadu_ps(p);
#elif defined(__SSE2__)
    return _mm_loadu_ps(p);
#endif
}

#if defined(__AVX2__)
#   define pfiSimdExtract_I8(v, index)  \
        _mm256_extract_epi8(v, index)
#elif defined(__SSE2__)
#   define pfiSimdExtract_I8(v, index)  \
        _mm_extract_epi8(v, index % 16)
#endif

#if defined(__AVX2__)
#   define pfiSimdExtract_I16(v, index)  \
        _mm256_extract_epi16(v, index)
#elif defined(__SSE2__)
#   define pfiSimdExtract_I16(v, index)  \
        _mm_extract_epi16(v, index % 8)
#endif

#if defined(__AVX2__)
#   define pfiSimdExtract_I32(v, index)  \
        _mm256_extract_epi32(v, index)
#elif defined(__SSE2__)
#   define pfiSimdExtract_I32(v, index)  \
        _mm_extract_epi32(v, index % 4)
#endif

#if defined(__AVX2__)
#   define pfiSimdExtract_F32(v, index)  \
        ((float*)&v)[index]
#elif defined(__SSE2__)
#   define pfiSimdExtract_F32(v, index)  \
        _mm_extract_ps(v, index % 4)
#endif

static inline int32_t
pfiSimdExtractVarIdx_I32(PFIsimdvi x, int32_t index)
{
#if defined(__AVX2__)
    __m128i idx = _mm_cvtsi32_si128(index);
    __m256i val = _mm256_permutevar8x32_epi32(x, _mm256_castsi128_si256(idx));
    return _mm_cvtsi128_si32(_mm256_castsi256_si128(val));
#elif defined(__SSE2__)
    union v_u { __m128i vec; int arr[4]; };
    union v_u v = { .vec = x };
    return v.arr[index % 4];
#endif
}

#if defined(__AVX2__)
#   define pfiSimdGather_I32(p, offsets, alignment) \
        _mm256_i32gather_epi32(p, offsets, alignment)
#elif defined(__SSE2__)
#   define pfiSimdGather_I32(p, offsets, alignment) \
        _mm_i32gather_epi32(p, offsets, alignment)
#endif

static inline PFIsimdvi
pfiSimdPackus_I16_I8(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_packus_epi16(x, y);
#elif defined(__SSE2__)
    return _mm_packus_epi16(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdPackus_I32_I16(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_packus_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_packus_epi32(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdShuffle_I8(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_shuffle_epi8(x, y);
#elif defined(__SSE4_1__)
    return _mm_shuffle_epi8(x, y);
#elif defined(__SSE2__)
    return _mm_shuffle_epi8_sse2(x, y);
#endif
}

#if defined(__AVX2__)
#   define pfiSimdShuffle_F32(v1, v2, mask)  \
        _mm256_shuffle_ps(v1, v2, mask)
#elif defined(__SSE2__)
#   define pfiSimdShuffle_F32(v1, v2, mask)  \
        _mm_shuffle_ps(v1, v2, mask)
#endif

static inline PFIsimdvi
pfiSimdConvert_U8_I32(PFIsimdvi x)
{
#if defined(__AVX2__)
    return _mm256_cvtepu8_epi32(
        _mm256_castsi256_si128(x));
#elif defined(__SSE2__)
    return _mm_cvtepu8_epi32(x);
#endif
}

static inline PFIsimdvi
pfiSimdConvert_I8_I32(PFIsimdvi x)
{
#if defined(__AVX2__)
    return _mm256_cvtepi8_epi32(
        _mm256_castsi256_si128(x));
#elif defined(__SSE2__)
    return _mm_cvtepi8_epi32(x);
#endif
}

static inline PFIsimdvi
pfiSimdConvert_I16_I32(PFIsimdvi x)
{
#if defined(__AVX2__)
    return _mm256_cvtepi16_epi32(
        _mm256_castsi256_si128(x));
#elif defined(__SSE2__)
    return _mm_cvtepi16_epi32(x);
#endif
}

static inline PFIsimdvi
pfiSimdConvert_F32_I32(PFIsimdvf x)
{
#if defined(__AVX2__)
    return _mm256_cvtps_epi32(x);
#elif defined(__SSE2__)
    return _mm_cvtps_epi32(x);
#endif
}

#if defined(__AVX2__)
#   ifdef FLT16_MAX
#       define pfiSimdConvert_F32_F16(x, imm)  \
            _mm256_castsi128_si256( \
                _mm256_cvtps_ph(x, imm))
#   else
static inline PFIsimdvi
pfiSimdConvert_F32_F16(PFIsimdvf x, const int imm)
{
    (void)imm;
    uint16_t m256i[16] = { 0 };
    m256i[0] = pfmFloatToHalf(pfiSimdExtract_F32(x, 0));
    m256i[1] = pfmFloatToHalf(pfiSimdExtract_F32(x, 1));
    m256i[2] = pfmFloatToHalf(pfiSimdExtract_F32(x, 2));
    m256i[3] = pfmFloatToHalf(pfiSimdExtract_F32(x, 3));
    m256i[4] = pfmFloatToHalf(pfiSimdExtract_F32(x, 4));
    m256i[5] = pfmFloatToHalf(pfiSimdExtract_F32(x, 5));
    m256i[6] = pfmFloatToHalf(pfiSimdExtract_F32(x, 6));
    m256i[7] = pfmFloatToHalf(pfiSimdExtract_F32(x, 7));
    return *(__m256i*)m256i;
}
#   endif
#elif defined(__SSE2__)
#   ifdef FLT16_MAX
#       define pfiSimdConvert_F32_F16(x, imm)  \
            _mm_cvtps_ph(x, imm)
#   else
static inline PFIsimdvi
pfiSimdConvert_F32_F16(PFIsimdvf x, const int imm)
{
    (void)imm;
    uint16_t m128i[8] = { 0 };
    m128i[0] = pfmFloatToHalf(pfiSimdExtract_F32(x, 0));
    m128i[1] = pfmFloatToHalf(pfiSimdExtract_F32(x, 1));
    m128i[2] = pfmFloatToHalf(pfiSimdExtract_F32(x, 2));
    m128i[3] = pfmFloatToHalf(pfiSimdExtract_F32(x, 3));
    return *(__m128i*)m128i;
}
#   endif
#endif

static inline PFIsimdvf
pfiSimdConvert_F16_F32(PFIsimdvi x)
{
#if defined(__AVX2__)
#   ifdef FLT16_MAX
        return _mm256_cvtph_ps(
            _mm256_castsi256_si128(x));
#   else
        float m256[8];
        m256[0] = pfmHalfToFloat(pfiSimdExtract_I16(x, 0));
        m256[1] = pfmHalfToFloat(pfiSimdExtract_I16(x, 2));
        m256[2] = pfmHalfToFloat(pfiSimdExtract_I16(x, 4));
        m256[3] = pfmHalfToFloat(pfiSimdExtract_I16(x, 6));
        m256[4] = pfmHalfToFloat(pfiSimdExtract_I16(x, 8));
        m256[5] = pfmHalfToFloat(pfiSimdExtract_I16(x, 10));
        m256[6] = pfmHalfToFloat(pfiSimdExtract_I16(x, 12));
        m256[7] = pfmHalfToFloat(pfiSimdExtract_I16(x, 14));
        return *(__m256*)m256;
#   endif
#elif defined(__SSE2__)
#   ifdef FLT16_MAX
        return _mm_cvtph_ps(x);
#   else
        float m128[4];
        m128[0] = pfmHalfToFloat(pfiSimdExtract_I16(x, 0));
        m128[0] = pfmHalfToFloat(pfiSimdExtract_I16(x, 2));
        m128[0] = pfmHalfToFloat(pfiSimdExtract_I16(x, 4));
        m128[0] = pfmHalfToFloat(pfiSimdExtract_I16(x, 6));
        return *(__m128*)m128;
#   endif
#endif
}

static inline PFIsimdvf
pfiSimdConvert_I32_F32(PFIsimdvi x)
{
#if defined(__AVX2__)
    return _mm256_cvtepi32_ps(x);
#elif defined(__SSE2__)
    return _mm_cvtepi32_ps(x);
#endif
}

static inline PFIsimdvi
pfiSimdCast_F32_I32(PFIsimdvf x)
{
#if defined(__AVX2__)
    return _mm256_castps_si256(x);
#elif defined(__SSE2__)
    return _mm_castps_si128(x);
#endif
}

static inline PFIsimdvf
pfiSimdCast_I32_F32(PFIsimdvi x)
{
#if defined(__AVX2__)
    return _mm256_castsi256_ps(x);
#elif defined(__SSE2__)
    return _mm_castsi128_ps(x);
#endif
}

static inline PFIsimdvi
pfiSimdMin_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_min_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_min_epi32(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdMin_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_min_ps(x, y);
#elif defined(__SSE2__)
    return _mm_min_ps(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdMax_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_max_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_max_epi32(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdMax_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_max_ps(x, y);
#elif defined(__SSE2__)
    return _mm_max_ps(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdClamp_I32(PFIsimdvi x, PFIsimdvi min, PFIsimdvi max)
{
#if defined(__AVX2__)
    return _mm256_min_epi32(_mm256_max_epi32(x, min), max);
#elif defined(__SSE2__)
    return _mm_min_epi32(_mm_max_epi32(x, min), max);
#endif
}

static inline PFIsimdvf
pfiSimdClamp_F32(PFIsimdvf x, PFIsimdvf min, PFIsimdvf max)
{
#if defined(__AVX2__)
    return _mm256_min_ps(_mm256_max_ps(x, min), max);
#elif defined(__SSE2__)
    return _mm_min_ps(_mm_max_ps(x, min), max);
#endif
}

static inline PFIsimdvi
pfiSimdAdd_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_add_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_add_epi32(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdAdd_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_add_ps(x, y);
#elif defined(__SSE2__)
    return _mm_add_ps(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdSub_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_sub_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_sub_epi32(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdSub_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_sub_ps(x, y);
#elif defined(__SSE2__)
    return _mm_sub_ps(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdMullo_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_mullo_epi32(x, y);
#elif defined(__SSE4_1__)
    return _mm_mullo_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_mullo_epi32_sse2(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdMul_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_mul_ps(x, y);
#elif defined(__SSE2__)
    return _mm_mul_ps(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdPow_F32(PFIsimdvf base, float exponent)
{
#if defined(__AVX2__)
    __m256 exp = _mm256_set1_ps(exponent);
    __m256 log_base = _mm256_log_ps(base);
    return _mm256_exp_ps(_mm256_mul_ps(log_base, exp));
#elif defined(__SSE2__)
    __m128 exp = _mm_set1_ps(exponent);
    __m128 log_base = _mm_log_ps(base);
    return _mm_exp_ps(_mm_mul_ps(log_base, exp));
#endif
}

static inline PFIsimdvf
pfiSimdDiv_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_div_ps(x, y);
#elif defined(__SSE2__)
    return _mm_div_ps(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdMod_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    __m256 quotient = _mm256_div_ps(x, y);                              // Calculate the quotient
    __m256 floor_quotient = _mm256_floor_ps(quotient);                  // Use floor to get the integer part
    __m256 floor_quotient_times_y = _mm256_mul_ps(floor_quotient, y);   // Multiply y by the integer part of the quotient
    return _mm256_sub_ps(x, floor_quotient_times_y);                    // Subtract this product from x to get the modulo result
#elif defined(__SSE2__)
    __m128 quotient = _mm_div_ps(x, y);
    __m128 floor_quotient = _mm_floor_ps(quotient);
    __m128 floor_quotient_times_y = _mm_mul_ps(floor_quotient, y);
    return _mm_sub_ps(x, floor_quotient_times_y);
#endif
}

static inline PFIsimdvi
pfiSimdNeg_I32(PFIsimdvi x)
{
#if defined(__AVX2__)
    return _mm256_sub_epi32(_mm256_setzero_si256(), x);
#elif defined(__SSE2__)
    return _mm_sub_epi32(_mm_setzero_si128(), x);
#endif
}

static inline PFIsimdvf
pfiSimdNeg_F32(PFIsimdvf x)
{
#if defined(__AVX2__)
    return _mm256_sub_ps(_mm256_setzero_ps(), x);
#elif defined(__SSE2__)
    return _mm_sub_ps(_mm_setzero_ps(), x);
#endif
}

static inline PFIsimdvf
pfiSimdRCP_F32(PFIsimdvf x)
{
#if defined(__AVX2__)
    return _mm256_rcp_ps(x);
#elif defined(__SSE2__)
    return _mm_rcp_ps(x);
#endif
}

static inline PFIsimdvf
pfiSimdSqrt_F32(PFIsimdvf x)
{
#if defined(__AVX2__)
    return _mm256_sqrt_ps(x);
#elif defined(__SSE2__)
    return _mm_sqrt_ps(x);
#endif
}

static inline PFIsimdvf
pfiSimdRSqrt_F32(PFIsimdvf x)
{
#if defined(__AVX2__)
    return _mm256_rsqrt_ps(x);
#elif defined(__SSE2__)
    return _mm_rsqrt_ps(x);
#endif
}

static inline PFIsimdvi
pfiSimdPermute_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_permutevar8x32_epi32(x, y);
#elif defined(__SSE4_1__)
    y = _mm_and_si128(y, _mm_set1_epi32(0x00000003));
    y = _mm_mullo_epi32(y, _mm_set1_epi32(0x04040404));
    y = _mm_or_si128(y, _mm_set1_epi32(0x03020100));
    return _mm_shuffle_epi8(x, y);
#elif defined(__SSE2__)
    y = _mm_and_si128(y, _mm_set1_epi32(0x00000003));
    y = _mm_mullo_epi32_sse2(y, _mm_set1_epi32(0x04040404));
    y = _mm_or_si128(y, _mm_set1_epi32(0x03020100));
    return _mm_shuffle_epi8_sse2(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdAnd_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_and_si256(x, y);
#elif defined(__SSE2__)
    return _mm_and_si128(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdAnd_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_and_ps(x, y);
#elif defined(__SSE2__)
    return _mm_and_ps(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdAndNot_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_andnot_si256(x, y);
#elif defined(__SSE2__)
    return _mm_andnot_si128(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdAndNot_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_andnot_ps(x, y);
#elif defined(__SSE2__)
    return _mm_andnot_ps(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdOr_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_or_si256(x, y);
#elif defined(__SSE2__)
    return _mm_or_si128(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdOr_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_or_ps(x, y);
#elif defined(__SSE2__)
    return _mm_or_ps(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdShr_I32(PFIsimdvi x, int32_t imm8)
{
#if defined(__AVX2__)
    return _mm256_srli_epi32(x, imm8);
#elif defined(__SSE2__)
    return _mm_srli_epi32(x, imm8);
#endif
}

static inline PFIsimdvi
pfiSimdShl_I32(PFIsimdvi x, int32_t imm8)
{
#if defined(__AVX2__)
    return _mm256_slli_epi32(x, imm8);
#elif defined(__SSE2__)
    return _mm_slli_epi32(x, imm8);
#endif
}

static inline int32_t
pfiSimdMoveMask_F32(PFIsimdvf x)
{
#if defined(__AVX2__)
    return _mm256_movemask_ps(x);
#elif defined(__SSE2__)
    return _mm_movemask_ps(x);
#endif
}

static inline int32_t
pfiSimdMoveMask_I8(PFIsimdvi x)
{
#if defined(__AVX2__)
    return _mm256_movemask_epi8(x);
#elif defined(__SSE2__)
    return _mm_movemask_epi8(x);
#endif
}

static inline PFIsimdvi
pfiSimdBlendV_I8(PFIsimdvi a, PFIsimdvi b, PFIsimdvi mask)
{
#if defined(__AVX2__)
    return _mm256_blendv_epi8(a, b, mask);
#elif defined(__SSE4_1__)
    return _mm_blendv_epi8(a, b, mask);
#elif defined(__SSE2__)
    return _mm_blendv_epi8_sse2(a, b, mask);
#endif
}

static inline PFIsimdvi
pfiSimdBlendV_I16(PFIsimdvi a, PFIsimdvi b, PFIsimdvi mask)
{
#if defined(__AVX2__)

    // Extend mask from 16 bits to 32 bits
    __m256i mask_ext = _mm256_unpacklo_epi16(mask, mask);
    __m256i mask_inv = _mm256_xor_si256(mask_ext, _mm256_set1_epi32(0xFFFFFFFF));
    
    // Select elements based on extended mask
    __m256i blend_a = _mm256_and_si256(a, mask_inv); // a & ~mask
    __m256i blend_b = _mm256_and_si256(b, mask_ext); // b & mask
    
    // Combine selected items
    return _mm256_or_si256(blend_a, blend_b); // (a & ~mask) | (b & mask)

#elif defined(__SSE2__)

    // Extend mask from 16 bits to 32 bits for multiplication by 0xFFFF
    __m128i mask_ext = _mm_unpacklo_epi16(mask, mask);
    __m128i mask_inv = _mm_xor_si128(mask_ext, _mm_set1_epi32(0xFFFFFFFF));

    // Select elements based on extended mask
    __m128i blend_a = _mm_and_si128(a, mask_inv); // a & ~mask
    __m128i blend_b = _mm_and_si128(b, mask_ext); // b & mask

    // Combine selected items
    return _mm_or_si128(blend_a, blend_b); // (a & ~mask) | (b & mask)

#endif
}

static inline PFIsimdvf
pfiSimdBlendV_F32(PFIsimdvf a, PFIsimdvf b, PFIsimdvf mask)
{
#if defined(__AVX2__)
    return _mm256_blendv_ps(a, b, mask);
#elif defined(__SSE4_1__)
    return _mm_blendv_ps(a, b, mask);
#elif defined(__SSE2__)
    return _mm_or_ps(
        _mm_and_ps(mask, a),
        _mm_andnot_ps(mask, b));
#endif
}

static inline int
pfiSimdAllZero_I32(PFIsimdvi x)
{
#if defined(__AVX2__)
    __m256i cmp = _mm256_cmpeq_epi32(x, _mm256_setzero_si256());
    return (_mm256_movemask_epi8(cmp) == 0xFFFF);
#elif defined(__SSE2__)
    __m128i cmp = _mm_cmpeq_epi32(x, _mm_setzero_si128());
    return (_mm_movemask_epi8(cmp) == 0xFFFF);
#endif
}

static inline int
pfiSimdAllZero_F32(PFIsimdvf x)
{
#if defined(__AVX2__)
    __m256 cmp = _mm256_cmp_ps(x, _mm256_setzero_ps(), _CMP_EQ_OS);
    return (_mm256_movemask_ps(cmp) == 0xFFFF);
#elif defined(__SSE2__)
    __m128 cmp = _mm_cmpeq_ps(x, _mm_setzero_ps());
    return (_mm_movemask_ps(cmp) == 0xFFFF);
#endif
}

static inline PFIsimdvi
pfiSimdCmpEQ_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_cmpeq_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_cmpeq_epi32(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdCmpEQ_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_cmp_ps(x, y, _CMP_EQ_OS);
#elif defined(__SSE2__)
    return _mm_cmpeq_ps(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdCmpNEQ_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    __m256i eq = _mm256_cmpeq_epi32(x, y);
    __m256i neq = _mm256_xor_si256(eq, _mm256_set1_epi32(-1));
    return neq;
#elif defined(__SSE2__)
    __m128i eq = _mm_cmpeq_epi32(x, y);
    __m128i neq = _mm_xor_si128(eq, _mm_set1_epi32(-1));
    return neq;
#endif
}

static inline PFIsimdvf
pfiSimdCmpNEQ_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_cmp_ps(x, y, _CMP_NEQ_OS);
#elif defined(__SSE2__)
    return _mm_cmpneq_ps(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdCmpLT_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_cmpgt_epi32(y, x);
#elif defined(__SSE2__)
    return _mm_cmplt_epi32(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdCmpLT_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_cmp_ps(x, y, _CMP_LT_OS);
#elif defined(__SSE2__)
    return _mm_cmplt_ps(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdCmpGT_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_cmpgt_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_cmpgt_epi32(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdCmpGT_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_cmp_ps(x, y, _CMP_GT_OS);
#elif defined(__SSE2__)
    return _mm_cmpgt_ps(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdCmpLE_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_cmpgt_epi32(y, x);
#elif defined(__SSE2__)
    return _mm_cmplt_epi32(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdCmpLE_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_cmp_ps(x, y, _CMP_LE_OS);
#elif defined(__SSE2__)
    return _mm_cmple_ps(x, y);
#endif
}

static inline PFIsimdvi
pfiSimdCmpGE_I32(PFIsimdvi x, PFIsimdvi y)
{
#if defined(__AVX2__)
    return _mm256_cmpgt_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_cmpgt_epi32(x, y);
#endif
}

static inline PFIsimdvf
pfiSimdCmpGE_F32(PFIsimdvf x, PFIsimdvf y)
{
#if defined(__AVX2__)
    return _mm256_cmp_ps(x, y, _CMP_GE_OS);
#elif defined(__SSE2__)
    return _mm_cmpge_ps(x, y);
#endif
}

/* 2D SIMD Vector function definitions */

static inline void
pfiVec2Zero_simd(PFIsimdv2f dst)
{
    dst[0] = pfiSimdSetZero_F32();
    dst[1] = pfiSimdSetZero_F32();
}

static inline void
pfiVec2One_simd(PFIsimdv2f dst, float v)
{
    dst[0] = pfiSimdSet1_F32(v);
    dst[1] = pfiSimdSet1_F32(v);
}

static inline void
pfiVec2Set_simd(PFIsimdv2f dst, float x, float y)
{
    dst[0] = pfiSimdSet1_F32(x);
    dst[1] = pfiSimdSet1_F32(y);
}

static inline void
pfiVec2Load_simd(PFIsimdv2f dst, const PFMvec2 src)
{
    dst[0] = pfiSimdSet1_F32(src[0]);
    dst[1] = pfiSimdSet1_F32(src[1]);
}

static inline void
pfiVec2Copy_simd(PFIsimdvf_ptr restrict dst, const PFIsimdvf_ptr restrict src)
{
    memcpy(dst, src, sizeof(PFIsimdv2f));
}

static inline void
pfiVec2Swap_simd(PFIsimdvf_ptr restrict a, PFIsimdvf_ptr restrict b)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        PFIsimdvf tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

static inline void
pfiVec2Blend_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2, PFIsimdvf mask)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdBlendV_F32(v1[i], v2[i], mask);
    }
}

static inline void
pfiVec2BlendR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2, PFIsimdvf mask)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdBlendV_F32(v1[i], v2[i], mask);
    }
}

static inline void
pfiVec2Neg_simd(PFIsimdv2f dst, const PFIsimdv2f v)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdNeg_F32(v[i]);
    }
}

static inline void
pfiVec2NegR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdNeg_F32(v[i]);
    }
}

static inline void
pfiVec2Add_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdAdd_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec2AddR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdAdd_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec2Sub_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdSub_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec2SubR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdSub_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec2Mul_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdMul_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec2MulR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdMul_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec2Div_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdDiv_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec2DivR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdDiv_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec2Offset_simd(PFIsimdv2f dst, const PFIsimdv2f v, PFIsimdvf offset)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdAdd_F32(v[i], offset);
    }
}

static inline void
pfiVec2OffsetR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v, PFIsimdvf offset)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdAdd_F32(v[i], offset);
    }
}

static inline void
pfiVec2Scale_simd(PFIsimdv2f dst, const PFIsimdv2f v, PFIsimdvf scale)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdMul_F32(v[i], scale);
    }
}

static inline void
pfiVec2ScaleR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v, PFIsimdvf scale)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = pfiSimdMul_F32(v[i], scale);
    }
}

static inline void
pfiVec2Normalize_simd(PFIsimdv2f dst, const PFIsimdv2f v)
{
    // Calculate the sum of squares of elements
    PFIsimdvf squaredLength = pfiSimdAdd_F32(
        pfiSimdMul_F32(v[0], v[0]),
        pfiSimdMul_F32(v[1], v[1]));

    // Calculate the inverse of the square root of length squared
    PFIsimdvf invLength = pfiSimdRSqrt_F32(squaredLength);

    // Normalize vectors
    dst[0] = pfiSimdMul_F32(v[0], invLength);
    dst[1] = pfiSimdMul_F32(v[1], invLength);
}

static inline void
pfiVec2NormalizeR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v)
{
    // Calculate the sum of squares of elements
    PFIsimdvf squaredLength = pfiSimdAdd_F32(
        pfiSimdMul_F32(v[0], v[0]),
        pfiSimdMul_F32(v[1], v[1]));

    // Calculate the inverse of the square root of length squared
    PFIsimdvf invLength = pfiSimdRSqrt_F32(squaredLength);

    // Normalize vectors
    dst[0] = pfiSimdMul_F32(v[0], invLength);
    dst[1] = pfiSimdMul_F32(v[1], invLength);
}

static inline PFIsimdvf
pfiVec2Length_simd(const PFIsimdv2f v)
{
    return pfiSimdSqrt_F32(pfiSimdAdd_F32(
        pfiSimdMul_F32(v[0], v[0]),
        pfiSimdMul_F32(v[1], v[1])));
}

static inline PFIsimdvf
pfiVec2LengthSq_simd(const PFIsimdv2f v)
{
    return pfiSimdAdd_F32(
        pfiSimdMul_F32(v[0], v[0]),
        pfiSimdMul_F32(v[1], v[1]));
}

static inline PFIsimdvf
pfiVec2Dot_simd(const PFIsimdv2f v1, const PFIsimdv2f v2)
{
    return pfiSimdAdd_F32(
        pfiSimdMul_F32(v1[0], v2[0]),
        pfiSimdMul_F32(v1[1], v2[1]));
}

static inline PFIsimdvf
pfiVec2Distance_simd(const PFIsimdv2f v1, const PFIsimdv2f v2)
{
    // Calculate the differences between corresponding elements of the vectors
    PFIsimdvf dt0 = pfiSimdSub_F32(v1[0], v2[0]);
    PFIsimdvf dt1 = pfiSimdSub_F32(v1[1], v2[1]);

    // Calculate the squared differences
    PFIsimdvf dt0Sq = pfiSimdMul_F32(dt0, dt0);
    PFIsimdvf dt1Sq = pfiSimdMul_F32(dt1, dt1);

    // Sum the squared differences
    PFIsimdvf distanceSq = pfiSimdAdd_F32(dt0Sq, dt1Sq);

    // Calculate and return the square root of the sum of squared differences
    return pfiSimdSqrt_F32(distanceSq);
}

static inline PFIsimdvf
pfiVec2DistanceSq_simd(const PFIsimdv2f v1, const PFIsimdv2f v2)
{
    // Calculate the differences between corresponding elements of the vectors
    PFIsimdvf dt0 = pfiSimdSub_F32(v1[0], v2[0]);
    PFIsimdvf dt1 = pfiSimdSub_F32(v1[1], v2[1]);

    // Calculate the squared differences
    PFIsimdvf dt0Sq = pfiSimdMul_F32(dt0, dt0);
    PFIsimdvf dt1Sq = pfiSimdMul_F32(dt1, dt1);

    // Sum the squared differences and return the result
    return pfiSimdAdd_F32(dt0Sq, dt1Sq);
}

static inline void
pfiVec2Direction_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2)
{
    // Calculate the differences between the elements of the two vectors
    PFIsimdvf tmp0 = pfiSimdSub_F32(v1[0], v2[0]);
    PFIsimdvf tmp1 = pfiSimdSub_F32(v1[1], v2[1]);

    // Calculate the sum of the squares of these differences to obtain the length squared
    PFIsimdvf lengthSq = pfiSimdAdd_F32(pfiSimdMul_F32(tmp0, tmp0), pfiSimdMul_F32(tmp1, tmp1));

    // Calculate the inverse of the square root of the length squared to normalize the differences
    PFIsimdvf invLength = pfiSimdRSqrt_F32(lengthSq);

    // Multiply each difference by this inverse to obtain the normalized direction
    dst[0] = pfiSimdMul_F32(tmp0, invLength);
    dst[1] = pfiSimdMul_F32(tmp1, invLength);
}

static inline void
pfiVec2DirectionR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2)
{
    // Calculate the differences between the elements of the two vectors
    dst[0] = pfiSimdSub_F32(v1[0], v2[0]);
    dst[1] = pfiSimdSub_F32(v1[1], v2[1]);

    // Calculate the sum of the squares of these differences to obtain the length squared
    PFIsimdvf lengthSq = pfiSimdAdd_F32(pfiSimdMul_F32(dst[0], dst[0]), pfiSimdMul_F32(dst[1], dst[1]));

    // Calculate the inverse of the square root of the length squared to normalize the differences
    PFIsimdvf invLength = pfiSimdRSqrt_F32(lengthSq);

    // Multiply each difference by this inverse to obtain the normalized direction
    dst[0] = pfiSimdMul_F32(dst[0], invLength);
    dst[1] = pfiSimdMul_F32(dst[1], invLength);
}

static inline void
pfiVec2Lerp_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2, PFIsimdvf t)
{
    dst[0] = pfiSimdAdd_F32(v1[0], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[0], v1[0])));
    dst[1] = pfiSimdAdd_F32(v1[1], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[1], v1[1])));
}

static inline void
pfiVec2LerpR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2, PFIsimdvf t)
{
    // Calculate the difference (v2 - v1)
    PFIsimdvf diff0 = pfiSimdSub_F32(v2[0], v1[0]);
    PFIsimdvf diff1 = pfiSimdSub_F32(v2[1], v1[1]);

    // Multiply the difference by t
    PFIsimdvf t_diff0 = pfiSimdMul_F32(t, diff0);
    PFIsimdvf t_diff1 = pfiSimdMul_F32(t, diff1);

    // Add the result to v1 to get the interpolation result
    dst[0] = pfiSimdAdd_F32(v1[0], t_diff0);
    dst[1] = pfiSimdAdd_F32(v1[1], t_diff1);
}

static inline void
pfiVec2BaryInterpSmooth_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, PFIsimdvf w1, PFIsimdvf w2, PFIsimdvf w3)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        PFIsimdvf v1_w1 = pfiSimdMul_F32(v1[i], w1);
        PFIsimdvf v2_w2 = pfiSimdMul_F32(v2[i], w2);
        PFIsimdvf v3_w3 = pfiSimdMul_F32(v3[i], w3);
        dst[i] = pfiSimdAdd_F32(pfiSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

static inline void
pfiVec2BaryInterpSmoothR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, PFIsimdvf w1, PFIsimdvf w2, PFIsimdvf w3)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        PFIsimdvf v1_w1 = pfiSimdMul_F32(v1[i], w1);
        PFIsimdvf v2_w2 = pfiSimdMul_F32(v2[i], w2);
        PFIsimdvf v3_w3 = pfiSimdMul_F32(v3[i], w3);
        dst[i] = pfiSimdAdd_F32(pfiSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

static inline void
pfiVec2BaryInterpSmoothV_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, const PFIsimdv3f w)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        PFIsimdvf v1_w1 = pfiSimdMul_F32(v1[i], w[0]);
        PFIsimdvf v2_w2 = pfiSimdMul_F32(v2[i], w[1]);
        PFIsimdvf v3_w3 = pfiSimdMul_F32(v3[i], w[2]);
        dst[i] = pfiSimdAdd_F32(pfiSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

static inline void
pfiVec2BaryInterpSmoothVR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, const PFIsimdv3f w)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        PFIsimdvf v1_w1 = pfiSimdMul_F32(v1[i], w[0]);
        PFIsimdvf v2_w2 = pfiSimdMul_F32(v2[i], w[1]);
        PFIsimdvf v3_w3 = pfiSimdMul_F32(v3[i], w[2]);
        dst[i] = pfiSimdAdd_F32(pfiSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

static inline void
pfiVec2BaryInterpFlat_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, PFIsimdvf w1, PFIsimdvf w2, PFIsimdvf w3)
{
    // Compare the weights to find the index of the maximum weight
    PFIsimdvf maxWeight = pfiSimdMax_F32(w1, pfiSimdMax_F32(w2, w3));
    
    // Compare maxWeight to each weight to find which one is the max
    PFIsimdvf mask1 = pfiSimdCmpEQ_F32(maxWeight, w1);
    PFIsimdvf mask2 = pfiSimdCmpEQ_F32(maxWeight, w2);
    PFIsimdvf mask3 = pfiSimdCmpEQ_F32(maxWeight, w3);

    // Use masks to select the corresponding color
    for (int_fast8_t i = 0; i < 2; ++i) {
        dst[i] = pfiSimdOr_F32(
            pfiSimdAnd_F32(mask1, v1[i]),
            pfiSimdOr_F32(
                pfiSimdAnd_F32(mask2, v2[i]),
                pfiSimdAnd_F32(mask3, v3[i])
            )
        );
    }
}

static inline void
pfiVec2BaryInterpFlatR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, PFIsimdvf w1, PFIsimdvf w2, PFIsimdvf w3)
{
    // Compare the weights to find the index of the maximum weight
    PFIsimdvf maxWeight = pfiSimdMax_F32(w1, pfiSimdMax_F32(w2, w3));
    
    // Compare maxWeight to each weight to find which one is the max
    PFIsimdvf mask1 = pfiSimdCmpEQ_F32(maxWeight, w1);
    PFIsimdvf mask2 = pfiSimdCmpEQ_F32(maxWeight, w2);
    PFIsimdvf mask3 = pfiSimdCmpEQ_F32(maxWeight, w3);

    // Use masks to select the corresponding color
    for (int_fast8_t i = 0; i < 2; ++i) {
        dst[i] = pfiSimdOr_F32(
            pfiSimdAnd_F32(mask1, v1[i]),
            pfiSimdOr_F32(
                pfiSimdAnd_F32(mask2, v2[i]),
                pfiSimdAnd_F32(mask3, v3[i])
            )
        );
    }
}

static inline void
pfiVec2BaryInterpFlatV_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, const PFIsimdv3f w)
{
    // Compare the weights to find the index of the maximum weight
    PFIsimdvf maxWeight = pfiSimdMax_F32(w[0], pfiSimdMax_F32(w[1], w[2]));
    
    // Compare maxWeight to each weight to find which one is the max
    PFIsimdvf mask1 = pfiSimdCmpEQ_F32(maxWeight, w[0]);
    PFIsimdvf mask2 = pfiSimdCmpEQ_F32(maxWeight, w[1]);
    PFIsimdvf mask3 = pfiSimdCmpEQ_F32(maxWeight, w[2]);

    // Use masks to select the corresponding color
    for (int_fast8_t i = 0; i < 2; ++i) {
        dst[i] = pfiSimdOr_F32(
            pfiSimdAnd_F32(mask1, v1[i]),
            pfiSimdOr_F32(
                pfiSimdAnd_F32(mask2, v2[i]),
                pfiSimdAnd_F32(mask3, v3[i])
            )
        );
    }
}

static inline void
pfiVec2BaryInterpFlatVR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, const PFIsimdv3f w)
{
    // Compare the weights to find the index of the maximum weight
    PFIsimdvf maxWeight = pfiSimdMax_F32(w[0], pfiSimdMax_F32(w[1], w[2]));
    
    // Compare maxWeight to each weight to find which one is the max
    PFIsimdvf mask1 = pfiSimdCmpEQ_F32(maxWeight, w[0]);
    PFIsimdvf mask2 = pfiSimdCmpEQ_F32(maxWeight, w[1]);
    PFIsimdvf mask3 = pfiSimdCmpEQ_F32(maxWeight, w[2]);

    // Use masks to select the corresponding color
    for (int_fast8_t i = 0; i < 2; ++i) {
        dst[i] = pfiSimdOr_F32(
            pfiSimdAnd_F32(mask1, v1[i]),
            pfiSimdOr_F32(
                pfiSimdAnd_F32(mask2, v2[i]),
                pfiSimdAnd_F32(mask3, v3[i])
            )
        );
    }
}

static inline void
pfiVec2Transform_simd(PFIsimdv2f dst, const PFIsimdv2f v, const float mat[16])
{
    // Load array elements into SIMD registers
    PFIsimdvf mat_col0 = pfiSimdSet1_F32(mat[0]);
    PFIsimdvf mat_col1 = pfiSimdSet1_F32(mat[1]);
    PFIsimdvf mat_col4 = pfiSimdSet1_F32(mat[4]);
    PFIsimdvf mat_col5 = pfiSimdSet1_F32(mat[5]);
    PFIsimdvf mat_col12 = pfiSimdSet1_F32(mat[12]);
    PFIsimdvf mat_col13 = pfiSimdSet1_F32(mat[13]);

    // Perform SIMD operations and store results in destination vector
    PFIsimdvf tmp0 = pfiSimdAdd_F32(pfiSimdAdd_F32(pfiSimdMul_F32(mat_col0, v[0]), pfiSimdMul_F32(mat_col4, v[1])), mat_col12);
    PFIsimdvf tmp1 = pfiSimdAdd_F32(pfiSimdAdd_F32(pfiSimdMul_F32(mat_col1, v[0]), pfiSimdMul_F32(mat_col5, v[1])), mat_col13);

    dst[0] = tmp0;
    dst[1] = tmp1;
}

static inline void
pfiVec2TransformR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v, const float mat[16])
{
    // Load array elements into SIMD registers
    PFIsimdvf mat_col0 = pfiSimdSet1_F32(mat[0]);
    PFIsimdvf mat_col1 = pfiSimdSet1_F32(mat[1]);
    PFIsimdvf mat_col4 = pfiSimdSet1_F32(mat[4]);
    PFIsimdvf mat_col5 = pfiSimdSet1_F32(mat[5]);
    PFIsimdvf mat_col12 = pfiSimdSet1_F32(mat[12]);
    PFIsimdvf mat_col13 = pfiSimdSet1_F32(mat[13]);

    // Perform SIMD operations and store results in destination vector
    dst[0] = pfiSimdAdd_F32(pfiSimdAdd_F32(pfiSimdMul_F32(mat_col0, v[0]), pfiSimdMul_F32(mat_col4, v[1])), mat_col12);
    dst[1] = pfiSimdAdd_F32(pfiSimdAdd_F32(pfiSimdMul_F32(mat_col1, v[0]), pfiSimdMul_F32(mat_col5, v[1])), mat_col13);
}

static inline void
pfiVec2TransformWT_simd(PFIsimdv2f dst, const PFIsimdv2f v, float wTranslation, const float mat[16])
{
    // Load array elements into SIMD registers
    PFIsimdvf mat_col0 = pfiSimdSet1_F32(mat[0]);
    PFIsimdvf mat_col1 = pfiSimdSet1_F32(mat[1]);
    PFIsimdvf mat_col4 = pfiSimdSet1_F32(mat[4]);
    PFIsimdvf mat_col5 = pfiSimdSet1_F32(mat[5]);
    PFIsimdvf mat_col12 = pfiSimdSet1_F32(mat[12]);
    PFIsimdvf mat_col13 = pfiSimdSet1_F32(mat[13]);
    
    // Load wTranslation into a SIMD register
    PFIsimdvf wTrans = pfiSimdSet1_F32(wTranslation);

    // Perform SIMD operations and store results in destination vector
    PFIsimdvf tmp0 = pfiSimdAdd_F32(
                    pfiSimdAdd_F32(pfiSimdMul_F32(mat_col0, v[0]), pfiSimdMul_F32(mat_col4, v[1])), 
                    pfiSimdMul_F32(wTrans, mat_col12));
    PFIsimdvf tmp1 = pfiSimdAdd_F32(
                    pfiSimdAdd_F32(pfiSimdMul_F32(mat_col1, v[0]), pfiSimdMul_F32(mat_col5, v[1])), 
                    pfiSimdMul_F32(wTrans, mat_col13));

    dst[0] = tmp0;
    dst[1] = tmp1;
}

static inline void
pfiVec2TransformWTR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v, float wTranslation, const float mat[16])
{
    // Load array elements into SIMD registers
    PFIsimdvf mat_col0 = pfiSimdSet1_F32(mat[0]);
    PFIsimdvf mat_col1 = pfiSimdSet1_F32(mat[1]);
    PFIsimdvf mat_col4 = pfiSimdSet1_F32(mat[4]);
    PFIsimdvf mat_col5 = pfiSimdSet1_F32(mat[5]);
    PFIsimdvf mat_col12 = pfiSimdSet1_F32(mat[12]);
    PFIsimdvf mat_col13 = pfiSimdSet1_F32(mat[13]);
    
    // Load wTranslation into a SIMD register
    PFIsimdvf wTrans = pfiSimdSet1_F32(wTranslation);

    // Perform SIMD operations and store results in destination vector
    dst[0] = pfiSimdAdd_F32(
                pfiSimdAdd_F32(pfiSimdMul_F32(mat_col0, v[0]), pfiSimdMul_F32(mat_col4, v[1])), 
                pfiSimdMul_F32(wTrans, mat_col12));
    dst[1] = pfiSimdAdd_F32(
                pfiSimdAdd_F32(pfiSimdMul_F32(mat_col1, v[0]), pfiSimdMul_F32(mat_col5, v[1])), 
                pfiSimdMul_F32(wTrans, mat_col13));
}

/* 3D SIMD Vector function definitions */

static inline void
pfiVec3Zero_simd(PFIsimdv3f dst)
{
    dst[0] = pfiSimdSetZero_F32();
    dst[1] = pfiSimdSetZero_F32();
    dst[2] = pfiSimdSetZero_F32();
}

static inline void
pfiVec3One_simd(PFIsimdv3f dst, float v)
{
    dst[0] = pfiSimdSet1_F32(v);
    dst[1] = pfiSimdSet1_F32(v);
    dst[2] = pfiSimdSet1_F32(v);
}

static inline void
pfiVec3Set_simd(PFIsimdv3f dst, float x, float y, float z)
{
    dst[0] = pfiSimdSet1_F32(x);
    dst[1] = pfiSimdSet1_F32(y);
    dst[2] = pfiSimdSet1_F32(z);
}

static inline void
pfiVec3Load_simd(PFIsimdv3f dst, const PFMvec3 src)
{
    dst[0] = pfiSimdSet1_F32(src[0]);
    dst[1] = pfiSimdSet1_F32(src[1]);
    dst[2] = pfiSimdSet1_F32(src[2]);
}

static inline void
pfiVec3Copy_simd(PFIsimdvf_ptr restrict dst, const PFIsimdvf_ptr restrict src)
{
    memcpy(dst, src, sizeof(PFIsimdv3f));
}

static inline void
pfiVec3Swap_simd(PFIsimdvf_ptr restrict a, PFIsimdvf_ptr restrict b)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        PFIsimdvf tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

static inline void
pfiVec3Blend_simd(PFIsimdv3f dst, const PFIsimdv3f v1, const PFIsimdv3f v2, PFIsimdvf mask)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdBlendV_F32(v1[i], v2[i], mask);
    }
}

static inline void
pfiVec3BlendR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v1, const PFIsimdv3f v2, PFIsimdvf mask)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdBlendV_F32(v1[i], v2[i], mask);
    }
}

static inline void
pfiVec3Neg_simd(PFIsimdv3f dst, const PFIsimdv3f v)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdNeg_F32(v[i]);
    }
}

static inline void
pfiVec3NegR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdNeg_F32(v[i]);
    }
}

static inline void
pfiVec3Add_simd(PFIsimdv3f dst, const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdAdd_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec3AddR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdAdd_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec3Sub_simd(PFIsimdv3f dst, const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdSub_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec3SubR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdSub_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec3Mul_simd(PFIsimdv3f dst, const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdMul_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec3MulR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdMul_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec3Div_simd(PFIsimdv3f dst, const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdDiv_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec3DivR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdDiv_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec3Offset_simd(PFIsimdv3f dst, const PFIsimdv3f v, PFIsimdvf offset)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdAdd_F32(v[i], offset);
    }
}

static inline void
pfiVec3OffsetR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v, PFIsimdvf offset)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdAdd_F32(v[i], offset);
    }
}

static inline void
pfiVec3Scale_simd(PFIsimdv3f dst, const PFIsimdv3f v, PFIsimdvf scale)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdMul_F32(v[i], scale);
    }
}

static inline void
pfiVec3ScaleR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v, PFIsimdvf scale)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdMul_F32(v[i], scale);
    }
}

static inline void
pfiVec3Normalize_simd(PFIsimdv3f dst, const PFIsimdv3f v)
{
    // Calculate the sum of squares of elements
    PFIsimdvf squaredLength = pfiSimdAdd_F32(
        pfiSimdMul_F32(v[0], v[0]),
        pfiSimdMul_F32(v[1], v[1]));

    squaredLength = pfiSimdAdd_F32(squaredLength,
        pfiSimdMul_F32(v[2], v[2]));

    // Calculate the inverse of the square root of length squared
    PFIsimdvf invLength = pfiSimdRSqrt_F32(squaredLength);

    // Normalize vectors
    dst[0] = pfiSimdMul_F32(v[0], invLength);
    dst[1] = pfiSimdMul_F32(v[1], invLength);
    dst[2] = pfiSimdMul_F32(v[2], invLength);
}

static inline void
pfiVec3NormalizeR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v)
{
    // Calculate the sum of squares of elements
    PFIsimdvf squaredLength = pfiSimdAdd_F32(
        pfiSimdMul_F32(v[0], v[0]),
        pfiSimdMul_F32(v[1], v[1]));

    squaredLength = pfiSimdAdd_F32(squaredLength,
        pfiSimdMul_F32(v[2], v[2]));

    // Calculate the inverse of the square root of length squared
    PFIsimdvf invLength = pfiSimdRSqrt_F32(squaredLength);

    // Normalize vectors
    dst[0] = pfiSimdMul_F32(v[0], invLength);
    dst[1] = pfiSimdMul_F32(v[1], invLength);
    dst[2] = pfiSimdMul_F32(v[2], invLength);
}

static inline PFIsimdvf
pfiVec3Length_simd(const PFIsimdv3f v)
{
    PFIsimdvf squaredLength = pfiSimdAdd_F32(
        pfiSimdMul_F32(v[0], v[0]),
        pfiSimdMul_F32(v[1], v[1]));

    squaredLength = pfiSimdAdd_F32(squaredLength,
        pfiSimdMul_F32(v[2], v[2]));

    return pfiSimdSqrt_F32(squaredLength);
}

static inline PFIsimdvf
pfiVec3LengthSq_simd(const PFIsimdv3f v)
{
    PFIsimdvf squaredLength = pfiSimdAdd_F32(
        pfiSimdMul_F32(v[0], v[0]),
        pfiSimdMul_F32(v[1], v[1]));

    return pfiSimdAdd_F32(squaredLength,
        pfiSimdMul_F32(v[2], v[2]));
}

static inline PFIsimdvf
pfiVec3Dot_simd(const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    PFIsimdvf dotProduct = pfiSimdAdd_F32(
        pfiSimdMul_F32(v1[0], v2[0]),
        pfiSimdMul_F32(v1[1], v2[1]));

    return pfiSimdAdd_F32(dotProduct,
        pfiSimdMul_F32(v1[2], v2[2]));
}

static inline void
pfiVec3Cross_simd(PFIsimdv3f dst, const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    PFIsimdv3f tmp = { 0 };

    tmp[0] = pfiSimdSub_F32(
        pfiSimdMul_F32(v1[1], v2[2]),
        pfiSimdMul_F32(v1[2], v2[1]));

    tmp[1] = pfiSimdSub_F32(
        pfiSimdMul_F32(v1[2], v2[0]),
        pfiSimdMul_F32(v1[0], v2[2]));

    tmp[2] = pfiSimdSub_F32(
        pfiSimdMul_F32(v1[0], v2[1]),
        pfiSimdMul_F32(v1[1], v2[0]));

    memcpy(dst, tmp, sizeof(PFIsimdv3f));
}

static inline void
pfiVec3CrossR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    dst[0] = pfiSimdSub_F32(
        pfiSimdMul_F32(v1[1], v2[2]),
        pfiSimdMul_F32(v1[2], v2[1]));

    dst[1] = pfiSimdSub_F32(
        pfiSimdMul_F32(v1[2], v2[0]),
        pfiSimdMul_F32(v1[0], v2[2]));

    dst[2] = pfiSimdSub_F32(
        pfiSimdMul_F32(v1[0], v2[1]),
        pfiSimdMul_F32(v1[1], v2[0]));
}

static inline PFIsimdvf
pfiVec3Distance_simd(const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    // Calculate the differences between corresponding elements of the vectors
    PFIsimdvf dt0 = pfiSimdSub_F32(v1[0], v2[0]);
    PFIsimdvf dt1 = pfiSimdSub_F32(v1[1], v2[1]);;
    PFIsimdvf dt2 = pfiSimdSub_F32(v1[1], v2[1]);

    // Calculate the squared differences
    PFIsimdvf dt0Sq = pfiSimdMul_F32(dt0, dt0);
    PFIsimdvf dt1Sq = pfiSimdMul_F32(dt1, dt1);
    PFIsimdvf dt2Sq = pfiSimdMul_F32(dt2, dt2);

    // Sum the squared differences
    PFIsimdvf distanceSq = pfiSimdAdd_F32(dt0Sq,
        pfiSimdAdd_F32(dt1Sq, dt2Sq));

    // Calculate and return the square root of the sum of squared differences
    return pfiSimdSqrt_F32(distanceSq);
}

static inline PFIsimdvf
pfiVec3DistanceSq_simd(const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    // Calculate the differences between corresponding elements of the vectors
    PFIsimdvf dt0 = pfiSimdSub_F32(v1[0], v2[0]);
    PFIsimdvf dt1 = pfiSimdSub_F32(v1[1], v2[1]);;
    PFIsimdvf dt2 = pfiSimdSub_F32(v1[1], v2[1]);

    // Calculate the squared differences
    PFIsimdvf dt0Sq = pfiSimdMul_F32(dt0, dt0);
    PFIsimdvf dt1Sq = pfiSimdMul_F32(dt1, dt1);
    PFIsimdvf dt2Sq = pfiSimdMul_F32(dt2, dt2);

    // Sum the squared differences
    PFIsimdvf distanceSq = pfiSimdAdd_F32(dt0Sq,
        pfiSimdAdd_F32(dt1Sq, dt2Sq));

    // Calculate and return the square root of the sum of squared differences
    return distanceSq;
}

static inline void
pfiVec3Direction_simd(PFIsimdv3f dst, const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    // Calculate the differences between the elements of the two vectors
    PFIsimdvf tmp0 = pfiSimdSub_F32(v1[0], v2[0]);
    PFIsimdvf tmp1 = pfiSimdSub_F32(v1[1], v2[1]);
    PFIsimdvf tmp2 = pfiSimdSub_F32(v1[2], v2[2]);

    // Calculate the sum of the squares of these differences to obtain the length squared
    PFIsimdvf lengthSq = pfiSimdAdd_F32(pfiSimdMul_F32(tmp0, tmp0), pfiSimdMul_F32(tmp1, tmp1));
    lengthSq = pfiSimdAdd_F32(lengthSq, pfiSimdMul_F32(tmp2, tmp2));

    // Add a small epsilon value to avoid division by zero
    lengthSq = pfiSimdMax_F32(lengthSq, *(PFIsimdvf*)GC_simd_f32_epsilon);

    // Calculate the inverse of the square root of the length squared to normalize the differences
    PFIsimdvf invLength = pfiSimdRSqrt_F32(lengthSq);

    // Multiply each difference by this inverse to obtain the normalized direction
    dst[0] = pfiSimdMul_F32(tmp0, invLength);
    dst[1] = pfiSimdMul_F32(tmp1, invLength);
    dst[2] = pfiSimdMul_F32(tmp2, invLength);
}

static inline void
pfiVec3DirectionR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v1, const PFIsimdv3f v2)
{
    // Calculate the differences between the elements of the two vectors
    dst[0] = pfiSimdSub_F32(v1[0], v2[0]);
    dst[1] = pfiSimdSub_F32(v1[1], v2[1]);
    dst[2] = pfiSimdSub_F32(v1[2], v2[2]);

    // Calculate the sum of the squares of these differences to obtain the length squared
    PFIsimdvf lengthSq = pfiSimdAdd_F32(pfiSimdMul_F32(dst[0], dst[0]), pfiSimdMul_F32(dst[1], dst[1]));
    lengthSq = pfiSimdAdd_F32(lengthSq, pfiSimdMul_F32(dst[2], dst[2]));

    // Add a small epsilon value to avoid division by zero
    lengthSq = pfiSimdMax_F32(lengthSq, *(PFIsimdvf*)GC_simd_f32_epsilon);

    // Calculate the inverse of the square root of the length squared to normalize the differences
    PFIsimdvf invLength = pfiSimdRSqrt_F32(lengthSq);

    // Multiply each difference by this inverse to obtain the normalized direction
    dst[0] = pfiSimdMul_F32(dst[0], invLength);
    dst[1] = pfiSimdMul_F32(dst[1], invLength);
    dst[2] = pfiSimdMul_F32(dst[2], invLength);
}

static inline void
pfiVec3Lerp_simd(PFIsimdv3f dst, const PFIsimdv3f v1, const PFIsimdv3f v2, PFIsimdvf t)
{
    dst[0] = pfiSimdAdd_F32(v1[0], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[0], v1[0])));
    dst[1] = pfiSimdAdd_F32(v1[1], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[1], v1[1])));
    dst[2] = pfiSimdAdd_F32(v1[2], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[2], v1[2])));
}

static inline void
pfiVec3LerpR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v1, const PFIsimdv3f v2, PFIsimdvf t)
{
    dst[0] = pfiSimdAdd_F32(v1[0], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[0], v1[0])));
    dst[1] = pfiSimdAdd_F32(v1[1], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[1], v1[1])));
    dst[2] = pfiSimdAdd_F32(v1[2], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[2], v1[2])));
}

static inline void
pfiVec3BaryInterpSmooth_simd(PFIsimdv3f dst, const PFIsimdv3f v1, const PFIsimdv3f v2, const PFIsimdv3f v3, PFIsimdvf w1, PFIsimdvf w2, PFIsimdvf w3)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        PFIsimdvf v1_w1 = pfiSimdMul_F32(v1[i], w1);
        PFIsimdvf v2_w2 = pfiSimdMul_F32(v2[i], w2);
        PFIsimdvf v3_w3 = pfiSimdMul_F32(v3[i], w3);
        dst[i] = pfiSimdAdd_F32(pfiSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

static inline void
pfiVec3BaryInterpSmoothR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v1, const PFIsimdv3f v2, const PFIsimdv3f v3, PFIsimdvf w1, PFIsimdvf w2, PFIsimdvf w3)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        PFIsimdvf v1_w1 = pfiSimdMul_F32(v1[i], w1);
        PFIsimdvf v2_w2 = pfiSimdMul_F32(v2[i], w2);
        PFIsimdvf v3_w3 = pfiSimdMul_F32(v3[i], w3);
        dst[i] = pfiSimdAdd_F32(pfiSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

static inline void
pfiVec3BaryInterpSmoothV_simd(PFIsimdv3f dst, const PFIsimdv3f v1, const PFIsimdv3f v2, const PFIsimdv3f v3, const PFIsimdv3f w)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        PFIsimdvf v1_w1 = pfiSimdMul_F32(v1[i], w[0]);
        PFIsimdvf v2_w2 = pfiSimdMul_F32(v2[i], w[1]);
        PFIsimdvf v3_w3 = pfiSimdMul_F32(v3[i], w[2]);
        dst[i] = pfiSimdAdd_F32(pfiSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

static inline void
pfiVec3BaryInterpSmoothVR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v1, const PFIsimdv3f v2, const PFIsimdv3f v3, const PFIsimdv3f w)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        PFIsimdvf v1_w1 = pfiSimdMul_F32(v1[i], w[0]);
        PFIsimdvf v2_w2 = pfiSimdMul_F32(v2[i], w[1]);
        PFIsimdvf v3_w3 = pfiSimdMul_F32(v3[i], w[2]);
        dst[i] = pfiSimdAdd_F32(pfiSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

static inline void
pfiVec3BaryInterpFlat_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, PFIsimdvf w1, PFIsimdvf w2, PFIsimdvf w3)
{
    // Compare the weights to find the index of the maximum weight
    PFIsimdvf maxWeight = pfiSimdMax_F32(w1, pfiSimdMax_F32(w2, w3));
    
    // Compare maxWeight to each weight to find which one is the max
    PFIsimdvf mask1 = pfiSimdCmpEQ_F32(maxWeight, w1);
    PFIsimdvf mask2 = pfiSimdCmpEQ_F32(maxWeight, w2);
    PFIsimdvf mask3 = pfiSimdCmpEQ_F32(maxWeight, w3);

    // Use masks to select the corresponding color
    for (int_fast8_t i = 0; i < 3; ++i) {
        dst[i] = pfiSimdOr_F32(
            pfiSimdAnd_F32(mask1, v1[i]),
            pfiSimdOr_F32(
                pfiSimdAnd_F32(mask2, v2[i]),
                pfiSimdAnd_F32(mask3, v3[i])
            )
        );
    }
}

static inline void
pfiVec3BaryInterpFlatR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, PFIsimdvf w1, PFIsimdvf w2, PFIsimdvf w3)
{
    // Compare the weights to find the index of the maximum weight
    PFIsimdvf maxWeight = pfiSimdMax_F32(w1, pfiSimdMax_F32(w2, w3));
    
    // Compare maxWeight to each weight to find which one is the max
    PFIsimdvf mask1 = pfiSimdCmpEQ_F32(maxWeight, w1);
    PFIsimdvf mask2 = pfiSimdCmpEQ_F32(maxWeight, w2);
    PFIsimdvf mask3 = pfiSimdCmpEQ_F32(maxWeight, w3);

    // Use masks to select the corresponding color
    for (int_fast8_t i = 0; i < 3; ++i) {
        dst[i] = pfiSimdOr_F32(
            pfiSimdAnd_F32(mask1, v1[i]),
            pfiSimdOr_F32(
                pfiSimdAnd_F32(mask2, v2[i]),
                pfiSimdAnd_F32(mask3, v3[i])
            )
        );
    }
}

static inline void
pfiVec3BaryInterpFlatV_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, const PFIsimdv3f w)
{
    // Compare the weights to find the index of the maximum weight
    PFIsimdvf maxWeight = pfiSimdMax_F32(w[0], pfiSimdMax_F32(w[1], w[2]));
    
    // Compare maxWeight to each weight to find which one is the max
    PFIsimdvf mask1 = pfiSimdCmpEQ_F32(maxWeight, w[0]);
    PFIsimdvf mask2 = pfiSimdCmpEQ_F32(maxWeight, w[1]);
    PFIsimdvf mask3 = pfiSimdCmpEQ_F32(maxWeight, w[2]);

    // Use masks to select the corresponding color
    for (int_fast8_t i = 0; i < 3; ++i) {
        dst[i] = pfiSimdOr_F32(
            pfiSimdAnd_F32(mask1, v1[i]),
            pfiSimdOr_F32(
                pfiSimdAnd_F32(mask2, v2[i]),
                pfiSimdAnd_F32(mask3, v3[i])
            )
        );
    }
}

static inline void
pfiVec3BaryInterpFlatVR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, const PFIsimdv3f w)
{
    // Compare the weights to find the index of the maximum weight
    PFIsimdvf maxWeight = pfiSimdMax_F32(w[0], pfiSimdMax_F32(w[1], w[2]));
    
    // Compare maxWeight to each weight to find which one is the max
    PFIsimdvf mask1 = pfiSimdCmpEQ_F32(maxWeight, w[0]);
    PFIsimdvf mask2 = pfiSimdCmpEQ_F32(maxWeight, w[1]);
    PFIsimdvf mask3 = pfiSimdCmpEQ_F32(maxWeight, w[2]);

    // Use masks to select the corresponding color
    for (int_fast8_t i = 0; i < 3; ++i) {
        dst[i] = pfiSimdOr_F32(
            pfiSimdAnd_F32(mask1, v1[i]),
            pfiSimdOr_F32(
                pfiSimdAnd_F32(mask2, v2[i]),
                pfiSimdAnd_F32(mask3, v3[i])
            )
        );
    }
}

static inline void
pfiVec3Transform_simd(PFIsimdv3f dst, const PFIsimdv3f v, const float mat[16])
{
    // Charger les éléments de la matrice dans les registres SIMD
    PFIsimdvf mat_col0 = pfiSimdSet1_F32(mat[0]);
    PFIsimdvf mat_col1 = pfiSimdSet1_F32(mat[1]);
    PFIsimdvf mat_col2 = pfiSimdSet1_F32(mat[2]);
    PFIsimdvf mat_col4 = pfiSimdSet1_F32(mat[4]);
    PFIsimdvf mat_col5 = pfiSimdSet1_F32(mat[5]);
    PFIsimdvf mat_col6 = pfiSimdSet1_F32(mat[6]);
    PFIsimdvf mat_col8 = pfiSimdSet1_F32(mat[8]);
    PFIsimdvf mat_col9 = pfiSimdSet1_F32(mat[9]);
    PFIsimdvf mat_col10 = pfiSimdSet1_F32(mat[10]);
    PFIsimdvf mat_col12 = pfiSimdSet1_F32(mat[12]);
    PFIsimdvf mat_col13 = pfiSimdSet1_F32(mat[13]);
    PFIsimdvf mat_col14 = pfiSimdSet1_F32(mat[14]);

    // Calculer les composants du vecteur transformé
    PFIsimdvf tmp0 = pfiSimdAdd_F32(
        pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col0, v[0]), pfiSimdMul_F32(mat_col4, v[1])),
            pfiSimdMul_F32(mat_col8, v[2])
        ),
        mat_col12
    );

    PFIsimdvf tmp1 = pfiSimdAdd_F32(
        pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col1, v[0]), pfiSimdMul_F32(mat_col5, v[1])),
            pfiSimdMul_F32(mat_col9, v[2])
        ),
        mat_col13
    );

    PFIsimdvf tmp2 = pfiSimdAdd_F32(
        pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col2, v[0]), pfiSimdMul_F32(mat_col6, v[1])),
            pfiSimdMul_F32(mat_col10, v[2])
        ),
        mat_col14
    );

    // Stocker les résultats dans le vecteur destination
    dst[0] = tmp0;
    dst[1] = tmp1;
    dst[2] = tmp2;
}

static inline void
pfiVec3TransformR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v, const float mat[16])
{
    PFIsimdvf mat_col0 = pfiSimdSet1_F32(mat[0]);
    PFIsimdvf mat_col1 = pfiSimdSet1_F32(mat[1]);
    PFIsimdvf mat_col2 = pfiSimdSet1_F32(mat[2]);
    PFIsimdvf mat_col4 = pfiSimdSet1_F32(mat[4]);
    PFIsimdvf mat_col5 = pfiSimdSet1_F32(mat[5]);
    PFIsimdvf mat_col6 = pfiSimdSet1_F32(mat[6]);
    PFIsimdvf mat_col8 = pfiSimdSet1_F32(mat[8]);
    PFIsimdvf mat_col9 = pfiSimdSet1_F32(mat[9]);
    PFIsimdvf mat_col10 = pfiSimdSet1_F32(mat[10]);
    PFIsimdvf mat_col12 = pfiSimdSet1_F32(mat[12]);
    PFIsimdvf mat_col13 = pfiSimdSet1_F32(mat[13]);
    PFIsimdvf mat_col14 = pfiSimdSet1_F32(mat[14]);

    dst[0] = pfiSimdAdd_F32(
        pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col0, v[0]), pfiSimdMul_F32(mat_col4, v[1])),
            pfiSimdMul_F32(mat_col8, v[2])
        ),
        mat_col12
    );

    dst[1] = pfiSimdAdd_F32(
        pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col1, v[0]), pfiSimdMul_F32(mat_col5, v[1])),
            pfiSimdMul_F32(mat_col9, v[2])
        ),
        mat_col13
    );

    dst[2] = pfiSimdAdd_F32(
        pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col2, v[0]), pfiSimdMul_F32(mat_col6, v[1])),
            pfiSimdMul_F32(mat_col10, v[2])
        ),
        mat_col14
    );
}

static inline void
pfiVec3TransformWT_simd(PFIsimdv3f dst, const PFIsimdv3f v, float wTranslation, const float mat[16])
{
    PFIsimdvf mat_col0 = pfiSimdSet1_F32(mat[0]);
    PFIsimdvf mat_col1 = pfiSimdSet1_F32(mat[1]);
    PFIsimdvf mat_col2 = pfiSimdSet1_F32(mat[2]);
    PFIsimdvf mat_col4 = pfiSimdSet1_F32(mat[4]);
    PFIsimdvf mat_col5 = pfiSimdSet1_F32(mat[5]);
    PFIsimdvf mat_col6 = pfiSimdSet1_F32(mat[6]);
    PFIsimdvf mat_col8 = pfiSimdSet1_F32(mat[8]);
    PFIsimdvf mat_col9 = pfiSimdSet1_F32(mat[9]);
    PFIsimdvf mat_col10 = pfiSimdSet1_F32(mat[10]);
    PFIsimdvf mat_col12 = pfiSimdSet1_F32(mat[12]);
    PFIsimdvf mat_col13 = pfiSimdSet1_F32(mat[13]);
    PFIsimdvf mat_col14 = pfiSimdSet1_F32(mat[14]);

    PFIsimdvf wTrans = pfiSimdSet1_F32(wTranslation);

    PFIsimdvf tmp0 = pfiSimdAdd_F32(
        pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col0, v[0]), pfiSimdMul_F32(mat_col4, v[1])),
            pfiSimdMul_F32(mat_col8, v[2])
        ),
        pfiSimdMul_F32(wTrans, mat_col12)
    );

    PFIsimdvf tmp1 = pfiSimdAdd_F32(
        pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col1, v[0]), pfiSimdMul_F32(mat_col5, v[1])),
            pfiSimdMul_F32(mat_col9, v[2])
        ),
        pfiSimdMul_F32(wTrans, mat_col13)
    );

    PFIsimdvf tmp2 = pfiSimdAdd_F32(
        pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col2, v[0]), pfiSimdMul_F32(mat_col6, v[1])),
            pfiSimdMul_F32(mat_col10, v[2])
        ),
        pfiSimdMul_F32(wTrans, mat_col14)
    );

    dst[0] = tmp0;
    dst[1] = tmp1;
    dst[2] = tmp2;
}

static inline void
pfiVec3TransformWTR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f v, float wTranslation, const float mat[16])
{
    PFIsimdvf mat_col0 = pfiSimdSet1_F32(mat[0]);
    PFIsimdvf mat_col1 = pfiSimdSet1_F32(mat[1]);
    PFIsimdvf mat_col2 = pfiSimdSet1_F32(mat[2]);
    PFIsimdvf mat_col4 = pfiSimdSet1_F32(mat[4]);
    PFIsimdvf mat_col5 = pfiSimdSet1_F32(mat[5]);
    PFIsimdvf mat_col6 = pfiSimdSet1_F32(mat[6]);
    PFIsimdvf mat_col8 = pfiSimdSet1_F32(mat[8]);
    PFIsimdvf mat_col9 = pfiSimdSet1_F32(mat[9]);
    PFIsimdvf mat_col10 = pfiSimdSet1_F32(mat[10]);
    PFIsimdvf mat_col12 = pfiSimdSet1_F32(mat[12]);
    PFIsimdvf mat_col13 = pfiSimdSet1_F32(mat[13]);
    PFIsimdvf mat_col14 = pfiSimdSet1_F32(mat[14]);

    PFIsimdvf wTrans = pfiSimdSet1_F32(wTranslation);

    dst[0] = pfiSimdAdd_F32(
        pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col0, v[0]), pfiSimdMul_F32(mat_col4, v[1])),
            pfiSimdMul_F32(mat_col8, v[2])
        ),
        pfiSimdMul_F32(wTrans, mat_col12)
    );

    dst[1] = pfiSimdAdd_F32(
        pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col1, v[0]), pfiSimdMul_F32(mat_col5, v[1])),
            pfiSimdMul_F32(mat_col9, v[2])
        ),
        pfiSimdMul_F32(wTrans, mat_col13)
    );

    dst[2] = pfiSimdAdd_F32(
        pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col2, v[0]), pfiSimdMul_F32(mat_col6, v[1])),
            pfiSimdMul_F32(mat_col10, v[2])
        ),
        pfiSimdMul_F32(wTrans, mat_col14)
    );
}

static inline void
pfiVec3Reflect_simd(PFIsimdv3f dst, const PFIsimdv3f incident, const PFIsimdv3f normal)
{
    PFIsimdvf dotProduct = pfiSimdAdd_F32(
        pfiSimdAdd_F32(
            pfiSimdMul_F32(incident[0], normal[0]),
            pfiSimdMul_F32(incident[1], normal[1])
        ),
        pfiSimdMul_F32(incident[2], normal[2])
    );

    PFIsimdvf dotProduct2 = pfiSimdMul_F32(dotProduct, *(PFIsimdvf*)GC_simd_f32_2);

    PFIsimdvf tmp0 = pfiSimdSub_F32(incident[0], pfiSimdMul_F32(dotProduct2, normal[0]));
    PFIsimdvf tmp1 = pfiSimdSub_F32(incident[1], pfiSimdMul_F32(dotProduct2, normal[1]));
    PFIsimdvf tmp2 = pfiSimdSub_F32(incident[2], pfiSimdMul_F32(dotProduct2, normal[2]));

    dst[0] = tmp0;
    dst[1] = tmp1;
    dst[2] = tmp2;
}

static inline void
pfiVec3ReflectR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv3f incident, const PFIsimdv3f normal)
{
    PFIsimdvf dotProduct = pfiSimdAdd_F32(
        pfiSimdAdd_F32(
            pfiSimdMul_F32(incident[0], normal[0]),
            pfiSimdMul_F32(incident[1], normal[1])
        ),
        pfiSimdMul_F32(incident[2], normal[2])
    );

    PFIsimdvf dotProduct2 = pfiSimdMul_F32(dotProduct, *(PFIsimdvf*)GC_simd_f32_2);

    dst[0] = pfiSimdSub_F32(incident[0], pfiSimdMul_F32(dotProduct2, normal[0]));
    dst[1] = pfiSimdSub_F32(incident[1], pfiSimdMul_F32(dotProduct2, normal[1]));
    dst[2] = pfiSimdSub_F32(incident[2], pfiSimdMul_F32(dotProduct2, normal[2]));
}

/* 4D SIMD Vector function definitions */

static inline void
pfiVec4Zero_simd(PFIsimdv4f dst)
{
    dst[0] = pfiSimdSetZero_F32();
    dst[1] = pfiSimdSetZero_F32();
    dst[2] = pfiSimdSetZero_F32();
    dst[3] = pfiSimdSetZero_F32();
}

static inline void
pfiVec4One_simd(PFIsimdv4f dst, float v)
{
    dst[0] = pfiSimdSet1_F32(v);
    dst[1] = pfiSimdSet1_F32(v);
    dst[2] = pfiSimdSet1_F32(v);
    dst[3] = pfiSimdSet1_F32(v);
}

static inline void
pfiVec4Set_simd(PFIsimdv4f dst, float x, float y, float z, float w)
{
    dst[0] = pfiSimdSet1_F32(x);
    dst[1] = pfiSimdSet1_F32(y);
    dst[2] = pfiSimdSet1_F32(z);
    dst[4] = pfiSimdSet1_F32(w);
}

static inline void
pfiVec4Load_simd(PFIsimdv4f dst, const PFMvec4 src)
{
    dst[0] = pfiSimdSet1_F32(src[0]);
    dst[1] = pfiSimdSet1_F32(src[1]);
    dst[2] = pfiSimdSet1_F32(src[2]);
    dst[3] = pfiSimdSet1_F32(src[3]);
}

static inline void
pfiVec4Copy_simd(PFIsimdvf_ptr restrict dst, const PFIsimdvf_ptr restrict src)
{
    memcpy(dst, src, sizeof(PFIsimdv4f));
}

static inline void
pfiVec4Swap_simd(PFIsimdvf_ptr restrict a, PFIsimdvf_ptr restrict b)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        PFIsimdvf tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

static inline void
pfiVec4Blend_simd(PFIsimdv4f dst, const PFIsimdv4f v1, const PFIsimdv4f v2, PFIsimdvf mask)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = pfiSimdBlendV_F32(v1[i], v2[i], mask);
    }
}

static inline void
pfiVec4BlendR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv4f v1, const PFIsimdv4f v2, PFIsimdvf mask)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = pfiSimdBlendV_F32(v1[i], v2[i], mask);
    }
}

static inline void
pfiVec4Neg_simd(PFIsimdv4f dst, const PFIsimdv4f v)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = pfiSimdNeg_F32(v[i]);
    }
}

static inline void
pfiVec4NegR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv4f v)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = pfiSimdNeg_F32(v[i]);
    }
}

static inline void
pfiVec4Add_simd(PFIsimdv4f dst, const PFIsimdv4f v1, const PFIsimdv4f v2)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = pfiSimdAdd_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec4AddR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv4f v1, const PFIsimdv4f v2)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = pfiSimdAdd_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec4Sub_simd(PFIsimdv4f dst, const PFIsimdv4f v1, const PFIsimdv4f v2)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = pfiSimdSub_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec4SubR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv4f v1, const PFIsimdv4f v2)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = pfiSimdSub_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec4Mul_simd(PFIsimdv4f dst, const PFIsimdv4f v1, const PFIsimdv4f v2)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = pfiSimdMul_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec4MulR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv4f v1, const PFIsimdv4f v2)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = pfiSimdMul_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec4Div_simd(PFIsimdv4f dst, const PFIsimdv4f v1, const PFIsimdv4f v2)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = pfiSimdDiv_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec4DivR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv4f v1, const PFIsimdv4f v2)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = pfiSimdDiv_F32(v1[i], v2[i]);
    }
}

static inline void
pfiVec4Offset_simd(PFIsimdv4f dst, const PFIsimdv4f v, PFIsimdvf offset)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdAdd_F32(v[i], offset);
    }
}

static inline void
pfiVec4OffsetR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv4f v, PFIsimdvf offset)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdAdd_F32(v[i], offset);
    }
}

static inline void
pfiVec4Scale_simd(PFIsimdv4f dst, const PFIsimdv4f v, PFIsimdvf scale)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdMul_F32(v[i], scale);
    }
}

static inline void
pfiVec4ScaleR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv4f v, PFIsimdvf scale)
{
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = pfiSimdMul_F32(v[i], scale);
    }
}

static inline void
pfiVec4Normalize_simd(PFIsimdv4f dst, const PFIsimdv4f v)
{
    // Calculate the sum of squares of elements
    PFIsimdvf squaredLength = pfiSimdAdd_F32(
        pfiSimdMul_F32(v[0], v[0]),
        pfiSimdMul_F32(v[1], v[1]));

    squaredLength = pfiSimdAdd_F32(squaredLength,
        pfiSimdMul_F32(v[2], v[2]));

    squaredLength = pfiSimdAdd_F32(squaredLength,
        pfiSimdMul_F32(v[3], v[3]));

    // Calculate the inverse of the square root of length squared
    PFIsimdvf invLength = pfiSimdRSqrt_F32(squaredLength);

    // Normalize vectors
    dst[0] = pfiSimdMul_F32(v[0], invLength);
    dst[1] = pfiSimdMul_F32(v[1], invLength);
    dst[2] = pfiSimdMul_F32(v[2], invLength);
    dst[3] = pfiSimdMul_F32(v[3], invLength);
}

static inline void
pfiVec4NormalizeR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv4f v)
{
    // Calculate the sum of squares of elements
    PFIsimdvf squaredLength = pfiSimdAdd_F32(
        pfiSimdMul_F32(v[0], v[0]),
        pfiSimdMul_F32(v[1], v[1]));

    squaredLength = pfiSimdAdd_F32(squaredLength,
        pfiSimdMul_F32(v[2], v[2]));

    squaredLength = pfiSimdAdd_F32(squaredLength,
        pfiSimdMul_F32(v[3], v[3]));

    // Calculate the inverse of the square root of length squared
    PFIsimdvf invLength = pfiSimdRSqrt_F32(squaredLength);

    // Normalize vectors
    dst[0] = pfiSimdMul_F32(v[0], invLength);
    dst[1] = pfiSimdMul_F32(v[1], invLength);
    dst[2] = pfiSimdMul_F32(v[2], invLength);
    dst[3] = pfiSimdMul_F32(v[3], invLength);
}

static inline PFIsimdvf
pfiVec4Length_simd(const PFIsimdv4f v)
{
    PFIsimdvf squaredLength = pfiSimdAdd_F32(
        pfiSimdMul_F32(v[0], v[0]),
        pfiSimdMul_F32(v[1], v[1]));

    squaredLength = pfiSimdAdd_F32(squaredLength,
        pfiSimdMul_F32(v[2], v[2]));

    squaredLength = pfiSimdAdd_F32(squaredLength,
        pfiSimdMul_F32(v[3], v[3]));

    return pfiSimdSqrt_F32(squaredLength);
}

static inline PFIsimdvf
pfiVec4LengthSq_simd(const PFIsimdv4f v)
{
    PFIsimdvf squaredLength = pfiSimdAdd_F32(
        pfiSimdMul_F32(v[0], v[0]),
        pfiSimdMul_F32(v[1], v[1]));

    squaredLength = pfiSimdAdd_F32(squaredLength,
        pfiSimdMul_F32(v[2], v[2]));

    return pfiSimdAdd_F32(squaredLength,
        pfiSimdMul_F32(v[3], v[3]));
}

static inline PFIsimdvf
pfiVec4Dot_simd(const PFIsimdv4f v1, const PFIsimdv4f v2)
{
    PFIsimdvf dotProduct = pfiSimdAdd_F32(
        pfiSimdMul_F32(v1[0], v2[0]),
        pfiSimdMul_F32(v1[1], v2[1]));

    dotProduct = pfiSimdAdd_F32(dotProduct,
        pfiSimdMul_F32(v1[2], v2[2]));

    return pfiSimdAdd_F32(dotProduct,
        pfiSimdMul_F32(v1[3], v2[3]));
}

static inline void
pfiVec4Lerp_simd(PFIsimdv4f dst, const PFIsimdv4f v1, const PFIsimdv4f v2, PFIsimdvf t)
{
    dst[0] = pfiSimdAdd_F32(v1[0], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[0], v1[0])));
    dst[1] = pfiSimdAdd_F32(v1[1], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[1], v1[1])));
    dst[2] = pfiSimdAdd_F32(v1[2], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[2], v1[2])));
    dst[3] = pfiSimdAdd_F32(v1[3], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[3], v1[3])));
}

static inline void
pfiVec4LerpR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv4f v1, const PFIsimdv4f v2, PFIsimdvf t)
{
    dst[0] = pfiSimdAdd_F32(v1[0], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[0], v1[0])));
    dst[1] = pfiSimdAdd_F32(v1[1], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[1], v1[1])));
    dst[2] = pfiSimdAdd_F32(v1[2], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[2], v1[2])));
    dst[3] = pfiSimdAdd_F32(v1[3], pfiSimdMul_F32(t, pfiSimdSub_F32(v2[3], v1[3])));
}

static inline void
pfiVec4BaryInterpSmooth_simd(PFIsimdv4f dst, const PFIsimdv4f v1, const PFIsimdv4f v2, const PFIsimdv4f v3, PFIsimdvf w1, PFIsimdvf w2, PFIsimdvf w3)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        PFIsimdvf v1_w1 = pfiSimdMul_F32(v1[i], w1);
        PFIsimdvf v2_w2 = pfiSimdMul_F32(v2[i], w2);
        PFIsimdvf v3_w3 = pfiSimdMul_F32(v3[i], w3);
        dst[i] = pfiSimdAdd_F32(pfiSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

static inline void
pfiVec4BaryInterpSmoothR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv4f v1, const PFIsimdv4f v2, const PFIsimdv4f v3, PFIsimdvf w1, PFIsimdvf w2, PFIsimdvf w3)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        PFIsimdvf v1_w1 = pfiSimdMul_F32(v1[i], w1);
        PFIsimdvf v2_w2 = pfiSimdMul_F32(v2[i], w2);
        PFIsimdvf v3_w3 = pfiSimdMul_F32(v3[i], w3);
        dst[i] = pfiSimdAdd_F32(pfiSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

static inline void
pfiVec4BaryInterpSmoothV_simd(PFIsimdv4f dst, const PFIsimdv4f v1, const PFIsimdv4f v2, const PFIsimdv4f v3, const PFIsimdv3f w)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        PFIsimdvf v1_w1 = pfiSimdMul_F32(v1[i], w[0]);
        PFIsimdvf v2_w2 = pfiSimdMul_F32(v2[i], w[1]);
        PFIsimdvf v3_w3 = pfiSimdMul_F32(v3[i], w[2]);
        dst[i] = pfiSimdAdd_F32(pfiSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

static inline void
pfiVec4BaryInterpSmoothVR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv4f v1, const PFIsimdv4f v2, const PFIsimdv4f v3, const PFIsimdv3f w)
{
    for (int_fast8_t i = 0; i < 4; i++) {
        PFIsimdvf v1_w1 = pfiSimdMul_F32(v1[i], w[0]);
        PFIsimdvf v2_w2 = pfiSimdMul_F32(v2[i], w[1]);
        PFIsimdvf v3_w3 = pfiSimdMul_F32(v3[i], w[2]);
        dst[i] = pfiSimdAdd_F32(pfiSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

static inline void
pfiVec4BaryInterpFlat_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, PFIsimdvf w1, PFIsimdvf w2, PFIsimdvf w3)
{
    // Compare the weights to find the index of the maximum weight
    PFIsimdvf maxWeight = pfiSimdMax_F32(w1, pfiSimdMax_F32(w2, w3));
    
    // Compare maxWeight to each weight to find which one is the max
    PFIsimdvf mask1 = pfiSimdCmpEQ_F32(maxWeight, w1);
    PFIsimdvf mask2 = pfiSimdCmpEQ_F32(maxWeight, w2);
    PFIsimdvf mask3 = pfiSimdCmpEQ_F32(maxWeight, w3);

    // Use masks to select the corresponding color
    for (int_fast8_t i = 0; i < 4; ++i) {
        dst[i] = pfiSimdOr_F32(
            pfiSimdAnd_F32(mask1, v1[i]),
            pfiSimdOr_F32(
                pfiSimdAnd_F32(mask2, v2[i]),
                pfiSimdAnd_F32(mask3, v3[i])
            )
        );
    }
}

static inline void
pfiVec4BaryInterpFlatR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, PFIsimdvf w1, PFIsimdvf w2, PFIsimdvf w3)
{
    // Compare the weights to find the index of the maximum weight
    PFIsimdvf maxWeight = pfiSimdMax_F32(w1, pfiSimdMax_F32(w2, w3));
    
    // Compare maxWeight to each weight to find which one is the max
    PFIsimdvf mask1 = pfiSimdCmpEQ_F32(maxWeight, w1);
    PFIsimdvf mask2 = pfiSimdCmpEQ_F32(maxWeight, w2);
    PFIsimdvf mask3 = pfiSimdCmpEQ_F32(maxWeight, w3);

    // Use masks to select the corresponding color
    for (int_fast8_t i = 0; i < 4; ++i) {
        dst[i] = pfiSimdOr_F32(
            pfiSimdAnd_F32(mask1, v1[i]),
            pfiSimdOr_F32(
                pfiSimdAnd_F32(mask2, v2[i]),
                pfiSimdAnd_F32(mask3, v3[i])
            )
        );
    }
}

static inline void
pfiVec4BaryInterpFlatV_simd(PFIsimdv2f dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, const PFIsimdv3f w)
{
    // Compare the weights to find the index of the maximum weight
    PFIsimdvf maxWeight = pfiSimdMax_F32(w[0], pfiSimdMax_F32(w[1], w[2]));
    
    // Compare maxWeight to each weight to find which one is the max
    PFIsimdvf mask1 = pfiSimdCmpEQ_F32(maxWeight, w[0]);
    PFIsimdvf mask2 = pfiSimdCmpEQ_F32(maxWeight, w[1]);
    PFIsimdvf mask3 = pfiSimdCmpEQ_F32(maxWeight, w[2]);

    // Use masks to select the corresponding color
    for (int_fast8_t i = 0; i < 4; ++i) {
        dst[i] = pfiSimdOr_F32(
            pfiSimdAnd_F32(mask1, v1[i]),
            pfiSimdOr_F32(
                pfiSimdAnd_F32(mask2, v2[i]),
                pfiSimdAnd_F32(mask3, v3[i])
            )
        );
    }
}

static inline void
pfiVec4BaryInterpFlatVR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv2f v1, const PFIsimdv2f v2, const PFIsimdv2f v3, const PFIsimdv3f w)
{
    // Compare the weights to find the index of the maximum weight
    PFIsimdvf maxWeight = pfiSimdMax_F32(w[0], pfiSimdMax_F32(w[1], w[2]));
    
    // Compare maxWeight to each weight to find which one is the max
    PFIsimdvf mask1 = pfiSimdCmpEQ_F32(maxWeight, w[0]);
    PFIsimdvf mask2 = pfiSimdCmpEQ_F32(maxWeight, w[1]);
    PFIsimdvf mask3 = pfiSimdCmpEQ_F32(maxWeight, w[2]);

    // Use masks to select the corresponding color
    for (int_fast8_t i = 0; i < 4; ++i) {
        dst[i] = pfiSimdOr_F32(
            pfiSimdAnd_F32(mask1, v1[i]),
            pfiSimdOr_F32(
                pfiSimdAnd_F32(mask2, v2[i]),
                pfiSimdAnd_F32(mask3, v3[i])
            )
        );
    }
}

static inline void
pfiVec4Transform_simd(PFIsimdv4f dst, const PFIsimdv4f v, const float mat[16])
{
    PFIsimdvf mat_col0 = pfiSimdSet1_F32(mat[0]);
    PFIsimdvf mat_col1 = pfiSimdSet1_F32(mat[1]);
    PFIsimdvf mat_col2 = pfiSimdSet1_F32(mat[2]);
    PFIsimdvf mat_col3 = pfiSimdSet1_F32(mat[3]);
    PFIsimdvf mat_col4 = pfiSimdSet1_F32(mat[4]);
    PFIsimdvf mat_col5 = pfiSimdSet1_F32(mat[5]);
    PFIsimdvf mat_col6 = pfiSimdSet1_F32(mat[6]);
    PFIsimdvf mat_col7 = pfiSimdSet1_F32(mat[7]);
    PFIsimdvf mat_col8 = pfiSimdSet1_F32(mat[8]);
    PFIsimdvf mat_col9 = pfiSimdSet1_F32(mat[9]);
    PFIsimdvf mat_col10 = pfiSimdSet1_F32(mat[10]);
    PFIsimdvf mat_col11 = pfiSimdSet1_F32(mat[11]);
    PFIsimdvf mat_col12 = pfiSimdSet1_F32(mat[12]);
    PFIsimdvf mat_col13 = pfiSimdSet1_F32(mat[13]);
    PFIsimdvf mat_col14 = pfiSimdSet1_F32(mat[14]);
    PFIsimdvf mat_col15 = pfiSimdSet1_F32(mat[15]);

    PFIsimdvf tmp0 = pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col0, v[0]), pfiSimdMul_F32(mat_col4, v[1])),
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col8, v[2]), pfiSimdMul_F32(mat_col12, v[3])));

    PFIsimdvf tmp1 = pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col1, v[0]), pfiSimdMul_F32(mat_col5, v[1])),
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col9, v[2]), pfiSimdMul_F32(mat_col13, v[3])));

    PFIsimdvf tmp2 = pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col2, v[0]), pfiSimdMul_F32(mat_col6, v[1])),
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col10, v[2]), pfiSimdMul_F32(mat_col14, v[3])));

    PFIsimdvf tmp3 = pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col3, v[0]), pfiSimdMul_F32(mat_col7, v[1])),
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col11, v[2]), pfiSimdMul_F32(mat_col15, v[3])));

    dst[0] = tmp0;
    dst[1] = tmp1;
    dst[2] = tmp2;
    dst[3] = tmp3;
}

static inline void
pfiVec4TransformR_simd(PFIsimdvf_ptr restrict dst, const PFIsimdv4f v, const float mat[16])
{
    PFIsimdvf mat_col0 = pfiSimdSet1_F32(mat[0]);
    PFIsimdvf mat_col1 = pfiSimdSet1_F32(mat[1]);
    PFIsimdvf mat_col2 = pfiSimdSet1_F32(mat[2]);
    PFIsimdvf mat_col3 = pfiSimdSet1_F32(mat[3]);
    PFIsimdvf mat_col4 = pfiSimdSet1_F32(mat[4]);
    PFIsimdvf mat_col5 = pfiSimdSet1_F32(mat[5]);
    PFIsimdvf mat_col6 = pfiSimdSet1_F32(mat[6]);
    PFIsimdvf mat_col7 = pfiSimdSet1_F32(mat[7]);
    PFIsimdvf mat_col8 = pfiSimdSet1_F32(mat[8]);
    PFIsimdvf mat_col9 = pfiSimdSet1_F32(mat[9]);
    PFIsimdvf mat_col10 = pfiSimdSet1_F32(mat[10]);
    PFIsimdvf mat_col11 = pfiSimdSet1_F32(mat[11]);
    PFIsimdvf mat_col12 = pfiSimdSet1_F32(mat[12]);
    PFIsimdvf mat_col13 = pfiSimdSet1_F32(mat[13]);
    PFIsimdvf mat_col14 = pfiSimdSet1_F32(mat[14]);
    PFIsimdvf mat_col15 = pfiSimdSet1_F32(mat[15]);

    dst[0] = pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col0, v[0]), pfiSimdMul_F32(mat_col4, v[1])),
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col8, v[2]), pfiSimdMul_F32(mat_col12, v[3])));

    dst[1] = pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col1, v[0]), pfiSimdMul_F32(mat_col5, v[1])),
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col9, v[2]), pfiSimdMul_F32(mat_col13, v[3])));

    dst[2] = pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col2, v[0]), pfiSimdMul_F32(mat_col6, v[1])),
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col10, v[2]), pfiSimdMul_F32(mat_col14, v[3])));

    dst[3] = pfiSimdAdd_F32(
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col3, v[0]), pfiSimdMul_F32(mat_col7, v[1])),
            pfiSimdAdd_F32(pfiSimdMul_F32(mat_col11, v[2]), pfiSimdMul_F32(mat_col15, v[3])));
}

#endif //PF_SIMD_SUPPORT
#endif //PF_INTERNAL_SIMD_H
