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

/* Defines and Macros */

#ifndef PFM_API
#   define PFM_API static inline
#endif //PFM_API

#ifdef PFM_USE_DOUBLE
#   define PFM_FLOAT double
#else
#   define PFM_FLOAT float
#endif

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

/* Types and Structs definitions */

typedef PFM_FLOAT PFMvec2[2];
typedef PFM_FLOAT PFMvec3[3];
typedef PFM_FLOAT PFMvec4[4];
typedef PFM_FLOAT PFMmat4[16];

/* 2D Vector functions definition */

PFM_API void pfmVec2Copy(PFMvec2 dst, const PFMvec2 src)
{
    memcpy(dst, src, sizeof(PFMvec2));
}

PFM_API void pfmVec2Neg(PFMvec2 dst, const PFMvec2 v)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = -v[i];
    }
}

PFM_API void pfmVec2Add(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void pfmVec2Sub(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void pfmVec2Mul(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void pfmVec2Div(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void pfmVec2Scale(PFMvec2 dst, const PFMvec2 v, PFM_FLOAT scalar)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void pfmVec2Normalize(PFMvec2 dst, const PFMvec2 v)
{
    PFM_FLOAT squaredLength = v[0]*v[0] + v[1]*v[1];
    if (squaredLength == 0.0f) return;

    PFM_FLOAT invLength = rsqrtf(squaredLength);
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v[i] * invLength;
    }
}

PFM_API PFM_FLOAT pfmVec2Dot(const PFMvec2 v1, const PFMvec2 v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1];
}

PFM_API void pfmVec2Transform(PFMvec2 dst, const PFMvec2 v, const PFMmat4 mat)
{
    PFMvec2 tmp = {
        mat[0]*v[0] + mat[4]*v[1] + mat[12],
        mat[1]*v[0] + mat[5]*v[1] + mat[13]
    };

    memcpy(dst, tmp, sizeof(PFMvec2));
}

/* 3D Vector functions definition */

PFM_API void pfmVec3Copy(PFMvec3 dst, const PFMvec3 src)
{
    memcpy(dst, src, sizeof(PFMvec3));
}

PFM_API void pfmVec3Neg(PFMvec3 dst, const PFMvec3 v)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = -v[i];
    }
}

PFM_API void pfmVec3Add(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void pfmVec3Sub(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void pfmVec3Mul(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void pfmVec3Div(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void pfmVec3Scale(PFMvec3 dst, const PFMvec3 v, PFM_FLOAT scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void pfmVec3Normalize(PFMvec3 dst, const PFMvec3 v)
{
    PFM_FLOAT squaredLength = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    if (squaredLength == 0.0f) return;

    PFM_FLOAT invLength = rsqrtf(squaredLength);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v[i] * invLength;
    }
}

PFM_API PFM_FLOAT pfmVec3Dot(const PFMvec3 v1, const PFMvec3 v2)
{
#ifdef _OPENMP
    PFM_FLOAT dotProduct = 0.0f;
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

PFM_API void pfmVec3Cross(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
    PFMvec3 tmp = {
        v1[1]*v2[2] - v1[2]*v2[1],
        v1[2]*v2[0] - v1[0]*v2[2],
        v1[0]*v2[1] - v1[1]*v2[0]
    };

    memcpy(dst, tmp, sizeof(PFMvec3));
}

PFM_API void pfmVec3Transform(PFMvec3 dst, const PFMvec3 v, const PFMmat4 mat)
{
    PFMvec3 tmp = {
        mat[0]*v[0] + mat[4]*v[1] + mat[8]*v[2] + mat[12],
        mat[1]*v[0] + mat[5]*v[1] + mat[9]*v[2] + mat[13],
        mat[2]*v[0] + mat[6]*v[1] + mat[10]*v[2] + mat[14]
    };

    memcpy(dst, tmp, sizeof(PFMvec3));
}

PFM_API void pfmVec3Reflect(PFMvec3 dst, const PFMvec3 incident, const PFMvec3 normal)
{
    PFM_FLOAT dotProduct = 0.0f;

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

/* 4D Vector functions definition */

PFM_API void pfmVec4Copy(PFMvec4 dst, const PFMvec4 src)
{
    memcpy(dst, src, sizeof(PFMvec4));
}

PFM_API void pfmVec4Neg(PFMvec4 dst, const PFMvec4 v)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = -v[i];
    }
}

PFM_API void pfmVec4Add(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void pfmVec4Sub(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void pfmVec4Mul(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void pfmVec4Div(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void pfmVec4Scale(PFMvec4 dst, const PFMvec4 v, PFM_FLOAT scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void pfmVec4Normalize(PFMvec4 dst, const PFMvec4 v)
{
    PFM_FLOAT squaredLength = v[0]*v[0] + v[1]*v[1] + v[2]*v[2] + v[3]*v[3];
    if (squaredLength == 0.0f) return;

    PFM_FLOAT invLength = rsqrtf(squaredLength);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v[i] * invLength;
    }
}

PFM_API PFM_FLOAT pfmVec4Dot(const PFMvec4 v1, const PFMvec4 v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2] + v1[3]*v2[3];
}

PFM_API void pfmVec4Transform(PFMvec4 dst, const PFMvec4 v, const PFMmat4 mat)
{
    PFMvec4 tmp = {
        mat[0]*v[0] + mat[4]*v[1] + mat[8]*v[2] + mat[12]*v[3],
        mat[1]*v[0] + mat[5]*v[1] + mat[9]*v[2] + mat[13]*v[3],
        mat[2]*v[0] + mat[6]*v[1] + mat[10]*v[2] + mat[14]*v[3],
        mat[3]*v[0] + mat[7]*v[1] + mat[11]*v[2] + mat[15]*v[3]
    };

    memcpy(dst, tmp, sizeof(PFMvec4));
}

/* Matrix 4x4 functions definition */

PFM_API void pfmMat4Copy(PFMmat4 dst, const PFMmat4 src)
{
    memcpy(dst, src, sizeof(PFMmat4));
}

PFM_API PFM_FLOAT pfmMat4Determinant(const PFMmat4 mat)
{
    PFM_FLOAT result = 0.0f;

    // Cache the matrix values (speed optimization)
    PFM_FLOAT a00 = mat[0],  a01 = mat[1],  a02 = mat[2],  a03 = mat[3];
    PFM_FLOAT a10 = mat[4],  a11 = mat[5],  a12 = mat[6],  a13 = mat[7];
    PFM_FLOAT a20 = mat[8],  a21 = mat[9],  a22 = mat[10], a23 = mat[11];
    PFM_FLOAT a30 = mat[12], a31 = mat[13], a32 = mat[14], a33 = mat[15];

    result = a30*a21*a12*a03 - a20*a31*a12*a03 - a30*a11*a22*a03 + a10*a31*a22*a03 +
             a20*a11*a32*a03 - a10*a21*a32*a03 - a30*a21*a02*a13 + a20*a31*a02*a13 +
             a30*a01*a22*a13 - a00*a31*a22*a13 - a20*a01*a32*a13 + a00*a21*a32*a13 +
             a30*a11*a02*a23 - a10*a31*a02*a23 - a30*a01*a12*a23 + a00*a31*a12*a23 +
             a10*a01*a32*a23 - a00*a11*a32*a23 - a20*a11*a02*a33 + a10*a21*a02*a33 +
             a20*a01*a12*a33 - a00*a21*a12*a33 - a10*a01*a22*a33 + a00*a11*a22*a33;

    return result;
}

PFM_API PFM_FLOAT pfmMat4Trace(const PFMmat4 mat)
{
    return mat[0] + mat[5] + mat[10] + mat[15];
}

PFM_API void pfmMat4Transpose(PFMmat4 dst, const PFMmat4 src)
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

PFM_API void pfmMat4Invert(PFMmat4 dst, const PFMmat4 src)
{
    // Cache the matrix values (speed optimization)
    PFM_FLOAT a00 = src[0],  a01 = src[1],  a02 = src[2],  a03 = src[3];
    PFM_FLOAT a10 = src[4],  a11 = src[5],  a12 = src[6],  a13 = src[7];
    PFM_FLOAT a20 = src[8],  a21 = src[9],  a22 = src[10], a23 = src[11];
    PFM_FLOAT a30 = src[12], a31 = src[13], a32 = src[14], a33 = src[15];

    PFM_FLOAT b00 = a00*a11 - a01*a10;
    PFM_FLOAT b01 = a00*a12 - a02*a10;
    PFM_FLOAT b02 = a00*a13 - a03*a10;
    PFM_FLOAT b03 = a01*a12 - a02*a11;
    PFM_FLOAT b04 = a01*a13 - a03*a11;
    PFM_FLOAT b05 = a02*a13 - a03*a12;
    PFM_FLOAT b06 = a20*a31 - a21*a30;
    PFM_FLOAT b07 = a20*a32 - a22*a30;
    PFM_FLOAT b08 = a20*a33 - a23*a30;
    PFM_FLOAT b09 = a21*a32 - a22*a31;
    PFM_FLOAT b10 = a21*a33 - a23*a31;
    PFM_FLOAT b11 = a22*a33 - a23*a32;

    // Calculate the invert determinant (inlined to avoid double-caching)
    PFM_FLOAT invDet = 1.0f/(b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);

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

PFM_API void pfmMat4Identity(PFMmat4 dst)
{
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;
}

PFM_API void pfmMat4Add(PFMmat4 dst, const PFMmat4 left, const PFMmat4 right)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 16; i++)
    {
        dst[i] = left[i] + right[i];
    }
}

PFM_API void pfmMat4Sub(PFMmat4 dst, const PFMmat4 left, const PFMmat4 right)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 16; i++)
    {
        dst[i] = left[i] + right[i];
    }
}

PFM_API void pfmMat4Mul(PFMmat4 dst, const PFMmat4 left, const PFMmat4 right)
{
    PFMmat4 result;

#   ifdef _OPENMP
#       pragma omp simd collapse(2)
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        for (int_fast8_t j = 0; j < 4; j++)
        {
            PFM_FLOAT sum = 0.0;

            for (int_fast8_t k = 0; k < 4; k++)
            {
                sum += left[i * 4 + k] * right[k * 4 + j];
            }

            result[i * 4 + j] = sum;
        }
    }

    memcpy(dst, result, sizeof(PFMmat4));
}

PFM_API void pfmMat4Translate(PFMmat4 dst, PFM_FLOAT x, PFM_FLOAT y, PFM_FLOAT z)
{
    memset(dst, 0, sizeof(PFMmat4));
    dst[12] = x, dst[13] = y, dst[14] = z;
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;
}

// NOTE: Angle should be provided in radians
PFM_API void pfmMat4Rotate(PFMmat4 dst, const PFMvec3 axis, PFM_FLOAT angle)
{
    // pfmMat4Identity()
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    PFM_FLOAT x = axis[0], y = axis[1], z = axis[2];
    PFM_FLOAT lengthSquared = x*x + y*y + z*z;

    if (lengthSquared != 1.0f && lengthSquared != 0.0f)
    {
        PFM_FLOAT ilength = 1.0f/sqrtf(lengthSquared);
        x *= ilength;
        y *= ilength;
        z *= ilength;
    }

    PFM_FLOAT sinres = sinf(angle);
    PFM_FLOAT cosres = cosf(angle);
    PFM_FLOAT t = 1.0f - cosres;

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
PFM_API void pfmMat4RotateX(PFMmat4 dst, PFM_FLOAT angle)
{
    // pfmMat4Identity()
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    PFM_FLOAT cosres = cosf(angle);
    PFM_FLOAT sinres = sinf(angle);

    dst[5]  = cosres;
    dst[6]  = sinres;
    dst[9]  = -sinres;
    dst[10] = cosres;
}

// NOTE: Angle must be provided in radians
PFM_API void pfmMat4RotateY(PFMmat4 dst, PFM_FLOAT angle)
{
    // pfmMat4Identity()
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    PFM_FLOAT cosres = cosf(angle);
    PFM_FLOAT sinres = sinf(angle);

    dst[0]  = cosres;
    dst[2]  = -sinres;
    dst[8]  = sinres;
    dst[10] = cosres;
}

// NOTE: Angle must be provided in radians
PFM_API void pfmMat4RotateZ(PFMmat4 dst, PFM_FLOAT angle)
{
    // pfmMat4Identity()
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    PFM_FLOAT cosres = cosf(angle);
    PFM_FLOAT sinres = sinf(angle);

    dst[0] = cosres;
    dst[1] = sinres;
    dst[4] = -sinres;
    dst[5] = cosres;
}

// NOTE: Angle must be provided in radians
PFM_API void pfmMat4RotateXYZ(PFMmat4 dst, const PFMvec3 angle)
{
    // pfmMat4Identity()
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    PFM_FLOAT cosz = cosf(-angle[2]);
    PFM_FLOAT sinz = sinf(-angle[2]);
    PFM_FLOAT cosy = cosf(-angle[1]);
    PFM_FLOAT siny = sinf(-angle[1]);
    PFM_FLOAT cosx = cosf(-angle[0]);
    PFM_FLOAT sinx = sinf(-angle[0]);

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
PFM_API void pfmMat4RotateZYX(PFMmat4 dst, const PFMvec3 angle)
{
    // pfmMat4Identity()
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    PFM_FLOAT cz = cosf(angle[2]);
    PFM_FLOAT sz = sinf(angle[2]);
    PFM_FLOAT cy = cosf(angle[1]);
    PFM_FLOAT sy = sinf(angle[1]);
    PFM_FLOAT cx = cosf(angle[0]);
    PFM_FLOAT sx = sinf(angle[0]);

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

PFM_API void pfmMat4Scale(PFMmat4 dst, PFM_FLOAT x, PFM_FLOAT y, PFM_FLOAT z)
{
    memset(dst, 0, sizeof(PFMmat4));
    dst[0] = x, dst[5] = y, dst[10] = z, dst[15] = 1.0;
}

PFM_API void pfmMat4Frustum(PFMmat4 dst, PFM_FLOAT left, PFM_FLOAT right, PFM_FLOAT bottom, PFM_FLOAT top, PFM_FLOAT nearPlane, PFM_FLOAT farPlane)
{
    memset(dst, 0, sizeof(PFMmat4));

    PFM_FLOAT rl = right - left;
    PFM_FLOAT tb = top - bottom;
    PFM_FLOAT fn = farPlane - nearPlane;

    dst[0] = (nearPlane*2.0f)/rl;
    dst[5] = (nearPlane*2.0f)/tb;

    dst[8] = (right + left)/rl;
    dst[9] = (top + bottom)/tb;
    dst[10] = -(farPlane + nearPlane)/fn;
    dst[11] = -1.0f;

    dst[14] = -(farPlane*nearPlane*2.0f)/fn;
}

// NOTE: Fovy angle must be provided in radians
PFM_API void pfmMat4Perspective(PFMmat4 dst, PFM_FLOAT fovY, PFM_FLOAT aspect, PFM_FLOAT nearPlane, PFM_FLOAT farPlane)
{
    memset(dst, 0, sizeof(PFMmat4));

    PFM_FLOAT top = nearPlane*tan(fovY*0.5);
    PFM_FLOAT bottom = -top;
    PFM_FLOAT right = top*aspect;
    PFM_FLOAT left = -right;

    // pfmMat4Frustum(-right, right, -top, top, near, far);
    PFM_FLOAT rl = right - left;
    PFM_FLOAT tb = top - bottom;
    PFM_FLOAT fn = farPlane - nearPlane;

    dst[0] = (nearPlane*2.0f)/rl;
    dst[5] = (nearPlane*2.0f)/tb;

    dst[8] = (right + left)/rl;
    dst[9] = (top + bottom)/tb;
    dst[10] = -(farPlane + nearPlane)/fn;
    dst[11] = -1.0f;

    dst[14] = -(farPlane*nearPlane*2.0f)/fn;
}

PFM_API void pfmMat4Ortho(PFMmat4 dst, PFM_FLOAT left, PFM_FLOAT right, PFM_FLOAT bottom, PFM_FLOAT top, PFM_FLOAT nearPlane, PFM_FLOAT farPlane)
{
    memset(dst, 0, sizeof(PFMmat4));

    PFM_FLOAT rl = (right - left);
    PFM_FLOAT tb = (top - bottom);
    PFM_FLOAT fn = (farPlane - nearPlane);

    dst[0] = 2.0f/rl;
    dst[5] = 2.0f/tb;

    dst[10] = -2.0f/fn;
    dst[11] = 0.0f;
    dst[12] = -(left + right)/rl;
    dst[13] = -(top + bottom)/tb;

    dst[14] = -(farPlane + nearPlane)/fn;
    dst[15] = 1.0f;
}

PFM_API void pfmMat4LookAt(PFMmat4 dst, const PFMvec3 eye, const PFMvec3 target, const PFMvec3 up)
{
    memset(dst, 0, sizeof(PFMmat4));

    PFM_FLOAT length = 0.0f;
    PFM_FLOAT ilength = 0.0f;

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
    ilength = 1.0f/length;
    vz[0] *= ilength;
    vz[1] *= ilength;
    vz[2] *= ilength;

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
    ilength = 1.0f/length;
    vx[0] *= ilength;
    vx[1] *= ilength;
    vx[2] *= ilength;

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

#endif //PFM_H