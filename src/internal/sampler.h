#ifndef PF_INTERNAL_SAMPLER_H
#define PF_INTERNAL_SAMPLER_H

#include "./context/context.h"

static inline void
pfInternal_Texture2DMap_REPEAT(const struct PFtex* tex, PFint* xOut, PFint* yOut, PFfloat u, PFfloat v)
{
    // Wrap UVs (use to int cast to round toward zero)
    u = (u - (PFint)u);
    v = (v - (PFint)v);

    // Upscale to nearest texture coordinates
    // NOTE: We use '(int)(x+0.5)' although this is incorrect
    //       regarding the direction of rounding in case of negative values
    //       and also less accurate than roundf, but it remains so much more
    //       efficient that it is preferable for now to opt for this option.
    *xOut = (PFint)(u*((PFint)tex->w - 1) + 0.5f);
    *yOut = (PFint)(v*((PFint)tex->h - 1) + 0.5f);

    // Make sure the coordinates are positive (in case of negative UV input)
    *xOut = abs(*xOut);
    *yOut = abs(*yOut);
}

static inline void
pfInternal_Texture2DMap_MIRRORED_REPEAT(const struct PFtex* tex, PFint* xOut, PFint* yOut, PFfloat u, PFfloat v)
{
    u = fmodf(fabsf(u), 2);
    v = fmodf(fabsf(v), 2);

    if (u > 1.0f) u = 1.0f - (u - 1.0f);
    if (v > 1.0f) v = 1.0f - (u - 1.0f);

    *xOut = (PFint)(u*((PFint)tex->w - 1) + 0.5f);
    *yOut = (PFint)(v*((PFint)tex->h - 1) + 0.5f);
}

static inline void
pfInternal_Texture2DMap_CLAMP_TO_EDGE(const struct PFtex* tex, PFint* xOut, PFint* yOut, PFfloat u, PFfloat v)
{
    u = CLAMP(u, 0.0f, 1.0f);
    v = CLAMP(v, 0.0f, 1.0f);

    *xOut = (PFint)(u*((PFint)tex->w - 1) + 0.5f);
    *yOut = (PFint)(v*((PFint)tex->h - 1) + 0.5f);
}

static inline void
pfInternal_SimdTexture2DMap_REPEAT(const struct PFtex* tex, PFMsimd_i* xOut, PFMsimd_i* yOut, const PFMsimd_vec2 texcoords)
{
    PFMsimd_f u = texcoords[0];
    PFMsimd_f v = texcoords[1];

    u = pfmSimdMul_F32(
        pfmSimdSub_F32(u, pfmSimdRound_F32(u, _MM_FROUND_TO_ZERO)),
        pfmSimdSet1_F32((float)(tex->w - 1)));

    v = pfmSimdMul_F32(
        pfmSimdSub_F32(v, pfmSimdRound_F32(v, _MM_FROUND_TO_ZERO)),
        pfmSimdSet1_F32((float)(tex->h - 1)));

    *xOut = pfmSimdAbs_I32(pfmSimdConvert_F32_I32(u));
    *yOut = pfmSimdAbs_I32(pfmSimdConvert_F32_I32(v));
}

static inline void
pfInternal_SimdTexture2DMap_MIRRORED_REPEAT(const struct PFtex* tex, PFMsimd_i* xOut, PFMsimd_i* yOut, const PFMsimd_vec2 texcoords)
{
    PFMsimd_f u = texcoords[0];
    PFMsimd_f v = texcoords[1];

    // Repeating UV coordinates in the interval [0..2]
    u = pfmSimdMod_F32(pfmSimdAbs_F32(u), pfmSimdSet1_F32(2.0f));
    v = pfmSimdMod_F32(pfmSimdAbs_F32(v), pfmSimdSet1_F32(2.0f));

    // Reflection to get the interval [0..1] if necessary
    PFMsimd_f uMirror = pfmSimdSub_F32(pfmSimdSet1_F32(1.0f), pfmSimdSub_F32(u, pfmSimdSet1_F32(1.0f)));
    PFMsimd_f vMirror = pfmSimdSub_F32(pfmSimdSet1_F32(1.0f), pfmSimdSub_F32(v, pfmSimdSet1_F32(1.0f)));

    u = pfmSimdBlendV_F32(u, uMirror, pfmSimdCmpGT_F32(u, pfmSimdSet1_F32(1.0f)));
    v = pfmSimdBlendV_F32(v, vMirror, pfmSimdCmpGT_F32(v, pfmSimdSet1_F32(1.0f)));

    // Conversion des coordonnées UV en indices de pixels
    u = pfmSimdMul_F32(u, pfmSimdSet1_F32((float)(tex->w - 1)));
    v = pfmSimdMul_F32(v, pfmSimdSet1_F32((float)(tex->h - 1)));

    *xOut = pfmSimdConvert_F32_I32(pfmSimdAdd_F32(u, pfmSimdSet1_F32(0.5f)));
    *yOut = pfmSimdConvert_F32_I32(pfmSimdAdd_F32(v, pfmSimdSet1_F32(0.5f)));
}

static inline void
pfInternal_SimdTexture2DMap_CLAMP_TO_EDGE(const struct PFtex* tex, PFMsimd_i* xOut, PFMsimd_i* yOut, const PFMsimd_vec2 texcoords)
{
    PFMsimd_f u = texcoords[0];
    PFMsimd_f v = texcoords[1];

    // Clamping des coordonnées UV
    u = pfmSimdClamp_F32(u, pfmSimdSet1_F32(0.0f), pfmSimdSet1_F32(1.0f));
    v = pfmSimdClamp_F32(v, pfmSimdSet1_F32(0.0f), pfmSimdSet1_F32(1.0f));

    // Conversion des coordonnées UV en indices de pixels
    u = pfmSimdMul_F32(u, pfmSimdSet1_F32((float)(tex->w - 1)));
    v = pfmSimdMul_F32(v, pfmSimdSet1_F32((float)(tex->h - 1)));

    *xOut = pfmSimdConvert_F32_I32(pfmSimdAdd_F32(u, pfmSimdSet1_F32(0.5f)));
    *yOut = pfmSimdConvert_F32_I32(pfmSimdAdd_F32(v, pfmSimdSet1_F32(0.5f)));
}

/* SISD Texture2D Sampler Functions */

static inline PFcolor
pfInternal_Texture2DSampler_NEAREST_REPEAT(const struct PFtex* tex, PFfloat u, PFfloat v)
{
    PFint x, y;
    pfInternal_Texture2DMap_REPEAT(tex, &x, &y, u, v);

    return tex->getter(tex->pixels, y*tex->w + x);
}

static inline PFcolor
pfInternal_Texture2DSampler_NEAREST_MIRRORED_REPEAT(const struct PFtex* tex, PFfloat u, PFfloat v)
{
    PFint x, y;
    pfInternal_Texture2DMap_MIRRORED_REPEAT(tex, &x, &y, u, v);

    return tex->getter(tex->pixels, y*tex->w + x);
}

static inline PFcolor
pfInternal_Texture2DSampler_NEAREST_CLAMP_TO_EDGE(const struct PFtex* tex, PFfloat u, PFfloat v)
{
    PFint x, y;
    pfInternal_Texture2DMap_CLAMP_TO_EDGE(tex, &x, &y, u, v);

    return tex->getter(tex->pixels, y*tex->w + x);
}

static inline PFcolor
pfInternal_Texture2DSampler_BILINEAR_REPEAT(const struct PFtex* tex, PFfloat u, PFfloat v)
{
    int x0, y0, x1, y1;
    float fx, fy;

    pfInternal_Texture2DMap_REPEAT(tex, &x0, &y0, u, v);
    pfInternal_Texture2DMap_REPEAT(tex, &x1, &y1, u + tex->tx, v + tex->ty);

    // Calculate fractions fx, fy
    fx = u * tex->w - x0;
    fy = v * tex->h - y0;

    // Get the colors of the four pixels
    PFcolor c00 = tex->getter(tex->pixels, y0 * tex->w + x0);
    PFcolor c10 = tex->getter(tex->pixels, y0 * tex->w + x1);
    PFcolor c01 = tex->getter(tex->pixels, y1 * tex->w + x0);
    PFcolor c11 = tex->getter(tex->pixels, y1 * tex->w + x1);

    // Interpolate colors horizontally
    PFcolor c0 = pfInternal_ColorLerpSmooth(c00, c10, fx);
    PFcolor c1 = pfInternal_ColorLerpSmooth(c01, c11, fx);

    // Interpolate colors vertically
    return pfInternal_ColorLerpSmooth(c0, c1, fy);
}

static inline PFcolor
pfInternal_Texture2DSampler_BILINEAR_MIRRORED_REPEAT(const struct PFtex* tex, PFfloat u, PFfloat v)
{
    int x0, y0, x1, y1;
    float fx, fy;

    pfInternal_Texture2DMap_MIRRORED_REPEAT(tex, &x0, &y0, u, v);
    pfInternal_Texture2DMap_MIRRORED_REPEAT(tex, &x1, &y1, u + tex->tx, v + tex->ty);

    // Calculate fractions fx, fy
    fx = u * tex->w - x0;
    fy = v * tex->h - y0;

    // Get the colors of the four pixels
    PFcolor c00 = tex->getter(tex->pixels, y0 * tex->w + x0);
    PFcolor c10 = tex->getter(tex->pixels, y0 * tex->w + x1);
    PFcolor c01 = tex->getter(tex->pixels, y1 * tex->w + x0);
    PFcolor c11 = tex->getter(tex->pixels, y1 * tex->w + x1);

    // Interpolate colors horizontally
    PFcolor c0 = pfInternal_ColorLerpSmooth(c00, c10, fx);
    PFcolor c1 = pfInternal_ColorLerpSmooth(c01, c11, fx);

    // Interpolate colors vertically
    return pfInternal_ColorLerpSmooth(c0, c1, fy);
}

static inline PFcolor
pfInternal_Texture2DSampler_BILINEAR_CLAMP_TO_EDGE(const struct PFtex* tex, PFfloat u, PFfloat v)
{
    int x0, y0, x1, y1;
    float fx, fy;

    pfInternal_Texture2DMap_CLAMP_TO_EDGE(tex, &x0, &y0, u, v);
    pfInternal_Texture2DMap_CLAMP_TO_EDGE(tex, &x1, &y1, u + tex->tx, v + tex->ty);

    // Calculate fractions fx, fy
    fx = u * tex->w - x0;
    fy = v * tex->h - y0;

    // Get the colors of the four pixels
    PFcolor c00 = tex->getter(tex->pixels, y0 * tex->w + x0);
    PFcolor c10 = tex->getter(tex->pixels, y0 * tex->w + x1);
    PFcolor c01 = tex->getter(tex->pixels, y1 * tex->w + x0);
    PFcolor c11 = tex->getter(tex->pixels, y1 * tex->w + x1);

    // Interpolate colors horizontally
    PFcolor c0 = pfInternal_ColorLerpSmooth(c00, c10, fx);
    PFcolor c1 = pfInternal_ColorLerpSmooth(c01, c11, fx);

    // Interpolate colors vertically
    return pfInternal_ColorLerpSmooth(c0, c1, fy);
}

/* SISD Texture2D Sampler Functions */

static inline PFMsimd_i
pfInternal_SimdTexture2DSampler_NEAREST_REPEAT(const struct PFtex* tex, const PFMsimd_vec2 texcoords)
{
    PFMsimd_i x, y;
    pfInternal_SimdTexture2DMap_REPEAT(
        tex, &x, &y, texcoords);

    PFMsimd_i offsets = pfmSimdAdd_I32(pfmSimdMullo_I32(
        y, pfmSimdSet1_I32(tex->w)), x);

    return tex->getterSimd(tex->pixels, offsets);
}

static inline PFMsimd_i
pfInternal_SimdTexture2DSampler_NEAREST_MIRRORED_REPEAT(const struct PFtex* tex, const PFMsimd_vec2 texcoords)
{
    PFMsimd_i x, y;
    pfInternal_SimdTexture2DMap_MIRRORED_REPEAT(
        tex, &x, &y, texcoords);

    PFMsimd_i offsets = pfmSimdAdd_I32(pfmSimdMullo_I32(
        y, pfmSimdSet1_I32(tex->w)), x);

    return tex->getterSimd(tex->pixels, offsets);
}

static inline PFMsimd_i
pfInternal_SimdTexture2DSampler_NEAREST_CLAMP_TO_EDGE(const struct PFtex* tex, const PFMsimd_vec2 texcoords)
{
    PFMsimd_i x, y;
    pfInternal_SimdTexture2DMap_CLAMP_TO_EDGE(
        tex, &x, &y, texcoords);

    PFMsimd_i offsets = pfmSimdAdd_I32(pfmSimdMullo_I32(
        y, pfmSimdSet1_I32(tex->w)), x);

    return tex->getterSimd(tex->pixels, offsets);
}

static inline PFMsimd_i
pfInternal_SimdTexture2DSampler_BILINEAR_REPEAT(const struct PFtex* tex, const PFMsimd_vec2 texcoords)
{
    PFMsimd_i x0, y0, x1, y1;
    PFMsimd_f fx, fy;

    PFMsimd_vec2 texelSize;
    pfmSimdVec2Set(texelSize, tex->tx, tex->ty);

    PFMsimd_vec2 texcoords2;
    pfmSimdVec2AddR(texcoords2, texcoords, texelSize);

    pfInternal_SimdTexture2DMap_REPEAT(tex, &x0, &y0, texcoords);
    pfInternal_SimdTexture2DMap_REPEAT(tex, &x1, &y1, texcoords2);

    // Calculate fractions fx, fy
    fx = pfmSimdSub_F32(pfmSimdMul_F32(texcoords[0], pfmSimdSet1_F32(tex->w)), pfmSimdConvert_I32_F32(x0));
    fy = pfmSimdSub_F32(pfmSimdMul_F32(texcoords[1], pfmSimdSet1_F32(tex->h)), pfmSimdConvert_I32_F32(y0));

    // Get the colors of the four pixels
    PFsimd_color c00; pfInternal_SimdColorUnpack(c00, tex->getterSimd(tex->pixels, y0 * tex->w + x0));
    PFsimd_color c10; pfInternal_SimdColorUnpack(c10, tex->getterSimd(tex->pixels, y0 * tex->w + x1));
    PFsimd_color c01; pfInternal_SimdColorUnpack(c01, tex->getterSimd(tex->pixels, y1 * tex->w + x0));
    PFsimd_color c11; pfInternal_SimdColorUnpack(c11, tex->getterSimd(tex->pixels, y1 * tex->w + x1));

    // Interpolate colors horizontally
    PFsimd_color c0; pfInternal_SimdColorLerpSmooth(c0, c00, c10, fx);
    PFsimd_color c1; pfInternal_SimdColorLerpSmooth(c1, c01, c11, fx);

    // Interpolate colors vertically
    PFsimd_color c; pfInternal_SimdColorLerpSmooth(c, c0, c1, fy);

    return pfInternal_SimdColorPack(c);
}

static inline PFMsimd_i
pfInternal_SimdTexture2DSampler_BILINEAR_MIRRORED_REPEAT(const struct PFtex* tex, const PFMsimd_vec2 texcoords)
{
    PFMsimd_i x0, y0, x1, y1;
    PFMsimd_f fx, fy;

    PFMsimd_vec2 texelSize;
    pfmSimdVec2Set(texelSize, tex->tx, tex->ty);

    PFMsimd_vec2 texcoords2;
    pfmSimdVec2AddR(texcoords2, texcoords, texelSize);

    pfInternal_SimdTexture2DMap_MIRRORED_REPEAT(tex, &x0, &y0, texcoords);
    pfInternal_SimdTexture2DMap_MIRRORED_REPEAT(tex, &x1, &y1, texcoords2);

    // Calculate fractions fx, fy
    fx = pfmSimdSub_F32(pfmSimdMul_F32(texcoords[0], pfmSimdSet1_F32(tex->w)), pfmSimdConvert_I32_F32(x0));
    fy = pfmSimdSub_F32(pfmSimdMul_F32(texcoords[1], pfmSimdSet1_F32(tex->h)), pfmSimdConvert_I32_F32(y0));

    // Get the colors of the four pixels
    PFsimd_color c00; pfInternal_SimdColorUnpack(c00, tex->getterSimd(tex->pixels, y0 * tex->w + x0));
    PFsimd_color c10; pfInternal_SimdColorUnpack(c10, tex->getterSimd(tex->pixels, y0 * tex->w + x1));
    PFsimd_color c01; pfInternal_SimdColorUnpack(c01, tex->getterSimd(tex->pixels, y1 * tex->w + x0));
    PFsimd_color c11; pfInternal_SimdColorUnpack(c11, tex->getterSimd(tex->pixels, y1 * tex->w + x1));

    // Interpolate colors horizontally
    PFsimd_color c0; pfInternal_SimdColorLerpSmooth(c0, c00, c10, fx);
    PFsimd_color c1; pfInternal_SimdColorLerpSmooth(c1, c01, c11, fx);

    // Interpolate colors vertically
    PFsimd_color c; pfInternal_SimdColorLerpSmooth(c, c0, c1, fy);

    return pfInternal_SimdColorPack(c);
}

static inline PFMsimd_i
pfInternal_SimdTexture2DSampler_BILINEAR_CLAMP_TO_EDGE(const struct PFtex* tex, const PFMsimd_vec2 texcoords)
{
    PFMsimd_i x0, y0, x1, y1;
    PFMsimd_f fx, fy;

    PFMsimd_vec2 texelSize;
    pfmSimdVec2Set(texelSize, tex->tx, tex->ty);

    PFMsimd_vec2 texcoords2;
    pfmSimdVec2AddR(texcoords2, texcoords, texelSize);

    pfInternal_SimdTexture2DMap_CLAMP_TO_EDGE(tex, &x0, &y0, texcoords);
    pfInternal_SimdTexture2DMap_CLAMP_TO_EDGE(tex, &x1, &y1, texcoords2);

    // Calculate fractions fx, fy
    fx = pfmSimdSub_F32(pfmSimdMul_F32(texcoords[0], pfmSimdSet1_F32(tex->w)), pfmSimdConvert_I32_F32(x0));
    fy = pfmSimdSub_F32(pfmSimdMul_F32(texcoords[1], pfmSimdSet1_F32(tex->h)), pfmSimdConvert_I32_F32(y0));

    // Get the colors of the four pixels
    PFsimd_color c00; pfInternal_SimdColorUnpack(c00, tex->getterSimd(tex->pixels, y0 * tex->w + x0));
    PFsimd_color c10; pfInternal_SimdColorUnpack(c10, tex->getterSimd(tex->pixels, y0 * tex->w + x1));
    PFsimd_color c01; pfInternal_SimdColorUnpack(c01, tex->getterSimd(tex->pixels, y1 * tex->w + x0));
    PFsimd_color c11; pfInternal_SimdColorUnpack(c11, tex->getterSimd(tex->pixels, y1 * tex->w + x1));

    // Interpolate colors horizontally
    PFsimd_color c0; pfInternal_SimdColorLerpSmooth(c0, c00, c10, fx);
    PFsimd_color c1; pfInternal_SimdColorLerpSmooth(c1, c01, c11, fx);

    // Interpolate colors vertically
    PFsimd_color c; pfInternal_SimdColorLerpSmooth(c, c0, c1, fy);

    return pfInternal_SimdColorPack(c);
}

/* Internal helper functions */

static inline PFboolean
pfInternal_IsTextureParameterValid(PFtexturewrap wrapMode, PFtexturefilter filterMode)
{
    return wrapMode >= PF_NEAREST && wrapMode <= PF_BILINEAR
        && filterMode >= PF_REPEAT && filterMode <= PF_CLAMP_TO_EDGE;
}

static inline void
pfInternal_GetTexture2DSampler(PFtexturesampler* sampler, PFtexturesampler_simd* samplerSimd, PFtexturewrap wrapMode, PFtexturefilter filterMode)
{
#   define ENTRY(FILTER, WRAP, FUNC) [FILTER][WRAP] = FUNC

    static const PFtexturesampler samplers[2][3] = {

        ENTRY(PF_NEAREST, PF_REPEAT, pfInternal_Texture2DSampler_NEAREST_REPEAT),
        ENTRY(PF_NEAREST, PF_MIRRORED_REPEAT, pfInternal_Texture2DSampler_NEAREST_MIRRORED_REPEAT),
        ENTRY(PF_NEAREST, PF_CLAMP_TO_EDGE, pfInternal_Texture2DSampler_NEAREST_CLAMP_TO_EDGE),

        ENTRY(PF_BILINEAR, PF_REPEAT, pfInternal_Texture2DSampler_BILINEAR_REPEAT),
        ENTRY(PF_BILINEAR, PF_MIRRORED_REPEAT, pfInternal_Texture2DSampler_BILINEAR_MIRRORED_REPEAT),
        ENTRY(PF_BILINEAR, PF_CLAMP_TO_EDGE, pfInternal_Texture2DSampler_BILINEAR_CLAMP_TO_EDGE),

    };

    static const PFtexturesampler_simd samplersSimd[2][3] = {

        ENTRY(PF_NEAREST, PF_REPEAT, pfInternal_SimdTexture2DSampler_NEAREST_REPEAT),
        ENTRY(PF_NEAREST, PF_MIRRORED_REPEAT, pfInternal_SimdTexture2DSampler_NEAREST_MIRRORED_REPEAT),
        ENTRY(PF_NEAREST, PF_CLAMP_TO_EDGE, pfInternal_SimdTexture2DSampler_NEAREST_CLAMP_TO_EDGE),

        ENTRY(PF_BILINEAR, PF_REPEAT, pfInternal_SimdTexture2DSampler_BILINEAR_REPEAT),
        ENTRY(PF_BILINEAR, PF_MIRRORED_REPEAT, pfInternal_SimdTexture2DSampler_BILINEAR_MIRRORED_REPEAT),
        ENTRY(PF_BILINEAR, PF_CLAMP_TO_EDGE, pfInternal_SimdTexture2DSampler_BILINEAR_CLAMP_TO_EDGE),

    };

    if (sampler) *sampler = samplers[filterMode][wrapMode];
    if (samplerSimd) *samplerSimd = samplersSimd[filterMode][wrapMode];

#   undef ENTRY
}

#endif //PF_INTERNAL_SAMPLER_H
