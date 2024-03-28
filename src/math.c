#include "pixelforge.h"
#include <math.h>

// PFvec2f

PFvec2f pfVec2fNeg(const PFvec2f* v)
{
    return (PFvec2f) {
        -v->x,
        -v->y
    };
}

PFvec2f pfVec2fAdd(const PFvec2f* v1, const PFvec2f* v2)
{
    return (PFvec2f) {
        v1->x + v2->x,
        v1->y + v2->y
    };
}

PFvec2f pfVec2fSub(const PFvec2f* v1, const PFvec2f* v2)
{
    return (PFvec2f) {
        v1->x - v2->x,
        v1->y - v2->y
    };
}

PFvec2f pfVec2fMul(const PFvec2f* v1, const PFvec2f* v2)
{
    return (PFvec2f) {
        v1->x*v2->x,
        v1->y*v2->y
    };
}

PFvec2f pfVec2fDiv(const PFvec2f* v1, const PFvec2f* v2)
{
    return (PFvec2f) {
        v2->x != 0.0f ? v1->x/v2->x : 0.0f,
        v2->y != 0.0f ? v1->y/v2->y : 0.0f
    };
}

PFvec2f pfVec2fScale(const PFvec2f* v, PFfloat scalar)
{
    return (PFvec2f) {
        v->x*scalar,
        v->y*scalar
    };
}

PFvec2f pfVec2fNormalize(const PFvec2f* v)
{
    PFfloat length = sqrt(v->x*v->x + v->y*v->y);

    if (length != 0.0f)
    {
        PFfloat invLength = 1.0f/length;

        return (PFvec2f) {
            v->x*invLength,
            v->y*invLength
        };
    }

    return *v;
}

PFfloat pfVec2fDot(const PFvec2f* v1, const PFvec2f* v2)
{
    return v1->x*v2->x + v1->y*v2->y;
}

PFvec2f pfVec2Transform(const PFvec2f* v, const PFmat4f* mat)
{
    return (PFvec2f) {
        mat->m0*v->x + mat->m4*v->y + mat->m12,
        mat->m1*v->x + mat->m5*v->y + mat->m13
    };
}

// PFvec3f

PFvec3f pfVec3fNeg(const PFvec3f* v)
{
    return (PFvec3f) { 
        -v->x,
        -v->y,
        -v->z
    };
}

PFvec3f pfVec3fAdd(const PFvec3f* v1, const PFvec3f* v2)
{
    return (PFvec3f) {
        v1->x + v2->x,
        v1->y + v2->y,
        v1->z + v2->z
    };
}

PFvec3f pfVec3fSub(const PFvec3f* v1, const PFvec3f* v2)
{
    return (PFvec3f) {
        v1->x - v2->x,
        v1->y - v2->y,
        v1->z - v2->z
    };
}

PFvec3f pfVec3fMul(const PFvec3f* v1, const PFvec3f* v2)
{
    return (PFvec3f) {
        v1->x*v2->x,
        v1->y*v2->y,
        v1->z*v2->z
    };
}

PFvec3f pfVec3fDiv(const PFvec3f* v1, const PFvec3f* v2)
{
    return (PFvec3f) {
        v2->x != 0.0f ? v1->x/v2->x : 0.0f,
        v2->y != 0.0f ? v1->y/v2->y : 0.0f,
        v2->z != 0.0f ? v1->z/v2->z : 0.0f
    };
}

PFvec3f pfVec3fScale(const PFvec3f* v, PFfloat scalar)
{
    return (PFvec3f) {
        v->x*scalar,
        v->y*scalar,
        v->z*scalar
    };
}

PFvec3f pfVec3fNormalize(const PFvec3f* v)
{
    PFfloat length = sqrt(v->x*v->x + v->y*v->y + v->z*v->z);

    if (length != 0.0f)
    {
        PFfloat invLength = 1.0f/length;

        return (PFvec3f) {
            v->x*invLength,
            v->y*invLength,
            v->z*invLength
        };
    }

    return *v;
}

PFfloat pfVec3fDot(const PFvec3f* v1, const PFvec3f* v2)
{
    return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;
}

PFvec3f pfVec3fCross(const PFvec3f* v1, const PFvec3f* v2)
{
    return (PFvec3f) {
        v1->y*v2->z - v1->z*v2->y,
        v1->z*v2->x - v1->x*v2->z,
        v1->x*v2->y - v1->y*v2->x
    };
}

PFvec3f pfVec3fTransform(const PFvec3f* v, const PFmat4f* mat)
{
    return (PFvec3f) {
        mat->m0*v->x + mat->m4*v->y + mat->m8*v->z + mat->m12,
        mat->m1*v->x + mat->m5*v->y + mat->m9*v->z + mat->m13,
        mat->m2*v->x + mat->m6*v->y + mat->m10*v->z + mat->m14
    };
}

PFvec3f pfVec3fReflect(const PFvec3f *incident, const PFvec3f *normal)
{
    PFfloat dotProduct = 2.0f * (
        incident->x*normal->x +
        incident->y*normal->y +
        incident->z*normal->z);

    return (PFvec3f) {
        incident->x - dotProduct*normal->x,
        incident->y - dotProduct*normal->y,
        incident->z - dotProduct*normal->z
    };
}

// PFvec4f

PFvec4f pfVec4fNeg(const PFvec4f* v)
{
    return (PFvec4f) {
        -v->x,
        -v->y,
        -v->z,
        -v->w
    };
}

PFvec4f pfVec4fAdd(const PFvec4f* v1, const PFvec4f* v2)
{
    return (PFvec4f) {
        v1->x + v2->x,
        v1->y + v2->y,
        v1->z + v2->z,
        v1->w + v2->w
    };
}

PFvec4f pfVec4fSub(const PFvec4f* v1, const PFvec4f* v2)
{
    return (PFvec4f) {
        v1->x - v2->x,
        v1->y - v2->y,
        v1->z - v2->z,
        v1->w - v2->w
    };
}

PFvec4f pfVec4fMul(const PFvec4f* v1, const PFvec4f* v2)
{
    return (PFvec4f) {
        v1->x*v2->x,
        v1->y*v2->y,
        v1->z*v2->z,
        v1->w*v2->w
    };
}

PFvec4f pfVec4fDiv(const PFvec4f* v1, const PFvec4f* v2)
{
    return (PFvec4f) {
        v2->x != 0.0f ? v1->x/v2->x : 0.0f,
        v2->y != 0.0f ? v1->y/v2->y : 0.0f,
        v2->z != 0.0f ? v1->z/v2->z : 0.0f,
        v2->w != 0.0f ? v1->w/v2->w : 0.0f
    };
}

PFvec4f pfVec4fScale(const PFvec4f* v, PFfloat scalar)
{
    return (PFvec4f) {
        v->x*scalar,
        v->y*scalar,
        v->z*scalar,
        v->w*scalar
    };
}

PFvec4f pfVec4fNormalize(const PFvec4f* v)
{
    PFfloat length = sqrt(v->x*v->x + v->y*v->y + v->z*v->z + v->w*v->w);

    if (length != 0.0f)
    {
        PFfloat invLength = 1.0f/length;

        return (PFvec4f) {
            v->x*invLength,
            v->y*invLength,
            v->z*invLength,
            v->w*invLength
        };        
    }

    return *v;
}

PFfloat pfVec4fDot(const PFvec4f* v1, const PFvec4f* v2)
{
    return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z + v1->w*v2->w;
}

PFvec4f pfVec4fTransform(const PFvec4f* v, const PFmat4f* mat)
{
    return (PFvec4f) {
        mat->m0*v->x + mat->m4*v->y + mat->m8*v->z + mat->m12*v->w,
        mat->m1*v->x + mat->m5*v->y + mat->m9*v->z + mat->m13*v->w,
        mat->m2*v->x + mat->m6*v->y + mat->m10*v->z + mat->m14*v->w,
        mat->m3*v->x + mat->m7*v->y + mat->m11*v->z + mat->m15*v->w
    };
}

// PFmat4f

PFfloat pfMat4fDeterminant(const PFmat4f* mat)
{
    PFfloat result = 0.0f;

    // Cache the matrix values (speed optimization)
    PFfloat a00 = mat->m0, a01 = mat->m1, a02 = mat->m2, a03 = mat->m3;
    PFfloat a10 = mat->m4, a11 = mat->m5, a12 = mat->m6, a13 = mat->m7;
    PFfloat a20 = mat->m8, a21 = mat->m9, a22 = mat->m10, a23 = mat->m11;
    PFfloat a30 = mat->m12, a31 = mat->m13, a32 = mat->m14, a33 = mat->m15;

    result = a30*a21*a12*a03 - a20*a31*a12*a03 - a30*a11*a22*a03 + a10*a31*a22*a03 +
             a20*a11*a32*a03 - a10*a21*a32*a03 - a30*a21*a02*a13 + a20*a31*a02*a13 +
             a30*a01*a22*a13 - a00*a31*a22*a13 - a20*a01*a32*a13 + a00*a21*a32*a13 +
             a30*a11*a02*a23 - a10*a31*a02*a23 - a30*a01*a12*a23 + a00*a31*a12*a23 +
             a10*a01*a32*a23 - a00*a11*a32*a23 - a20*a11*a02*a33 + a10*a21*a02*a33 +
             a20*a01*a12*a33 - a00*a21*a12*a33 - a10*a01*a22*a33 + a00*a11*a22*a33;

    return result;
}

PFfloat pfMat4fTrace(const PFmat4f* mat)
{
    PFfloat result = (mat->m0 + mat->m5 + mat->m10 + mat->m15);

    return result;
}

PFmat4f pfMat4fTranspose(const PFmat4f* mat)
{
    PFmat4f result = { 0 };

    result.m0 = mat->m0;
    result.m1 = mat->m4;
    result.m2 = mat->m8;
    result.m3 = mat->m12;
    result.m4 = mat->m1;
    result.m5 = mat->m5;
    result.m6 = mat->m9;
    result.m7 = mat->m13;
    result.m8 = mat->m2;
    result.m9 = mat->m6;
    result.m10 = mat->m10;
    result.m11 = mat->m14;
    result.m12 = mat->m3;
    result.m13 = mat->m7;
    result.m14 = mat->m11;
    result.m15 = mat->m15;

    return result;
}

PFmat4f pfMat4fInvert(const PFmat4f* mat)
{
    PFmat4f result = { 0 };

    // Cache the matrix values (speed optimization)
    PFfloat a00 = mat->m0, a01 = mat->m1, a02 = mat->m2, a03 = mat->m3;
    PFfloat a10 = mat->m4, a11 = mat->m5, a12 = mat->m6, a13 = mat->m7;
    PFfloat a20 = mat->m8, a21 = mat->m9, a22 = mat->m10, a23 = mat->m11;
    PFfloat a30 = mat->m12, a31 = mat->m13, a32 = mat->m14, a33 = mat->m15;

    PFfloat b00 = a00*a11 - a01*a10;
    PFfloat b01 = a00*a12 - a02*a10;
    PFfloat b02 = a00*a13 - a03*a10;
    PFfloat b03 = a01*a12 - a02*a11;
    PFfloat b04 = a01*a13 - a03*a11;
    PFfloat b05 = a02*a13 - a03*a12;
    PFfloat b06 = a20*a31 - a21*a30;
    PFfloat b07 = a20*a32 - a22*a30;
    PFfloat b08 = a20*a33 - a23*a30;
    PFfloat b09 = a21*a32 - a22*a31;
    PFfloat b10 = a21*a33 - a23*a31;
    PFfloat b11 = a22*a33 - a23*a32;

    // Calculate the invert determinant (inlined to avoid double-caching)
    PFfloat invDet = 1.0f/(b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);

    result.m0 = (a11*b11 - a12*b10 + a13*b09)*invDet;
    result.m1 = (-a01*b11 + a02*b10 - a03*b09)*invDet;
    result.m2 = (a31*b05 - a32*b04 + a33*b03)*invDet;
    result.m3 = (-a21*b05 + a22*b04 - a23*b03)*invDet;
    result.m4 = (-a10*b11 + a12*b08 - a13*b07)*invDet;
    result.m5 = (a00*b11 - a02*b08 + a03*b07)*invDet;
    result.m6 = (-a30*b05 + a32*b02 - a33*b01)*invDet;
    result.m7 = (a20*b05 - a22*b02 + a23*b01)*invDet;
    result.m8 = (a10*b10 - a11*b08 + a13*b06)*invDet;
    result.m9 = (-a00*b10 + a01*b08 - a03*b06)*invDet;
    result.m10 = (a30*b04 - a31*b02 + a33*b00)*invDet;
    result.m11 = (-a20*b04 + a21*b02 - a23*b00)*invDet;
    result.m12 = (-a10*b09 + a11*b07 - a12*b06)*invDet;
    result.m13 = (a00*b09 - a01*b07 + a02*b06)*invDet;
    result.m14 = (-a30*b03 + a31*b01 - a32*b00)*invDet;
    result.m15 = (a20*b03 - a21*b01 + a22*b00)*invDet;

    return result;
}

PFmat4f pfMat4fIdentity(void)
{
    return (PFmat4f) {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

PFmat4f pfMat4fAdd(const PFmat4f* left, const PFmat4f* right)
{
    PFmat4f result = { 0 };

    result.m0 = left->m0 + right->m0;
    result.m1 = left->m1 + right->m1;
    result.m2 = left->m2 + right->m2;
    result.m3 = left->m3 + right->m3;
    result.m4 = left->m4 + right->m4;
    result.m5 = left->m5 + right->m5;
    result.m6 = left->m6 + right->m6;
    result.m7 = left->m7 + right->m7;
    result.m8 = left->m8 + right->m8;
    result.m9 = left->m9 + right->m9;
    result.m10 = left->m10 + right->m10;
    result.m11 = left->m11 + right->m11;
    result.m12 = left->m12 + right->m12;
    result.m13 = left->m13 + right->m13;
    result.m14 = left->m14 + right->m14;
    result.m15 = left->m15 + right->m15;

    return result;
}

PFmat4f pfMat4fSub(const PFmat4f* left, const PFmat4f* right)
{
    PFmat4f result = { 0 };

    result.m0 = left->m0 - right->m0;
    result.m1 = left->m1 - right->m1;
    result.m2 = left->m2 - right->m2;
    result.m3 = left->m3 - right->m3;
    result.m4 = left->m4 - right->m4;
    result.m5 = left->m5 - right->m5;
    result.m6 = left->m6 - right->m6;
    result.m7 = left->m7 - right->m7;
    result.m8 = left->m8 - right->m8;
    result.m9 = left->m9 - right->m9;
    result.m10 = left->m10 - right->m10;
    result.m11 = left->m11 - right->m11;
    result.m12 = left->m12 - right->m12;
    result.m13 = left->m13 - right->m13;
    result.m14 = left->m14 - right->m14;
    result.m15 = left->m15 - right->m15;

    return result;
}

PFmat4f pfMat4fMul(const PFmat4f* left, const PFmat4f* right)
{
    PFmat4f result = { 0 };

    result.m0 = left->m0*right->m0 + left->m1*right->m4 + left->m2*right->m8 + left->m3*right->m12;
    result.m1 = left->m0*right->m1 + left->m1*right->m5 + left->m2*right->m9 + left->m3*right->m13;
    result.m2 = left->m0*right->m2 + left->m1*right->m6 + left->m2*right->m10 + left->m3*right->m14;
    result.m3 = left->m0*right->m3 + left->m1*right->m7 + left->m2*right->m11 + left->m3*right->m15;
    result.m4 = left->m4*right->m0 + left->m5*right->m4 + left->m6*right->m8 + left->m7*right->m12;
    result.m5 = left->m4*right->m1 + left->m5*right->m5 + left->m6*right->m9 + left->m7*right->m13;
    result.m6 = left->m4*right->m2 + left->m5*right->m6 + left->m6*right->m10 + left->m7*right->m14;
    result.m7 = left->m4*right->m3 + left->m5*right->m7 + left->m6*right->m11 + left->m7*right->m15;
    result.m8 = left->m8*right->m0 + left->m9*right->m4 + left->m10*right->m8 + left->m11*right->m12;
    result.m9 = left->m8*right->m1 + left->m9*right->m5 + left->m10*right->m9 + left->m11*right->m13;
    result.m10 = left->m8*right->m2 + left->m9*right->m6 + left->m10*right->m10 + left->m11*right->m14;
    result.m11 = left->m8*right->m3 + left->m9*right->m7 + left->m10*right->m11 + left->m11*right->m15;
    result.m12 = left->m12*right->m0 + left->m13*right->m4 + left->m14*right->m8 + left->m15*right->m12;
    result.m13 = left->m12*right->m1 + left->m13*right->m5 + left->m14*right->m9 + left->m15*right->m13;
    result.m14 = left->m12*right->m2 + left->m13*right->m6 + left->m14*right->m10 + left->m15*right->m14;
    result.m15 = left->m12*right->m3 + left->m13*right->m7 + left->m14*right->m11 + left->m15*right->m15;

    return result;
}

PFmat4f pfMat4fTranslate(PFfloat x, PFfloat y, PFfloat z)
{
    return (PFmat4f) {
        1.0f, 0.0f, 0.0f, x,
        0.0f, 1.0f, 0.0f, y,
        0.0f, 0.0f, 1.0f, z,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

// NOTE: Angle should be provided in radians
PFmat4f pfMat4fRotate(const PFvec3f* axis, PFfloat angle)
{
    PFmat4f result = { 0 };

    PFfloat x = axis->x, y = axis->y, z = axis->z;

    PFfloat lengthSquared = x*x + y*y + z*z;

    if ((lengthSquared != 1.0f) && (lengthSquared != 0.0f))
    {
        PFfloat ilength = 1.0f/sqrtf(lengthSquared);
        x *= ilength;
        y *= ilength;
        z *= ilength;
    }

    PFfloat sinres = sinf(angle);
    PFfloat cosres = cosf(angle);
    PFfloat t = 1.0f - cosres;

    result.m0 = x*x*t + cosres;
    result.m1 = y*x*t + z*sinres;
    result.m2 = z*x*t - y*sinres;
    result.m3 = 0.0f;

    result.m4 = x*y*t - z*sinres;
    result.m5 = y*y*t + cosres;
    result.m6 = z*y*t + x*sinres;
    result.m7 = 0.0f;

    result.m8 = x*z*t + y*sinres;
    result.m9 = y*z*t - x*sinres;
    result.m10 = z*z*t + cosres;
    result.m11 = 0.0f;

    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = 0.0f;
    result.m15 = 1.0f;

    return result;
}

// NOTE: Angle must be provided in radians
PFmat4f pfMat4fRotateX(PFfloat angle)
{
    PFmat4f result = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    }; // MatrixIdentity()

    PFfloat cosres = cosf(angle);
    PFfloat sinres = sinf(angle);

    result.m5 = cosres;
    result.m6 = sinres;
    result.m9 = -sinres;
    result.m10 = cosres;

    return result;
}

// NOTE: Angle must be provided in radians
PFmat4f pfMat4fRotateY(PFfloat angle)
{
    PFmat4f result = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    }; // MatrixIdentity()

    PFfloat cosres = cosf(angle);
    PFfloat sinres = sinf(angle);

    result.m0 = cosres;
    result.m2 = -sinres;
    result.m8 = sinres;
    result.m10 = cosres;

    return result;
}

// NOTE: Angle must be provided in radians
PFmat4f pfMat4fRotateZ(PFfloat angle)
{
    PFmat4f result = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    }; // MatrixIdentity()

    PFfloat cosres = cosf(angle);
    PFfloat sinres = sinf(angle);

    result.m0 = cosres;
    result.m1 = sinres;
    result.m4 = -sinres;
    result.m5 = cosres;

    return result;
}

// NOTE: Angle must be provided in radians
PFmat4f pfMat4fRotateXYZ(const PFvec3f* angle)
{
    PFmat4f result = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    }; // MatrixIdentity()

    PFfloat cosz = cosf(-angle->z);
    PFfloat sinz = sinf(-angle->z);
    PFfloat cosy = cosf(-angle->y);
    PFfloat siny = sinf(-angle->y);
    PFfloat cosx = cosf(-angle->x);
    PFfloat sinx = sinf(-angle->x);

    result.m0 = cosz*cosy;
    result.m1 = (cosz*siny*sinx) - (sinz*cosx);
    result.m2 = (cosz*siny*cosx) + (sinz*sinx);

    result.m4 = sinz*cosy;
    result.m5 = (sinz*siny*sinx) + (cosz*cosx);
    result.m6 = (sinz*siny*cosx) - (cosz*sinx);

    result.m8 = -siny;
    result.m9 = cosy*sinx;
    result.m10= cosy*cosx;

    return result;
}

// NOTE: Angle must be provided in radians
PFmat4f pfMat4fRotateZYX(const PFvec3f* angle)
{
    PFmat4f result = { 0 };

    PFfloat cz = cosf(angle->z);
    PFfloat sz = sinf(angle->z);
    PFfloat cy = cosf(angle->y);
    PFfloat sy = sinf(angle->y);
    PFfloat cx = cosf(angle->x);
    PFfloat sx = sinf(angle->x);

    result.m0 = cz*cy;
    result.m4 = cz*sy*sx - cx*sz;
    result.m8 = sz*sx + cz*cx*sy;
    result.m12 = 0;

    result.m1 = cy*sz;
    result.m5 = cz*cx + sz*sy*sx;
    result.m9 = cx*sz*sy - cz*sx;
    result.m13 = 0;

    result.m2 = -sy;
    result.m6 = cy*sx;
    result.m10 = cy*cx;
    result.m14 = 0;

    result.m3 = 0;
    result.m7 = 0;
    result.m11 = 0;
    result.m15 = 1;

    return result;
}

PFmat4f pfMat4fScale(PFfloat x, PFfloat y, PFfloat z)
{
    return (PFmat4f) {
        x, 0.0f, 0.0f, 0.0f,
        0.0f, y, 0.0f, 0.0f,
        0.0f, 0.0f, z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

PFmat4f pfMat4fFrustum(PFdouble left, PFdouble right, PFdouble bottom, PFdouble top, PFdouble near, PFdouble far)
{
    PFmat4f result = { 0 };

    PFfloat rl = (PFfloat)(right - left);
    PFfloat tb = (PFfloat)(top - bottom);
    PFfloat fn = (PFfloat)(far - near);

    result.m0 = ((PFfloat)near*2.0f)/rl;
    result.m1 = 0.0f;
    result.m2 = 0.0f;
    result.m3 = 0.0f;

    result.m4 = 0.0f;
    result.m5 = ((PFfloat)near*2.0f)/tb;
    result.m6 = 0.0f;
    result.m7 = 0.0f;

    result.m8 = ((PFfloat)right + (PFfloat)left)/rl;
    result.m9 = ((PFfloat)top + (PFfloat)bottom)/tb;
    result.m10 = -((PFfloat)far + (PFfloat)near)/fn;
    result.m11 = -1.0f;

    result.m12 = 0.0f;
    result.m13 = 0.0f;
    result.m14 = -((PFfloat)far*(PFfloat)near*2.0f)/fn;
    result.m15 = 0.0f;

    return result;
}

// NOTE: Fovy angle must be provided in radians
PFmat4f pfMat4fPerspective(PFdouble fovY, PFdouble aspect, PFdouble nearPlane, PFdouble farPlane)
{
    PFmat4f result = { 0 };

    PFdouble top = nearPlane*tan(fovY*0.5);
    PFdouble bottom = -top;
    PFdouble right = top*aspect;
    PFdouble left = -right;

    // pfMat4fFrustum(-right, right, -top, top, near, far);
    PFfloat rl = (PFfloat)(right - left);
    PFfloat tb = (PFfloat)(top - bottom);
    PFfloat fn = (PFfloat)(farPlane - nearPlane);

    result.m0 = ((PFfloat)nearPlane*2.0f)/rl;
    result.m5 = ((PFfloat)nearPlane*2.0f)/tb;
    result.m8 = ((PFfloat)right + (PFfloat)left)/rl;
    result.m9 = ((PFfloat)top + (PFfloat)bottom)/tb;
    result.m10 = -((PFfloat)farPlane + (PFfloat)nearPlane)/fn;
    result.m11 = -1.0f;
    result.m14 = -((PFfloat)farPlane*(PFfloat)nearPlane*2.0f)/fn;

    return result;
}

PFmat4f pfMat4fOrtho(PFdouble left, PFdouble right, PFdouble bottom, PFdouble top, PFdouble nearPlane, PFdouble farPlane)
{
    PFmat4f result = { 0 };

    PFfloat rl = (PFfloat)(right - left);
    PFfloat tb = (PFfloat)(top - bottom);
    PFfloat fn = (PFfloat)(farPlane - nearPlane);

    result.m0 = 2.0f/rl;
    result.m1 = 0.0f;
    result.m2 = 0.0f;
    result.m3 = 0.0f;
    result.m4 = 0.0f;
    result.m5 = 2.0f/tb;
    result.m6 = 0.0f;
    result.m7 = 0.0f;
    result.m8 = 0.0f;
    result.m9 = 0.0f;
    result.m10 = -2.0f/fn;
    result.m11 = 0.0f;
    result.m12 = -((PFfloat)left + (PFfloat)right)/rl;
    result.m13 = -((PFfloat)top + (PFfloat)bottom)/tb;
    result.m14 = -((PFfloat)farPlane + (PFfloat)nearPlane)/fn;
    result.m15 = 1.0f;

    return result;
}

PFmat4f pfMat4fLookAt(const PFvec3f* eye, const PFvec3f* target, const PFvec3f* up)
{
    PFmat4f result = { 0 };

    PFfloat length = 0.0f;
    PFfloat ilength = 0.0f;

    // pfVec3fSub(eye, target)
    PFvec3f vz = {
        eye->x - target->x,
        eye->y - target->y,
        eye->z - target->z
    };

    // pfVec3fNormalize(vz)
    PFvec3f v = vz;
    length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    vz.x *= ilength;
    vz.y *= ilength;
    vz.z *= ilength;

    // pfVec3Cross(up, vz)
    PFvec3f vx = {
        up->y*vz.z - up->z*vz.y,
        up->z*vz.x - up->x*vz.z,
        up->x*vz.y - up->y*vz.x
    };

    // pfVec3fNormalize(x)
    v = vx;
    length = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (length == 0.0f) length = 1.0f;
    ilength = 1.0f/length;
    vx.x *= ilength;
    vx.y *= ilength;
    vx.z *= ilength;

    // pfVec3Cross(vz, vx)
    PFvec3f vy = {
        vz.y*vx.z - vz.z*vx.y,
        vz.z*vx.x - vz.x*vx.z,
        vz.x*vx.y - vz.y*vx.x
    };

    result.m0 = vx.x;
    result.m1 = vy.x;
    result.m2 = vz.x;
    result.m3 = 0.0f;
    result.m4 = vx.y;
    result.m5 = vy.y;
    result.m6 = vz.y;
    result.m7 = 0.0f;
    result.m8 = vx.z;
    result.m9 = vy.z;
    result.m10 = vz.z;
    result.m11 = 0.0f;
    result.m12 = -(vx.x*eye->x + vx.y*eye->y + vx.z*eye->z);   // pfVec3Dot(vx, eye)
    result.m13 = -(vy.x*eye->x + vy.y*eye->y + vy.z*eye->z);   // pfVec3Dot(vy, eye)
    result.m14 = -(vz.x*eye->x + vz.y*eye->y + vz.z*eye->z);   // pfVec3Dot(vz, eye)
    result.m15 = 1.0f;

    return result;
}
