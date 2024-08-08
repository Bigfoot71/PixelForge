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

#ifndef PFM_H
#define PFM_H

#include <stdint.h>
#include <string.h>
#include <math.h>

#if defined(__AVX2__)
#   include <immintrin.h>
#   define PF_SIMD_SIZE 8
#elif defined(__SSE2__)
#   include <immintrin.h>
#   define PF_SIMD_SIZE 4
#else
#   define PF_SIMD_SIZE 1
#endif

/* Defines and Macros */

#ifndef PFM_API
#   define PFM_API static inline
#endif //PFM_API

#ifndef M_PI
#   define M_PI 3.14159265358979323846
#endif //M_PI

#ifndef DEG2RAD
#   define DEG2RAD(deg) ((deg) * M_PI / 180.0)
#endif //DEG2RAD

#ifndef RAD2DEG
#   define RAD2DEG(deg) ((deg) * 180.0 / M_PI)
#endif //RAD2DEG

#ifndef PFM_FISR
#   define rsqrtf(x) (1.0f/sqrtf(x))
#else
// NOTE: More useful on older platforms.
// SEE: http://www.lomont.org/papers/2003/InvSqrt.pdf
PFM_API float rsqrtf(float x)
{
    float xhalf = 0.5f*x;
    int i = *(int*)&x;              // get bits for floating value
    i = 0x5f375a86 - (i >> 1);      // gives initial guess y0
    x = *(float*)&i;                // convert bits back to float
    x = x*(1.5f - xhalf*x*x);       // Newton step, repeating increases accuracy
    return x;
}
#endif //PFM_FISR

/* SISD types definitions */

typedef float PFMvec2[2];
typedef float PFMvec3[3];
typedef float PFMvec4[4];
typedef float PFMmat4[16];

typedef float* aPFMvec2;
typedef float* aPFMvec3;
typedef float* aPFMvec4;
typedef float* aPFMmat4;

/* SIMD types definitions */

#if defined(__AVX2__)
typedef __m256 PFMsimd_f;
typedef __m256i PFMsimd_i;
#elif defined(__SSE2__)
typedef __m128 PFMsimd_f;
typedef __m128i PFMsimd_i;
#else
typedef double PFMsimd_f;
typedef int64_t PFMsimd_i;
#endif

typedef PFMsimd_f PFMsimd_vec2[2];
typedef PFMsimd_f PFMsimd_vec3[3];
typedef PFMsimd_f PFMsimd_vec4[4];

typedef PFMsimd_f* aPFMsimd_vec2;
typedef PFMsimd_f* aPFMsimd_vec3;
typedef PFMsimd_f* aPFMsimd_vec4;

/* 2D Vector function definitions */

PFM_API void
pfmVec2Zero(PFMvec2 dst)
{
    memset(dst, 0, sizeof(PFMvec2));
}

PFM_API void
pfmVec2One(PFMvec2 dst, float v)
{
    dst[0] = dst[1] = v;
}

PFM_API void
pfmVec2Set(PFMvec2 dst, float x, float y)
{
    dst[0] = x, dst[1] = y;
}

PFM_API void
pfmVec2Copy(aPFMvec2 restrict dst, const aPFMvec2 restrict src)
{
    memcpy(dst, src, sizeof(PFMvec2));
}

PFM_API void
pfmVec2Swap(aPFMvec2 restrict a, aPFMvec2 restrict b)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        float tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

PFM_API void
pfmVec2Neg(PFMvec2 dst, const PFMvec2 v)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = -v[i];
    }
}

PFM_API void
pfmVec2NegR(aPFMvec2 restrict dst, const PFMvec2 v)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = -v[i];
    }
}

PFM_API void
pfmVec2Add(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void
pfmVec2AddR(aPFMvec2 restrict dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void
pfmVec2Sub(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void
pfmVec2SubR(aPFMvec2 restrict dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void
pfmVec2Mul(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void
pfmVec2MulR(aPFMvec2 restrict dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void
pfmVec2Div(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void
pfmVec2DivR(aPFMvec2 restrict dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void
pfmVec2Offset(PFMvec2 dst, const PFMvec2 v, float scalar)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v[i] + scalar;
    }
}

PFM_API void
pfmVec2OffsetR(aPFMvec2 restrict dst, const PFMvec2 v, float scalar)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v[i] + scalar;
    }
}

PFM_API void
pfmVec2Scale(PFMvec2 dst, const PFMvec2 v, float scalar)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void
pfmVec2ScaleR(aPFMvec2 restrict dst, const PFMvec2 v, float scalar)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void
pfmVec2Normalize(PFMvec2 dst, const PFMvec2 v)
{
    float squaredLength = v[0]*v[0] + v[1]*v[1];
    if (squaredLength == 0.0f) return;

    float invLength = rsqrtf(squaredLength);
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v[i] * invLength;
    }
}

PFM_API void
pfmVec2NormalizeR(aPFMvec2 restrict dst, const PFMvec2 v)
{
    float squaredLength = v[0]*v[0] + v[1]*v[1];
    if (squaredLength == 0.0f) return;

    float invLength = rsqrtf(squaredLength);
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v[i] * invLength;
    }
}

PFM_API float
pfmVec2Length(const PFMvec2 v)
{
    return sqrtf(v[0]*v[0] + v[1]*v[1]);
}

PFM_API float
pfmVec2LengthSq(const PFMvec2 v)
{
    return v[0]*v[0] + v[1]*v[1];
}

PFM_API float
pfmVec2Dot(const PFMvec2 v1, const PFMvec2 v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1];
}

PFM_API float
pfmVec2Distance(const PFMvec2 v1, const PFMvec2 v2)
{
    PFMvec2 dt = {
        v1[0] - v2[0],
        v1[1] - v2[1]
    };

    float distanceSq = dt[0]*dt[0] +
                           dt[1]*dt[1];

#ifdef PFM_FISR
    return distanceSq*rsqrtf(distanceSq);
#else
    return sqrtf(distanceSq);
#endif
}

PFM_API float
pfmVec2DistanceSq(const PFMvec2 v1, const PFMvec2 v2)
{
    PFMvec2 dt = { v1[0] - v2[0], v1[1] - v2[1] };
    return dt[0]*dt[0] + dt[1]*dt[1];
}

PFM_API void
pfmVec2Direction(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2)
{
    PFMvec2 tmp;
    float lengthSq = 0.0f;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 2; i++)
    {
        tmp[i] = v1[i] - v2[i];
        lengthSq += tmp[i]*tmp[i];
    }

    float invLength = rsqrtf(lengthSq);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = tmp[i]*invLength;
    }
}

PFM_API void
pfmVec2DirectionR(aPFMvec2 restrict dst, const PFMvec2 v1, const PFMvec2 v2)
{
    float lengthSq = 0.0f;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i] - v2[i];
        lengthSq += dst[i]*dst[i];
    }

    float invLength = rsqrtf(lengthSq);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = dst[i]*invLength;
    }
}

PFM_API void
pfmVec2Lerp(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2, float t)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i] + t*(v2[i]-v1[i]);
    }
}

PFM_API void
pfmVec2LerpR(aPFMvec2 restrict dst, const PFMvec2 v1, const PFMvec2 v2, float t)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i] + t*(v2[i]-v1[i]);
    }
}

PFM_API void
pfmVec2BaryInterp(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2, const PFMvec2 v3, float w1, float w2, float w3)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

PFM_API void
pfmVec2BaryInterpR(aPFMvec2 restrict dst, const PFMvec2 v1, const PFMvec2 v2, const PFMvec2 v3, float w1, float w2, float w3)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

PFM_API void
pfmVec2BaryInterpV(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2, const PFMvec2 v3, const PFMvec3 w)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = w[0]*v1[i] + w[1]*v2[i] + w[2]*v3[i];
    }
}

PFM_API void
pfmVec2BaryInterpVR(aPFMvec2 restrict dst, const PFMvec2 v1, const PFMvec2 v2, const PFMvec2 v3, const PFMvec3 w)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = w[0]*v1[i] + w[1]*v2[i] + w[2]*v3[i];
    }
}

PFM_API void
pfmVec2Transform(PFMvec2 dst, const PFMvec2 v, const PFMmat4 mat)
{
    PFMvec2 tmp = {
        mat[0]*v[0] + mat[4]*v[1] + mat[12],
        mat[1]*v[0] + mat[5]*v[1] + mat[13]
    };

    memcpy(dst, tmp, sizeof(PFMvec2));
}

PFM_API void
pfmVec2TransformR(aPFMvec2 restrict dst, const PFMvec2 v, const PFMmat4 mat)
{
    dst[0] = mat[0]*v[0] + mat[4]*v[1] + mat[12];
    dst[1] = mat[1]*v[0] + mat[5]*v[1] + mat[13];
}

PFM_API void
pfmVec2TransformWT(PFMvec2 dst, const PFMvec2 v, float wTranslation, const PFMmat4 mat)
{
    PFMvec2 tmp = {
        mat[0]*v[0] + mat[4]*v[1] + wTranslation*mat[12],
        mat[1]*v[0] + mat[5]*v[1] + wTranslation*mat[13]
    };

    memcpy(dst, tmp, sizeof(PFMvec2));
}

PFM_API void
pfmVec2TransformWTR(aPFMvec2 restrict dst, const PFMvec2 v, float wTranslation, const PFMmat4 mat)
{
    dst[0] = mat[0]*v[0] + mat[4]*v[1] + wTranslation*mat[12];
    dst[1] = mat[1]*v[0] + mat[5]*v[1] + wTranslation*mat[13];
}

/* 3D Vector function definitions */

PFM_API void
pfmVec3Zero(PFMvec3 dst)
{
    memset(dst, 0, sizeof(PFMvec3));
}

PFM_API void
pfmVec3One(PFMvec3 dst, float v)
{
    dst[0] = dst[1] = dst[2] = v;
}

PFM_API void
pfmVec3Set(PFMvec3 dst, float x, float y, float z)
{
    dst[0] = x, dst[1] = y, dst[2] = z;
}

PFM_API void
pfmVec3Copy(aPFMvec3 restrict dst, const aPFMvec3 restrict src)
{
    memcpy(dst, src, sizeof(PFMvec3));
}

PFM_API void
pfmVec3Swap(aPFMvec3 restrict a, aPFMvec3 restrict b)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        float tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

PFM_API void
pfmVec3Neg(PFMvec3 dst, const PFMvec3 v)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = -v[i];
    }
}

PFM_API void
pfmVec3NegR(aPFMvec3 restrict dst, const PFMvec3 v)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = -v[i];
    }
}

PFM_API void
pfmVec3Add(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void
pfmVec3AddR(aPFMvec3 restrict dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void
pfmVec3Sub(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void
pfmVec3SubR(aPFMvec3 restrict dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void
pfmVec3Mul(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void
pfmVec3MulR(aPFMvec3 restrict dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void
pfmVec3Div(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void
pfmVec3DivR(aPFMvec3 restrict dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void
pfmVec3Offset(PFMvec3 dst, const PFMvec3 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v[i] + scalar;
    }
}

PFM_API void
pfmVec3OffsetR(aPFMvec3 restrict dst, const PFMvec3 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v[i] + scalar;
    }
}

PFM_API void
pfmVec3Scale(PFMvec3 dst, const PFMvec3 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void
pfmVec3ScaleR(aPFMvec3 restrict dst, const PFMvec3 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void
pfmVec3Normalize(PFMvec3 dst, const PFMvec3 v)
{
    float squaredLength = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    if (squaredLength == 0.0f) return;

    float invLength = rsqrtf(squaredLength);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v[i] * invLength;
    }
}

PFM_API void
pfmVec3NormalizeR(aPFMvec3 restrict dst, const PFMvec3 v)
{
    float squaredLength = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    if (squaredLength == 0.0f) return;

    float invLength = rsqrtf(squaredLength);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v[i] * invLength;
    }
}

PFM_API float
pfmVec3Length(const PFMvec3 v)
{
    return sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

PFM_API float
pfmVec3LengthSq(const PFMvec3 v)
{
    return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

PFM_API float
pfmVec3Dot(const PFMvec3 v1, const PFMvec3 v2)
{
#ifdef _OPENMP
    float dotProduct = 0.0f;
#   pragma omp simd
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dotProduct += v1[i]*v2[i];
    }
    return dotProduct;
#else
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
#endif
}

PFM_API void
pfmVec3Cross(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
    PFMvec3 tmp = {
        v1[1]*v2[2] - v1[2]*v2[1],
        v1[2]*v2[0] - v1[0]*v2[2],
        v1[0]*v2[1] - v1[1]*v2[0]
    };

    memcpy(dst, tmp, sizeof(PFMvec3));
}

PFM_API void
pfmVec3CrossR(aPFMvec3 restrict dst, const PFMvec3 v1, const PFMvec3 v2)
{
    dst[0] = v1[1]*v2[2] - v1[2]*v2[1];
    dst[1] = v1[2]*v2[0] - v1[0]*v2[2];
    dst[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

PFM_API float
pfmVec3Distance(const PFMvec3 v1, const PFMvec3 v2)
{
    // NOTE: Here, the loop version adds a conditional branch, in doubt we keep it simple.

    PFMvec3 dt = {
        v1[0] - v2[0],
        v1[1] - v2[1],
        v1[2] - v2[2]
    };

    float distanceSq = dt[0]*dt[0] +
                           dt[1]*dt[1] +
                           dt[2]*dt[2];

#ifdef PFM_FISR
    return distanceSq*rsqrtf(distanceSq);
#else
    return sqrtf(distanceSq);
#endif
}

PFM_API float
pfmVec3DistanceSq(const PFMvec3 v1, const PFMvec3 v2)
{
    // NOTE 1: The code generated by GCC 11/13 in O3 utilizes SIMD operations more efficiently than the non-loop version
    // NOTE 2: Still with GCC 13 in O3, the code generated with 'omp simd' is the same as without, but on GCC versions lower than 11.1 the code generated with 'omp simd' retains the loop...

    float distanceSq = 0.0f;
    for (int_fast8_t i = 0; i < 3; i++)
    {
        float dt = v1[i] - v2[i];
        distanceSq += dt*dt;
    }
    return distanceSq;
}

PFM_API void
pfmVec3Direction(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
    PFMvec3 tmp;
    float lengthSq = 0.0f;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        tmp[i] = v1[i] - v2[i];
        lengthSq += tmp[i]*tmp[i];
    }

    float invLength = rsqrtf(lengthSq);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = tmp[i]*invLength;
    }
}

PFM_API void
pfmVec3DirectionR(aPFMvec3 restrict dst, const PFMvec3 v1, const PFMvec3 v2)
{
    float lengthSq = 0.0f;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i] - v2[i];
        lengthSq += dst[i]*dst[i];
    }

    float invLength = rsqrtf(lengthSq);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = dst[i]*invLength;
    }
}

PFM_API void
pfmVec3Lerp(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2, float t)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i] + t*(v2[i]-v1[i]);
    }
}

PFM_API void
pfmVec3LerpR(aPFMvec3 restrict dst, const PFMvec3 v1, const PFMvec3 v2, float t)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i] + t*(v2[i]-v1[i]);
    }
}

PFM_API void
pfmVec3BaryInterp(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2, const PFMvec3 v3, float w1, float w2, float w3)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

PFM_API void
pfmVec3BaryInterpR(aPFMvec3 restrict dst, const PFMvec3 v1, const PFMvec3 v2, const PFMvec3 v3, float w1, float w2, float w3)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

PFM_API void
pfmVec3BaryInterpV(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2, const PFMvec3 v3, const PFMvec3 w)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = w[0]*v1[i] + w[1]*v2[i] + w[2]*v3[i];
    }
}

PFM_API void
pfmVec3BaryInterpVR(aPFMvec3 restrict dst, const PFMvec3 v1, const PFMvec3 v2, const PFMvec3 v3, const PFMvec3 w)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = w[0]*v1[i] + w[1]*v2[i] + w[2]*v3[i];
    }
}

PFM_API void
pfmVec3Transform(PFMvec3 dst, const PFMvec3 v, const PFMmat4 mat)
{
    PFMvec3 tmp = {
        mat[0]*v[0] + mat[4]*v[1] + mat[8]*v[2] + mat[12],
        mat[1]*v[0] + mat[5]*v[1] + mat[9]*v[2] + mat[13],
        mat[2]*v[0] + mat[6]*v[1] + mat[10]*v[2] + mat[14]
    };

    memcpy(dst, tmp, sizeof(PFMvec3));
}

PFM_API void
pfmVec3TransformR(aPFMvec3 restrict dst, const PFMvec3 v, const PFMmat4 mat)
{
    dst[0] = mat[0]*v[0] + mat[4]*v[1] + mat[8]*v[2] + mat[12];
    dst[1] = mat[1]*v[0] + mat[5]*v[1] + mat[9]*v[2] + mat[13];
    dst[2] = mat[2]*v[0] + mat[6]*v[1] + mat[10]*v[2] + mat[14];
}

PFM_API void
pfmVec3TransformWT(PFMvec3 dst, const PFMvec3 v, float wTranslation, const PFMmat4 mat)
{
    PFMvec3 tmp = {
        mat[0]*v[0] + mat[4]*v[1] + mat[8]*v[2] + wTranslation*mat[12],
        mat[1]*v[0] + mat[5]*v[1] + mat[9]*v[2] + wTranslation*mat[13],
        mat[2]*v[0] + mat[6]*v[1] + mat[10]*v[2] + wTranslation*mat[14]
    };

    memcpy(dst, tmp, sizeof(PFMvec3));
}

PFM_API void
pfmVec3TransformWTR(aPFMvec3 restrict dst, const PFMvec3 v, float wTranslation, const PFMmat4 mat)
{
    dst[0] = mat[0]*v[0] + mat[4]*v[1] + mat[8]*v[2] + wTranslation*mat[12];
    dst[1] = mat[1]*v[0] + mat[5]*v[1] + mat[9]*v[2] + wTranslation*mat[13];
    dst[2] = mat[2]*v[0] + mat[6]*v[1] + mat[10]*v[2] + wTranslation*mat[14];
}

PFM_API void
pfmVec3Reflect(PFMvec3 dst, const PFMvec3 incident, const PFMvec3 normal)
{
    float dotProduct = 0.0f;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dotProduct += incident[i]*normal[i];
    }

    dotProduct *= 2.0f;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = incident[i] - dotProduct*normal[i];
    }
}

PFM_API void
pfmVec3ReflectR(aPFMvec3 restrict dst, const PFMvec3 incident, const PFMvec3 normal)
{
    float dotProduct = 0.0f;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dotProduct += incident[i]*normal[i];
    }

    dotProduct *= 2.0f;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = incident[i] - dotProduct*normal[i];
    }
}

/* 4D Vector function definitions */

PFM_API void
pfmVec4Zero(PFMvec4 dst)
{
    memset(dst, 0, sizeof(PFMvec4));
}

PFM_API void
pfmVec4One(PFMvec4 dst, float v)
{
    dst[0] = dst[1] = dst[2] = dst[3] = v;
}

PFM_API void
pfmVec4Set(PFMvec4 dst, float x, float y, float z, float w)
{
    dst[0] = x, dst[1] = y, dst[2] = z, dst[3] = w;
}

PFM_API void
pfmVec4Copy(aPFMvec4 restrict dst, const aPFMvec4 restrict src)
{
    memcpy(dst, src, sizeof(PFMvec4));
}

PFM_API void
pfmVec4Swap(aPFMvec4 restrict a, aPFMvec4 restrict b)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        float tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

PFM_API void
pfmVec4Neg(PFMvec4 dst, const PFMvec4 v)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = -v[i];
    }
}

PFM_API void
pfmVec4NegR(aPFMvec4 restrict dst, const PFMvec4 v)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = -v[i];
    }
}

PFM_API void
pfmVec4Add(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void
pfmVec4AddR(aPFMvec4 restrict dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void
pfmVec4Sub(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void
pfmVec4SubR(aPFMvec4 restrict dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void
pfmVec4Mul(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void
pfmVec4MulR(aPFMvec4 restrict dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void
pfmVec4Div(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void
pfmVec4DivR(aPFMvec4 restrict dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void
pfmVec4Offset(PFMvec4 dst, const PFMvec4 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v[i] + scalar;
    }
}

PFM_API void
pfmVec4OffsetR(aPFMvec4 restrict dst, const PFMvec4 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v[i] + scalar;
    }
}

PFM_API void
pfmVec4Scale(PFMvec4 dst, const PFMvec4 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void
pfmVec4ScaleR(aPFMvec4 restrict dst, const PFMvec4 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void
pfmVec4Normalize(PFMvec4 dst, const PFMvec4 v)
{
    float squaredLength = v[0]*v[0] + v[1]*v[1] + v[2]*v[2] + v[3]*v[3];
    if (squaredLength == 0.0f) return;

    float invLength = rsqrtf(squaredLength);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v[i] * invLength;
    }
}

PFM_API void
pfmVec4NormalizeR(aPFMvec4 restrict dst, const PFMvec4 v)
{
    float squaredLength = v[0]*v[0] + v[1]*v[1] + v[2]*v[2] + v[3]*v[3];
    if (squaredLength == 0.0f) return;

    float invLength = rsqrtf(squaredLength);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v[i] * invLength;
    }
}

PFM_API float
pfmVec4Length(const PFMvec4 v)
{
    return sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2] + v[3]*v[3]);
}

PFM_API float
pfmVec4LengthSq(const PFMvec4 v)
{
    return v[0]*v[0] + v[1]*v[1] + v[2]*v[2] + v[3]*v[3];
}

PFM_API float
pfmVec4Dot(const PFMvec4 v1, const PFMvec4 v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2] + v1[3]*v2[3];
}

PFM_API void
pfmVec4Lerp(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2, float t)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i] + t*(v2[i]-v1[i]);
    }
}

PFM_API void
pfmVec4LerpR(aPFMvec4 restrict dst, const PFMvec4 v1, const PFMvec4 v2, float t)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i] + t*(v2[i]-v1[i]);
    }
}

PFM_API void
pfmVec4BaryInterp(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2, const PFMvec4 v3, float w1, float w2, float w3)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

PFM_API void
pfmVec4BaryInterpR(aPFMvec4 restrict dst, const PFMvec4 v1, const PFMvec4 v2, const PFMvec4 v3, float w1, float w2, float w3)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

PFM_API void
pfmVec4BaryInterpV(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2, const PFMvec4 v3, const PFMvec3 w)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = w[0]*v1[i] + w[1]*v2[i] + w[2]*v3[i];
    }
}

PFM_API void
pfmVec4BaryInterpVR(aPFMvec4 restrict dst, const PFMvec4 v1, const PFMvec4 v2, const PFMvec4 v3, const PFMvec3 w)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = w[0]*v1[i] + w[1]*v2[i] + w[2]*v3[i];
    }
}

PFM_API void
pfmVec4Transform(PFMvec4 dst, const PFMvec4 v, const PFMmat4 mat)
{
    PFMvec4 tmp = {
        mat[0]*v[0] + mat[4]*v[1] + mat[8]*v[2] + mat[12]*v[3],
        mat[1]*v[0] + mat[5]*v[1] + mat[9]*v[2] + mat[13]*v[3],
        mat[2]*v[0] + mat[6]*v[1] + mat[10]*v[2] + mat[14]*v[3],
        mat[3]*v[0] + mat[7]*v[1] + mat[11]*v[2] + mat[15]*v[3]
    };

    memcpy(dst, tmp, sizeof(PFMvec4));
}

PFM_API void
pfmVec4TransformR(aPFMvec4 restrict dst, const PFMvec4 v, const PFMmat4 mat)
{
    dst[0] = mat[0]*v[0] + mat[4]*v[1] + mat[8]*v[2] + mat[12]*v[3];
    dst[1] = mat[1]*v[0] + mat[5]*v[1] + mat[9]*v[2] + mat[13]*v[3];
    dst[2] = mat[2]*v[0] + mat[6]*v[1] + mat[10]*v[2] + mat[14]*v[3];
    dst[3] = mat[3]*v[0] + mat[7]*v[1] + mat[11]*v[2] + mat[15]*v[3];
}

/* Matrix 4x4 function definitions */

PFM_API void
pfmMat4Copy(aPFMmat4 restrict dst, const aPFMmat4 restrict src)
{
    memcpy(dst, src, sizeof(PFMmat4));
}

PFM_API float
pfmMat4Determinant(const PFMmat4 mat)
{
    float result = 0.0f;

    // Cache the matrix values (speed optimization)
    float a00 = mat[0],  a01 = mat[1],  a02 = mat[2],  a03 = mat[3];
    float a10 = mat[4],  a11 = mat[5],  a12 = mat[6],  a13 = mat[7];
    float a20 = mat[8],  a21 = mat[9],  a22 = mat[10], a23 = mat[11];
    float a30 = mat[12], a31 = mat[13], a32 = mat[14], a33 = mat[15];

    result = a30*a21*a12*a03 - a20*a31*a12*a03 - a30*a11*a22*a03 + a10*a31*a22*a03 +
             a20*a11*a32*a03 - a10*a21*a32*a03 - a30*a21*a02*a13 + a20*a31*a02*a13 +
             a30*a01*a22*a13 - a00*a31*a22*a13 - a20*a01*a32*a13 + a00*a21*a32*a13 +
             a30*a11*a02*a23 - a10*a31*a02*a23 - a30*a01*a12*a23 + a00*a31*a12*a23 +
             a10*a01*a32*a23 - a00*a11*a32*a23 - a20*a11*a02*a33 + a10*a21*a02*a33 +
             a20*a01*a12*a33 - a00*a21*a12*a33 - a10*a01*a22*a33 + a00*a11*a22*a33;

    return result;
}

PFM_API float
pfmMat4Trace(const PFMmat4 mat)
{
    return mat[0] + mat[5] + mat[10] + mat[15];
}

PFM_API void
pfmMat4Transpose(PFMmat4 dst, const PFMmat4 src)
{
    // NOTE 1: Seems more optimized in O3 by GCC 13 without "omp simd collapse(2)"
    // NOTE 2: Also using "omp simd" produces exactly the same code in O3 with GCC 13.

    PFMmat4 result;
    for (int_fast8_t i = 0; i < 4; i++)
    {
        for (int_fast8_t j = 0; j < 4; j++)
        {
            result[i * 4 + j] = src[j * 4 + i];
        }
    }

    memcpy(dst, result, sizeof(PFMmat4));
}

PFM_API void
pfmMat4TransposeR(aPFMmat4 restrict dst, const PFMmat4 src)
{
    // NOTE 1: Seems more optimized in O3 by GCC 13 without "omp simd collapse(2)"
    // NOTE 2: Also using "omp simd" produces exactly the same code in O3 with GCC 13.

    for (int_fast8_t i = 0; i < 4; i++)
    {
        for (int_fast8_t j = 0; j < 4; j++)
        {
            dst[i * 4 + j] = src[j * 4 + i];
        }
    }
}

PFM_API void
pfmMat4Invert(PFMmat4 dst, const PFMmat4 src)
{
    // Cache the matrix values (speed optimization)
    float a00 = src[0],  a01 = src[1],  a02 = src[2],  a03 = src[3];
    float a10 = src[4],  a11 = src[5],  a12 = src[6],  a13 = src[7];
    float a20 = src[8],  a21 = src[9],  a22 = src[10], a23 = src[11];
    float a30 = src[12], a31 = src[13], a32 = src[14], a33 = src[15];

    float b00 = a00*a11 - a01*a10;
    float b01 = a00*a12 - a02*a10;
    float b02 = a00*a13 - a03*a10;
    float b03 = a01*a12 - a02*a11;
    float b04 = a01*a13 - a03*a11;
    float b05 = a02*a13 - a03*a12;
    float b06 = a20*a31 - a21*a30;
    float b07 = a20*a32 - a22*a30;
    float b08 = a20*a33 - a23*a30;
    float b09 = a21*a32 - a22*a31;
    float b10 = a21*a33 - a23*a31;
    float b11 = a22*a33 - a23*a32;

    // Calculate the invert determinant (inlined to avoid double-caching)
    float invDet = 1.0f/(b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);

    dst[0] = (a11*b11 - a12*b10 + a13*b09)*invDet;
    dst[1] = (-a01*b11 + a02*b10 - a03*b09)*invDet;
    dst[2] = (a31*b05 - a32*b04 + a33*b03)*invDet;
    dst[3] = (-a21*b05 + a22*b04 - a23*b03)*invDet;
    dst[4] = (-a10*b11 + a12*b08 - a13*b07)*invDet;
    dst[5] = (a00*b11 - a02*b08 + a03*b07)*invDet;
    dst[6] = (-a30*b05 + a32*b02 - a33*b01)*invDet;
    dst[7] = (a20*b05 - a22*b02 + a23*b01)*invDet;
    dst[8] = (a10*b10 - a11*b08 + a13*b06)*invDet;
    dst[9] = (-a00*b10 + a01*b08 - a03*b06)*invDet;
    dst[10] = (a30*b04 - a31*b02 + a33*b00)*invDet;
    dst[11] = (-a20*b04 + a21*b02 - a23*b00)*invDet;
    dst[12] = (-a10*b09 + a11*b07 - a12*b06)*invDet;
    dst[13] = (a00*b09 - a01*b07 + a02*b06)*invDet;
    dst[14] = (-a30*b03 + a31*b01 - a32*b00)*invDet;
    dst[15] = (a20*b03 - a21*b01 + a22*b00)*invDet;
}

PFM_API void
pfmMat4Identity(PFMmat4 dst)
{
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;
}

PFM_API void
pfmMat4Add(PFMmat4 dst, const PFMmat4 left, const PFMmat4 right)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 16; i++)
    {
        dst[i] = left[i] + right[i];
    }
}

PFM_API void
pfmMat4AddR(aPFMmat4 restrict dst, const PFMmat4 left, const PFMmat4 right)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 16; i++)
    {
        dst[i] = left[i] + right[i];
    }
}

PFM_API void
pfmMat4Sub(PFMmat4 dst, const PFMmat4 left, const PFMmat4 right)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 16; i++)
    {
        dst[i] = left[i] + right[i];
    }
}

PFM_API void
pfmMat4SubR(aPFMmat4 restrict dst, const PFMmat4 left, const PFMmat4 right)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 16; i++)
    {
        dst[i] = left[i] + right[i];
    }
}

PFM_API void
pfmMat4Mul(PFMmat4 dst, const PFMmat4 left, const PFMmat4 right)
{
    PFMmat4 result;

#   ifdef _OPENMP
#       pragma omp simd collapse(2)
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        for (int_fast8_t j = 0; j < 4; j++)
        {
            float sum = 0.0;

            for (int_fast8_t k = 0; k < 4; k++)
            {
                sum += left[i * 4 + k] * right[k * 4 + j];
            }

            result[i * 4 + j] = sum;
        }
    }

    memcpy(dst, result, sizeof(PFMmat4));
}

PFM_API void
pfmMat4MulR(aPFMmat4 restrict dst, const PFMmat4 left, const PFMmat4 right)
{
#   ifdef _OPENMP
#       pragma omp simd collapse(2)
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        for (int_fast8_t j = 0; j < 4; j++)
        {
            float sum = 0.0;

            for (int_fast8_t k = 0; k < 4; k++)
            {
                sum += left[i * 4 + k] * right[k * 4 + j];
            }

            dst[i * 4 + j] = sum;
        }
    }
}

PFM_API void
pfmMat4Translate(PFMmat4 dst, float x, float y, float z)
{
    memset(dst, 0, sizeof(PFMmat4));
    dst[12] = x, dst[13] = y, dst[14] = z;
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;
}

// NOTE: Angle should be provided in radians
PFM_API void
pfmMat4Rotate(PFMmat4 dst, const PFMvec3 axis, float angle)
{
    // pfmMat4Identity()
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    float x = axis[0], y = axis[1], z = axis[2];
    float lengthSq = x*x + y*y + z*z;

    if (lengthSq != 1.0f && lengthSq != 0.0f)
    {
        float invLenght = rsqrtf(lengthSq);
        x *= invLenght;
        y *= invLenght;
        z *= invLenght;
    }

    float sinres = sinf(angle);
    float cosres = cosf(angle);
    float t = 1.0f - cosres;

    dst[0]  = x*x*t + cosres;
    dst[1]  = y*x*t + z*sinres;
    dst[2]  = z*x*t - y*sinres;

    dst[4]  = x*y*t - z*sinres;
    dst[5]  = y*y*t + cosres;
    dst[6]  = z*y*t + x*sinres;

    dst[8]  = x*z*t + y*sinres;
    dst[9]  = y*z*t - x*sinres;
    dst[10] = z*z*t + cosres;
}

// NOTE: Angle must be provided in radians
PFM_API void
pfmMat4RotateX(PFMmat4 dst, float angle)
{
    // pfmMat4Identity()
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    float cosres = cosf(angle);
    float sinres = sinf(angle);

    dst[5]  = cosres;
    dst[6]  = sinres;
    dst[9]  = -sinres;
    dst[10] = cosres;
}

// NOTE: Angle must be provided in radians
PFM_API void
pfmMat4RotateY(PFMmat4 dst, float angle)
{
    // pfmMat4Identity()
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    float cosres = cosf(angle);
    float sinres = sinf(angle);

    dst[0]  = cosres;
    dst[2]  = -sinres;
    dst[8]  = sinres;
    dst[10] = cosres;
}

// NOTE: Angle must be provided in radians
PFM_API void
pfmMat4RotateZ(PFMmat4 dst, float angle)
{
    // pfmMat4Identity()
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    float cosres = cosf(angle);
    float sinres = sinf(angle);

    dst[0] = cosres;
    dst[1] = sinres;
    dst[4] = -sinres;
    dst[5] = cosres;
}

// NOTE: Angle must be provided in radians
PFM_API void
pfmMat4RotateXYZ(PFMmat4 dst, const PFMvec3 angle)
{
    // pfmMat4Identity()
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    float cosz = cosf(-angle[2]);
    float sinz = sinf(-angle[2]);
    float cosy = cosf(-angle[1]);
    float siny = sinf(-angle[1]);
    float cosx = cosf(-angle[0]);
    float sinx = sinf(-angle[0]);

    dst[0]  = cosz*cosy;
    dst[1]  = (cosz*siny*sinx) - (sinz*cosx);
    dst[2]  = (cosz*siny*cosx) + (sinz*sinx);

    dst[4]  = sinz*cosy;
    dst[5]  = (sinz*siny*sinx) + (cosz*cosx);
    dst[6]  = (sinz*siny*cosx) - (cosz*sinx);

    dst[8]  = -siny;
    dst[9]  = cosy*sinx;
    dst[10] = cosy*cosx;
}

// NOTE: Angle must be provided in radians
PFM_API void
pfmMat4RotateZYX(PFMmat4 dst, const PFMvec3 angle)
{
    // pfmMat4Identity()
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    float cz = cosf(angle[2]);
    float sz = sinf(angle[2]);
    float cy = cosf(angle[1]);
    float sy = sinf(angle[1]);
    float cx = cosf(angle[0]);
    float sx = sinf(angle[0]);

    dst[0] = cz*cy;
    dst[4] = cz*sy*sx - cx*sz;
    dst[8] = sz*sx + cz*cx*sy;

    dst[1] = cy*sz;
    dst[5] = cz*cx + sz*sy*sx;
    dst[9] = cx*sz*sy - cz*sx;

    dst[2] = -sy;
    dst[6] = cy*sx;
    dst[10] = cy*cx;
}

PFM_API void
pfmMat4Scale(PFMmat4 dst, float x, float y, float z)
{
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = x, dst[5] = y, dst[10] = z, dst[15] = 1.0;
}

PFM_API void
pfmMat4Frustum(PFMmat4 dst, float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    memset(dst, 0, sizeof(PFMmat4));

    float rl = right - left;
    float tb = top - bottom;
    float fn = farPlane - nearPlane;

    dst[0] = (nearPlane*2.0f)/rl;
    dst[5] = (nearPlane*2.0f)/tb;

    dst[8] = (right + left)/rl;
    dst[9] = (top + bottom)/tb;
    dst[10] = -(farPlane + nearPlane)/fn;
    dst[11] = -1.0f;

    dst[14] = -(farPlane*nearPlane*2.0f)/fn;
}

// NOTE: Fovy angle must be provided in radians
PFM_API void
pfmMat4Perspective(PFMmat4 dst, float fovY, float aspect, float nearPlane, float farPlane)
{
    memset(dst, 0, sizeof(PFMmat4));

    float top = nearPlane*tan(fovY*0.5);
    float bottom = -top;
    float right = top*aspect;
    float left = -right;

    // pfmMat4Frustum(-right, right, -top, top, near, far);
    float rl = right - left;
    float tb = top - bottom;
    float fn = farPlane - nearPlane;

    dst[0] = (nearPlane*2.0f)/rl;
    dst[5] = (nearPlane*2.0f)/tb;

    dst[8] = (right + left)/rl;
    dst[9] = (top + bottom)/tb;
    dst[10] = -(farPlane + nearPlane)/fn;
    dst[11] = -1.0f;

    dst[14] = -(farPlane*nearPlane*2.0f)/fn;
}

PFM_API void
pfmMat4Ortho(PFMmat4 dst, float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    memset(dst, 0, sizeof(PFMmat4));

    float rl = (right - left);
    float tb = (top - bottom);
    float fn = (farPlane - nearPlane);

    dst[0] = 2.0f/rl;
    dst[5] = 2.0f/tb;

    dst[10] = -2.0f/fn;
    dst[11] = 0.0f;
    dst[12] = -(left + right)/rl;
    dst[13] = -(top + bottom)/tb;

    dst[14] = -(farPlane + nearPlane)/fn;
    dst[15] = 1.0f;
}

PFM_API void
pfmMat4LookAt(PFMmat4 dst, const PFMvec3 eye, const PFMvec3 target, const PFMvec3 up)
{
    memset(dst, 0, sizeof(PFMmat4));

    float length = 0.0f;
    float invLenght = 0.0f;

    // pfmVec3Sub(eye, target)
    PFMvec3 vz = {
        eye[0] - target[0],
        eye[1] - target[1],
        eye[2] - target[2]
    };

    // pfmVec3Normalize(vz)
    PFMvec3 v = { vz[0], vz[1], vz[2] };
    length = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (length == 0.0f) length = 1.0f;
    invLenght = 1.0f/length;
    vz[0] *= invLenght;
    vz[1] *= invLenght;
    vz[2] *= invLenght;

    // pfVec3Cross(up, vz)
    PFMvec3 vx = {
        up[1]*vz[2] - up[2]*vz[1],
        up[2]*vz[0] - up[0]*vz[2],
        up[0]*vz[1] - up[1]*vz[0]
    };

    // pfmVec3Normalize(x)
    for (int_fast8_t i = 0; i < 3; i++) v[i] = vx[i];
    length = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (length == 0.0f) length = 1.0f;
    invLenght = 1.0f/length;
    vx[0] *= invLenght;
    vx[1] *= invLenght;
    vx[2] *= invLenght;

    // pfVec3Cross(vz, vx)
    PFMvec3 vy = {
        vz[1]*vx[2] - vz[2]*vx[1],
        vz[2]*vx[0] - vz[0]*vx[2],
        vz[0]*vx[1] - vz[1]*vx[0]
    };

    dst[0] = vx[0];
    dst[1] = vy[0];
    dst[2] = vz[0];

    dst[4] = vx[1];
    dst[5] = vy[1];
    dst[6] = vz[1];

    dst[8] = vx[2];
    dst[9] = vy[2];
    dst[10] = vz[2];

    dst[12] = -(vx[0]*eye[0] + vx[1]*eye[1] + vx[2]*eye[2]);   // pfVec3Dot(vx, eye)
    dst[13] = -(vy[0]*eye[0] + vy[1]*eye[1] + vy[2]*eye[2]);   // pfVec3Dot(vy, eye)
    dst[14] = -(vz[0]*eye[0] + vz[1]*eye[1] + vz[2]*eye[2]);   // pfVec3Dot(vz, eye)
    dst[15] = 1.0f;
}


/* SIMD function defintions */

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

#ifdef _MSC_VER
# define ALIGN32_BEG __declspec(align(32))
# define ALIGN32_END 
#else /* gcc or icc */
# define ALIGN32_BEG
# define ALIGN32_END __attribute__((aligned(32)))
#endif

/* declare some AVX constants -- why can't I figure a better way to do that? */
#define _PI32AVX_CONST(Name, Val) static const ALIGN32_BEG int _pi32avx_##Name[4] ALIGN32_END = { Val, Val, Val, Val }
#define _PS256_CONST(Name, Val) static const ALIGN32_BEG float _ps256_##Name[8] ALIGN32_END = { Val, Val, Val, Val, Val, Val, Val, Val }
#define _PI32_CONST256(Name, Val) static const ALIGN32_BEG int _pi32_256_##Name[8] ALIGN32_END = { Val, Val, Val, Val, Val, Val, Val, Val }
#define _PS256_CONST_TYPE(Name, Type, Val) static const ALIGN32_BEG Type _ps256_##Name[8] ALIGN32_END = { Val, Val, Val, Val, Val, Val, Val, Val }

_PI32AVX_CONST(1, 1);
_PI32AVX_CONST(inv1, ~1);
_PI32AVX_CONST(2, 2);
_PI32AVX_CONST(4, 4);

_PS256_CONST(1  , 1.0f);
_PS256_CONST(0p5, 0.5f);
/* the smallest non denormalized float number */
_PS256_CONST_TYPE(min_norm_pos, int, 0x00800000);
_PS256_CONST_TYPE(mant_mask, int, 0x7f800000);
_PS256_CONST_TYPE(inv_mant_mask, int, ~0x7f800000);

_PS256_CONST_TYPE(sign_mask, int, 0x80000000);
_PS256_CONST_TYPE(inv_sign_mask, int, ~0x80000000);

_PI32_CONST256(0, 0);
_PI32_CONST256(1, 1);
_PI32_CONST256(inv1, ~1);
_PI32_CONST256(2, 2);
_PI32_CONST256(4, 4);
_PI32_CONST256(0x7f, 0x7f);

_PS256_CONST(cephes_SQRTHF, 0.707106781186547524);
_PS256_CONST(cephes_log_p0, 7.0376836292E-2);
_PS256_CONST(cephes_log_p1, - 1.1514610310E-1);
_PS256_CONST(cephes_log_p2, 1.1676998740E-1);
_PS256_CONST(cephes_log_p3, - 1.2420140846E-1);
_PS256_CONST(cephes_log_p4, + 1.4249322787E-1);
_PS256_CONST(cephes_log_p5, - 1.6668057665E-1);
_PS256_CONST(cephes_log_p6, + 2.0000714765E-1);
_PS256_CONST(cephes_log_p7, - 2.4999993993E-1);
_PS256_CONST(cephes_log_p8, + 3.3333331174E-1);
_PS256_CONST(cephes_log_q1, -2.12194440e-4);
_PS256_CONST(cephes_log_q2, 0.693359375);

/**
 * natural logarithm computed for 8 simultaneous float 
 * return NaN for x <= 0
 */
PFM_API __m256
_mm256_log_ps(__m256 x)
{
    __m256i imm0;
    __m256 one = *(__m256*)_ps256_1;

    __m256 invalid_mask = _mm256_cmp_ps(x, _mm256_setzero_ps(), _CMP_LE_OS);

    x = _mm256_max_ps(x, *(__m256*)_ps256_min_norm_pos);  /* cut off denormalized stuff */

    imm0 = _mm256_srli_epi32(_mm256_castps_si256(x), 23);

    // keep only the fractional part
    x = _mm256_and_ps(x, *(__m256*)_ps256_inv_mant_mask);
    x = _mm256_or_ps(x, *(__m256*)_ps256_0p5);

    imm0 = _mm256_sub_epi32(imm0, *(__m256i*)_pi32_256_0x7f);
    __m256 e = _mm256_cvtepi32_ps(imm0);

    e = _mm256_add_ps(e, one);

    /* part2: 
       if( x < SQRTHF )
       {
            e -= 1;
            x = x + x - 1.0;
       }    else { x = x - 1.0; }
    */
    __m256 mask = _mm256_cmp_ps(x, *(__m256*)_ps256_cephes_SQRTHF, _CMP_LT_OS);
    __m256 tmp = _mm256_and_ps(x, mask);
    x = _mm256_sub_ps(x, one);
    e = _mm256_sub_ps(e, _mm256_and_ps(one, mask));
    x = _mm256_add_ps(x, tmp);

    __m256 z = _mm256_mul_ps(x,x);

    __m256 y = *(__m256*)_ps256_cephes_log_p0;
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)_ps256_cephes_log_p1);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)_ps256_cephes_log_p2);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)_ps256_cephes_log_p3);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)_ps256_cephes_log_p4);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)_ps256_cephes_log_p5);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)_ps256_cephes_log_p6);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)_ps256_cephes_log_p7);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, *(__m256*)_ps256_cephes_log_p8);
    y = _mm256_mul_ps(y, x);

    y = _mm256_mul_ps(y, z);
  
    tmp = _mm256_mul_ps(e, *(__m256*)_ps256_cephes_log_q1);
    y = _mm256_add_ps(y, tmp);

    tmp = _mm256_mul_ps(z, *(__m256*)_ps256_0p5);
    y = _mm256_sub_ps(y, tmp);

    tmp = _mm256_mul_ps(e, *(__m256*)_ps256_cephes_log_q2);
    x = _mm256_add_ps(x, y);
    x = _mm256_add_ps(x, tmp);
    x = _mm256_or_ps(x, invalid_mask); // negative arg will be NAN

    return x;
}

_PS256_CONST(exp_hi,	88.3762626647949f);
_PS256_CONST(exp_lo,	-88.3762626647949f);

_PS256_CONST(cephes_LOG2EF, 1.44269504088896341);
_PS256_CONST(cephes_exp_C1, 0.693359375);
_PS256_CONST(cephes_exp_C2, -2.12194440e-4);

_PS256_CONST(cephes_exp_p0, 1.9875691500E-4);
_PS256_CONST(cephes_exp_p1, 1.3981999507E-3);
_PS256_CONST(cephes_exp_p2, 8.3334519073E-3);
_PS256_CONST(cephes_exp_p3, 4.1665795894E-2);
_PS256_CONST(cephes_exp_p4, 1.6666665459E-1);
_PS256_CONST(cephes_exp_p5, 5.0000001201E-1);

PFM_API __m256
_mm256_exp_ps(__m256 x)
{
  __m256 tmp = _mm256_setzero_ps(), fx;
  __m256i imm0;
  __m256 one = *(__m256*)_ps256_1;

  x = _mm256_min_ps(x, *(__m256*)_ps256_exp_hi);
  x = _mm256_max_ps(x, *(__m256*)_ps256_exp_lo);

  // express exp(x) as exp(g + n*log(2))
  fx = _mm256_mul_ps(x, *(__m256*)_ps256_cephes_LOG2EF);
  fx = _mm256_add_ps(fx, *(__m256*)_ps256_0p5);

  tmp = _mm256_floor_ps(fx);

  // if greater, substract 1
  __m256 mask = _mm256_cmp_ps(tmp, fx, _CMP_GT_OS);    
  mask = _mm256_and_ps(mask, one);
  fx = _mm256_sub_ps(tmp, mask);

  tmp = _mm256_mul_ps(fx, *(__m256*)_ps256_cephes_exp_C1);
  __m256 z = _mm256_mul_ps(fx, *(__m256*)_ps256_cephes_exp_C2);
  x = _mm256_sub_ps(x, tmp);
  x = _mm256_sub_ps(x, z);

  z = _mm256_mul_ps(x,x);
  
  __m256 y = *(__m256*)_ps256_cephes_exp_p0;
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(__m256*)_ps256_cephes_exp_p1);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(__m256*)_ps256_cephes_exp_p2);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(__m256*)_ps256_cephes_exp_p3);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(__m256*)_ps256_cephes_exp_p4);
  y = _mm256_mul_ps(y, x);
  y = _mm256_add_ps(y, *(__m256*)_ps256_cephes_exp_p5);
  y = _mm256_mul_ps(y, z);
  y = _mm256_add_ps(y, x);
  y = _mm256_add_ps(y, one);

  // build 2^n
  imm0 = _mm256_cvttps_epi32(fx);
  imm0 = _mm256_add_epi32(imm0, *(__m256i*)_pi32_256_0x7f);
  imm0 = _mm256_slli_epi32(imm0, 23);
  __m256 pow2n = _mm256_castsi256_ps(imm0);
  y = _mm256_mul_ps(y, pow2n);
  return y;
}

#elif defined(__SSE2__)

PFM_API __m128i
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

PFM_API __m128i
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

PFM_API __m128i
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

#ifdef _MSC_VER
# define ALIGN16_BEG __declspec(align(16))
# define ALIGN16_END 
#else /* gcc or icc */
# define ALIGN16_BEG
# define ALIGN16_END __attribute__((aligned(16)))
#endif

/* declare some SSE constants -- why can't I figure a better way to do that? */
#define _PS_CONST(Name, Val) static const ALIGN16_BEG float _ps_##Name[4] ALIGN16_END = { Val, Val, Val, Val }
#define _PI32_CONST(Name, Val) static const ALIGN16_BEG int _pi32_##Name[4] ALIGN16_END = { Val, Val, Val, Val }
#define _PS_CONST_TYPE(Name, Type, Val) static const ALIGN16_BEG Type _ps_##Name[4] ALIGN16_END = { Val, Val, Val, Val }

_PS_CONST(1  , 1.0f);
_PS_CONST(0p5, 0.5f);
/* the smallest non denormalized float number */
_PS_CONST_TYPE(min_norm_pos, int, 0x00800000);
_PS_CONST_TYPE(mant_mask, int, 0x7f800000);
_PS_CONST_TYPE(inv_mant_mask, int, ~0x7f800000);

_PS_CONST_TYPE(sign_mask, int, (int)0x80000000);
_PS_CONST_TYPE(inv_sign_mask, int, ~0x80000000);

_PI32_CONST(1, 1);
_PI32_CONST(inv1, ~1);
_PI32_CONST(2, 2);
_PI32_CONST(4, 4);
_PI32_CONST(0x7f, 0x7f);

_PS_CONST(cephes_SQRTHF, 0.707106781186547524);
_PS_CONST(cephes_log_p0, 7.0376836292E-2);
_PS_CONST(cephes_log_p1, - 1.1514610310E-1);
_PS_CONST(cephes_log_p2, 1.1676998740E-1);
_PS_CONST(cephes_log_p3, - 1.2420140846E-1);
_PS_CONST(cephes_log_p4, + 1.4249322787E-1);
_PS_CONST(cephes_log_p5, - 1.6668057665E-1);
_PS_CONST(cephes_log_p6, + 2.0000714765E-1);
_PS_CONST(cephes_log_p7, - 2.4999993993E-1);
_PS_CONST(cephes_log_p8, + 3.3333331174E-1);
_PS_CONST(cephes_log_q1, -2.12194440e-4);
_PS_CONST(cephes_log_q2, 0.693359375);

/**
 * natural logarithm computed for 4 simultaneous float 
 * return NaN for x <= 0
 */
PFM_API __m128
_mm_log_ps(__m128 x)
{
  __m128i emm0;
  __m128 one = *(__m128*)_ps_1;

  __m128 invalid_mask = _mm_cmple_ps(x, _mm_setzero_ps());

  x = _mm_max_ps(x, *(__m128*)_ps_min_norm_pos); // cut off denormalized stuff
  emm0 = _mm_srli_epi32(_mm_castps_si128(x), 23);

  // keep only the fractional part
  x = _mm_and_ps(x, *(__m128*)_ps_inv_mant_mask);
  x = _mm_or_ps(x, *(__m128*)_ps_0p5);

  emm0 = _mm_sub_epi32(emm0, *(__m128i*)_pi32_0x7f);
  __m128 e = _mm_cvtepi32_ps(emm0);

  e = _mm_add_ps(e, one);

    /* part2: 
       if( x < SQRTHF )
       {
            e -= 1;
            x = x + x - 1.0;
       }    else { x = x - 1.0; }
    */
    __m128 mask = _mm_cmplt_ps(x, *(__m128*)_ps_cephes_SQRTHF);
    __m128 tmp = _mm_and_ps(x, mask);
    x = _mm_sub_ps(x, one);
    e = _mm_sub_ps(e, _mm_and_ps(one, mask));
    x = _mm_add_ps(x, tmp);

    __m128 z = _mm_mul_ps(x,x);

    __m128 y = *(__m128*)_ps_cephes_log_p0;
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p1);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p2);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p3);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p4);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p5);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p6);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p7);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p8);
    y = _mm_mul_ps(y, x);

    y = _mm_mul_ps(y, z);

    tmp = _mm_mul_ps(e, *(__m128*)_ps_cephes_log_q1);
    y = _mm_add_ps(y, tmp);

    tmp = _mm_mul_ps(z, *(__m128*)_ps_0p5);
    y = _mm_sub_ps(y, tmp);

    tmp = _mm_mul_ps(e, *(__m128*)_ps_cephes_log_q2);
    x = _mm_add_ps(x, y);
    x = _mm_add_ps(x, tmp);
    x = _mm_or_ps(x, invalid_mask); // negative arg will be NAN
    return x;
}

_PS_CONST(exp_hi,	88.3762626647949f);
_PS_CONST(exp_lo,	-88.3762626647949f);

_PS_CONST(cephes_LOG2EF, 1.44269504088896341);
_PS_CONST(cephes_exp_C1, 0.693359375);
_PS_CONST(cephes_exp_C2, -2.12194440e-4);

_PS_CONST(cephes_exp_p0, 1.9875691500E-4);
_PS_CONST(cephes_exp_p1, 1.3981999507E-3);
_PS_CONST(cephes_exp_p2, 8.3334519073E-3);
_PS_CONST(cephes_exp_p3, 4.1665795894E-2);
_PS_CONST(cephes_exp_p4, 1.6666665459E-1);
_PS_CONST(cephes_exp_p5, 5.0000001201E-1);

PFM_API __m128
_mm_exp_ps(__m128 x)
{
    __m128 tmp = _mm_setzero_ps(), fx;
    __m128i emm0;
    __m128 one = *(__m128*)_ps_1;

    x = _mm_min_ps(x, *(__m128*)_ps_exp_hi);
    x = _mm_max_ps(x, *(__m128*)_ps_exp_lo);

    // express exp(x) as exp(g + n*log(2))
    fx = _mm_mul_ps(x, *(__m128*)_ps_cephes_LOG2EF);
    fx = _mm_add_ps(fx, *(__m128*)_ps_0p5);

    // how to perform a floorf with SSE: just below
    emm0 = _mm_cvttps_epi32(fx);
    tmp  = _mm_cvtepi32_ps(emm0);

    // if greater, substract 1
    __m128 mask = _mm_cmpgt_ps(tmp, fx);    
    mask = _mm_and_ps(mask, one);
    fx = _mm_sub_ps(tmp, mask);

    tmp = _mm_mul_ps(fx, *(__m128*)_ps_cephes_exp_C1);
    __m128 z = _mm_mul_ps(fx, *(__m128*)_ps_cephes_exp_C2);
    x = _mm_sub_ps(x, tmp);
    x = _mm_sub_ps(x, z);

    z = _mm_mul_ps(x,x);
  
    __m128 y = *(__m128*)_ps_cephes_exp_p0;
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p1);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p2);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p3);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p4);
    y = _mm_mul_ps(y, x);
    y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p5);
    y = _mm_mul_ps(y, z);
    y = _mm_add_ps(y, x);
    y = _mm_add_ps(y, one);

    // build 2^n
    emm0 = _mm_cvttps_epi32(fx);
    emm0 = _mm_add_epi32(emm0, *(__m128i*)_pi32_0x7f);
    emm0 = _mm_slli_epi32(emm0, 23);
    __m128 pow2n = _mm_castsi128_ps(emm0);

    y = _mm_mul_ps(y, pow2n);
    return y;
}

#endif // (__AVX2__) || (__SSE2__)

PFM_API PFMsimd_f
pfmSimdSet1_F32(float x)
{
#if defined(__AVX2__)
    return _mm256_set1_ps(x);
#elif defined(__SSE2__)
    return _mm_set1_ps(x);
#else
    PFMsimd_f result = 0;
    ((float*)&result)[0] = x;
    ((float*)&result)[1] = x;
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdSet1_I32(int32_t x)
{
#if defined(__AVX2__)
    return _mm256_set1_epi32(x);
#elif defined(__SSE2__)
    return _mm_set1_epi32(x);
#else
    PFMsimd_i result = 0;
    ((int32_t*)&result)[0] = x;
    ((int32_t*)&result)[1] = x;
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdSetR_x4_I8(int8_t i0, int8_t i1, int8_t i2, int8_t i3)
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
#else
    PFMsimd_i result = 0;
    ((int8_t*)&result)[0] = i0;
    ((int8_t*)&result)[1] = i1;
    ((int8_t*)&result)[2] = i2;
    ((int8_t*)&result)[3] = i3;
    ((int8_t*)&result)[4] = i0;
    ((int8_t*)&result)[5] = i1;
    ((int8_t*)&result)[6] = i2;
    ((int8_t*)&result)[7] = i3;
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdSetR_I32(int32_t i0, int32_t i1, int32_t i2, int32_t i3, int32_t i4, int32_t i5, int32_t i6, int32_t i7)
{
#if defined(__AVX2__)
    return _mm256_setr_epi32(i0, i1, i2, i3, i4, i5, i6, i7);
#elif defined(__SSE2__)
    (void)i4, (void)i5, (void)i6, (void)i7;
    return _mm_setr_epi32(i0, i1, i2, i3);
#else
    (void)i2, (void)i3, (void)i4, (void)i5, (void)i6, (void)i7;
    PFMsimd_i result = 0;
    ((int32_t*)&result)[0] = i0;
    ((int32_t*)&result)[1] = i1;
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdSetZero_I32(void)
{
#if defined(__AVX2__)
    return _mm256_setzero_si256();
#elif defined(__SSE2__)
    return _mm_setzero_si128();
#else
    return 0;
#endif
}

PFM_API PFMsimd_f
pfmSimdSetZero_F32(void)
{
#if defined(__AVX2__)
    return _mm256_setzero_ps();
#elif defined(__SSE2__)
    return _mm_setzero_ps();
#else
    return 0.0;
#endif
}

#if defined(__AVX2__)
#   define pfmSimdRound_F32(x, imm) \
        _mm256_round_ps(x, imm)
#elif defined(__SSE2__)
#   define pfmSimdRound_F32(x, imm) \
        _mm_round_ps(x, imm)
#else
PFM_API PFMsimd_f
pfmSimdRound_F32(PFMsimd_f x, const int imm)
{
    (void)imm;
    ((float*)&x)[0] = roundf(((float*)&x)[0]);
    ((float*)&x)[1] = roundf(((float*)&x)[1]);
    return x;
}
#endif

PFM_API PFMsimd_i
pfmSimdAbs_I32(PFMsimd_i x)
{
#if defined(__AVX2__)
    return _mm256_abs_epi32(x);
#elif defined(__SSE2__)
    return _mm_abs_epi32(x);
#else
    ((int32_t*)&x)[0] = abs(((int32_t*)&x)[0]);
    ((int32_t*)&x)[1] = abs(((int32_t*)&x)[1]);
    return x;
#endif
}

PFM_API PFMsimd_f
pfmSimdAbs_F32(PFMsimd_f x)
{
#if defined(__AVX2__)
    return _mm256_andnot_ps(
        _mm256_set1_ps(-0.0f), x);
#elif defined(__SSE2__)
    return _mm_andnot_ps(
        _mm_set1_ps(-0.0f), x);
#else
    ((float*)&x)[0] = fabsf(((float*)&x)[0]);
    ((float*)&x)[1] = fabsf(((float*)&x)[1]);
    return x;
#endif
}

PFM_API PFMsimd_i
pfmSimdUnpackLo_I8(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_unpacklo_epi8(x, y);
#elif defined(__SSE2__)
    return _mm_unpacklo_epi8(x, y);
#else
    PFMsimd_i result = 0;
    ((int8_t*)&result)[0] = ((int8_t*)&x)[0];
    ((int8_t*)&result)[1] = ((int8_t*)&y)[0];
    ((int8_t*)&result)[2] = ((int8_t*)&x)[1];
    ((int8_t*)&result)[3] = ((int8_t*)&y)[1];
    ((int8_t*)&result)[4] = ((int8_t*)&x)[2];
    ((int8_t*)&result)[5] = ((int8_t*)&y)[2];
    ((int8_t*)&result)[6] = ((int8_t*)&x)[3];
    ((int8_t*)&result)[7] = ((int8_t*)&y)[3];
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdUnpackLo_I16(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_unpacklo_epi16(x, y);
#elif defined(__SSE2__)
    return _mm_unpacklo_epi16(x, y);
#else
    PFMsimd_i result = 0;
    ((int16_t*)&result)[0] = ((int16_t*)&x)[0];
    ((int16_t*)&result)[1] = ((int16_t*)&y)[0];
    ((int16_t*)&result)[2] = ((int16_t*)&x)[1];
    ((int16_t*)&result)[3] = ((int16_t*)&y)[1];
    return result;
#endif
}

PFM_API void
pfmSimdStore_I8(void* p, PFMsimd_i x)
{
#if defined(__AVX2__)
    _mm256_storeu_si256((__m256i*)p, x);    //< REVIEW: si64 ???
#elif defined(__SSE2__)
    _mm_storeu_si32((__m128i*)p, x);
#else
    int8_t* dest = (int8_t*)p;
    for (int_fast8_t i = 0; i < 8; ++i) {
        dest[i] = ((int8_t*)&x)[i];
    }
#endif
}

PFM_API void
pfmSimdStore_I16(void* p, PFMsimd_i x)
{
#if defined(__AVX2__)
    _mm256_storeu_si256((__m256i*)p, x);    //< REVIEW: si128 ???
#elif defined(__SSE2__)
    _mm_storeu_si64((__m128i*)p, x);
#else
    int16_t* dest = (int16_t*)p;
    for (int_fast8_t i = 0; i < 4; ++i) {
        dest[i] = ((int16_t*)&x)[i];
    }
#endif
}

PFM_API void
pfmSimdStore_I32(void* p, PFMsimd_i x)
{
#if defined(__AVX2__)
    _mm256_storeu_si256((__m256i*)p, x);
#elif defined(__SSE2__)
    _mm_storeu_si128((__m128i*)p, x);
#else
    int32_t* dest = (int32_t*)p;
    for (int_fast8_t i = 0; i < 2; ++i) {
        dest[i] = ((int32_t*)&x)[i];
    }
#endif
}

PFM_API void
pfmSimdStore_F32(void* p, PFMsimd_f x)
{
#if defined(__AVX2__)
    _mm256_storeu_ps((float*)p, x);
#elif defined(__SSE2__)
    _mm_storeu_ps((float*)p, x);
#else
    float* dest = (float*)p;
    dest[0] = ((float*)&x)[0];
    dest[1] = ((float*)&x)[1];
#endif
}

PFM_API PFMsimd_i
pfmSimdLoad_I32(const void* p)
{
#if defined(__AVX2__)
    return _mm256_loadu_si256((const __m256i*)p);
#elif defined(__SSE2__)
    return _mm_loadu_si128((const __m128i*)p);
#else
    PFMsimd_i result = 0;
    ((int32_t*)&result)[0] = ((const int32_t*)p)[0];
    ((int32_t*)&result)[1] = ((const int32_t*)p)[1];
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdLoad_F32(const void* p)
{
#if defined(__AVX2__)
    return _mm256_loadu_ps(p);
#elif defined(__SSE2__)
    return _mm_loadu_ps(p);
#else
    PFMsimd_f result = 0;
    ((float*)&result)[0] = ((const float*)p)[0];
    ((float*)&result)[1] = ((const float*)p)[1];
    return result;
#endif
}

#if defined(__AVX2__)
#   define pfmSimdExtract_I32(v, index)  \
        _mm256_extract_epi32(v, index)
#elif defined(__SSE2__)
#   define pfmSimdExtract_I32(v, index)  \
        _mm_extract_epi32(v, index % 4)
#else
#   define pfmSimdExtract_I32(v, index)  \
        *((int32_t*)&v + (index % 2))
#endif

PFM_API int32_t
pfmSimdExtractVarIdx_I32(PFMsimd_i x, int32_t index)
{
#if defined(__AVX2__)
    __m128i idx = _mm_cvtsi32_si128(index);
    __m256i val = _mm256_permutevar8x32_epi32(x, _mm256_castsi128_si256(idx));
    return _mm_cvtsi128_si32(_mm256_castsi256_si128(val));
#elif defined(__SSE2__)
    union v_u { __m128i vec; int arr[4]; };
    union v_u v = { .vec = x };
    return v.arr[index % 4];
#else
    return *((int32_t*)&x + (index % 2));
#endif
}

#if defined(__AVX2__)
#   define pfmSimdGather_I32(p, offsets, alignment) \
        _mm256_i32gather_epi32(p, offsets, alignment)
#elif defined(__SSE2__)
#   define pfmSimdGather_I32(p, offsets, alignment) \
        _mm_i32gather_epi32(p, offsets, alignment)
#else
#   define pfmSimdGather_I32(p, offsets, alignment) \
        (int8_t[2]) { \
            ((const int8_t*)p)[*((int8_t*)&offsets + 0 * alignment)], \
            ((const int8_t*)p)[*((int8_t*)&offsets + 1 * alignment)] \
        }
#endif

PFM_API PFMsimd_i
pfmSimdShuffle_I8(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_shuffle_epi8(x, y);
#elif defined(__SSE4_1__)
    return _mm_shuffle_epi8(x, y);
#elif defined(__SSE2__)
    return _mm_shuffle_epi8_sse2(x, y);
#else
    PFMsimd_i result = 0;
    for (int_fast8_t i = 0; i < 16; ++i) {
        int8_t index = ((int8_t*)&y)[i];
        if (index >= 0 && index < 16) {
            ((int8_t*)&result)[i] = ((int8_t*)&x)[index];
        } else {
            ((int8_t*)&result)[i] = 0;
        }
    }
    return result;
#endif
}

#if defined(__AVX2__)
#   define pfmSimdShuffle_F32(v1, v2, mask)  \
        _mm256_shuffle_ps(v1, v2, mask)
#elif defined(__SSE2__)
#   define pfmSimdShuffle_F32(v1, v2, mask)  \
        _mm_shuffle_ps(v1, v2, mask)
#else
PFM_API PFMsimd_f
pfmSimdShuffle_F32(PFMsimd_f v1, PFMsimd_f v2, int mask)
{
    PFMsimd_f result;
    ((float*)&result)[0] = ((float*)&v1)[(mask & 0x03)];
    ((float*)&result)[1] = ((float*)&v1)[(mask >> 2) & 0x03];
    ((float*)&result)[2] = ((float*)&v2)[(mask >> 4) & 0x03];
    ((float*)&result)[3] = ((float*)&v2)[(mask >> 6) & 0x03];
    return result;
}
#endif

PFM_API PFMsimd_i
pfmSimdConvert_F32_I32(PFMsimd_f x)
{
#if defined(__AVX2__)
    return _mm256_cvtps_epi32(x);
#elif defined(__SSE2__)
    return _mm_cvtps_epi32(x);
#else
    PFMsimd_i result;
    ((int32_t*)&result)[0] = (int32_t)((float*)&x)[0];
    ((int32_t*)&result)[1] = (int32_t)((float*)&x)[1];
    return result;
#endif
}

// TODO: Review the management of '_mm256_cvtph_ps' which receives a '__m128i' to return a 
//       '__m256', and the same problem on the contrary with the macro using '_mm256_cvtps_ph'

/*
#if defined(__AVX2__)
#   define pfmSimdConvert_F32_F16(x, imm)  \
        _mm256_cvtps_ph(x, index)
#elif defined(__SSE2__)
#   define pfmSimdConvert_F32_F16(x, imm)  \
        _mm_cvtps_ph(x, imm)
#else
PFM_API PFMsimd_f
pfmSimdConvert_F32_F16(PFMsimd_f x, const int imm)
{
    // REVIEW: Incorrect behavior

    (void)imm;
    PFMsimd_f result = 0;
    for (int_fast8_t i = 0; i < 2; ++i)
    {
        const float in = ((float*)&x)[i];
        const uint32_t b = (*(uint32_t*)&in)+0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
        const uint32_t e = (b&0x7F800000)>>23; // exponent
        const uint32_t m = b&0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
        ((uint16_t*)&result)[i] = (b&0x80000000)>>16 | (e>112)*((((e-112)<<10)&0x7C00)|m>>13) | ((e<113)&(e>101))*((((0x007FF000+m)>>(125-e))+1)>>1) | (e>143)*0x7FFF; // sign : normalized : denormalized : saturate
    }
    return result;
}
#endif

PFM_API PFMsimd_i
pfmSimdConvert_F16_F32(PFMsimd_f x)
{
#if defined(__AVX2__)
    return _mm256_cvtph_ps(x);
#elif defined(__SSE2__)
    return _mm_cvtph_ps(x);
#else
    // REVIEW: Incorrect behavior

    PFMsimd_f result = 0;
    for (int_fast8_t i = 0; i < 2; ++i)
    {
        const uint16_t in = ((uint16_t*)&x)[i];
        const uint32_t e = (in&0x7C00)>>10; // exponent
        const uint32_t m = (in&0x03FF)<<13; // mantissa
        const float fm = (float)m;
        const uint32_t v = (*(uint32_t*)&fm)>>23; // evil log2 bit hack to count leading zeros in denormalized format
        const uint32_t r = (in&0x8000)<<16 | (e!=0)*((e+112)<<23|m) | ((e==0)&(m!=0))*((v-37)<<23|((m<<(150-v))&0x007FE000)); // sign : normalized : denormalized
        ((float*)&result)[i] = *(float*)&r;
    }
    return result;
#endif
}
*/

PFM_API PFMsimd_f
pfmSimdConvert_I32_F32(PFMsimd_i x)
{
#if defined(__AVX2__)
    return _mm256_cvtepi32_ps(x);
#elif defined(__SSE2__)
    return _mm_cvtepi32_ps(x);
#else
    PFMsimd_f result;
    ((float*)&result)[0] = (float)((int32_t*)&x)[0];
    ((float*)&result)[1] = (float)((int32_t*)&x)[1];
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdCast_F32_I32(PFMsimd_f x)
{
#if defined(__AVX2__)
    return _mm256_castps_si256(x);
#elif defined(__SSE2__)
    return _mm_castps_si128(x);
#else
    return *(PFMsimd_i*)&x;
#endif
}

PFM_API PFMsimd_f
pfmSimdCast_I32_F32(PFMsimd_i x)
{
#if defined(__AVX2__)
    return _mm256_castsi256_ps(x);
#elif defined(__SSE2__)
    return _mm_castsi128_ps(x);
#else
    return *(PFMsimd_f*)&x;
#endif
}

PFM_API PFMsimd_i
pfmSimdMin_I32(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_min_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_min_epi32(x, y);
#else
    PFMsimd_i result;
    ((int32_t*)&result)[0] = fminf(((int32_t*)&x)[0], ((int32_t*)&y)[0]);
    ((int32_t*)&result)[1] = fminf(((int32_t*)&x)[1], ((int32_t*)&y)[1]);
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdMin_F32(PFMsimd_f x, PFMsimd_f y)
{
#if defined(__AVX2__)
    return _mm256_min_ps(x, y);
#elif defined(__SSE2__)
    return _mm_min_ps(x, y);
#else
    PFMsimd_f result;
    ((float*)&result)[0] = fminf(((float*)&x)[0], ((float*)&y)[0]);
    ((float*)&result)[1] = fminf(((float*)&x)[1], ((float*)&y)[1]);
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdMax_I32(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_max_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_max_epi32(x, y);
#else
    PFMsimd_i result;
    ((int32_t*)&result)[0] = fmaxf(((int32_t*)&x)[0], ((int32_t*)&y)[0]);
    ((int32_t*)&result)[1] = fmaxf(((int32_t*)&x)[1], ((int32_t*)&y)[1]);
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdMax_F32(PFMsimd_f x, PFMsimd_f y)
{
#if defined(__AVX2__)
    return _mm256_max_ps(x, y);
#elif defined(__SSE2__)
    return _mm_max_ps(x, y);
#else
    PFMsimd_f result;
    ((float*)&result)[0] = fmaxf(((float*)&x)[0], ((float*)&y)[0]);
    ((float*)&result)[1] = fmaxf(((float*)&x)[1], ((float*)&y)[1]);
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdClamp_I32(PFMsimd_i x, PFMsimd_i min, PFMsimd_i max)
{
#if defined(__AVX2__)
    return _mm256_min_epi32(_mm256_max_epi32(x, min), max);
#elif defined(__SSE2__)
    return _mm_min_epi32(_mm_max_epi32(x, min), max);
#else
    PFMsimd_i result;
    ((int32_t*)&result)[0] = fminf(fmaxf(((int32_t*)&x)[0], ((int32_t*)&min)[0]), ((int32_t*)&max)[0]);
    ((int32_t*)&result)[1] = fminf(fmaxf(((int32_t*)&x)[1], ((int32_t*)&min)[1]), ((int32_t*)&max)[1]);
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdClamp_F32(PFMsimd_f x, PFMsimd_f min, PFMsimd_f max)
{
#if defined(__AVX2__)
    return _mm256_min_ps(_mm256_max_ps(x, min), max);
#elif defined(__SSE2__)
    return _mm_min_ps(_mm_max_ps(x, min), max);
#else
    PFMsimd_f result;
    ((float*)&result)[0] = fminf(fmaxf(((float*)&x)[0], ((float*)&min)[0]), ((float*)&max)[0]);
    ((float*)&result)[1] = fminf(fmaxf(((float*)&x)[1], ((float*)&min)[1]), ((float*)&max)[1]);
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdAdd_I32(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_add_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_add_epi32(x, y);
#else
    PFMsimd_i result;
    ((int32_t*)&result)[0] = ((int32_t*)&x)[0] + ((int32_t*)&y)[0];
    ((int32_t*)&result)[1] = ((int32_t*)&x)[1] + ((int32_t*)&y)[1];
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdAdd_F32(PFMsimd_f x, PFMsimd_f y)
{
#if defined(__AVX2__)
    return _mm256_add_ps(x, y);
#elif defined(__SSE2__)
    return _mm_add_ps(x, y);
#else
    PFMsimd_f result;
    ((float*)&result)[0] = ((float*)&x)[0] + ((float*)&y)[0];
    ((float*)&result)[1] = ((float*)&x)[1] + ((float*)&y)[1];
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdSub_I32(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_sub_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_sub_epi32(x, y);
#else
    PFMsimd_i result;
    ((int32_t*)&result)[0] = ((int32_t*)&x)[0] - ((int32_t*)&y)[0];
    ((int32_t*)&result)[1] = ((int32_t*)&x)[1] - ((int32_t*)&y)[1];
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdSub_F32(PFMsimd_f x, PFMsimd_f y)
{
#if defined(__AVX2__)
    return _mm256_sub_ps(x, y);
#elif defined(__SSE2__)
    return _mm_sub_ps(x, y);
#else
    PFMsimd_f result;
    ((float*)&result)[0] = ((float*)&x)[0] - ((float*)&y)[0];
    ((float*)&result)[1] = ((float*)&x)[1] - ((float*)&y)[1];
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdMullo_I32(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_mullo_epi32(x, y);
#elif defined(__SSE4_1__)
    return _mm_mullo_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_mullo_epi32_sse2(x, y);
#else
    PFMsimd_i result;
    ((int32_t*)&result)[0] = ((int32_t*)&x)[0] * ((int32_t*)&y)[0];
    ((int32_t*)&result)[1] = ((int32_t*)&x)[1] * ((int32_t*)&y)[1];
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdMul_F32(PFMsimd_f x, PFMsimd_f y)
{
#if defined(__AVX2__)
    return _mm256_mul_ps(x, y);
#elif defined(__SSE2__)
    return _mm_mul_ps(x, y);
#else
    PFMsimd_f result;
    ((float*)&result)[0] = ((float*)&x)[0] * ((float*)&y)[0];
    ((float*)&result)[1] = ((float*)&x)[1] * ((float*)&y)[1];
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdPow_F32(PFMsimd_f base, float exponent)
{
#if defined(__AVX2__)
    __m256 exp = _mm256_set1_ps(exponent);
    __m256 log_base = _mm256_log_ps(base);
    return _mm256_exp_ps(_mm256_mul_ps(log_base, exp));
#elif defined(__SSE2__)
    __m128 exp = _mm_set1_ps(exponent);
    __m128 log_base = _mm_log_ps(base);
    return _mm_exp_ps(_mm_mul_ps(log_base, exp));
#else
    PFMsimd_f result;
    ((float*)&result)[0] = powf(((float*)&x)[0], exponent);
    ((float*)&result)[1] = powf(((float*)&x)[1], exponent);
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdDiv_F32(PFMsimd_f x, PFMsimd_f y)
{
#if defined(__AVX2__)
    return _mm256_div_ps(x, y);
#elif defined(__SSE2__)
    return _mm_div_ps(x, y);
#else
    PFMsimd_f result;
    ((float*)&result)[0] = ((float*)&x)[0] / ((float*)&y)[0];
    ((float*)&result)[1] = ((float*)&x)[1] / ((float*)&y)[1];
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdMod_F32(PFMsimd_f x, PFMsimd_f y)
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
#else
    PFMsimd_f result;
    ((float*)&result)[0] = fmodf(((float*)&x)[0], ((float*)&y)[0]);
    ((float*)&result)[1] = fmodf(((float*)&x)[1], ((float*)&y)[1]);
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdNeg_I32(PFMsimd_i x)
{
#if defined(__AVX2__)
    return _mm256_sub_epi32(_mm256_setzero_si256(), x);
#elif defined(__SSE2__)
    return _mm_sub_epi32(_mm_setzero_si128(), x);
#else
    PFMsimd_i result;
    ((int32_t*)&result)[0] = -((int32_t*)&x)[0];
    ((int32_t*)&result)[1] = -((int32_t*)&x)[1];
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdNeg_F32(PFMsimd_f x)
{
#if defined(__AVX2__)
    return _mm256_sub_ps(_mm256_setzero_ps(), x);
#elif defined(__SSE2__)
    return _mm_sub_ps(_mm_setzero_ps(), x);
#else
    PFMsimd_f result;
    ((float*)&result)[0] = -((float*)&x)[0];
    ((float*)&result)[1] = -((float*)&x)[1];
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdRCP_F32(PFMsimd_f x)
{
#if defined(__AVX2__)
    return _mm256_rcp_ps(x);
#elif defined(__SSE2__)
    return _mm_rcp_ps(x);
#else
    PFMsimd_f result;
    ((float*)&result)[0] = 1.0f / ((float*)&x)[0];
    ((float*)&result)[1] = 1.0f / ((float*)&x)[1];
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdSqrt_F32(PFMsimd_f x)
{
#if defined(__AVX2__)
    return _mm256_sqrt_ps(x);
#elif defined(__SSE2__)
    return _mm_sqrt_ps(x);
#else
    PFMsimd_f result;
    ((float*)&result)[0] = sqrtf(((float*)&x)[0]);
    ((float*)&result)[1] = sqrtf(((float*)&x)[1]);
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdRSqrt_F32(PFMsimd_f x)
{
#if defined(__AVX2__)
    return _mm256_rsqrt_ps(x);
#elif defined(__SSE2__)
    return _mm_rsqrt_ps(x);
#else
    PFMsimd_f result;
    ((float*)&result)[0] = rsqrtf(((float*)&x)[0]);
    ((float*)&result)[1] = rsqrtf(((float*)&x)[1]);
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdPermute_I32(PFMsimd_i x, PFMsimd_i y)
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
    PFMsimd_i result = 0;
    for (int_fast8_t i = 0; i < 2; ++i) {
        int index = ((int32_t*)&y)[i] % 2;
        ((int32_t*)&result)[i] = ((int32_t*)&x)[index];
    }
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdAnd_I32(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_and_si256(x, y);
#elif defined(__SSE2__)
    return _mm_and_si128(x, y);
#else
    PFMsimd_i result;
    ((int32_t*)&result)[0] = ((int32_t*)&x)[0] & ((int32_t*)&y)[0];
    ((int32_t*)&result)[1] = ((int32_t*)&x)[1] & ((int32_t*)&y)[1];
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdAnd_F32(PFMsimd_f x, PFMsimd_f y)
{
#if defined(__AVX2__)
    return _mm256_and_ps(x, y);
#elif defined(__SSE2__)
    return _mm_and_ps(x, y);
#else
    PFMsimd_f result;
    ((int32_t*)&result)[0] = ((int32_t*)&x)[0] & ((int32_t*)&y)[0];
    ((int32_t*)&result)[1] = ((int32_t*)&x)[1] & ((int32_t*)&y)[1];
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdOr_I32(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_or_si256(x, y);
#elif defined(__SSE2__)
    return _mm_or_si128(x, y);
#else
    PFMsimd_i result;
    ((int32_t*)&result)[0] = ((int32_t*)&x)[0] | ((int32_t*)&y)[0];
    ((int32_t*)&result)[1] = ((int32_t*)&x)[1] | ((int32_t*)&y)[1];
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdOr_F32(PFMsimd_f x, PFMsimd_f y)
{
#if defined(__AVX2__)
    return _mm256_or_ps(x, y);
#elif defined(__SSE2__)
    return _mm_or_ps(x, y);
#else
    PFMsimd_f result;
    ((int32_t*)&result)[0] = ((int32_t*)&x)[0] | ((int32_t*)&y)[0];
    ((int32_t*)&result)[1] = ((int32_t*)&x)[1] | ((int32_t*)&y)[1];
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdShr_I32(PFMsimd_i x, int32_t imm8)
{
#if defined(__AVX2__)
    return _mm256_srli_epi32(x, imm8);
#elif defined(__SSE2__)
    return _mm_srli_epi32(x, imm8);
#else
    PFMsimd_i result;
    ((int32_t*)&result)[0] = ((int32_t*)&x)[0] >> imm8;
    ((int32_t*)&result)[1] = ((int32_t*)&x)[1] >> imm8;
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdShl_I32(PFMsimd_i x, int32_t imm8)
{
#if defined(__AVX2__)
    return _mm256_slli_epi32(x, imm8);
#elif defined(__SSE2__)
    return _mm_slli_epi32(x, imm8);
#else
    PFMsimd_i result;
    ((int32_t*)&result)[0] = ((int32_t*)&x)[0] << imm8;
    ((int32_t*)&result)[1] = ((int32_t*)&x)[1] << imm8;
    return result;
#endif
}

PFM_API int32_t
pfmSimdMoveMask_F32(PFMsimd_f x)
{
#if defined(__AVX2__)
    return _mm256_movemask_ps(x);
#elif defined(__SSE2__)
    return _mm_movemask_ps(x);
#else
    int32_t mask = 0;
    float* fx = (float*)&x;
    mask |= ((*(int32_t*)&fx[0] >> 31) & 0x1) << 0;
    mask |= ((*(int32_t*)&fx[1] >> 31) & 0x1) << 1;
    return mask;
#endif
}

PFM_API int32_t
pfmSimdMoveMask_I8(PFMsimd_i x)
{
#if defined(__AVX2__)
    return _mm256_movemask_epi8(x);
#elif defined(__SSE2__)
    return _mm_movemask_epi8(x);
#else
    int32_t mask = 0;
    int8_t* ix = (int8_t*)&x;
    for (int_fast8_t i = 0; i < 8; ++i) {
        mask |= ((ix[i] >> 7) & 0x1) << i;
    }
    return mask;
#endif
}

PFM_API PFMsimd_i
pfmSimdBlendV_I8(PFMsimd_i a, PFMsimd_i b, PFMsimd_i mask)
{
#if defined(__AVX2__)
    return _mm256_blendv_epi8(a, b, mask);
#elif defined(__SSE4_1__)
    return _mm_blendv_epi8(a, b, mask);
#elif defined(__SSE2__)
    return _mm_blendv_epi8_sse2(a, b, mask);
#else
    PFMsimd_i result;
    ((int8_t*)&result)[0] = ((int8_t*)&mask)[0] ? ((int8_t*)&x)[0] : ((int8_t*)&y)[0];
    ((int8_t*)&result)[1] = ((int8_t*)&mask)[1] ? ((int8_t*)&x)[1] : ((int8_t*)&y)[1];
    ((int8_t*)&result)[2] = ((int8_t*)&mask)[2] ? ((int8_t*)&x)[2] : ((int8_t*)&y)[2];
    ((int8_t*)&result)[3] = ((int8_t*)&mask)[3] ? ((int8_t*)&x)[3] : ((int8_t*)&y)[3];
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdBlendV_I16(PFMsimd_i a, PFMsimd_i b, PFMsimd_i mask)
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

#else
    PFMsimd_i result;
    int16_t* pa = (int16_t*)&a;
    int16_t* pb = (int16_t*)&b;
    int16_t* pmask = (int16_t*)&mask;
    int16_t* presult = (int16_t*)&result;
    for (int i = 0; i < 4; ++i) {
        presult[i] = (pmask[i] < 0) ? pb[i] : pa[i];
    }
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdBlendV_F32(PFMsimd_f a, PFMsimd_f b, PFMsimd_f mask)
{
#if defined(__AVX2__)
    return _mm256_blendv_ps(a, b, mask);
#elif defined(__SSE4_1__)
    return _mm_blendv_ps(a, b, mask);
#elif defined(__SSE2__)
    return _mm_or_ps(
        _mm_and_ps(mask, a),
        _mm_andnot_ps(mask, b));
#else
    PFMsimd_i result;
    float* pa = (float*)&a;
    float* pb = (float*)&b;
    float* pmask = (float*)&mask;
    float* presult = (float*)&result;
    for (int i = 0; i < 2; ++i) {
        presult[i] = (pmask[i] < 0) ? pb[i] : pa[i];
    }
    return result;
#endif
}

PFM_API int
pfmSimdAllZero_I32(PFMsimd_i x)
{
#if defined(__AVX2__)
    __m256i cmp = _mm256_cmpeq_epi32(x, _mm256_setzero_si256());
    return (_mm256_movemask_epi8(cmp) == 0xFFFF);
#elif defined(__SSE2__)
    __m128i cmp = _mm_cmpeq_epi32(x, _mm_setzero_si128());
    return (_mm_movemask_epi8(cmp) == 0xFFFF);
#else
    int32_t* px = (int32_t*)&x;
    for (int_fast8_t i = 0; i < 2; ++i) {
        if (px[i] != 0) return 0;
    }
    return 1;
#endif
}

PFM_API int
pfmSimdAllZero_F32(PFMsimd_f x)
{
#if defined(__AVX2__)
    __m256 cmp = _mm256_cmp_ps(x, _mm256_setzero_ps(), _CMP_EQ_OS);
    return (_mm256_movemask_ps(cmp) == 0xFFFF);
#elif defined(__SSE2__)
    __m128 cmp = _mm_cmpeq_ps(x, _mm_setzero_ps());
    return (_mm_movemask_ps(cmp) == 0xFFFF);
#else
    int32_t* px = (int32_t*)&x;
    for (int_fast8_t i = 0; i < 2; ++i) {
        if (px[i] != 0) return 0;
    }
    return 1;
#endif
}

PFM_API PFMsimd_i
pfmSimdCmpEQ_I32(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_cmpeq_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_cmpeq_epi32(x, y);
#else
    PFMsimd_i result;
    int32_t* px = (int32_t*)&x;
    int32_t* py = (int32_t*)&y;
    int32_t* presult = (int32_t*)&result;
    for (int_fast8_t i = 0; i < 2; ++i) {
        presult[i] = (px[i] == py[i]) ? 0xFFFFFFFF : 0x00000000;
    }
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdCmpEQ_F32(PFMsimd_f x, PFMsimd_f y)
{
#if defined(__AVX2__)
    return _mm256_cmp_ps(x, y, _CMP_EQ_OS);
#elif defined(__SSE2__)
    return _mm_cmpeq_ps(x, y);
#else
    PFMsimd_i result;
    float* fx = (float*)&x;
    float* fy = (float*)&y;
    int32_t* presult = (int32_t*)&result;
    for (int_fast8_t i = 0; i < 2; ++i) {
        presult[i] = (fx[i] == fy[i]) ? 0xFFFFFFFF : 0x00000000;
    }
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdCmpNEQ_I32(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    __m256i eq = _mm256_cmpeq_epi32(x, y);
    __m256i neq = _mm256_xor_si256(eq, _mm256_set1_epi32(-1));
    return neq;
#elif defined(__SSE2__)
    __m128i eq = _mm_cmpeq_epi32(x, y);
    __m128i neq = _mm_xor_si128(eq, _mm_set1_epi32(-1));
    return neq;
#else
    PFMsimd_i result;
    int32_t* px = (int32_t*)&x;
    int32_t* py = (int32_t*)&y;
    int32_t* presult = (int32_t*)&result;
    for (int_fast8_t i = 0; i < 2; ++i) {
        presult[i] = (px[i] != py[i]) ? 0xFFFFFFFF : 0x00000000;
    }
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdCmpNEQ_F32(PFMsimd_f x, PFMsimd_f y)
{
#if defined(__AVX2__)
    return _mm256_cmp_ps(x, y, _CMP_NEQ_OS);
#elif defined(__SSE2__)
    return _mm_cmpneq_ps(x, y);
#else
    PFMsimd_i result;
    float* fx = (float*)&x;
    float* fy = (float*)&y;
    int32_t* presult = (int32_t*)&result;
    for (int_fast8_t i = 0; i < 2; ++i) {
        presult[i] = (fx[i] != fy[i]) ? 0xFFFFFFFF : 0x00000000;
    }
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdCmpLT_I32(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_cmpgt_epi32(y, x);
#elif defined(__SSE2__)
    return _mm_cmplt_epi32(x, y);
#else
    PFMsimd_i result;
    int32_t* px = (int32_t*)&x;
    int32_t* py = (int32_t*)&y;
    int32_t* presult = (int32_t*)&result;
    for (int_fast8_t i = 0; i < 2; ++i) {
        presult[i] = (px[i] < py[i]) ? 0xFFFFFFFF : 0x00000000;
    }
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdCmpLT_F32(PFMsimd_f x, PFMsimd_f y)
{
#if defined(__AVX2__)
    return _mm256_cmp_ps(x, y, _CMP_LT_OS);
#elif defined(__SSE2__)
    return _mm_cmplt_ps(x, y);
#else
    PFMsimd_i result;
    float* fx = (float*)&x;
    float* fy = (float*)&y;
    int32_t* presult = (int32_t*)&result;
    for (int_fast8_t i = 0; i < 2; ++i) {
        presult[i] = (fx[i] < fy[i]) ? 0xFFFFFFFF : 0x00000000;
    }
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdCmpGT_I32(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_cmpgt_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_cmpgt_epi32(x, y);
#else
    PFMsimd_i result;
    int32_t* px = (int32_t*)&x;
    int32_t* py = (int32_t*)&y;
    int32_t* presult = (int32_t*)&result;
    for (int_fast8_t i = 0; i < 2; ++i) {
        presult[i] = (px[i] > py[i]) ? 0xFFFFFFFF : 0x00000000;
    }
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdCmpGT_F32(PFMsimd_f x, PFMsimd_f y)
{
#if defined(__AVX2__)
    return _mm256_cmp_ps(x, y, _CMP_GT_OS);
#elif defined(__SSE2__)
    return _mm_cmpgt_ps(x, y);
#else
    PFMsimd_i result;
    float* fx = (float*)&x;
    float* fy = (float*)&y;
    int32_t* presult = (int32_t*)&result;
    for (int_fast8_t i = 0; i < 2; ++i) {
        presult[i] = (fx[i] > fy[i]) ? 0xFFFFFFFF : 0x00000000;
    }
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdCmpLE_I32(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_cmpgt_epi32(y, x);
#elif defined(__SSE2__)
    return _mm_cmplt_epi32(x, y);
#else
    PFMsimd_i result;
    int32_t* px = (int32_t*)&x;
    int32_t* py = (int32_t*)&y;
    int32_t* presult = (int32_t*)&result;
    for (int_fast8_t i = 0; i < 2; ++i) {
        presult[i] = (px[i] <= py[i]) ? 0xFFFFFFFF : 0x00000000;
    }
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdCmpLE_F32(PFMsimd_f x, PFMsimd_f y)
{
#if defined(__AVX2__)
    return _mm256_cmp_ps(x, y, _CMP_LE_OS);
#elif defined(__SSE2__)
    return _mm_cmple_ps(x, y);
#else
    PFMsimd_i result;
    float* fx = (float*)&x;
    float* fy = (float*)&y;
    int32_t* presult = (int32_t*)&result;
    for (int_fast8_t i = 0; i < 2; ++i) {
        presult[i] = (fx[i] <= fy[i]) ? 0xFFFFFFFF : 0x00000000;
    }
    return result;
#endif
}

PFM_API PFMsimd_i
pfmSimdCmpGE_I32(PFMsimd_i x, PFMsimd_i y)
{
#if defined(__AVX2__)
    return _mm256_cmpgt_epi32(x, y);
#elif defined(__SSE2__)
    return _mm_cmpgt_epi32(x, y);
#else
    PFMsimd_i result;
    int32_t* px = (int32_t*)&x;
    int32_t* py = (int32_t*)&y;
    int32_t* presult = (int32_t*)&result;
    for (int_fast8_t i = 0; i < 2; ++i) {
        presult[i] = (px[i] >= py[i]) ? 0xFFFFFFFF : 0x00000000;
    }
    return result;
#endif
}

PFM_API PFMsimd_f
pfmSimdCmpGE_F32(PFMsimd_f x, PFMsimd_f y)
{
#if defined(__AVX2__)
    return _mm256_cmp_ps(x, y, _CMP_GE_OS);
#elif defined(__SSE2__)
    return _mm_cmpge_ps(x, y);
#else
    PFMsimd_i result;
    float* fx = (float*)&x;
    float* fy = (float*)&y;
    int32_t* presult = (int32_t*)&result;
    for (int_fast8_t i = 0; i < 2; ++i) {
        presult[i] = (fx[i] >= fy[i]) ? 0xFFFFFFFF : 0x00000000;
    }
    return result;
#endif
}

/* 2D SIMD Vector functions definition */

PFM_API void
pfmSimdVec2Zero(PFMsimd_vec2 dst)
{
    dst[0] = pfmSimdSetZero_F32();
    dst[1] = pfmSimdSetZero_F32();
}

PFM_API void
pfmSimdVec2One(PFMsimd_vec2 dst, float v)
{
    dst[0] = pfmSimdSet1_F32(v);
    dst[1] = pfmSimdSet1_F32(v);
}

PFM_API void
pfmSimdVec2Set(PFMsimd_vec2 dst, float x, float y)
{
    dst[0] = pfmSimdSet1_F32(x);
    dst[1] = pfmSimdSet1_F32(y);
}

PFM_API void
pfmSimdVec2Load(PFMsimd_vec2 dst, const PFMvec2 src)
{
    dst[0] = pfmSimdSet1_F32(src[0]);
    dst[1] = pfmSimdSet1_F32(src[1]);
}

PFM_API void
pfmSimdVec2Copy(aPFMsimd_vec2 restrict dst, const aPFMsimd_vec2 restrict src)
{
    memcpy(dst, src, sizeof(PFMsimd_vec2));
}

PFM_API void
pfmSimdVec2Swap(aPFMsimd_vec2 restrict a, aPFMsimd_vec2 restrict b)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        PFMsimd_f tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

PFM_API void
pfmSimdVec2Neg(PFMsimd_vec2 dst, const PFMsimd_vec2 v)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = pfmSimdNeg_F32(v[i]);
    }
}

PFM_API void
pfmSimdVec2NegR(aPFMsimd_vec2 restrict dst, const PFMsimd_vec2 v)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = pfmSimdNeg_F32(v[i]);
    }
}

PFM_API void
pfmSimdVec2Add(PFMsimd_vec2 dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = pfmSimdAdd_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec2AddR(aPFMsimd_vec2 restrict dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = pfmSimdAdd_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec2Sub(PFMsimd_vec2 dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = pfmSimdSub_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec2SubR(aPFMsimd_vec2 restrict dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = pfmSimdSub_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec2Mul(PFMsimd_vec2 dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = pfmSimdMul_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec2MulR(aPFMsimd_vec2 restrict dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = pfmSimdMul_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec2Div(PFMsimd_vec2 dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = pfmSimdDiv_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec2DivR(aPFMsimd_vec2 restrict dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = pfmSimdDiv_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec2Offset(PFMsimd_vec2 dst, const PFMsimd_vec2 v, PFMsimd_f offset)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = pfmSimdAdd_F32(v[i], offset);
    }
}

PFM_API void
pfmSimdVec2OffsetR(aPFMsimd_vec2 restrict dst, const PFMsimd_vec2 v, PFMsimd_f offset)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = pfmSimdAdd_F32(v[i], offset);
    }
}

PFM_API void
pfmSimdVec2Scale(PFMsimd_vec2 dst, const PFMsimd_vec2 v, PFMsimd_f scale)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = pfmSimdMul_F32(v[i], scale);
    }
}

PFM_API void
pfmSimdVec2ScaleR(aPFMsimd_vec2 restrict dst, const PFMsimd_vec2 v, PFMsimd_f scale)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = pfmSimdMul_F32(v[i], scale);
    }
}

PFM_API void
pfmSimdVec2Normalize(PFMsimd_vec2 dst, const PFMsimd_vec2 v)
{
    // Calculate the sum of squares of elements
    PFMsimd_f squaredLength = pfmSimdAdd_F32(
        pfmSimdMul_F32(v[0], v[0]),
        pfmSimdMul_F32(v[1], v[1]));

    // Calculate the inverse of the square root of length squared
    PFMsimd_f invLength = pfmSimdRSqrt_F32(squaredLength);

    // Normalize vectors
    dst[0] = pfmSimdMul_F32(v[0], invLength);
    dst[1] = pfmSimdMul_F32(v[1], invLength);
}

PFM_API void
pfmSimdVec2NormalizeR(aPFMsimd_vec2 restrict dst, const PFMsimd_vec2 v)
{
    // Calculate the sum of squares of elements
    PFMsimd_f squaredLength = pfmSimdAdd_F32(
        pfmSimdMul_F32(v[0], v[0]),
        pfmSimdMul_F32(v[1], v[1]));

    // Calculate the inverse of the square root of length squared
    PFMsimd_f invLength = pfmSimdRSqrt_F32(squaredLength);

    // Normalize vectors
    dst[0] = pfmSimdMul_F32(v[0], invLength);
    dst[1] = pfmSimdMul_F32(v[1], invLength);
}

PFM_API PFMsimd_f
pfmSimdVec2Length(const PFMsimd_vec2 v)
{
    return pfmSimdSqrt_F32(pfmSimdAdd_F32(
        pfmSimdMul_F32(v[0], v[0]),
        pfmSimdMul_F32(v[1], v[1])));
}

PFM_API PFMsimd_f
pfmSimdVec2LengthSq(const PFMsimd_vec2 v)
{
    return pfmSimdAdd_F32(
        pfmSimdMul_F32(v[0], v[0]),
        pfmSimdMul_F32(v[1], v[1]));
}

PFM_API PFMsimd_f
pfmSimdVec2Dot(const PFMsimd_vec2 v1, const PFMsimd_vec2 v2)
{
    return pfmSimdAdd_F32(
        pfmSimdMul_F32(v1[0], v2[0]),
        pfmSimdMul_F32(v1[1], v2[1]));
}

PFM_API PFMsimd_f
pfmSimdVec2Distance(const PFMsimd_vec2 v1, const PFMsimd_vec2 v2)
{
    // Calculate the differences between corresponding elements of the vectors
    PFMsimd_f dt0 = pfmSimdSub_F32(v1[0], v2[0]);
    PFMsimd_f dt1 = pfmSimdSub_F32(v1[1], v2[1]);

    // Calculate the squared differences
    PFMsimd_f dt0Sq = pfmSimdMul_F32(dt0, dt0);
    PFMsimd_f dt1Sq = pfmSimdMul_F32(dt1, dt1);

    // Sum the squared differences
    PFMsimd_f distanceSq = pfmSimdAdd_F32(dt0Sq, dt1Sq);

    // Calculate and return the square root of the sum of squared differences
    return pfmSimdSqrt_F32(distanceSq);
}

PFM_API PFMsimd_f
pfmSimdVec2DistanceSq(const PFMsimd_vec2 v1, const PFMsimd_vec2 v2)
{
    // Calculate the differences between corresponding elements of the vectors
    PFMsimd_f dt0 = pfmSimdSub_F32(v1[0], v2[0]);
    PFMsimd_f dt1 = pfmSimdSub_F32(v1[1], v2[1]);

    // Calculate the squared differences
    PFMsimd_f dt0Sq = pfmSimdMul_F32(dt0, dt0);
    PFMsimd_f dt1Sq = pfmSimdMul_F32(dt1, dt1);

    // Sum the squared differences and return the result
    return pfmSimdAdd_F32(dt0Sq, dt1Sq);
}

PFM_API void
pfmSimdVec2Direction(PFMsimd_vec2 dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2)
{
    // Calculate the differences between the elements of the two vectors
    PFMsimd_f tmp0 = pfmSimdSub_F32(v1[0], v2[0]);
    PFMsimd_f tmp1 = pfmSimdSub_F32(v1[1], v2[1]);

    // Calculate the sum of the squares of these differences to obtain the length squared
    PFMsimd_f lengthSq = pfmSimdAdd_F32(pfmSimdMul_F32(tmp0, tmp0), pfmSimdMul_F32(tmp1, tmp1));

    // Calculate the inverse of the square root of the length squared to normalize the differences
    PFMsimd_f invLength = pfmSimdRSqrt_F32(lengthSq);

    // Multiply each difference by this inverse to obtain the normalized direction
    dst[0] = pfmSimdMul_F32(tmp0, invLength);
    dst[1] = pfmSimdMul_F32(tmp1, invLength);
}

PFM_API void
pfmSimdVec2DirectionR(aPFMsimd_vec2 restrict dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2)
{
    // Calculate the differences between the elements of the two vectors
    dst[0] = pfmSimdSub_F32(v1[0], v2[0]);
    dst[1] = pfmSimdSub_F32(v1[1], v2[1]);

    // Calculate the sum of the squares of these differences to obtain the length squared
    PFMsimd_f lengthSq = pfmSimdAdd_F32(pfmSimdMul_F32(dst[0], dst[0]), pfmSimdMul_F32(dst[1], dst[1]));

    // Calculate the inverse of the square root of the length squared to normalize the differences
    PFMsimd_f invLength = pfmSimdRSqrt_F32(lengthSq);

    // Multiply each difference by this inverse to obtain the normalized direction
    dst[0] = pfmSimdMul_F32(dst[0], invLength);
    dst[1] = pfmSimdMul_F32(dst[1], invLength);
}

PFM_API void
pfmSimdVec2Lerp(PFMsimd_vec2 dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2, PFMsimd_f t)
{
    dst[0] = pfmSimdAdd_F32(v1[0], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[0], v1[0])));
    dst[1] = pfmSimdAdd_F32(v1[1], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[1], v1[1])));
}

PFM_API void
pfmSimdVec2LerpR(aPFMsimd_vec2 restrict dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2, PFMsimd_f t)
{
    // Calculate the difference (v2 - v1)
    PFMsimd_f diff0 = pfmSimdSub_F32(v2[0], v1[0]);
    PFMsimd_f diff1 = pfmSimdSub_F32(v2[1], v1[1]);

    // Multiply the difference by t
    PFMsimd_f t_diff0 = pfmSimdMul_F32(t, diff0);
    PFMsimd_f t_diff1 = pfmSimdMul_F32(t, diff1);

    // Add the result to v1 to get the interpolation result
    dst[0] = pfmSimdAdd_F32(v1[0], t_diff0);
    dst[1] = pfmSimdAdd_F32(v1[1], t_diff1);
}

PFM_API void
pfmSimdVec2BaryInterp(PFMsimd_vec2 dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2, const PFMsimd_vec2 v3, PFMsimd_f w1, PFMsimd_f w2, PFMsimd_f w3)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        PFMsimd_f v1_w1 = pfmSimdMul_F32(v1[i], w1);
        PFMsimd_f v2_w2 = pfmSimdMul_F32(v2[i], w2);
        PFMsimd_f v3_w3 = pfmSimdMul_F32(v3[i], w3);
        dst[i] = pfmSimdAdd_F32(pfmSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}


PFM_API void
pfmSimdVec2BaryInterpR(aPFMsimd_vec2 restrict dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2, const PFMsimd_vec2 v3, PFMsimd_f w1, PFMsimd_f w2, PFMsimd_f w3)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        PFMsimd_f v1_w1 = pfmSimdMul_F32(v1[i], w1);
        PFMsimd_f v2_w2 = pfmSimdMul_F32(v2[i], w2);
        PFMsimd_f v3_w3 = pfmSimdMul_F32(v3[i], w3);
        dst[i] = pfmSimdAdd_F32(pfmSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

PFM_API void
pfmSimdVec2BaryInterpV(PFMsimd_vec2 dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2, const PFMsimd_vec2 v3, const PFMsimd_vec3 w)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        PFMsimd_f v1_w1 = pfmSimdMul_F32(v1[i], w[0]);
        PFMsimd_f v2_w2 = pfmSimdMul_F32(v2[i], w[1]);
        PFMsimd_f v3_w3 = pfmSimdMul_F32(v3[i], w[2]);
        dst[i] = pfmSimdAdd_F32(pfmSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

PFM_API void
pfmSimdVec2BaryInterpVR(aPFMsimd_vec2 restrict dst, const PFMsimd_vec2 v1, const PFMsimd_vec2 v2, const PFMsimd_vec2 v3, const PFMsimd_vec3 w)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        PFMsimd_f v1_w1 = pfmSimdMul_F32(v1[i], w[0]);
        PFMsimd_f v2_w2 = pfmSimdMul_F32(v2[i], w[1]);
        PFMsimd_f v3_w3 = pfmSimdMul_F32(v3[i], w[2]);
        dst[i] = pfmSimdAdd_F32(pfmSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

PFM_API void pfmSimdVec2Transform(PFMsimd_vec2 dst, const PFMsimd_vec2 v, const float mat[16])
{
    // Load array elements into SIMD registers
    PFMsimd_f mat_col0 = pfmSimdSet1_F32(mat[0]);
    PFMsimd_f mat_col1 = pfmSimdSet1_F32(mat[1]);
    PFMsimd_f mat_col4 = pfmSimdSet1_F32(mat[4]);
    PFMsimd_f mat_col5 = pfmSimdSet1_F32(mat[5]);
    PFMsimd_f mat_col12 = pfmSimdSet1_F32(mat[12]);
    PFMsimd_f mat_col13 = pfmSimdSet1_F32(mat[13]);

    // Perform SIMD operations and store results in destination vector
    PFMsimd_f tmp0 = pfmSimdAdd_F32(pfmSimdAdd_F32(pfmSimdMul_F32(mat_col0, v[0]), pfmSimdMul_F32(mat_col4, v[1])), mat_col12);
    PFMsimd_f tmp1 = pfmSimdAdd_F32(pfmSimdAdd_F32(pfmSimdMul_F32(mat_col1, v[0]), pfmSimdMul_F32(mat_col5, v[1])), mat_col13);

    dst[0] = tmp0;
    dst[1] = tmp1;
}

PFM_API void
pfmSimdVec2TransformR(aPFMsimd_vec2 restrict dst, const PFMsimd_vec2 v, const float mat[16])
{
    // Load array elements into SIMD registers
    PFMsimd_f mat_col0 = pfmSimdSet1_F32(mat[0]);
    PFMsimd_f mat_col1 = pfmSimdSet1_F32(mat[1]);
    PFMsimd_f mat_col4 = pfmSimdSet1_F32(mat[4]);
    PFMsimd_f mat_col5 = pfmSimdSet1_F32(mat[5]);
    PFMsimd_f mat_col12 = pfmSimdSet1_F32(mat[12]);
    PFMsimd_f mat_col13 = pfmSimdSet1_F32(mat[13]);

    // Perform SIMD operations and store results in destination vector
    dst[0] = pfmSimdAdd_F32(pfmSimdAdd_F32(pfmSimdMul_F32(mat_col0, v[0]), pfmSimdMul_F32(mat_col4, v[1])), mat_col12);
    dst[1] = pfmSimdAdd_F32(pfmSimdAdd_F32(pfmSimdMul_F32(mat_col1, v[0]), pfmSimdMul_F32(mat_col5, v[1])), mat_col13);
}

PFM_API void
pfmSimdVec2TransformWT(PFMsimd_vec2 dst, const PFMsimd_vec2 v, float wTranslation, const float mat[16])
{
    // Load array elements into SIMD registers
    PFMsimd_f mat_col0 = pfmSimdSet1_F32(mat[0]);
    PFMsimd_f mat_col1 = pfmSimdSet1_F32(mat[1]);
    PFMsimd_f mat_col4 = pfmSimdSet1_F32(mat[4]);
    PFMsimd_f mat_col5 = pfmSimdSet1_F32(mat[5]);
    PFMsimd_f mat_col12 = pfmSimdSet1_F32(mat[12]);
    PFMsimd_f mat_col13 = pfmSimdSet1_F32(mat[13]);
    
    // Load wTranslation into a SIMD register
    PFMsimd_f wTrans = pfmSimdSet1_F32(wTranslation);

    // Perform SIMD operations and store results in destination vector
    PFMsimd_f tmp0 = pfmSimdAdd_F32(
                    pfmSimdAdd_F32(pfmSimdMul_F32(mat_col0, v[0]), pfmSimdMul_F32(mat_col4, v[1])), 
                    pfmSimdMul_F32(wTrans, mat_col12));
    PFMsimd_f tmp1 = pfmSimdAdd_F32(
                    pfmSimdAdd_F32(pfmSimdMul_F32(mat_col1, v[0]), pfmSimdMul_F32(mat_col5, v[1])), 
                    pfmSimdMul_F32(wTrans, mat_col13));

    dst[0] = tmp0;
    dst[1] = tmp1;
}


PFM_API void
pfmSimdVec2TransformWTR(aPFMsimd_vec2 restrict dst, const PFMsimd_vec2 v, float wTranslation, const float mat[16])
{
    // Load array elements into SIMD registers
    PFMsimd_f mat_col0 = pfmSimdSet1_F32(mat[0]);
    PFMsimd_f mat_col1 = pfmSimdSet1_F32(mat[1]);
    PFMsimd_f mat_col4 = pfmSimdSet1_F32(mat[4]);
    PFMsimd_f mat_col5 = pfmSimdSet1_F32(mat[5]);
    PFMsimd_f mat_col12 = pfmSimdSet1_F32(mat[12]);
    PFMsimd_f mat_col13 = pfmSimdSet1_F32(mat[13]);
    
    // Load wTranslation into a SIMD register
    PFMsimd_f wTrans = pfmSimdSet1_F32(wTranslation);

    // Perform SIMD operations and store results in destination vector
    dst[0] = pfmSimdAdd_F32(
                pfmSimdAdd_F32(pfmSimdMul_F32(mat_col0, v[0]), pfmSimdMul_F32(mat_col4, v[1])), 
                pfmSimdMul_F32(wTrans, mat_col12));
    dst[1] = pfmSimdAdd_F32(
                pfmSimdAdd_F32(pfmSimdMul_F32(mat_col1, v[0]), pfmSimdMul_F32(mat_col5, v[1])), 
                pfmSimdMul_F32(wTrans, mat_col13));
}

/* 3D SIMD Vector functions definition */

PFM_API void
pfmSimdVec3Zero(PFMsimd_vec3 dst)
{
    dst[0] = pfmSimdSetZero_F32();
    dst[1] = pfmSimdSetZero_F32();
    dst[2] = pfmSimdSetZero_F32();
}

PFM_API void
pfmSimdVec3One(PFMsimd_vec3 dst, float v)
{
    dst[0] = pfmSimdSet1_F32(v);
    dst[1] = pfmSimdSet1_F32(v);
    dst[2] = pfmSimdSet1_F32(v);
}

PFM_API void
pfmSimdVec3Set(PFMsimd_vec3 dst, float x, float y, float z)
{
    dst[0] = pfmSimdSet1_F32(x);
    dst[1] = pfmSimdSet1_F32(y);
    dst[2] = pfmSimdSet1_F32(z);
}

PFM_API void
pfmSimdVec3Load(PFMsimd_vec3 dst, const PFMvec3 src)
{
    dst[0] = pfmSimdSet1_F32(src[0]);
    dst[1] = pfmSimdSet1_F32(src[1]);
    dst[2] = pfmSimdSet1_F32(src[2]);
}

PFM_API void
pfmSimdVec3Copy(aPFMsimd_vec3 restrict dst, const aPFMsimd_vec3 restrict src)
{
    memcpy(dst, src, sizeof(PFMsimd_vec3));
}

PFM_API void
pfmSimdVec3Swap(aPFMsimd_vec3 restrict a, aPFMsimd_vec3 restrict b)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        PFMsimd_f tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

PFM_API void
pfmSimdVec3Neg(PFMsimd_vec3 dst, const PFMsimd_vec3 v)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdNeg_F32(v[i]);
    }
}

PFM_API void
pfmSimdVec3NegR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdNeg_F32(v[i]);
    }
}

PFM_API void
pfmSimdVec3Add(PFMsimd_vec3 dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdAdd_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec3AddR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdAdd_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec3Sub(PFMsimd_vec3 dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdSub_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec3SubR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdSub_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec3Mul(PFMsimd_vec3 dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdMul_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec3MulR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdMul_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec3Div(PFMsimd_vec3 dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdDiv_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec3DivR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdDiv_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec3Offset(PFMsimd_vec3 dst, const PFMsimd_vec3 v, PFMsimd_f offset)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdAdd_F32(v[i], offset);
    }
}

PFM_API void
pfmSimdVec3OffsetR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v, PFMsimd_f offset)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdAdd_F32(v[i], offset);
    }
}

PFM_API void
pfmSimdVec3Scale(PFMsimd_vec3 dst, const PFMsimd_vec3 v, PFMsimd_f scale)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdMul_F32(v[i], scale);
    }
}

PFM_API void
pfmSimdVec3ScaleR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v, PFMsimd_f scale)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdMul_F32(v[i], scale);
    }
}

PFM_API void
pfmSimdVec3Normalize(PFMsimd_vec3 dst, const PFMsimd_vec3 v)
{
    // Calculate the sum of squares of elements
    PFMsimd_f squaredLength = pfmSimdAdd_F32(
        pfmSimdMul_F32(v[0], v[0]),
        pfmSimdMul_F32(v[1], v[1]));

    squaredLength = pfmSimdAdd_F32(squaredLength,
        pfmSimdMul_F32(v[2], v[2]));

    // Calculate the inverse of the square root of length squared
    PFMsimd_f invLength = pfmSimdRSqrt_F32(squaredLength);

    // Normalize vectors
    dst[0] = pfmSimdMul_F32(v[0], invLength);
    dst[1] = pfmSimdMul_F32(v[1], invLength);
    dst[2] = pfmSimdMul_F32(v[2], invLength);
}

PFM_API void
pfmSimdVec3NormalizeR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v)
{
    // Calculate the sum of squares of elements
    PFMsimd_f squaredLength = pfmSimdAdd_F32(
        pfmSimdMul_F32(v[0], v[0]),
        pfmSimdMul_F32(v[1], v[1]));

    squaredLength = pfmSimdAdd_F32(squaredLength,
        pfmSimdMul_F32(v[2], v[2]));

    // Calculate the inverse of the square root of length squared
    PFMsimd_f invLength = pfmSimdRSqrt_F32(squaredLength);

    // Normalize vectors
    dst[0] = pfmSimdMul_F32(v[0], invLength);
    dst[1] = pfmSimdMul_F32(v[1], invLength);
    dst[2] = pfmSimdMul_F32(v[2], invLength);
}

PFM_API PFMsimd_f
pfmSimdVec3Length(const PFMsimd_vec3 v)
{
    PFMsimd_f squaredLength = pfmSimdAdd_F32(
        pfmSimdMul_F32(v[0], v[0]),
        pfmSimdMul_F32(v[1], v[1]));

    squaredLength = pfmSimdAdd_F32(squaredLength,
        pfmSimdMul_F32(v[2], v[2]));

    return pfmSimdSqrt_F32(squaredLength);
}

PFM_API PFMsimd_f
pfmSimdVec3LengthSq(const PFMsimd_vec3 v)
{
    PFMsimd_f squaredLength = pfmSimdAdd_F32(
        pfmSimdMul_F32(v[0], v[0]),
        pfmSimdMul_F32(v[1], v[1]));

    return pfmSimdAdd_F32(squaredLength,
        pfmSimdMul_F32(v[2], v[2]));
}

PFM_API PFMsimd_f
pfmSimdVec3Dot(const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    PFMsimd_f dotProduct = pfmSimdAdd_F32(
        pfmSimdMul_F32(v1[0], v2[0]),
        pfmSimdMul_F32(v1[1], v2[1]));

    return pfmSimdAdd_F32(dotProduct,
        pfmSimdMul_F32(v1[2], v2[2]));
}

PFM_API void
pfmSimdVec3Cross(PFMsimd_vec3 dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    PFMsimd_vec3 tmp = { 0 };

    tmp[0] = pfmSimdSub_F32(
        pfmSimdMul_F32(v1[1], v2[2]),
        pfmSimdMul_F32(v1[2], v2[1]));

    tmp[1] = pfmSimdSub_F32(
        pfmSimdMul_F32(v1[2], v2[0]),
        pfmSimdMul_F32(v1[0], v2[2]));

    tmp[2] = pfmSimdSub_F32(
        pfmSimdMul_F32(v1[0], v2[1]),
        pfmSimdMul_F32(v1[1], v2[0]));

    memcpy(dst, tmp, sizeof(PFMsimd_vec3));
}

PFM_API void
pfmSimdVec3CrossR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    dst[0] = pfmSimdSub_F32(
        pfmSimdMul_F32(v1[1], v2[2]),
        pfmSimdMul_F32(v1[2], v2[1]));

    dst[1] = pfmSimdSub_F32(
        pfmSimdMul_F32(v1[2], v2[0]),
        pfmSimdMul_F32(v1[0], v2[2]));

    dst[2] = pfmSimdSub_F32(
        pfmSimdMul_F32(v1[0], v2[1]),
        pfmSimdMul_F32(v1[1], v2[0]));
}

PFM_API PFMsimd_f
pfmSimdVec3Distance(const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    // Calculate the differences between corresponding elements of the vectors
    PFMsimd_f dt0 = pfmSimdSub_F32(v1[0], v2[0]);
    PFMsimd_f dt1 = pfmSimdSub_F32(v1[1], v2[1]);;
    PFMsimd_f dt2 = pfmSimdSub_F32(v1[1], v2[1]);

    // Calculate the squared differences
    PFMsimd_f dt0Sq = pfmSimdMul_F32(dt0, dt0);
    PFMsimd_f dt1Sq = pfmSimdMul_F32(dt1, dt1);
    PFMsimd_f dt2Sq = pfmSimdMul_F32(dt2, dt2);

    // Sum the squared differences
    PFMsimd_f distanceSq = pfmSimdAdd_F32(dt0Sq,
        pfmSimdAdd_F32(dt1Sq, dt2Sq));

    // Calculate and return the square root of the sum of squared differences
    return pfmSimdSqrt_F32(distanceSq);
}

PFM_API PFMsimd_f
pfmSimdVec3DistanceSq(const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    // Calculate the differences between corresponding elements of the vectors
    PFMsimd_f dt0 = pfmSimdSub_F32(v1[0], v2[0]);
    PFMsimd_f dt1 = pfmSimdSub_F32(v1[1], v2[1]);;
    PFMsimd_f dt2 = pfmSimdSub_F32(v1[1], v2[1]);

    // Calculate the squared differences
    PFMsimd_f dt0Sq = pfmSimdMul_F32(dt0, dt0);
    PFMsimd_f dt1Sq = pfmSimdMul_F32(dt1, dt1);
    PFMsimd_f dt2Sq = pfmSimdMul_F32(dt2, dt2);

    // Sum the squared differences
    PFMsimd_f distanceSq = pfmSimdAdd_F32(dt0Sq,
        pfmSimdAdd_F32(dt1Sq, dt2Sq));

    // Calculate and return the square root of the sum of squared differences
    return distanceSq;
}

PFM_API void
pfmSimdVec3Direction(PFMsimd_vec3 dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    // Calculate the differences between the elements of the two vectors
    PFMsimd_f tmp0 = pfmSimdSub_F32(v1[0], v2[0]);
    PFMsimd_f tmp1 = pfmSimdSub_F32(v1[1], v2[1]);
    PFMsimd_f tmp2 = pfmSimdSub_F32(v1[2], v2[2]);

    // Calculate the sum of the squares of these differences to obtain the length squared
    PFMsimd_f lengthSq = pfmSimdAdd_F32(pfmSimdMul_F32(tmp0, tmp0), pfmSimdMul_F32(tmp1, tmp1));
    lengthSq = pfmSimdAdd_F32(lengthSq, pfmSimdMul_F32(tmp2, tmp2));

    // Add a small epsilon value to avoid division by zero
    lengthSq = pfmSimdMax_F32(lengthSq, pfmSimdSet1_F32(1e-8f));

    // Calculate the inverse of the square root of the length squared to normalize the differences
    PFMsimd_f invLength = pfmSimdRSqrt_F32(lengthSq);

    // Multiply each difference by this inverse to obtain the normalized direction
    dst[0] = pfmSimdMul_F32(tmp0, invLength);
    dst[1] = pfmSimdMul_F32(tmp1, invLength);
    dst[2] = pfmSimdMul_F32(tmp2, invLength);
}

PFM_API void
pfmSimdVec3DirectionR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2)
{
    // Calculate the differences between the elements of the two vectors
    dst[0] = pfmSimdSub_F32(v1[0], v2[0]);
    dst[1] = pfmSimdSub_F32(v1[1], v2[1]);
    dst[2] = pfmSimdSub_F32(v1[2], v2[2]);

    // Calculate the sum of the squares of these differences to obtain the length squared
    PFMsimd_f lengthSq = pfmSimdAdd_F32(pfmSimdMul_F32(dst[0], dst[0]), pfmSimdMul_F32(dst[1], dst[1]));
    lengthSq = pfmSimdAdd_F32(lengthSq, pfmSimdMul_F32(dst[2], dst[2]));

    // Add a small epsilon value to avoid division by zero
    lengthSq = pfmSimdMax_F32(lengthSq, pfmSimdSet1_F32(1e-8f));

    // Calculate the inverse of the square root of the length squared to normalize the differences
    PFMsimd_f invLength = pfmSimdRSqrt_F32(lengthSq);

    // Multiply each difference by this inverse to obtain the normalized direction
    dst[0] = pfmSimdMul_F32(dst[0], invLength);
    dst[1] = pfmSimdMul_F32(dst[1], invLength);
    dst[2] = pfmSimdMul_F32(dst[2], invLength);
}

PFM_API void
pfmSimdVec3Lerp(PFMsimd_vec3 dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2, PFMsimd_f t)
{
    dst[0] = pfmSimdAdd_F32(v1[0], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[0], v1[0])));
    dst[1] = pfmSimdAdd_F32(v1[1], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[1], v1[1])));
    dst[2] = pfmSimdAdd_F32(v1[2], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[2], v1[2])));
}

PFM_API void
pfmSimdVec3LerpR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2, PFMsimd_f t)
{
    dst[0] = pfmSimdAdd_F32(v1[0], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[0], v1[0])));
    dst[1] = pfmSimdAdd_F32(v1[1], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[1], v1[1])));
    dst[2] = pfmSimdAdd_F32(v1[2], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[2], v1[2])));
}

PFM_API void
pfmSimdVec3BaryInterp(PFMsimd_vec3 dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2, const PFMsimd_vec3 v3, PFMsimd_f w1, PFMsimd_f w2, PFMsimd_f w3)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        PFMsimd_f v1_w1 = pfmSimdMul_F32(v1[i], w1);
        PFMsimd_f v2_w2 = pfmSimdMul_F32(v2[i], w2);
        PFMsimd_f v3_w3 = pfmSimdMul_F32(v3[i], w3);
        dst[i] = pfmSimdAdd_F32(pfmSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}


PFM_API void
pfmSimdVec3BaryInterpR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2, const PFMsimd_vec3 v3, PFMsimd_f w1, PFMsimd_f w2, PFMsimd_f w3)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        PFMsimd_f v1_w1 = pfmSimdMul_F32(v1[i], w1);
        PFMsimd_f v2_w2 = pfmSimdMul_F32(v2[i], w2);
        PFMsimd_f v3_w3 = pfmSimdMul_F32(v3[i], w3);
        dst[i] = pfmSimdAdd_F32(pfmSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

PFM_API void
pfmSimdVec3BaryInterpV(PFMsimd_vec3 dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2, const PFMsimd_vec3 v3, const PFMsimd_vec3 w)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        PFMsimd_f v1_w1 = pfmSimdMul_F32(v1[i], w[0]);
        PFMsimd_f v2_w2 = pfmSimdMul_F32(v2[i], w[1]);
        PFMsimd_f v3_w3 = pfmSimdMul_F32(v3[i], w[2]);
        dst[i] = pfmSimdAdd_F32(pfmSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

PFM_API void
pfmSimdVec3BaryInterpVR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v1, const PFMsimd_vec3 v2, const PFMsimd_vec3 v3, const PFMsimd_vec3 w)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        PFMsimd_f v1_w1 = pfmSimdMul_F32(v1[i], w[0]);
        PFMsimd_f v2_w2 = pfmSimdMul_F32(v2[i], w[1]);
        PFMsimd_f v3_w3 = pfmSimdMul_F32(v3[i], w[2]);
        dst[i] = pfmSimdAdd_F32(pfmSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

PFM_API void
pfmSimdVec3Transform(PFMsimd_vec3 dst, const PFMsimd_vec3 v, const float mat[16])
{
    // Charger les éléments de la matrice dans les registres SIMD
    PFMsimd_f mat_col0 = pfmSimdSet1_F32(mat[0]);
    PFMsimd_f mat_col1 = pfmSimdSet1_F32(mat[1]);
    PFMsimd_f mat_col2 = pfmSimdSet1_F32(mat[2]);
    PFMsimd_f mat_col4 = pfmSimdSet1_F32(mat[4]);
    PFMsimd_f mat_col5 = pfmSimdSet1_F32(mat[5]);
    PFMsimd_f mat_col6 = pfmSimdSet1_F32(mat[6]);
    PFMsimd_f mat_col8 = pfmSimdSet1_F32(mat[8]);
    PFMsimd_f mat_col9 = pfmSimdSet1_F32(mat[9]);
    PFMsimd_f mat_col10 = pfmSimdSet1_F32(mat[10]);
    PFMsimd_f mat_col12 = pfmSimdSet1_F32(mat[12]);
    PFMsimd_f mat_col13 = pfmSimdSet1_F32(mat[13]);
    PFMsimd_f mat_col14 = pfmSimdSet1_F32(mat[14]);

    // Calculer les composants du vecteur transformé
    PFMsimd_f tmp0 = pfmSimdAdd_F32(
        pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col0, v[0]), pfmSimdMul_F32(mat_col4, v[1])),
            pfmSimdMul_F32(mat_col8, v[2])
        ),
        mat_col12
    );

    PFMsimd_f tmp1 = pfmSimdAdd_F32(
        pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col1, v[0]), pfmSimdMul_F32(mat_col5, v[1])),
            pfmSimdMul_F32(mat_col9, v[2])
        ),
        mat_col13
    );

    PFMsimd_f tmp2 = pfmSimdAdd_F32(
        pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col2, v[0]), pfmSimdMul_F32(mat_col6, v[1])),
            pfmSimdMul_F32(mat_col10, v[2])
        ),
        mat_col14
    );

    // Stocker les résultats dans le vecteur destination
    dst[0] = tmp0;
    dst[1] = tmp1;
    dst[2] = tmp2;
}

PFM_API void
pfmSimdVec3TransformR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v, const float mat[16])
{
    PFMsimd_f mat_col0 = pfmSimdSet1_F32(mat[0]);
    PFMsimd_f mat_col1 = pfmSimdSet1_F32(mat[1]);
    PFMsimd_f mat_col2 = pfmSimdSet1_F32(mat[2]);
    PFMsimd_f mat_col4 = pfmSimdSet1_F32(mat[4]);
    PFMsimd_f mat_col5 = pfmSimdSet1_F32(mat[5]);
    PFMsimd_f mat_col6 = pfmSimdSet1_F32(mat[6]);
    PFMsimd_f mat_col8 = pfmSimdSet1_F32(mat[8]);
    PFMsimd_f mat_col9 = pfmSimdSet1_F32(mat[9]);
    PFMsimd_f mat_col10 = pfmSimdSet1_F32(mat[10]);
    PFMsimd_f mat_col12 = pfmSimdSet1_F32(mat[12]);
    PFMsimd_f mat_col13 = pfmSimdSet1_F32(mat[13]);
    PFMsimd_f mat_col14 = pfmSimdSet1_F32(mat[14]);

    dst[0] = pfmSimdAdd_F32(
        pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col0, v[0]), pfmSimdMul_F32(mat_col4, v[1])),
            pfmSimdMul_F32(mat_col8, v[2])
        ),
        mat_col12
    );

    dst[1] = pfmSimdAdd_F32(
        pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col1, v[0]), pfmSimdMul_F32(mat_col5, v[1])),
            pfmSimdMul_F32(mat_col9, v[2])
        ),
        mat_col13
    );

    dst[2] = pfmSimdAdd_F32(
        pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col2, v[0]), pfmSimdMul_F32(mat_col6, v[1])),
            pfmSimdMul_F32(mat_col10, v[2])
        ),
        mat_col14
    );
}

PFM_API void
pfmSimdVec3TransformWT(PFMsimd_vec3 dst, const PFMsimd_vec3 v, float wTranslation, const float mat[16])
{
    PFMsimd_f mat_col0 = pfmSimdSet1_F32(mat[0]);
    PFMsimd_f mat_col1 = pfmSimdSet1_F32(mat[1]);
    PFMsimd_f mat_col2 = pfmSimdSet1_F32(mat[2]);
    PFMsimd_f mat_col4 = pfmSimdSet1_F32(mat[4]);
    PFMsimd_f mat_col5 = pfmSimdSet1_F32(mat[5]);
    PFMsimd_f mat_col6 = pfmSimdSet1_F32(mat[6]);
    PFMsimd_f mat_col8 = pfmSimdSet1_F32(mat[8]);
    PFMsimd_f mat_col9 = pfmSimdSet1_F32(mat[9]);
    PFMsimd_f mat_col10 = pfmSimdSet1_F32(mat[10]);
    PFMsimd_f mat_col12 = pfmSimdSet1_F32(mat[12]);
    PFMsimd_f mat_col13 = pfmSimdSet1_F32(mat[13]);
    PFMsimd_f mat_col14 = pfmSimdSet1_F32(mat[14]);

    PFMsimd_f wTrans = pfmSimdSet1_F32(wTranslation);

    PFMsimd_f tmp0 = pfmSimdAdd_F32(
        pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col0, v[0]), pfmSimdMul_F32(mat_col4, v[1])),
            pfmSimdMul_F32(mat_col8, v[2])
        ),
        pfmSimdMul_F32(wTrans, mat_col12)
    );

    PFMsimd_f tmp1 = pfmSimdAdd_F32(
        pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col1, v[0]), pfmSimdMul_F32(mat_col5, v[1])),
            pfmSimdMul_F32(mat_col9, v[2])
        ),
        pfmSimdMul_F32(wTrans, mat_col13)
    );

    PFMsimd_f tmp2 = pfmSimdAdd_F32(
        pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col2, v[0]), pfmSimdMul_F32(mat_col6, v[1])),
            pfmSimdMul_F32(mat_col10, v[2])
        ),
        pfmSimdMul_F32(wTrans, mat_col14)
    );

    dst[0] = tmp0;
    dst[1] = tmp1;
    dst[2] = tmp2;
}

PFM_API void
pfmSimdVec3TransformWTR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 v, float wTranslation, const float mat[16])
{
    PFMsimd_f mat_col0 = pfmSimdSet1_F32(mat[0]);
    PFMsimd_f mat_col1 = pfmSimdSet1_F32(mat[1]);
    PFMsimd_f mat_col2 = pfmSimdSet1_F32(mat[2]);
    PFMsimd_f mat_col4 = pfmSimdSet1_F32(mat[4]);
    PFMsimd_f mat_col5 = pfmSimdSet1_F32(mat[5]);
    PFMsimd_f mat_col6 = pfmSimdSet1_F32(mat[6]);
    PFMsimd_f mat_col8 = pfmSimdSet1_F32(mat[8]);
    PFMsimd_f mat_col9 = pfmSimdSet1_F32(mat[9]);
    PFMsimd_f mat_col10 = pfmSimdSet1_F32(mat[10]);
    PFMsimd_f mat_col12 = pfmSimdSet1_F32(mat[12]);
    PFMsimd_f mat_col13 = pfmSimdSet1_F32(mat[13]);
    PFMsimd_f mat_col14 = pfmSimdSet1_F32(mat[14]);

    PFMsimd_f wTrans = pfmSimdSet1_F32(wTranslation);

    dst[0] = pfmSimdAdd_F32(
        pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col0, v[0]), pfmSimdMul_F32(mat_col4, v[1])),
            pfmSimdMul_F32(mat_col8, v[2])
        ),
        pfmSimdMul_F32(wTrans, mat_col12)
    );

    dst[1] = pfmSimdAdd_F32(
        pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col1, v[0]), pfmSimdMul_F32(mat_col5, v[1])),
            pfmSimdMul_F32(mat_col9, v[2])
        ),
        pfmSimdMul_F32(wTrans, mat_col13)
    );

    dst[2] = pfmSimdAdd_F32(
        pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col2, v[0]), pfmSimdMul_F32(mat_col6, v[1])),
            pfmSimdMul_F32(mat_col10, v[2])
        ),
        pfmSimdMul_F32(wTrans, mat_col14)
    );
}

PFM_API void
pfmSimdVec3Reflect(PFMsimd_vec3 dst, const PFMsimd_vec3 incident, const PFMsimd_vec3 normal)
{
    PFMsimd_f dotProduct = pfmSimdAdd_F32(
        pfmSimdAdd_F32(
            pfmSimdMul_F32(incident[0], normal[0]),
            pfmSimdMul_F32(incident[1], normal[1])
        ),
        pfmSimdMul_F32(incident[2], normal[2])
    );

    PFMsimd_f dotProduct2 = pfmSimdMul_F32(dotProduct, pfmSimdSet1_F32(2.0f));

    PFMsimd_f tmp0 = pfmSimdSub_F32(incident[0], pfmSimdMul_F32(dotProduct2, normal[0]));
    PFMsimd_f tmp1 = pfmSimdSub_F32(incident[1], pfmSimdMul_F32(dotProduct2, normal[1]));
    PFMsimd_f tmp2 = pfmSimdSub_F32(incident[2], pfmSimdMul_F32(dotProduct2, normal[2]));

    dst[0] = tmp0;
    dst[1] = tmp1;
    dst[2] = tmp2;
}

PFM_API void
pfmSimdVec3ReflectR(aPFMsimd_vec3 restrict dst, const PFMsimd_vec3 incident, const PFMsimd_vec3 normal)
{
    PFMsimd_f dotProduct = pfmSimdAdd_F32(
        pfmSimdAdd_F32(
            pfmSimdMul_F32(incident[0], normal[0]),
            pfmSimdMul_F32(incident[1], normal[1])
        ),
        pfmSimdMul_F32(incident[2], normal[2])
    );

    PFMsimd_f dotProduct2 = pfmSimdMul_F32(dotProduct, pfmSimdSet1_F32(2.0f));

    dst[0] = pfmSimdSub_F32(incident[0], pfmSimdMul_F32(dotProduct2, normal[0]));
    dst[1] = pfmSimdSub_F32(incident[1], pfmSimdMul_F32(dotProduct2, normal[1]));
    dst[2] = pfmSimdSub_F32(incident[2], pfmSimdMul_F32(dotProduct2, normal[2]));
}

/* 4D SIMD Vector functions definition */

PFM_API void
pfmSimdVec4Zero(PFMsimd_vec4 dst)
{
    dst[0] = pfmSimdSetZero_F32();
    dst[1] = pfmSimdSetZero_F32();
    dst[2] = pfmSimdSetZero_F32();
    dst[3] = pfmSimdSetZero_F32();
}

PFM_API void
pfmSimdVec4One(PFMsimd_vec4 dst, float v)
{
    dst[0] = pfmSimdSet1_F32(v);
    dst[1] = pfmSimdSet1_F32(v);
    dst[2] = pfmSimdSet1_F32(v);
    dst[3] = pfmSimdSet1_F32(v);
}

PFM_API void
pfmSimdVec4Set(PFMsimd_vec4 dst, float x, float y, float z, float w)
{
    dst[0] = pfmSimdSet1_F32(x);
    dst[1] = pfmSimdSet1_F32(y);
    dst[2] = pfmSimdSet1_F32(z);
    dst[4] = pfmSimdSet1_F32(w);
}

PFM_API void
pfmSimdVec4Load(PFMsimd_vec4 dst, const PFMvec4 src)
{
    dst[0] = pfmSimdSet1_F32(src[0]);
    dst[1] = pfmSimdSet1_F32(src[1]);
    dst[2] = pfmSimdSet1_F32(src[2]);
    dst[3] = pfmSimdSet1_F32(src[3]);
}

PFM_API void
pfmSimdVec4Copy(aPFMsimd_vec4 restrict dst, const aPFMsimd_vec4 restrict src)
{
    memcpy(dst, src, sizeof(PFMsimd_vec4));
}

PFM_API void
pfmSimdVec4Swap(aPFMsimd_vec4 restrict a, aPFMsimd_vec4 restrict b)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        PFMsimd_f tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

PFM_API void
pfmSimdVec4Neg(PFMsimd_vec4 dst, const PFMsimd_vec4 v)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = pfmSimdNeg_F32(v[i]);
    }
}

PFM_API void
pfmSimdVec4NegR(aPFMsimd_vec4 restrict dst, const PFMsimd_vec4 v)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = pfmSimdNeg_F32(v[i]);
    }
}

PFM_API void
pfmSimdVec4Add(PFMsimd_vec4 dst, const PFMsimd_vec4 v1, const PFMsimd_vec4 v2)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = pfmSimdAdd_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec4AddR(aPFMsimd_vec4 restrict dst, const PFMsimd_vec4 v1, const PFMsimd_vec4 v2)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = pfmSimdAdd_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec4Sub(PFMsimd_vec4 dst, const PFMsimd_vec4 v1, const PFMsimd_vec4 v2)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = pfmSimdSub_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec4SubR(aPFMsimd_vec4 restrict dst, const PFMsimd_vec4 v1, const PFMsimd_vec4 v2)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = pfmSimdSub_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec4Mul(PFMsimd_vec4 dst, const PFMsimd_vec4 v1, const PFMsimd_vec4 v2)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = pfmSimdMul_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec4MulR(aPFMsimd_vec4 restrict dst, const PFMsimd_vec4 v1, const PFMsimd_vec4 v2)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = pfmSimdMul_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec4Div(PFMsimd_vec4 dst, const PFMsimd_vec4 v1, const PFMsimd_vec4 v2)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = pfmSimdDiv_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec4DivR(aPFMsimd_vec4 restrict dst, const PFMsimd_vec4 v1, const PFMsimd_vec4 v2)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = pfmSimdDiv_F32(v1[i], v2[i]);
    }
}

PFM_API void
pfmSimdVec4Offset(PFMsimd_vec4 dst, const PFMsimd_vec4 v, PFMsimd_f offset)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdAdd_F32(v[i], offset);
    }
}

PFM_API void
pfmSimdVec4OffsetR(aPFMsimd_vec4 restrict dst, const PFMsimd_vec4 v, PFMsimd_f offset)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdAdd_F32(v[i], offset);
    }
}

PFM_API void
pfmSimdVec4Scale(PFMsimd_vec4 dst, const PFMsimd_vec4 v, PFMsimd_f scale)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdMul_F32(v[i], scale);
    }
}

PFM_API void
pfmSimdVec4ScaleR(aPFMsimd_vec4 restrict dst, const PFMsimd_vec4 v, PFMsimd_f scale)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = pfmSimdMul_F32(v[i], scale);
    }
}

PFM_API void
pfmSimdVec4Normalize(PFMsimd_vec4 dst, const PFMsimd_vec4 v)
{
    // Calculate the sum of squares of elements
    PFMsimd_f squaredLength = pfmSimdAdd_F32(
        pfmSimdMul_F32(v[0], v[0]),
        pfmSimdMul_F32(v[1], v[1]));

    squaredLength = pfmSimdAdd_F32(squaredLength,
        pfmSimdMul_F32(v[2], v[2]));

    squaredLength = pfmSimdAdd_F32(squaredLength,
        pfmSimdMul_F32(v[3], v[3]));

    // Calculate the inverse of the square root of length squared
    PFMsimd_f invLength = pfmSimdRSqrt_F32(squaredLength);

    // Normalize vectors
    dst[0] = pfmSimdMul_F32(v[0], invLength);
    dst[1] = pfmSimdMul_F32(v[1], invLength);
    dst[2] = pfmSimdMul_F32(v[2], invLength);
    dst[3] = pfmSimdMul_F32(v[3], invLength);
}

PFM_API void
pfmSimdVec4NormalizeR(aPFMsimd_vec4 restrict dst, const PFMsimd_vec4 v)
{
    // Calculate the sum of squares of elements
    PFMsimd_f squaredLength = pfmSimdAdd_F32(
        pfmSimdMul_F32(v[0], v[0]),
        pfmSimdMul_F32(v[1], v[1]));

    squaredLength = pfmSimdAdd_F32(squaredLength,
        pfmSimdMul_F32(v[2], v[2]));

    squaredLength = pfmSimdAdd_F32(squaredLength,
        pfmSimdMul_F32(v[3], v[3]));

    // Calculate the inverse of the square root of length squared
    PFMsimd_f invLength = pfmSimdRSqrt_F32(squaredLength);

    // Normalize vectors
    dst[0] = pfmSimdMul_F32(v[0], invLength);
    dst[1] = pfmSimdMul_F32(v[1], invLength);
    dst[2] = pfmSimdMul_F32(v[2], invLength);
    dst[3] = pfmSimdMul_F32(v[3], invLength);
}

PFM_API PFMsimd_f
pfmSimdVec4Length(const PFMsimd_vec4 v)
{
    PFMsimd_f squaredLength = pfmSimdAdd_F32(
        pfmSimdMul_F32(v[0], v[0]),
        pfmSimdMul_F32(v[1], v[1]));

    squaredLength = pfmSimdAdd_F32(squaredLength,
        pfmSimdMul_F32(v[2], v[2]));

    squaredLength = pfmSimdAdd_F32(squaredLength,
        pfmSimdMul_F32(v[3], v[3]));

    return pfmSimdSqrt_F32(squaredLength);
}

PFM_API PFMsimd_f
pfmSimdVec4LengthSq(const PFMsimd_vec4 v)
{
    PFMsimd_f squaredLength = pfmSimdAdd_F32(
        pfmSimdMul_F32(v[0], v[0]),
        pfmSimdMul_F32(v[1], v[1]));

    squaredLength = pfmSimdAdd_F32(squaredLength,
        pfmSimdMul_F32(v[2], v[2]));

    return pfmSimdAdd_F32(squaredLength,
        pfmSimdMul_F32(v[3], v[3]));
}

PFM_API PFMsimd_f
pfmSimdVec4Dot(const PFMsimd_vec4 v1, const PFMsimd_vec4 v2)
{
    PFMsimd_f dotProduct = pfmSimdAdd_F32(
        pfmSimdMul_F32(v1[0], v2[0]),
        pfmSimdMul_F32(v1[1], v2[1]));

    dotProduct = pfmSimdAdd_F32(dotProduct,
        pfmSimdMul_F32(v1[2], v2[2]));

    return pfmSimdAdd_F32(dotProduct,
        pfmSimdMul_F32(v1[3], v2[3]));
}

PFM_API void
pfmSimdVec4Lerp(PFMsimd_vec4 dst, const PFMsimd_vec4 v1, const PFMsimd_vec4 v2, PFMsimd_f t)
{
    dst[0] = pfmSimdAdd_F32(v1[0], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[0], v1[0])));
    dst[1] = pfmSimdAdd_F32(v1[1], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[1], v1[1])));
    dst[2] = pfmSimdAdd_F32(v1[2], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[2], v1[2])));
    dst[3] = pfmSimdAdd_F32(v1[3], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[3], v1[3])));
}

PFM_API void
pfmSimdVec4LerpR(aPFMsimd_vec4 restrict dst, const PFMsimd_vec4 v1, const PFMsimd_vec4 v2, PFMsimd_f t)
{
    dst[0] = pfmSimdAdd_F32(v1[0], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[0], v1[0])));
    dst[1] = pfmSimdAdd_F32(v1[1], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[1], v1[1])));
    dst[2] = pfmSimdAdd_F32(v1[2], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[2], v1[2])));
    dst[3] = pfmSimdAdd_F32(v1[3], pfmSimdMul_F32(t, pfmSimdSub_F32(v2[3], v1[3])));
}

PFM_API void
pfmSimdVec4BaryInterp(PFMsimd_vec4 dst, const PFMsimd_vec4 v1, const PFMsimd_vec4 v2, const PFMsimd_vec4 v3, PFMsimd_f w1, PFMsimd_f w2, PFMsimd_f w3)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        PFMsimd_f v1_w1 = pfmSimdMul_F32(v1[i], w1);
        PFMsimd_f v2_w2 = pfmSimdMul_F32(v2[i], w2);
        PFMsimd_f v3_w3 = pfmSimdMul_F32(v3[i], w3);
        dst[i] = pfmSimdAdd_F32(pfmSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

PFM_API void
pfmSimdVec4BaryInterpR(aPFMsimd_vec4 restrict dst, const PFMsimd_vec4 v1, const PFMsimd_vec4 v2, const PFMsimd_vec4 v3, PFMsimd_f w1, PFMsimd_f w2, PFMsimd_f w3)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        PFMsimd_f v1_w1 = pfmSimdMul_F32(v1[i], w1);
        PFMsimd_f v2_w2 = pfmSimdMul_F32(v2[i], w2);
        PFMsimd_f v3_w3 = pfmSimdMul_F32(v3[i], w3);
        dst[i] = pfmSimdAdd_F32(pfmSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

PFM_API void
pfmSimdVec4BaryInterpV(PFMsimd_vec4 dst, const PFMsimd_vec4 v1, const PFMsimd_vec4 v2, const PFMsimd_vec4 v3, const PFMsimd_vec3 w)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        PFMsimd_f v1_w1 = pfmSimdMul_F32(v1[i], w[0]);
        PFMsimd_f v2_w2 = pfmSimdMul_F32(v2[i], w[1]);
        PFMsimd_f v3_w3 = pfmSimdMul_F32(v3[i], w[2]);
        dst[i] = pfmSimdAdd_F32(pfmSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

PFM_API void
pfmSimdVec4BaryInterpVR(aPFMsimd_vec4 restrict dst, const PFMsimd_vec4 v1, const PFMsimd_vec4 v2, const PFMsimd_vec4 v3, const PFMsimd_vec3 w)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        PFMsimd_f v1_w1 = pfmSimdMul_F32(v1[i], w[0]);
        PFMsimd_f v2_w2 = pfmSimdMul_F32(v2[i], w[1]);
        PFMsimd_f v3_w3 = pfmSimdMul_F32(v3[i], w[2]);
        dst[i] = pfmSimdAdd_F32(pfmSimdAdd_F32(v1_w1, v2_w2), v3_w3);
    }
}

PFM_API void
pfmSimdVec4Transform(PFMsimd_vec4 dst, const PFMsimd_vec4 v, const float mat[16])
{
    PFMsimd_f mat_col0 = pfmSimdSet1_F32(mat[0]);
    PFMsimd_f mat_col1 = pfmSimdSet1_F32(mat[1]);
    PFMsimd_f mat_col2 = pfmSimdSet1_F32(mat[2]);
    PFMsimd_f mat_col3 = pfmSimdSet1_F32(mat[3]);
    PFMsimd_f mat_col4 = pfmSimdSet1_F32(mat[4]);
    PFMsimd_f mat_col5 = pfmSimdSet1_F32(mat[5]);
    PFMsimd_f mat_col6 = pfmSimdSet1_F32(mat[6]);
    PFMsimd_f mat_col7 = pfmSimdSet1_F32(mat[7]);
    PFMsimd_f mat_col8 = pfmSimdSet1_F32(mat[8]);
    PFMsimd_f mat_col9 = pfmSimdSet1_F32(mat[9]);
    PFMsimd_f mat_col10 = pfmSimdSet1_F32(mat[10]);
    PFMsimd_f mat_col11 = pfmSimdSet1_F32(mat[11]);
    PFMsimd_f mat_col12 = pfmSimdSet1_F32(mat[12]);
    PFMsimd_f mat_col13 = pfmSimdSet1_F32(mat[13]);
    PFMsimd_f mat_col14 = pfmSimdSet1_F32(mat[14]);
    PFMsimd_f mat_col15 = pfmSimdSet1_F32(mat[15]);

    PFMsimd_f tmp0 = pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col0, v[0]), pfmSimdMul_F32(mat_col4, v[1])),
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col8, v[2]), pfmSimdMul_F32(mat_col12, v[3])));

    PFMsimd_f tmp1 = pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col1, v[0]), pfmSimdMul_F32(mat_col5, v[1])),
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col9, v[2]), pfmSimdMul_F32(mat_col13, v[3])));

    PFMsimd_f tmp2 = pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col2, v[0]), pfmSimdMul_F32(mat_col6, v[1])),
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col10, v[2]), pfmSimdMul_F32(mat_col14, v[3])));

    PFMsimd_f tmp3 = pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col3, v[0]), pfmSimdMul_F32(mat_col7, v[1])),
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col11, v[2]), pfmSimdMul_F32(mat_col15, v[3])));

    dst[0] = tmp0;
    dst[1] = tmp1;
    dst[2] = tmp2;
    dst[3] = tmp3;
}

PFM_API void
pfmSimdVec4TransformR(aPFMsimd_vec4 restrict dst, const PFMsimd_vec4 v, const float mat[16])
{
    PFMsimd_f mat_col0 = pfmSimdSet1_F32(mat[0]);
    PFMsimd_f mat_col1 = pfmSimdSet1_F32(mat[1]);
    PFMsimd_f mat_col2 = pfmSimdSet1_F32(mat[2]);
    PFMsimd_f mat_col3 = pfmSimdSet1_F32(mat[3]);
    PFMsimd_f mat_col4 = pfmSimdSet1_F32(mat[4]);
    PFMsimd_f mat_col5 = pfmSimdSet1_F32(mat[5]);
    PFMsimd_f mat_col6 = pfmSimdSet1_F32(mat[6]);
    PFMsimd_f mat_col7 = pfmSimdSet1_F32(mat[7]);
    PFMsimd_f mat_col8 = pfmSimdSet1_F32(mat[8]);
    PFMsimd_f mat_col9 = pfmSimdSet1_F32(mat[9]);
    PFMsimd_f mat_col10 = pfmSimdSet1_F32(mat[10]);
    PFMsimd_f mat_col11 = pfmSimdSet1_F32(mat[11]);
    PFMsimd_f mat_col12 = pfmSimdSet1_F32(mat[12]);
    PFMsimd_f mat_col13 = pfmSimdSet1_F32(mat[13]);
    PFMsimd_f mat_col14 = pfmSimdSet1_F32(mat[14]);
    PFMsimd_f mat_col15 = pfmSimdSet1_F32(mat[15]);

    dst[0] = pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col0, v[0]), pfmSimdMul_F32(mat_col4, v[1])),
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col8, v[2]), pfmSimdMul_F32(mat_col12, v[3])));

    dst[1] = pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col1, v[0]), pfmSimdMul_F32(mat_col5, v[1])),
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col9, v[2]), pfmSimdMul_F32(mat_col13, v[3])));

    dst[2] = pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col2, v[0]), pfmSimdMul_F32(mat_col6, v[1])),
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col10, v[2]), pfmSimdMul_F32(mat_col14, v[3])));

    dst[3] = pfmSimdAdd_F32(
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col3, v[0]), pfmSimdMul_F32(mat_col7, v[1])),
            pfmSimdAdd_F32(pfmSimdMul_F32(mat_col11, v[2]), pfmSimdMul_F32(mat_col15, v[3])));
}

#endif //PFM_H
