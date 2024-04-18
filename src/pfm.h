#ifndef PFM_H
#define PFM_H

#include <stdint.h>
#include <string.h>
#include <math.h>

/* Defines and Macros */

#ifndef PFM_API
#   define PFM_API static inline
#endif //PFM_API

#ifdef PFM_DOUBLE
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

/* Types and Structs definitions */

typedef PFM_FLOAT PFvec2f[2];
typedef PFM_FLOAT PFvec3f[3];
typedef PFM_FLOAT PFvec4f[4];
typedef PFM_FLOAT PFmat4f[16];

/* 2D Vector functions definition */

PFM_API void pfVec2fCopy(PFvec2f dst, const PFvec2f src)
{
    memcpy(dst, src, sizeof(PFvec2f));
}

PFM_API void pfVec2fNeg(PFvec2f dst, const PFvec2f v)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = -v[i];
    }
}

PFM_API void pfVec2fAdd(PFvec2f dst, const PFvec2f v1, const PFvec2f v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void pfVec2fSub(PFvec2f dst, const PFvec2f v1, const PFvec2f v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void pfVec2fMul(PFvec2f dst, const PFvec2f v1, const PFvec2f v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void pfVec2fDiv(PFvec2f dst, const PFvec2f v1, const PFvec2f v2)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void pfVec2fScale(PFvec2f dst, const PFvec2f v, PFM_FLOAT scalar)
{
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void pfVec2fNormalize(PFvec2f dst, const PFvec2f v)
{
    PFM_FLOAT squaredLength = v[0]*v[0] + v[1]*v[1];
    if (squaredLength == 0.0f) return;

    PFM_FLOAT invLength = 1.0f / sqrtf(squaredLength);
    for (int_fast8_t i = 0; i < 2; i++)
    {
        dst[i] = v[i] * invLength;
    }
}

PFM_API PFM_FLOAT pfVec2fDot(const PFvec2f v1, const PFvec2f v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1];
}

PFM_API void pfVec2Transform(PFvec2f dst, const PFvec2f v, const PFmat4f mat)
{
    PFvec2f tmp = {
        mat[0]*v[0] + mat[4]*v[1] + mat[12],
        mat[1]*v[0] + mat[5]*v[1] + mat[13]
    };

    memcpy(dst, tmp, sizeof(PFvec2f));
}

/* 3D Vector functions definition */

PFM_API void pfVec3fCopy(PFvec3f dst, const PFvec3f src)
{
    memcpy(dst, src, sizeof(PFvec3f));
}

PFM_API void pfVec3fNeg(PFvec3f dst, const PFvec3f v)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = -v[i];
    }
}

PFM_API void pfVec3fAdd(PFvec3f dst, const PFvec3f v1, const PFvec3f v2)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void pfVec3fSub(PFvec3f dst, const PFvec3f v1, const PFvec3f v2)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void pfVec3fMul(PFvec3f dst, const PFvec3f v1, const PFvec3f v2)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void pfVec3fDiv(PFvec3f dst, const PFvec3f v1, const PFvec3f v2)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void pfVec3fScale(PFvec3f dst, const PFvec3f v, PFM_FLOAT scalar)
{
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void pfVec3fNormalize(PFvec3f dst, const PFvec3f v)
{
    PFM_FLOAT squaredLength = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    if (squaredLength == 0.0f) return;

    PFM_FLOAT invLength = 1.0f / sqrtf(squaredLength);
    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = v[i] * invLength;
    }
}

PFM_API PFM_FLOAT pfVec3fDot(const PFvec3f v1, const PFvec3f v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

PFM_API void pfVec3fCross(PFvec3f dst, const PFvec3f v1, const PFvec3f v2)
{
    PFvec3f tmp = {
        v1[1]*v2[2] - v1[2]*v2[1],
        v1[2]*v2[0] - v1[0]*v2[2],
        v1[0]*v2[1] - v1[1]*v2[0]
    };

    memcpy(dst, tmp, sizeof(PFvec3f));
}

PFM_API void pfVec3fTransform(PFvec3f dst, const PFvec3f v, const PFmat4f mat)
{
    PFvec3f tmp = {
        mat[0]*v[0] + mat[4]*v[1] + mat[8]*v[2] + mat[12],
        mat[1]*v[0] + mat[5]*v[1] + mat[9]*v[2] + mat[13],
        mat[2]*v[0] + mat[6]*v[1] + mat[10]*v[2] + mat[14]
    };

    memcpy(dst, tmp, sizeof(PFvec3f));
}

PFM_API void pfVec3fReflect(PFvec3f dst, const PFvec3f incident, const PFvec3f normal)
{
    PFM_FLOAT dotProduct = 2.0f*(
        incident[0]*normal[0] +
        incident[1]*normal[1] +
        incident[2]*normal[2]);

    for (int_fast8_t i = 0; i < 3; i++)
    {
        dst[i] = incident[i] - dotProduct*normal[i];
    }
}

/* 4D Vector functions definition */

PFM_API void pfVec4fCopy(PFvec4f dst, const PFvec4f src)
{
    memcpy(dst, src, sizeof(PFvec4f));
}

PFM_API void pfVec4fNeg(PFvec4f dst, const PFvec4f v)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = -v[i];
    }
}

PFM_API void pfVec4fAdd(PFvec4f dst, const PFvec4f v1, const PFvec4f v2)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void pfVec4fSub(PFvec4f dst, const PFvec4f v1, const PFvec4f v2)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void pfVec4fMul(PFvec4f dst, const PFvec4f v1, const PFvec4f v2)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void pfVec4fDiv(PFvec4f dst, const PFvec4f v1, const PFvec4f v2)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void pfVec4fScale(PFvec4f dst, const PFvec4f v, PFM_FLOAT scalar)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void pfVec4fNormalize(PFvec4f dst, const PFvec4f v)
{
    PFM_FLOAT squaredLength = v[0]*v[0] + v[1]*v[1] + v[2]*v[2] + v[3]*v[3];
    if (squaredLength == 0.0f) return;

    PFM_FLOAT invLength = 1.0f / sqrtf(squaredLength);
    for (int_fast8_t i = 0; i < 4; i++)
    {
        dst[i] = v[i] * invLength;
    }
}

PFM_API PFM_FLOAT pfVec4fDot(const PFvec4f v1, const PFvec4f v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2] + v1[3]*v2[3];
}

PFM_API void pfVec4fTransform(PFvec4f dst, const PFvec4f v, const PFmat4f mat)
{
    PFvec4f tmp = {
        mat[0]*v[0] + mat[4]*v[1] + mat[8]*v[2] + mat[12]*v[3],
        mat[1]*v[0] + mat[5]*v[1] + mat[9]*v[2] + mat[13]*v[3],
        mat[2]*v[0] + mat[6]*v[1] + mat[10]*v[2] + mat[14]*v[3],
        mat[3]*v[0] + mat[7]*v[1] + mat[11]*v[2] + mat[15]*v[3]
    };

    memcpy(dst, tmp, sizeof(PFvec4f));
}

/* Matrix 4x4 functions definition */

PFM_API void pfMat4fCopy(PFmat4f dst, const PFmat4f src)
{
    memcpy(dst, src, sizeof(PFmat4f));
}

PFM_API PFM_FLOAT pfMat4fDeterminant(const PFmat4f mat)
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

PFM_API PFM_FLOAT pfMat4fTrace(const PFmat4f mat)
{
    return mat[0] + mat[5] + mat[10] + mat[15];
}

PFM_API void pfMat4fTranspose(PFmat4f dst, const PFmat4f src)
{
    for (int_fast8_t i = 0; i < 4; i++)
    {
        for (int_fast8_t j = 0; j < 4; j++)
        {
            dst[i * 4 + j] = src[j * 4 + i];
        }
    }
}

PFM_API void pfMat4fInvert(PFmat4f dst, const PFmat4f src)
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

PFM_API void pfMat4fIdentity(PFmat4f dst)
{
    memset(dst, 0, sizeof(PFmat4f));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;
}

PFM_API void pfMat4fAdd(PFmat4f dst, const PFmat4f left, const PFmat4f right)
{
    for (int_fast8_t i = 0; i < 16; i++)
    {
        dst[i] = left[i] + right[i];
    }
}

PFM_API void pfMat4fSub(PFmat4f dst, const PFmat4f left, const PFmat4f right)
{
    for (int_fast8_t i = 0; i < 16; i++)
    {
        dst[i] = left[i] + right[i];
    }
}

PFM_API void pfMat4fMul(PFmat4f dst, const PFmat4f left, const PFmat4f right)
{
    PFmat4f result;

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

    memcpy(dst, result, sizeof(PFmat4f));
}

PFM_API void pfMat4fTranslate(PFmat4f dst, PFM_FLOAT x, PFM_FLOAT y, PFM_FLOAT z)
{
    memset(dst, 0, sizeof(PFmat4f));
    dst[3] = x, dst[7] = y, dst[11] = z;
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;
}

// NOTE: Angle should be provided in radians
PFM_API void pfMat4fRotate(PFmat4f dst, const PFvec3f axis, PFM_FLOAT angle)
{
    // pfMat4fIdentity()
    memset(dst, 0, sizeof(PFmat4f));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    PFM_FLOAT x = axis[0], y = axis[1], z = axis[2];
    PFM_FLOAT lengthSquared = x*x + y*y + z*z;

    if ((lengthSquared != 1.0f) && (lengthSquared != 0.0f))
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
PFM_API void pfMat4fRotateX(PFmat4f dst, PFM_FLOAT angle)
{
    // pfMat4fIdentity()
    memset(dst, 0, sizeof(PFmat4f));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    PFM_FLOAT cosres = cosf(angle);
    PFM_FLOAT sinres = sinf(angle);

    dst[5]  = cosres;
    dst[6]  = sinres;
    dst[9]  = -sinres;
    dst[10] = cosres;
}

// NOTE: Angle must be provided in radians
PFM_API void pfMat4fRotateY(PFmat4f dst, PFM_FLOAT angle)
{
    // pfMat4fIdentity()
    memset(dst, 0, sizeof(PFmat4f));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    PFM_FLOAT cosres = cosf(angle);
    PFM_FLOAT sinres = sinf(angle);

    dst[0]  = cosres;
    dst[2]  = -sinres;
    dst[8]  = sinres;
    dst[10] = cosres;
}

// NOTE: Angle must be provided in radians
PFM_API void pfMat4fRotateZ(PFmat4f dst, PFM_FLOAT angle)
{
    // pfMat4fIdentity()
    memset(dst, 0, sizeof(PFmat4f));
    dst[0] = dst[5] = dst[10] = dst[15] = 1.0;

    PFM_FLOAT cosres = cosf(angle);
    PFM_FLOAT sinres = sinf(angle);

    dst[0] = cosres;
    dst[1] = sinres;
    dst[4] = -sinres;
    dst[5] = cosres;
}

// NOTE: Angle must be provided in radians
PFM_API void pfMat4fRotateXYZ(PFmat4f dst, const PFvec3f angle)
{
    // pfMat4fIdentity()
    memset(dst, 0, sizeof(PFmat4f));
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
PFM_API void pfMat4fRotateZYX(PFmat4f dst, const PFvec3f angle)
{
    // pfMat4fIdentity()
    memset(dst, 0, sizeof(PFmat4f));
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

PFM_API void pfMat4fScale(PFmat4f dst, PFM_FLOAT x, PFM_FLOAT y, PFM_FLOAT z)
{
    memset(dst, 0, sizeof(PFmat4f));
    dst[0] = x, dst[5] = y, dst[10] = z, dst[15] = 1.0;
}

PFM_API void pfMat4fFrustum(PFmat4f dst, PFM_FLOAT left, PFM_FLOAT right, PFM_FLOAT bottom, PFM_FLOAT top, PFM_FLOAT near, PFM_FLOAT far)
{
    memset(dst, 0, sizeof(PFmat4f));

    PFM_FLOAT rl = right - left;
    PFM_FLOAT tb = top - bottom;
    PFM_FLOAT fn = far - near;

    dst[0] = (near*2.0f)/rl;
    dst[5] = (near*2.0f)/tb;

    dst[8] = (right + left)/rl;
    dst[9] = (top + bottom)/tb;
    dst[10] = -(far + near)/fn;
    dst[11] = -1.0f;

    dst[14] = -(far*near*2.0f)/fn;
}

// NOTE: Fovy angle must be provided in radians
PFM_API void pfMat4fPerspective(PFmat4f dst, PFM_FLOAT fovY, PFM_FLOAT aspect, PFM_FLOAT nearPlane, PFM_FLOAT farPlane)
{
    memset(dst, 0, sizeof(PFmat4f));

    PFM_FLOAT top = nearPlane*tan(fovY*0.5);
    PFM_FLOAT bottom = -top;
    PFM_FLOAT right = top*aspect;
    PFM_FLOAT left = -right;

    // pfMat4fFrustum(-right, right, -top, top, near, far);
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

PFM_API void pfMat4fOrtho(PFmat4f dst, PFM_FLOAT left, PFM_FLOAT right, PFM_FLOAT bottom, PFM_FLOAT top, PFM_FLOAT nearPlane, PFM_FLOAT farPlane)
{
    memset(dst, 0, sizeof(PFmat4f));

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

PFM_API void pfMat4fLookAt(PFmat4f dst, const PFvec3f eye, const PFvec3f target, const PFvec3f up)
{
    memset(dst, 0, sizeof(PFmat4f));

    PFM_FLOAT length = 0.0f;
    PFM_FLOAT ilength = 0.0f;

    // pfVec3fSub(eye, target)
    PFvec3f vz = {
        eye[0] - target[0],
        eye[1] - target[1],
        eye[2] - target[2]
    };

    // pfVec3fNormalize(vz)
    PFvec3f v = { vz[0], vz[1], vz[2] };
    length = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    vz[0] *= ilength;
    vz[1] *= ilength;
    vz[2] *= ilength;

    // pfVec3Cross(up, vz)
    PFvec3f vx = {
        up[1]*vz[2] - up[2]*vz[1],
        up[2]*vz[0] - up[0]*vz[2],
        up[0]*vz[1] - up[1]*vz[0]
    };

    // pfVec3fNormalize(x)
    for (int_fast8_t i = 0; i < 3; i++) v[i] = vx[i];
    length = sqrtf(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    vx[0] *= ilength;
    vx[1] *= ilength;
    vx[2] *= ilength;

    // pfVec3Cross(vz, vx)
    PFvec3f vy = {
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