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

#ifndef PFSIMD_H
#define PFSIMD_H

#include <stdint.h>
#include <stddef.h>

#if defined(__AVX2__)
#include <immintrin.h>
#define PF_SIMD_SIZE 8
typedef __m256 PFsimdf;
typedef __m256i PFsimdi;
#elif defined(__SSE2__)
#include <immintrin.h>
#define PF_SIMD_SIZE 4
typedef __m128 PFsimdf;
typedef __m128i PFsimdi;
#else
#define PF_SIMD_SIZE 1
typedef float PFsimdf;
typedef int32_t PFsimdi;
#endif

#ifdef __SSE2__

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

static inline __m128i
_mm_blendv_epi8_sse2(__m128i x, __m128i y, __m128i mask)
{
    __m128i not_mask = _mm_andnot_si128(mask, x);   // _mm_andnot_si128(mask, x) : bits of x where mask is 0
    __m128i masked_y = _mm_and_si128(mask, y);      // _mm_and_si128(mask, y) : bits of y where mask is 1
    return _mm_or_si128(not_mask, masked_y);        // Combine the two results to get the final result
}

#endif

static inline PFsimdf
pfSimdSetOne_F32(float x)
{
#if defined(__AVX2__)
    return _mm256_set1_ps(x);
#elif defined(__SSE2__)
    return _mm_set1_ps(x);
#else
    return x;
#endif
}

static inline PFsimdi
pfSimdSetOne_I32(int32_t x)
{
#if defined(__AVX2__)
    return _mm256_set1_epi32(x);
#elif defined(__SSE2__)
    return _mm_set1_epi32(x);
#else
    return x;
#endif
}

static inline PFsimdi
pfSimdSetR_I32(int32_t i0, int32_t i1, int32_t i2, int32_t i3, int32_t i4, int32_t i5, int32_t i6, int32_t i7)
{
#if defined(__AVX2__)
    return _mm256_setr_epi32(i0, i1, i2, i3, i4, i5, i6, i7);
#elif defined(__SSE2__)
    (void)i4, (void)i5, (void)i6, (void)i7;
    return _mm_setr_epi32(i0, i1, i2, i3);
#else
    (void)i1, (void)i2, (void)i3, (void)i4, (void)i5, (void)i6, (void)i7;
    return i0; // For scalar fallback, just return the first value
#endif
}

static inline PFsimdi
pfSimdSetZero_I32(void)
{
#if defined(__AVX2__)
    return _mm256_setzero_si256();
#elif defined(__SSE2__)
    return _mm_setzero_si128();
#else
    return 0;
#endif
}

static inline PFsimdf
pfSimdSetZero_F32(void)
{
#if defined(__AVX2__)
    return _mm256_setzero_ps();
#elif defined(__SSE2__)
    return _mm_setzero_ps();
#else
    return 0.0f;
#endif
}

static inline PFsimdi
pfSimdAdd_I32(PFsimdi x, PFsimdi y)
{
#if defined(__AVX2__)
    return _mm256_add_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_add_epi32(x, y);
#else
    return x + y;
#endif
}

static inline PFsimdi
pfSimdMullo_I32(PFsimdi x, PFsimdi y)
{
#if defined(__AVX2__)
    return _mm256_mullo_epi32(x, y);
#elif defined(__SSE4_1__)
    return _mm_mullo_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_mullo_epi32_sse2(x, y);
#else
    return x * y;
#endif
}

static inline PFsimdf
pfSimdCVT_I32_F32(PFsimdi x)
{
#if defined(__AVX2__)
    return _mm256_cvtepi32_ps(x);
#elif defined(__SSE2__)
    return _mm_cvtepi32_ps(x);
#else
    return (PFsimdf)x;
#endif
}

static inline PFsimdf
pfSimdMul_F32(PFsimdf x, PFsimdf y)
{
#if defined(__AVX2__)
    return _mm256_mul_ps(x, y);
#elif defined(__SSE2__)
    return _mm_mul_ps(x, y);
#else
    return x * y;
#endif
}

static inline PFsimdi
pfSimdCVT_F32_I32(PFsimdf x)
{
#if defined(__AVX2__)
    return _mm256_cvtps_epi32(x);
#elif defined(__SSE2__)
    return _mm_cvtps_epi32(x);
#else
    return (PFsimdi)x;
#endif
}

static inline PFsimdf
pfSimdRCP_F32(PFsimdf x)
{
#if defined(__AVX2__)
    return _mm256_rcp_ps(x);
#elif defined(__SSE2__)
    return _mm_rcp_ps(x);
#else
    return 1.0f / x;
#endif
}

static inline void
pfSimdStore_F32(void* p, PFsimdf x)
{
#if defined(__AVX2__)
    _mm256_storeu_ps((float*)p, x);
#elif defined(__SSE2__)
    _mm_storeu_ps((float*)p, x);
#else
    *(float*)p = x;
#endif
}

static inline void
pfSimdStore_I32(void* p, PFsimdi x)
{
#if defined(__AVX2__)
    _mm256_storeu_si256((__m256i*)p, x);
#elif defined(__SSE2__)
    _mm_storeu_si128((__m128i*)p, x);
#else
    *(int32_t*)p = x;
#endif
}

static inline void
pfSimdStore_SI32(void* p, PFsimdi x)
{
#if defined(__AVX2__)
    _mm_storeu_si32((int32_t*)p, _mm256_castsi256_si128(x));
#elif defined(__SSE2__)
    _mm_storeu_si32((int32_t*)p, x);
#else
    *(int32_t*)p = (int32_t)x;
#endif
}

static inline PFsimdi
pfSimdLoad_I32(const void* p)
{
#if defined(__AVX2__)
    return _mm256_loadu_si256((const __m256i*)p);
#elif defined(__SSE2__)
    return _mm_loadu_si128((const __m128i*)p);
#else
    return *(const int32_t*)p;
#endif
}

#if defined(__AVX2__)
#   define pfSimdExtract_I32(v, index)  \
        _mm256_extract_epi32(x, index)
#elif defined(__SSE2__)
#   define pfSimdExtract_I32(v, index)  \
        _mm_extract_epi32(x, index)
#else
#   define pfSimdExtract_I32(v, index)  \
        (v)
#endif

static inline int32_t
pfSimdExtractVarIdx_I32(PFsimdi x, int32_t index)
{
#if defined(__AVX2__)
    __m128i idx = _mm_cvtsi32_si128(index);
    __m256i val = _mm256_permutevar8x32_epi32(x, _mm256_castsi128_si256(idx));
    return _mm_cvtsi128_si32(_mm256_castsi256_si128(val));
#elif defined(__SSE2__)
    union v_u { __m128i vec; int arr[4]; };
    union v_u v = { .vec = x };
    return v.arr[index];
#else
    (void)index;
    return x; // Scalar fallback does not support extraction
#endif
}

static inline PFsimdi
pfSimdPermute_I32(PFsimdi x, PFsimdi y)
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
#else
    (void)y;
    return x;
#endif
}

static inline PFsimdi
pfSimdSRLI_I32(PFsimdi x, int32_t imm8)
{
#if defined(__AVX2__)
    return _mm256_srli_epi32(x, imm8);
#elif defined(__SSE2__)
    return _mm_srli_epi32(x, imm8);
#else
    return x >> imm8;
#endif
}

static inline PFsimdi
pfSimdAnd_I32(PFsimdi x, PFsimdi y)
{
#if defined(__AVX2__)
    return _mm256_and_si256(x, y);
#elif defined(__SSE2__)
    return _mm_and_si128(x, y);
#else
    return x & y;
#endif
}

static inline PFsimdi
pfSimdOr_I32(PFsimdi x, PFsimdi y)
{
#if defined(__AVX2__)
    return _mm256_or_si256(x, y);
#elif defined(__SSE2__)
    return _mm_or_si128(x, y);
#else
    return x | y;
#endif
}

static inline PFsimdi
pfSimdSLLI_I32(PFsimdi x, int32_t imm8)
{
#if defined(__AVX2__)
    return _mm256_slli_epi32(x, imm8);
#elif defined(__SSE2__)
    return _mm_slli_epi32(x, imm8);
#else
    return x << imm8;
#endif
}

static inline int32_t
pfSimdMoveMask_F32(PFsimdf x)
{
#if defined(__AVX2__)
    return _mm256_movemask_ps(x);
#elif defined(__SSE2__)
    return _mm_movemask_ps(x);
#else
    return x != 0.0f;
#endif
}

static inline int32_t
pfSimdMoveMask_I8(PFsimdi x)
{
#if defined(__AVX2__)
    return _mm256_movemask_epi8(x);
#elif defined(__SSE2__)
    return _mm_movemask_epi8(x);
#else
    return x != 0;
#endif
}

static inline PFsimdi
pfSimdBlendV_I8(PFsimdi a, PFsimdi b, PFsimdi mask)
{
#if defined(__AVX2__)
    return _mm256_blendv_epi8(a, b, mask);
#elif defined(__SSE4_1__)
    return _mm_blendv_epi8(a, b, mask);
#elif defined(__SSE2__)
    return _mm_blendv_epi8_sse2(a, b, mask);
#else
    return (mask ? b : a);
#endif
}

static inline PFsimdi
pfSimdCmpEQ_I32(PFsimdi x, PFsimdi y)
{
#if defined(__AVX2__)
    return _mm256_cmpeq_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_cmpeq_epi32(x, y);
#else
    return x == y;
#endif
}

static inline PFsimdi
pfSimdCmpLT_I32(PFsimdi x, PFsimdi y)
{
#if defined(__AVX2__)
    return _mm256_cmpgt_epi32(y, x);
#elif defined(__SSE2__)
    return _mm_cmplt_epi32(x, y);
#else
    return x < y;
#endif
}

static inline PFsimdi
pfSimdCmpGT_I32(PFsimdi x, PFsimdi y)
{
#if defined(__AVX2__)
    return _mm256_cmpgt_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_cmpgt_epi32(x, y);
#else
    return x > y;
#endif
}

static inline PFsimdi
pfSimdCmpLE_I32(PFsimdi x, PFsimdi y)
{
#if defined(__AVX2__)
    return _mm256_cmpgt_epi32(y, x);
#elif defined(__SSE2__)
    return _mm_cmplt_epi32(x, y);
#else
    return x <= y;
#endif
}

static inline PFsimdi
pfSimdCmpGE_I32(PFsimdi x, PFsimdi y)
{
#if defined(__AVX2__)
    return _mm256_cmpgt_epi32(y, x);
#elif defined(__SSE2__)
    return _mm_cmpgt_epi32(y, x);
#else
    return x >= y;
#endif
}

#endif //PFSIMD_H
