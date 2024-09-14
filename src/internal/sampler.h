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

#ifndef PF_INTERNAL_SAMPLER_H
#define PF_INTERNAL_SAMPLER_H

#include "./context/context.h"

/* Texture2D Mapper Functions */

static inline void
pfiTexture2DMap_REPEAT(const struct PFtex* tex, PFint* xOut, PFint* yOut, PFfloat u, PFfloat v)
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
pfiTexture2DMap_MIRRORED_REPEAT(const struct PFtex* tex, PFint* xOut, PFint* yOut, PFfloat u, PFfloat v)
{
    u = fmodf(fabsf(u), 2);
    v = fmodf(fabsf(v), 2);

    if (u > 1.0f) u = 1.0f - (u - 1.0f);
    if (v > 1.0f) v = 1.0f - (u - 1.0f);

    *xOut = (PFint)(u*((PFint)tex->w - 1) + 0.5f);
    *yOut = (PFint)(v*((PFint)tex->h - 1) + 0.5f);
}

static inline void
pfiTexture2DMap_CLAMP_TO_EDGE(const struct PFtex* tex, PFint* xOut, PFint* yOut, PFfloat u, PFfloat v)
{
    u = CLAMP(u, 0.0f, 1.0f);
    v = CLAMP(v, 0.0f, 1.0f);

    *xOut = (PFint)(u*((PFint)tex->w - 1) + 0.5f);
    *yOut = (PFint)(v*((PFint)tex->h - 1) + 0.5f);
}

/* Texture2D Sampler Functions */

static inline PFcolor
pfiTexture2DSampler_NEAREST_REPEAT(const struct PFtex* tex, PFfloat u, PFfloat v)
{
    PFint x, y;
    pfiTexture2DMap_REPEAT(tex, &x, &y, u, v);

    return tex->getter(tex->pixels, y*tex->w + x);
}

static inline PFcolor
pfiTexture2DSampler_NEAREST_MIRRORED_REPEAT(const struct PFtex* tex, PFfloat u, PFfloat v)
{
    PFint x, y;
    pfiTexture2DMap_MIRRORED_REPEAT(tex, &x, &y, u, v);

    return tex->getter(tex->pixels, y*tex->w + x);
}

static inline PFcolor
pfiTexture2DSampler_NEAREST_CLAMP_TO_EDGE(const struct PFtex* tex, PFfloat u, PFfloat v)
{
    PFint x, y;
    pfiTexture2DMap_CLAMP_TO_EDGE(tex, &x, &y, u, v);

    return tex->getter(tex->pixels, y*tex->w + x);
}

static inline PFcolor
pfiTexture2DSampler_BILINEAR_REPEAT(const struct PFtex* tex, PFfloat u, PFfloat v)
{
    int x0, y0, x1, y1;
    float fx, fy;

    pfiTexture2DMap_REPEAT(tex, &x0, &y0, u, v);
    pfiTexture2DMap_REPEAT(tex, &x1, &y1, u + tex->tx, v + tex->ty);

    // Calculate fractions fx, fy
    fx = u * tex->w - x0;
    fy = v * tex->h - y0;

    // Get the colors of the four pixels
    PFcolor c00 = tex->getter(tex->pixels, y0 * tex->w + x0);
    PFcolor c10 = tex->getter(tex->pixels, y0 * tex->w + x1);
    PFcolor c01 = tex->getter(tex->pixels, y1 * tex->w + x0);
    PFcolor c11 = tex->getter(tex->pixels, y1 * tex->w + x1);

    // Interpolate colors horizontally
    PFcolor c0 = pfiColorLerpSmooth(c00, c10, fx);
    PFcolor c1 = pfiColorLerpSmooth(c01, c11, fx);

    // Interpolate colors vertically
    return pfiColorLerpSmooth(c0, c1, fy);
}

static inline PFcolor
pfiTexture2DSampler_BILINEAR_MIRRORED_REPEAT(const struct PFtex* tex, PFfloat u, PFfloat v)
{
    int x0, y0, x1, y1;
    float fx, fy;

    pfiTexture2DMap_MIRRORED_REPEAT(tex, &x0, &y0, u, v);
    pfiTexture2DMap_MIRRORED_REPEAT(tex, &x1, &y1, u + tex->tx, v + tex->ty);

    // Calculate fractions fx, fy
    fx = u * tex->w - x0;
    fy = v * tex->h - y0;

    // Get the colors of the four pixels
    PFcolor c00 = tex->getter(tex->pixels, y0 * tex->w + x0);
    PFcolor c10 = tex->getter(tex->pixels, y0 * tex->w + x1);
    PFcolor c01 = tex->getter(tex->pixels, y1 * tex->w + x0);
    PFcolor c11 = tex->getter(tex->pixels, y1 * tex->w + x1);

    // Interpolate colors horizontally
    PFcolor c0 = pfiColorLerpSmooth(c00, c10, fx);
    PFcolor c1 = pfiColorLerpSmooth(c01, c11, fx);

    // Interpolate colors vertically
    return pfiColorLerpSmooth(c0, c1, fy);
}

static inline PFcolor
pfiTexture2DSampler_BILINEAR_CLAMP_TO_EDGE(const struct PFtex* tex, PFfloat u, PFfloat v)
{
    int x0, y0, x1, y1;
    float fx, fy;

    pfiTexture2DMap_CLAMP_TO_EDGE(tex, &x0, &y0, u, v);
    pfiTexture2DMap_CLAMP_TO_EDGE(tex, &x1, &y1, u + tex->tx, v + tex->ty);

    // Calculate fractions fx, fy
    fx = u * tex->w - x0;
    fy = v * tex->h - y0;

    // Get the colors of the four pixels
    PFcolor c00 = tex->getter(tex->pixels, y0 * tex->w + x0);
    PFcolor c10 = tex->getter(tex->pixels, y0 * tex->w + x1);
    PFcolor c01 = tex->getter(tex->pixels, y1 * tex->w + x0);
    PFcolor c11 = tex->getter(tex->pixels, y1 * tex->w + x1);

    // Interpolate colors horizontally
    PFcolor c0 = pfiColorLerpSmooth(c00, c10, fx);
    PFcolor c1 = pfiColorLerpSmooth(c01, c11, fx);

    // Interpolate colors vertically
    return pfiColorLerpSmooth(c0, c1, fy);
}

static const PFtexturesampler GC_textureSamplers[2][3] = {

    [PF_NEAREST][PF_REPEAT]             = pfiTexture2DSampler_NEAREST_REPEAT,
    [PF_NEAREST][PF_MIRRORED_REPEAT]    = pfiTexture2DSampler_NEAREST_MIRRORED_REPEAT,
    [PF_NEAREST][PF_CLAMP_TO_EDGE]      = pfiTexture2DSampler_NEAREST_CLAMP_TO_EDGE,

    [PF_BILINEAR][PF_REPEAT]            = pfiTexture2DSampler_BILINEAR_REPEAT,
    [PF_BILINEAR][PF_MIRRORED_REPEAT]   = pfiTexture2DSampler_BILINEAR_MIRRORED_REPEAT,
    [PF_BILINEAR][PF_CLAMP_TO_EDGE]     = pfiTexture2DSampler_BILINEAR_CLAMP_TO_EDGE,

};

/* SIMD Implementation */

#if PF_SIMD_SUPPORT

/* SIMD - Texture2D Mapper Functions */

static inline void
pfiTexture2DMap_REPEAT_simd(const struct PFtex* tex, PFsimdvi* xOut, PFsimdvi* yOut, const PFsimdv2f texcoords)
{
    PFsimdvf u = texcoords[0];
    PFsimdvf v = texcoords[1];

    u = pfiSimdMul_F32(
        pfiSimdSub_F32(u, pfiSimdRound_F32(u, _MM_FROUND_TO_ZERO)),
        pfiSimdSet1_F32(tex->w - 1));

    v = pfiSimdMul_F32(
        pfiSimdSub_F32(v, pfiSimdRound_F32(v, _MM_FROUND_TO_ZERO)),
        pfiSimdSet1_F32(tex->h - 1));

    *xOut = pfiSimdAbs_I32(pfiSimdConvert_F32_I32(u));
    *yOut = pfiSimdAbs_I32(pfiSimdConvert_F32_I32(v));
}

static inline void
pfiTexture2DMap_MIRRORED_REPEAT_simd(const struct PFtex* tex, PFsimdvi* xOut, PFsimdvi* yOut, const PFsimdv2f texcoords)
{
    PFsimdvf u = texcoords[0];
    PFsimdvf v = texcoords[1];

    // Repeating UV coordinates in the interval [0..2]
    u = pfiSimdMod_F32(pfiSimdAbs_F32(u), *(PFsimdvf*)GC_simd_f32_2);
    v = pfiSimdMod_F32(pfiSimdAbs_F32(v), *(PFsimdvf*)GC_simd_f32_2);

    // Reflection to get the interval [0..1] if necessary
    PFsimdvf uMirror = pfiSimdSub_F32(*(PFsimdvf*)GC_simd_f32_1, pfiSimdSub_F32(u, *(PFsimdvf*)GC_simd_f32_1));
    PFsimdvf vMirror = pfiSimdSub_F32(*(PFsimdvf*)GC_simd_f32_1, pfiSimdSub_F32(v, *(PFsimdvf*)GC_simd_f32_1));

    u = pfiSimdBlendV_F32(u, uMirror, pfiSimdCmpGT_F32(u, *(PFsimdvf*)GC_simd_f32_1));
    v = pfiSimdBlendV_F32(v, vMirror, pfiSimdCmpGT_F32(v, *(PFsimdvf*)GC_simd_f32_1));

    // Conversion des coordonnées UV en indices de pixels
    u = pfiSimdMul_F32(u, pfiSimdSet1_F32((float)(tex->w - 1)));
    v = pfiSimdMul_F32(v, pfiSimdSet1_F32((float)(tex->h - 1)));

    *xOut = pfiSimdConvert_F32_I32(pfiSimdAdd_F32(u, *(PFsimdvf*)GC_simd_f32_0p5));
    *yOut = pfiSimdConvert_F32_I32(pfiSimdAdd_F32(v, *(PFsimdvf*)GC_simd_f32_0p5));
}

static inline void
pfiTexture2DMap_CLAMP_TO_EDGE_simd(const struct PFtex* tex, PFsimdvi* xOut, PFsimdvi* yOut, const PFsimdv2f texcoords)
{
    PFsimdvf u = texcoords[0];
    PFsimdvf v = texcoords[1];

    // Clamping des coordonnées UV
    u = pfiSimdClamp_F32(u, *(PFsimdvf*)GC_simd_f32_0, *(PFsimdvf*)GC_simd_f32_1);
    v = pfiSimdClamp_F32(v, *(PFsimdvf*)GC_simd_f32_0, *(PFsimdvf*)GC_simd_f32_1);

    // Conversion des coordonnées UV en indices de pixels
    u = pfiSimdMul_F32(u, pfiSimdSet1_F32((float)(tex->w - 1)));
    v = pfiSimdMul_F32(v, pfiSimdSet1_F32((float)(tex->h - 1)));

    *xOut = pfiSimdConvert_F32_I32(pfiSimdAdd_F32(u, *(PFsimdvf*)GC_simd_f32_0p5));
    *yOut = pfiSimdConvert_F32_I32(pfiSimdAdd_F32(v, *(PFsimdvf*)GC_simd_f32_0p5));
}

/* SIMD - Texture2D Sampler Functions */

static inline PFsimdvi
pfiTexture2DSampler_NEAREST_REPEAT_simd(const struct PFtex* tex, const PFsimdv2f texcoords)
{
    PFsimdvi x, y;
    pfiTexture2DMap_REPEAT_simd(
        tex, &x, &y, texcoords);

    PFsimdvi offsets = pfiSimdAdd_I32(pfiSimdMullo_I32(
        y, pfiSimdSet1_I32(tex->w)), x);

    return tex->getterSimd(tex->pixels, offsets);
}

static inline PFsimdvi
pfiTexture2DSampler_NEAREST_MIRRORED_REPEAT_simd(const struct PFtex* tex, const PFsimdv2f texcoords)
{
    PFsimdvi x, y;
    pfiTexture2DMap_MIRRORED_REPEAT_simd(
        tex, &x, &y, texcoords);

    PFsimdvi offsets = pfiSimdAdd_I32(pfiSimdMullo_I32(
        y, pfiSimdSet1_I32(tex->w)), x);

    return tex->getterSimd(tex->pixels, offsets);
}

static inline PFsimdvi
pfiTexture2DSampler_NEAREST_CLAMP_TO_EDGE_simd(const struct PFtex* tex, const PFsimdv2f texcoords)
{
    PFsimdvi x, y;
    pfiTexture2DMap_CLAMP_TO_EDGE_simd(
        tex, &x, &y, texcoords);

    PFsimdvi offsets = pfiSimdAdd_I32(pfiSimdMullo_I32(
        y, pfiSimdSet1_I32(tex->w)), x);

    return tex->getterSimd(tex->pixels, offsets);
}

static inline PFsimdvi
pfiTexture2DSampler_BILINEAR_REPEAT_simd(const struct PFtex* tex, const PFsimdv2f texcoords)
{
    PFsimdvi x0, y0, x1, y1;
    PFsimdvf fx, fy;

    PFsimdv2f texelSize;
    pfiVec2Set_simd(texelSize, tex->tx, tex->ty);

    PFsimdv2f texcoords2;
    pfiVec2AddR_simd(texcoords2, texcoords, texelSize);

    pfiTexture2DMap_REPEAT_simd(tex, &x0, &y0, texcoords);
    pfiTexture2DMap_REPEAT_simd(tex, &x1, &y1, texcoords2);

    // Calculate fractions fx, fy
    fx = pfiSimdSub_F32(pfiSimdMul_F32(texcoords[0], pfiSimdSet1_F32(tex->w)), pfiSimdConvert_I32_F32(x0));
    fy = pfiSimdSub_F32(pfiSimdMul_F32(texcoords[1], pfiSimdSet1_F32(tex->h)), pfiSimdConvert_I32_F32(y0));

    // Get the colors of the four pixels
    PFcolor_simd c00; pfiColorUnpack_simd(c00, tex->getterSimd(tex->pixels, y0 * tex->w + x0));
    PFcolor_simd c10; pfiColorUnpack_simd(c10, tex->getterSimd(tex->pixels, y0 * tex->w + x1));
    PFcolor_simd c01; pfiColorUnpack_simd(c01, tex->getterSimd(tex->pixels, y1 * tex->w + x0));
    PFcolor_simd c11; pfiColorUnpack_simd(c11, tex->getterSimd(tex->pixels, y1 * tex->w + x1));

    // Interpolate colors horizontally
    PFcolor_simd c0; pfiColorLerpSmooth_simd(c0, c00, c10, fx);
    PFcolor_simd c1; pfiColorLerpSmooth_simd(c1, c01, c11, fx);

    // Interpolate colors vertically
    PFcolor_simd c; pfiColorLerpSmooth_simd(c, c0, c1, fy);

    return pfiColorPack_simd(c);
}

static inline PFsimdvi
pfiTexture2DSampler_BILINEAR_MIRRORED_REPEAT_simd(const struct PFtex* tex, const PFsimdv2f texcoords)
{
    PFsimdvi x0, y0, x1, y1;
    PFsimdvf fx, fy;

    PFsimdv2f texelSize;
    pfiVec2Set_simd(texelSize, tex->tx, tex->ty);

    PFsimdv2f texcoords2;
    pfiVec2AddR_simd(texcoords2, texcoords, texelSize);

    pfiTexture2DMap_MIRRORED_REPEAT_simd(tex, &x0, &y0, texcoords);
    pfiTexture2DMap_MIRRORED_REPEAT_simd(tex, &x1, &y1, texcoords2);

    // Calculate fractions fx, fy
    fx = pfiSimdSub_F32(pfiSimdMul_F32(texcoords[0], pfiSimdSet1_F32(tex->w)), pfiSimdConvert_I32_F32(x0));
    fy = pfiSimdSub_F32(pfiSimdMul_F32(texcoords[1], pfiSimdSet1_F32(tex->h)), pfiSimdConvert_I32_F32(y0));

    // Get the colors of the four pixels
    PFcolor_simd c00; pfiColorUnpack_simd(c00, tex->getterSimd(tex->pixels, y0 * tex->w + x0));
    PFcolor_simd c10; pfiColorUnpack_simd(c10, tex->getterSimd(tex->pixels, y0 * tex->w + x1));
    PFcolor_simd c01; pfiColorUnpack_simd(c01, tex->getterSimd(tex->pixels, y1 * tex->w + x0));
    PFcolor_simd c11; pfiColorUnpack_simd(c11, tex->getterSimd(tex->pixels, y1 * tex->w + x1));

    // Interpolate colors horizontally
    PFcolor_simd c0; pfiColorLerpSmooth_simd(c0, c00, c10, fx);
    PFcolor_simd c1; pfiColorLerpSmooth_simd(c1, c01, c11, fx);

    // Interpolate colors vertically
    PFcolor_simd c; pfiColorLerpSmooth_simd(c, c0, c1, fy);

    return pfiColorPack_simd(c);
}

static inline PFsimdvi
pfiTexture2DSampler_BILINEAR_CLAMP_TO_EDGE_simd(const struct PFtex* tex, const PFsimdv2f texcoords)
{
    PFsimdvi x0, y0, x1, y1;
    PFsimdvf fx, fy;

    PFsimdv2f texelSize;
    pfiVec2Set_simd(texelSize, tex->tx, tex->ty);

    PFsimdv2f texcoords2;
    pfiVec2AddR_simd(texcoords2, texcoords, texelSize);

    pfiTexture2DMap_CLAMP_TO_EDGE_simd(tex, &x0, &y0, texcoords);
    pfiTexture2DMap_CLAMP_TO_EDGE_simd(tex, &x1, &y1, texcoords2);

    // Calculate fractions fx, fy
    fx = pfiSimdSub_F32(pfiSimdMul_F32(texcoords[0], pfiSimdSet1_F32(tex->w)), pfiSimdConvert_I32_F32(x0));
    fy = pfiSimdSub_F32(pfiSimdMul_F32(texcoords[1], pfiSimdSet1_F32(tex->h)), pfiSimdConvert_I32_F32(y0));

    // Get the colors of the four pixels
    PFcolor_simd c00; pfiColorUnpack_simd(c00, tex->getterSimd(tex->pixels, y0 * tex->w + x0));
    PFcolor_simd c10; pfiColorUnpack_simd(c10, tex->getterSimd(tex->pixels, y0 * tex->w + x1));
    PFcolor_simd c01; pfiColorUnpack_simd(c01, tex->getterSimd(tex->pixels, y1 * tex->w + x0));
    PFcolor_simd c11; pfiColorUnpack_simd(c11, tex->getterSimd(tex->pixels, y1 * tex->w + x1));

    // Interpolate colors horizontally
    PFcolor_simd c0; pfiColorLerpSmooth_simd(c0, c00, c10, fx);
    PFcolor_simd c1; pfiColorLerpSmooth_simd(c1, c01, c11, fx);

    // Interpolate colors vertically
    PFcolor_simd c; pfiColorLerpSmooth_simd(c, c0, c1, fy);

    return pfiColorPack_simd(c);
}

static const PFtexturesampler_simd GC_textureSamplers_simd[2][3] = {

    [PF_NEAREST][PF_REPEAT]             = pfiTexture2DSampler_NEAREST_REPEAT_simd,
    [PF_NEAREST][PF_MIRRORED_REPEAT]    = pfiTexture2DSampler_NEAREST_MIRRORED_REPEAT_simd,
    [PF_NEAREST][PF_CLAMP_TO_EDGE]      = pfiTexture2DSampler_NEAREST_CLAMP_TO_EDGE_simd,

    [PF_BILINEAR][PF_REPEAT]            = pfiTexture2DSampler_BILINEAR_REPEAT_simd,
    [PF_BILINEAR][PF_MIRRORED_REPEAT]   = pfiTexture2DSampler_BILINEAR_MIRRORED_REPEAT_simd,
    [PF_BILINEAR][PF_CLAMP_TO_EDGE]     = pfiTexture2DSampler_BILINEAR_CLAMP_TO_EDGE_simd,

};

#endif //PF_SIMD_SUPPORT


/* Internal helper functions */

static inline PFboolean
pfiIsTextureParameterValid(PFtexturewrap wrapMode, PFtexturefilter filterMode)
{
    return (wrapMode >= PF_REPEAT && wrapMode <= PF_CLAMP_TO_EDGE)
        && (filterMode >= PF_NEAREST && filterMode <= PF_BILINEAR);
}

#endif //PF_INTERNAL_SAMPLER_H
