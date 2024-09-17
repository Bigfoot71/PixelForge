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

/* Customizable macros and definitions */

#ifndef PFM_API
#   define PFM_API static inline
#endif //PFM_API

#ifndef PFM_FISR
#   define rsqrtf(x) (1.0f / sqrtf(x))
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

#ifndef PFM_FX32_FRACTIONAL_BITS
#   define PFM_FX32_FRACTIONAL_BITS 16
#endif //PFM_FX32_FRACTIONAL_BITS

#ifndef PFM_FX16_FRACTIONAL_BITS
#   define PFM_FX16_FRACTIONAL_BITS 8
#endif //PFM_FX16_FRACTIONAL_BITS

/* Constant macros and definitions */

#define PFM_PI 3.14159265358979323846
#define PFM_TAU (2.0 * PFM_PI)

#define PFM_DEG2RAD (PFM_PI / 180.0)
#define PFM_RAD2DEG (180.0 / PFM_PI)

#define PFM_FX32_ONE (1 << PFM_FX32_FRACTIONAL_BITS)
#define PFM_FX16_ONE (1 << PFM_FX16_FRACTIONAL_BITS)
#define PFM_FRACT16_ONE 0xFFFF

/* Platform Specific */

#ifndef PFM_RESTRICT
#   ifdef _MSC_VER
#       define PFM_RESTRICT __restrict
#   else //ANY_PLATFORM
#       define PFM_RESTRICT restrict
#   endif //PLATFORM
#endif //PFM_RESTRICT

/* Types definitions */

typedef int32_t PFMfx32;
typedef int16_t PFMfx16;
typedef uint16_t PFMfract16;

typedef float PFMvec2[2];
typedef float PFMvec3[3];
typedef float PFMvec4[4];
typedef float PFMmat4[16];

typedef float* aPFMvec2;
typedef float* aPFMvec3;
typedef float* aPFMvec4;
typedef float* aPFMmat4;

/* Conversion Helpers */

/**
 * NOTE: Half/Float conversion code comes from Ogre (3D engine)
 * SOURCE: https://github.com/OGRECave/ogre/blob/master/OgreMain/include/OgreBitwise.h
 */

PFM_API uint16_t
pfmFloatToHalfI(uint32_t ui)
{
    int s = (ui >> 16) & 0x8000;
    int em = ui & 0x7fffffff;

    // bias exponent and round to nearest; 112 is relative exponent bias (127-15)
    int h = (em - (112 << 23) + (1 << 12)) >> 13;

    // underflow: flush to zero; 113 encodes exponent -14
    h = (em < (113 << 23)) ? 0 : h;

    // overflow: infinity; 143 encodes exponent 16
    h = (em >= (143 << 23)) ? 0x7c00 : h;

    // NaN; note that we convert all types of NaN to qNaN
    h = (em > (255 << 23)) ? 0x7e00 : h;

    return (uint16_t)(s | h);
}

PFM_API uint32_t
pfmHalfToFloatI(uint16_t h)
{
    uint32_t s = (unsigned)(h & 0x8000) << 16;
    int em = h & 0x7fff;

    // bias exponent and pad mantissa with 0; 112 is relative exponent bias (127-15)
    int r = (em + (112 << 10)) << 13;

    // denormal: flush to zero
    r = (em < (1 << 10)) ? 0 : r;

    // infinity/NaN; note that we preserve NaN payload as a byproduct of unifying inf/nan cases
    // 112 is an exponent bias fixup; since we already applied it once, applying it twice converts 31 to 255
    r += (em >= (31 << 10)) ? (112 << 23) : 0;

    return s | r;
}

PFM_API uint16_t
pfmFloatToHalf(float i)
{
    union { float f; uint32_t i; } v;
    v.f = i;
    return pfmFloatToHalfI(v.i);
}

PFM_API float
pfmHalfToFloat(uint16_t y)
{
    union { float f; uint32_t i; } v;
    v.i = pfmHalfToFloatI(y);
    return v.f;
}

/* Fixed point 32 bits */

PFM_API PFMfx32
pfmFloatToFX32(float x)
{
    return (PFMfx32)(x * (1 << PFM_FX32_FRACTIONAL_BITS));
}

PFM_API float
pfmFX32ToFloat(PFMfx32 x)
{
    return (float)x / (1 << PFM_FX32_FRACTIONAL_BITS);
}

PFM_API PFMfx32
pfmIntToFX32(int x)
{
    return (PFMfx32)(x << PFM_FX32_FRACTIONAL_BITS);
}

PFM_API int
pfmFX32ToInt(PFMfx32 x)
{
    return (int)(x >> PFM_FX32_FRACTIONAL_BITS);
}

PFM_API PFMfx32
pfmFX32Abs(PFMfx32 x)
{
    return x < 0 ? -x : x;
}

PFM_API PFMfx32
pfmFX32Round(PFMfx32 x)
{
    PFMfx32 fractionalPart = x & ((1 << PFM_FX32_FRACTIONAL_BITS) - 1);
    if (fractionalPart >= (1 << (PFM_FX32_FRACTIONAL_BITS - 1))) {
        return (x >> PFM_FX32_FRACTIONAL_BITS) + 1;
    } else {
        return x >> PFM_FX32_FRACTIONAL_BITS;
    }
}

PFM_API PFMfx32
pfmFX32Floor(PFMfx32 x)
{
    return x & ~((1 << PFM_FX32_FRACTIONAL_BITS) - 1);
}

PFM_API PFMfx32
pfmFX32Fract(PFMfx32 x)
{
    return x & ((1 << PFM_FX32_FRACTIONAL_BITS) - 1);
}

PFM_API PFMfx32
pfmFX32Mul(PFMfx32 x, PFMfx32 y)
{
    return (PFMfx32)(((int64_t)x * y) >> PFM_FX32_FRACTIONAL_BITS);
}

PFM_API PFMfx32
pfmFX32Div(PFMfx32 x, PFMfx32 y)
{
    return (PFMfx32)(((int64_t)x << PFM_FX32_FRACTIONAL_BITS) / y);
}

PFM_API PFMfx32 pfmFX32Sqrt(PFMfx32 x)
{
    if (x <= 0) return 0;

    // Initial estimate
    PFMfx32 r = (x >> 1) + (1 << (PFM_FX32_FRACTIONAL_BITS - 1));

    r = (r + pfmFX32Div(x, r)) >> 1; // An iteration of Newton-Raphson
    r = (r + pfmFX32Div(x, r)) >> 1; // Second iteration for more precision

    return r;
}

PFM_API PFMfx32 pfmFX32RSqrt(PFMfx32 x)
{
    if (x <= 0) return 0;

    // Initial estimate based on the "Fast Inverse Square Root" algorithm
    int32_t i = 0x5f3759df - (x >> 1);
    PFMfx32 r = *(PFMfx32*)&i;

    // An iteration of Newton-Raphson
    PFMfx32 halfx = x >> 1;
    r = pfmFX32Mul(r, 0x30000000 - pfmFX32Mul(halfx, pfmFX32Mul(r, r)));

    // Second iteration for more precision
    r = pfmFX32Mul(r, 0x30000000 - pfmFX32Mul(halfx, pfmFX32Mul(r, r)));

    return r;
}

/* Fixed point - 16 bits */

PFM_API PFMfx16
pfmFloatToFX16(float x)
{
    return (PFMfx16)(x * (1 << PFM_FX16_FRACTIONAL_BITS));
}

PFM_API float
pfmFX16ToFloat(PFMfx16 x)
{
    return (float)x / (1 << PFM_FX16_FRACTIONAL_BITS);
}

PFM_API PFMfx16
pfmIntToFX16(int x)
{
    return (PFMfx32)(x << PFM_FX16_FRACTIONAL_BITS);
}

PFM_API int
pfmFX16ToInt(PFMfx16 x)
{
    return (int)(x >> PFM_FX16_FRACTIONAL_BITS);
}

PFM_API PFMfx16
pfmFX16Abs(PFMfx16 x)
{
    return x < 0 ? -x : x;
}

PFM_API PFMfx16
pfmFX16Round(PFMfx16 x)
{
    PFMfx16 fractionalPart = x & ((1 << PFM_FX16_FRACTIONAL_BITS) - 1);
    if (fractionalPart >= (1 << (PFM_FX16_FRACTIONAL_BITS - 1))) {
        return (x >> PFM_FX16_FRACTIONAL_BITS) + 1;
    } else {
        return x >> PFM_FX16_FRACTIONAL_BITS;
    }
}

PFM_API PFMfx16
pfmFX16Floor(PFMfx16 x)
{
    return x & ~((1 << PFM_FX16_FRACTIONAL_BITS) - 1);
}

PFM_API PFMfx16
pfmFX16Fract(PFMfx16 x)
{
    return x & ((1 << PFM_FX16_FRACTIONAL_BITS) - 1);
}

PFM_API PFMfx16
pfmFX16Add(PFMfx16 x, PFMfx16 y)
{
    return x + y;
}

PFM_API PFMfx16
pfmFX16Sub(PFMfx16 x, PFMfx16 y)
{
    return x - y;
}

PFM_API PFMfx16
pfmFX16Mul(PFMfx16 x, PFMfx16 y)
{
    return (PFMfx16)(((int32_t)x * y) >> PFM_FX16_FRACTIONAL_BITS);
}

PFM_API PFMfx16
pfmFX16Div(PFMfx16 x, PFMfx16 y)
{
    return (PFMfx16)(((int32_t)x << PFM_FX16_FRACTIONAL_BITS) / y);
}

/* Fract type 16 bits */

PFM_API PFMfract16
pfmFloatToFract16(float x)
{
    if (x <= 0.0f) return 0;
    if (x >= 1.0f) return PFM_FRACT16_ONE;
    return (PFMfract16)(x * PFM_FRACT16_ONE + 0.5f);
}

PFM_API float
pfmFract16ToFloat(PFMfract16 x)
{
    return (float)x / PFM_FRACT16_ONE;
}

PFM_API PFMfract16
pfmFX16ToFract16(PFMfx16 x)
{
    if (x <= 0) return 0;
    if (x >= (1 << PFM_FX16_FRACTIONAL_BITS)) return PFM_FRACT16_ONE;
    return (PFMfract16)((uint32_t)x * PFM_FRACT16_ONE >> PFM_FX16_FRACTIONAL_BITS);
}

PFM_API PFMfx16
pfmFract16ToFX16(PFMfract16 x)
{
    return (PFMfx16)((uint32_t)x * (1 << PFM_FX16_FRACTIONAL_BITS) / PFM_FRACT16_ONE);
}

PFM_API PFMfract16
pfmFract16Add(PFMfract16 x, PFMfract16 y)
{
    uint32_t result = (uint32_t)x + y;
    return (result > PFM_FRACT16_ONE) ? PFM_FRACT16_ONE : (PFMfract16)result;
}

PFM_API PFMfract16
pfmFract16Sub(PFMfract16 x, PFMfract16 y)
{
    return (x > y) ? (x - y) : 0;
}

PFM_API PFMfract16
pfmFract16Mul(PFMfract16 x, PFMfract16 y)
{
    return (PFMfract16)(((uint32_t)x * y + PFM_FRACT16_ONE / 2) >> 16);
}

PFM_API PFMfract16
pfmFract16Div(PFMfract16 x, PFMfract16 y)
{
    return (PFMfract16)(((uint32_t)x << 16) / y);
}

/* Scalar functions */

PFM_API float
pfmClamp(float x, float min, float max)
{
    return x < min ? min : (x > max ? max : x);
}

PFM_API float
pfmSaturate(float x)
{
    return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
}

PFM_API float
pfmWrap(float value, float min, float max)
{
    float range = max - min;
    return min + fmod(value - min, range);
}

PFM_API float
pfmWrapAngle(float angle)
{
    float wrapped = fmod(angle, PFM_TAU);
    if (wrapped < -PFM_PI) {
        wrapped += PFM_TAU;
    } else if (wrapped > PFM_PI) {
        wrapped -= PFM_TAU;
    }
    return wrapped;
}

PFM_API float
pfmNormalize(float value, float start, float end)
{
    return (value - start) / (end - start);
}

PFM_API float
pfmRemap(float value, float input_start, float input_end, float output_start, float output_end)
{
    return (value - input_start) / (input_end - input_start) * (output_end - output_start) + output_start;
}

PFM_API float
pfmFract(float x)
{
    return x - floorf(x);
}

PFM_API float
pfmStep(float edge, float x)
{
    return (x < edge) ? 0.0 : 1.0;
}

PFM_API int
pfmSign(int x)
{
    return (x > 0) - (x < 0);
}

PFM_API int
pfmApprox(float a, float b, float epsilon)
{
    return fabsf(a - b) < epsilon;
}

PFM_API float
pfmLerp(float a, float b, float t)
{
    return a + t * (b - a);
}

PFM_API float
pfmLerpAngle(float a, float b, float t)
{
    float diff = pfmWrapAngle(b - a);
    return a + diff * t;
}

PFM_API float
pfmInverseLerp(float a, float b, float value)
{
    return (value - a) / (b - a);
}

PFM_API float
pfmSmoothstep(float edge0, float edge1, float x)
{
    float t = (x - edge0) / (edge1 - edge0);
    t = t < 0.0 ? 0.0 : (t > 1.0 ? 1.0 : t);
    return t * t * t * (t * (t * 6 - 15) + 10);
}

PFM_API float
pfmExpDecay(float initial, float decay_rate, float time)
{
    return initial * expf(-decay_rate * time);
}

PFM_API float
pfmMoveTowards(float current, float target, float max_delta)
{
    float delta = target - current;
    float distance = fabsf(delta);
    if (distance <= max_delta) {
        return target;
    } else {
        return current + (delta / distance) * max_delta;
    }
}

PFM_API uint64_t
pfmNextPOT(uint64_t x)
{
    if (x == 0) return 1;
    if ((x & (x - 1)) == 0) return x << 1; // (x * 2)
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32; //< Only for 64 bits
    x++;
    return x;
}

PFM_API uint64_t
pfmPreviousPOT(uint64_t x)
{
    if (x == 0) return 0;
    if ((x & (x - 1)) == 0) return x >> 1; // (x / 2)
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32; //< Only for 64 bits
    return x - (x >> 1);
}

PFM_API uint64_t
pfmNearestPOT(uint64_t x)
{
    uint64_t next = pfmNextPOT(x);
    uint64_t prev = pfmPreviousPOT(x);
    return (x - prev < next - x) ? prev : next;
}

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
pfmVec2Copy(aPFMvec2 PFM_RESTRICT dst, const aPFMvec2 PFM_RESTRICT src)
{
    memcpy(dst, src, sizeof(PFMvec2));
}

PFM_API void
pfmVec2Swap(aPFMvec2 PFM_RESTRICT a, aPFMvec2 PFM_RESTRICT b)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        float tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

PFM_API void
pfmVec2Neg(PFMvec2 dst, const PFMvec2 v)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = -v[i];
    }
}

PFM_API void
pfmVec2NegR(aPFMvec2 PFM_RESTRICT dst, const PFMvec2 v)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = -v[i];
    }
}

PFM_API void
pfmVec2Add(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void
pfmVec2AddR(aPFMvec2 PFM_RESTRICT dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void
pfmVec2Sub(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void
pfmVec2SubR(aPFMvec2 PFM_RESTRICT dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void
pfmVec2Mul(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void
pfmVec2MulR(aPFMvec2 PFM_RESTRICT dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void
pfmVec2Div(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void
pfmVec2DivR(aPFMvec2 PFM_RESTRICT dst, const PFMvec2 v1, const PFMvec2 v2)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void
pfmVec2Offset(PFMvec2 dst, const PFMvec2 v, float scalar)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v[i] + scalar;
    }
}

PFM_API void
pfmVec2OffsetR(aPFMvec2 PFM_RESTRICT dst, const PFMvec2 v, float scalar)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v[i] + scalar;
    }
}

PFM_API void
pfmVec2Scale(PFMvec2 dst, const PFMvec2 v, float scalar)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void
pfmVec2ScaleR(aPFMvec2 PFM_RESTRICT dst, const PFMvec2 v, float scalar)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void
pfmVec2Normalize(PFMvec2 dst, const PFMvec2 v)
{
    float squaredLength = v[0]*v[0] + v[1]*v[1];
    if (squaredLength == 0.0f) return;

    float invLength = rsqrtf(squaredLength);
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v[i] * invLength;
    }
}

PFM_API void
pfmVec2NormalizeR(aPFMvec2 PFM_RESTRICT dst, const PFMvec2 v)
{
    float squaredLength = v[0]*v[0] + v[1]*v[1];
    if (squaredLength == 0.0f) return;

    float invLength = rsqrtf(squaredLength);
    for (int_fast8_t i = 0; i < 2; i++) {
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
    for (int_fast8_t i = 0; i < 2; i++) {
        tmp[i] = v1[i] - v2[i];
        lengthSq += tmp[i]*tmp[i];
    }

    float invLength = rsqrtf(lengthSq);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = tmp[i]*invLength;
    }
}

PFM_API void
pfmVec2DirectionR(aPFMvec2 PFM_RESTRICT dst, const PFMvec2 v1, const PFMvec2 v2)
{
    float lengthSq = 0.0f;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v1[i] - v2[i];
        lengthSq += dst[i]*dst[i];
    }

    float invLength = rsqrtf(lengthSq);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = dst[i]*invLength;
    }
}

PFM_API void
pfmVec2Lerp(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2, float t)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v1[i] + t*(v2[i]-v1[i]);
    }
}

PFM_API void
pfmVec2LerpR(aPFMvec2 PFM_RESTRICT dst, const PFMvec2 v1, const PFMvec2 v2, float t)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = v1[i] + t*(v2[i]-v1[i]);
    }
}

PFM_API void
pfmVec2BaryInterp(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2, const PFMvec2 v3, float w1, float w2, float w3)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

PFM_API void
pfmVec2BaryInterpR(aPFMvec2 PFM_RESTRICT dst, const PFMvec2 v1, const PFMvec2 v2, const PFMvec2 v3, float w1, float w2, float w3)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

PFM_API void
pfmVec2BaryInterpV(PFMvec2 dst, const PFMvec2 v1, const PFMvec2 v2, const PFMvec2 v3, const PFMvec3 w)
{
    for (int_fast8_t i = 0; i < 2; i++) {
        dst[i] = w[0]*v1[i] + w[1]*v2[i] + w[2]*v3[i];
    }
}

PFM_API void
pfmVec2BaryInterpVR(aPFMvec2 PFM_RESTRICT dst, const PFMvec2 v1, const PFMvec2 v2, const PFMvec2 v3, const PFMvec3 w)
{
    for (int_fast8_t i = 0; i < 2; i++) {
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
pfmVec2TransformR(aPFMvec2 PFM_RESTRICT dst, const PFMvec2 v, const PFMmat4 mat)
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
pfmVec2TransformWTR(aPFMvec2 PFM_RESTRICT dst, const PFMvec2 v, float wTranslation, const PFMmat4 mat)
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
pfmVec3Copy(aPFMvec3 PFM_RESTRICT dst, const aPFMvec3 PFM_RESTRICT src)
{
    memcpy(dst, src, sizeof(PFMvec3));
}

PFM_API void
pfmVec3Swap(aPFMvec3 PFM_RESTRICT a, aPFMvec3 PFM_RESTRICT b)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
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
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = -v[i];
    }
}

PFM_API void
pfmVec3NegR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = -v[i];
    }
}

PFM_API void
pfmVec3Add(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void
pfmVec3AddR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void
pfmVec3Sub(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void
pfmVec3SubR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void
pfmVec3Mul(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void
pfmVec3MulR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void
pfmVec3Div(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void
pfmVec3DivR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v1, const PFMvec3 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void
pfmVec3Offset(PFMvec3 dst, const PFMvec3 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v[i] + scalar;
    }
}

PFM_API void
pfmVec3OffsetR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v[i] + scalar;
    }
}

PFM_API void
pfmVec3Scale(PFMvec3 dst, const PFMvec3 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void
pfmVec3ScaleR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
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
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v[i] * invLength;
    }
}

PFM_API void
pfmVec3NormalizeR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v)
{
    float squaredLength = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
    if (squaredLength == 0.0f) return;

    float invLength = rsqrtf(squaredLength);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
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
    for (int_fast8_t i = 0; i < 3; i++) {
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
pfmVec3CrossR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v1, const PFMvec3 v2)
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
    for (int_fast8_t i = 0; i < 3; i++) {
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
    for (int_fast8_t i = 0; i < 3; i++) {
        tmp[i] = v1[i] - v2[i];
        lengthSq += tmp[i]*tmp[i];
    }

    float invLength = rsqrtf(lengthSq);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = tmp[i]*invLength;
    }
}

PFM_API void
pfmVec3DirectionR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v1, const PFMvec3 v2)
{
    float lengthSq = 0.0f;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v1[i] - v2[i];
        lengthSq += dst[i]*dst[i];
    }

    float invLength = rsqrtf(lengthSq);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = dst[i]*invLength;
    }
}

PFM_API void
pfmVec3Lerp(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2, float t)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v1[i] + t*(v2[i]-v1[i]);
    }
}

PFM_API void
pfmVec3LerpR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v1, const PFMvec3 v2, float t)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = v1[i] + t*(v2[i]-v1[i]);
    }
}

PFM_API void
pfmVec3BaryInterp(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2, const PFMvec3 v3, float w1, float w2, float w3)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

PFM_API void
pfmVec3BaryInterpR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v1, const PFMvec3 v2, const PFMvec3 v3, float w1, float w2, float w3)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

PFM_API void
pfmVec3BaryInterpV(PFMvec3 dst, const PFMvec3 v1, const PFMvec3 v2, const PFMvec3 v3, const PFMvec3 w)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = w[0]*v1[i] + w[1]*v2[i] + w[2]*v3[i];
    }
}

PFM_API void
pfmVec3BaryInterpVR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v1, const PFMvec3 v2, const PFMvec3 v3, const PFMvec3 w)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
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
pfmVec3TransformR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v, const PFMmat4 mat)
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
pfmVec3TransformWTR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 v, float wTranslation, const PFMmat4 mat)
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
    for (int_fast8_t i = 0; i < 3; i++) {
        dotProduct += incident[i]*normal[i];
    }

    dotProduct *= 2.0f;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dst[i] = incident[i] - dotProduct*normal[i];
    }
}

PFM_API void
pfmVec3ReflectR(aPFMvec3 PFM_RESTRICT dst, const PFMvec3 incident, const PFMvec3 normal)
{
    float dotProduct = 0.0f;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
        dotProduct += incident[i]*normal[i];
    }

    dotProduct *= 2.0f;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 3; i++) {
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
pfmVec4Copy(aPFMvec4 PFM_RESTRICT dst, const aPFMvec4 PFM_RESTRICT src)
{
    memcpy(dst, src, sizeof(PFMvec4));
}

PFM_API void
pfmVec4Swap(aPFMvec4 PFM_RESTRICT a, aPFMvec4 PFM_RESTRICT b)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
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
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = -v[i];
    }
}

PFM_API void
pfmVec4NegR(aPFMvec4 PFM_RESTRICT dst, const PFMvec4 v)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = -v[i];
    }
}

PFM_API void
pfmVec4Add(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void
pfmVec4AddR(aPFMvec4 PFM_RESTRICT dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = v1[i] + v2[i];
    }
}

PFM_API void
pfmVec4Sub(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void
pfmVec4SubR(aPFMvec4 PFM_RESTRICT dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = v1[i] - v2[i];
    }
}

PFM_API void
pfmVec4Mul(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void
pfmVec4MulR(aPFMvec4 PFM_RESTRICT dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = v1[i]*v2[i];
    }
}

PFM_API void
pfmVec4Div(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void
pfmVec4DivR(aPFMvec4 PFM_RESTRICT dst, const PFMvec4 v1, const PFMvec4 v2)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = v1[i]/v2[i];
    }
}

PFM_API void
pfmVec4Offset(PFMvec4 dst, const PFMvec4 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = v[i] + scalar;
    }
}

PFM_API void
pfmVec4OffsetR(aPFMvec4 PFM_RESTRICT dst, const PFMvec4 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = v[i] + scalar;
    }
}

PFM_API void
pfmVec4Scale(PFMvec4 dst, const PFMvec4 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = v[i]*scalar;
    }
}

PFM_API void
pfmVec4ScaleR(aPFMvec4 PFM_RESTRICT dst, const PFMvec4 v, float scalar)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
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
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = v[i] * invLength;
    }
}

PFM_API void
pfmVec4NormalizeR(aPFMvec4 PFM_RESTRICT dst, const PFMvec4 v)
{
    float squaredLength = v[0]*v[0] + v[1]*v[1] + v[2]*v[2] + v[3]*v[3];
    if (squaredLength == 0.0f) return;

    float invLength = rsqrtf(squaredLength);

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
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
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = v1[i] + t*(v2[i]-v1[i]);
    }
}

PFM_API void
pfmVec4LerpR(aPFMvec4 PFM_RESTRICT dst, const PFMvec4 v1, const PFMvec4 v2, float t)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = v1[i] + t*(v2[i]-v1[i]);
    }
}

PFM_API void
pfmVec4BaryInterp(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2, const PFMvec4 v3, float w1, float w2, float w3)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

PFM_API void
pfmVec4BaryInterpR(aPFMvec4 PFM_RESTRICT dst, const PFMvec4 v1, const PFMvec4 v2, const PFMvec4 v3, float w1, float w2, float w3)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = w1*v1[i] + w2*v2[i] + w3*v3[i];
    }
}

PFM_API void
pfmVec4BaryInterpV(PFMvec4 dst, const PFMvec4 v1, const PFMvec4 v2, const PFMvec4 v3, const PFMvec3 w)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        dst[i] = w[0]*v1[i] + w[1]*v2[i] + w[2]*v3[i];
    }
}

PFM_API void
pfmVec4BaryInterpVR(aPFMvec4 PFM_RESTRICT dst, const PFMvec4 v1, const PFMvec4 v2, const PFMvec4 v3, const PFMvec3 w)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
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
pfmVec4TransformR(aPFMvec4 PFM_RESTRICT dst, const PFMvec4 v, const PFMmat4 mat)
{
    dst[0] = mat[0]*v[0] + mat[4]*v[1] + mat[8]*v[2] + mat[12]*v[3];
    dst[1] = mat[1]*v[0] + mat[5]*v[1] + mat[9]*v[2] + mat[13]*v[3];
    dst[2] = mat[2]*v[0] + mat[6]*v[1] + mat[10]*v[2] + mat[14]*v[3];
    dst[3] = mat[3]*v[0] + mat[7]*v[1] + mat[11]*v[2] + mat[15]*v[3];
}

/* Matrix 4x4 function definitions */

PFM_API void
pfmMat4Copy(aPFMmat4 PFM_RESTRICT dst, const aPFMmat4 PFM_RESTRICT src)
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
    for (int_fast8_t i = 0; i < 4; i++) {
        for (int_fast8_t j = 0; j < 4; j++) {
            result[i * 4 + j] = src[j * 4 + i];
        }
    }

    memcpy(dst, result, sizeof(PFMmat4));
}

PFM_API void
pfmMat4TransposeR(aPFMmat4 PFM_RESTRICT dst, const PFMmat4 src)
{
    // NOTE 1: Seems more optimized in O3 by GCC 13 without "omp simd collapse(2)"
    // NOTE 2: Also using "omp simd" produces exactly the same code in O3 with GCC 13.

    for (int_fast8_t i = 0; i < 4; i++) {
        for (int_fast8_t j = 0; j < 4; j++) {
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
    for (int_fast8_t i = 0; i < 16; i++) {
        dst[i] = left[i] + right[i];
    }
}

PFM_API void
pfmMat4AddR(aPFMmat4 PFM_RESTRICT dst, const PFMmat4 left, const PFMmat4 right)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 16; i++) {
        dst[i] = left[i] + right[i];
    }
}

PFM_API void
pfmMat4Sub(PFMmat4 dst, const PFMmat4 left, const PFMmat4 right)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 16; i++) {
        dst[i] = left[i] + right[i];
    }
}

PFM_API void
pfmMat4SubR(aPFMmat4 PFM_RESTRICT dst, const PFMmat4 left, const PFMmat4 right)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 16; i++) {
        dst[i] = left[i] + right[i];
    }
}

PFM_API void
pfmMat4Mul(PFMmat4 dst, const PFMmat4 left, const PFMmat4 right)
{
    PFMmat4 result;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
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
pfmMat4MulR(aPFMmat4 PFM_RESTRICT dst, const PFMmat4 left, const PFMmat4 right)
{
#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        for (int_fast8_t j = 0; j < 4; j++) {
            float sum = 0.0;
            for (int_fast8_t k = 0; k < 4; k++) {
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

    if (lengthSq != 1.0f && lengthSq != 0.0f) {
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

    float top = nearPlane*tanf(fovY*0.5f);
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

#endif //PFM_H
