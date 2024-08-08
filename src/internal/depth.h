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

#ifndef PF_INTERNAL_DEPTH_H
#define PF_INTERNAL_DEPTH_H

#include "./context/context.h"
#include "../pixelforge.h"
#include "../pfm.h"

/* SISD Depth testing functions */

static inline PFboolean
pfInternal_DepthTest_EQ(PFfloat src, PFfloat dst)
{
    return (src == dst);
}

static inline PFboolean
pfInternal_DepthTest_NEQ(PFfloat src, PFfloat dst)
{
    return (src != dst);
}

static inline PFboolean
pfInternal_DepthTest_LT(PFfloat src, PFfloat dst)
{
    return (src < dst);
}

static inline PFboolean
pfInternal_DepthTest_LE(PFfloat src, PFfloat dst)
{
    return (src <= dst);
}

static inline PFboolean
pfInternal_DepthTest_GT(PFfloat src, PFfloat dst)
{
    return (src > dst);
}

static inline PFboolean
pfInternal_DepthTest_GE(PFfloat src, PFfloat dst)
{
    return (src >= dst);
}

/* SIMD Depth testing functions */

static inline PFMsimd_f
pfInternal_SimdDepthTest_EQ(PFMsimd_f src, PFMsimd_f dst)
{
    return pfmSimdCmpEQ_F32(src, dst);
}

static inline PFMsimd_f
pfInternal_SimdDepthTest_NEQ(PFMsimd_f src, PFMsimd_f dst)
{
    return pfmSimdCmpEQ_F32(src, dst);
}

static inline PFMsimd_f
pfInternal_SimdDepthTest_LT(PFMsimd_f src, PFMsimd_f dst)
{
    return pfmSimdCmpLT_F32(src, dst);
}

static inline PFMsimd_f
pfInternal_SimdDepthTest_LE(PFMsimd_f src, PFMsimd_f dst)
{
    return pfmSimdCmpLE_F32(src, dst);
}

static inline PFMsimd_f
pfInternal_SimdDepthTest_GT(PFMsimd_f src, PFMsimd_f dst)
{
    return pfmSimdCmpGT_F32(src, dst);
}

static inline PFMsimd_f
pfInternal_SimdDepthTest_GE(PFMsimd_f src, PFMsimd_f dst)
{
    return pfmSimdCmpGE_F32(src, dst);
}

/* Get Depth Function */

static inline PFboolean
pfInternal_IsDepthModeValid(PFdepthmode mode)
{
    return (mode >= PF_EQUAL && mode <= PF_GEQUAL);
}

static inline void
pfInternal_GetDepthFuncs(PFdepthmode mode, PFdepthfunc* depthTest, PFdepthfunc_simd* depthTestSimd)
{
#   define ENTRY(MODE, FUNC) [MODE] = FUNC

    static const PFdepthfunc depthTests[6] = {
        ENTRY(PF_EQUAL, pfInternal_DepthTest_EQ),
        ENTRY(PF_NOTEQUAL, pfInternal_DepthTest_NEQ),
        ENTRY(PF_LESS, pfInternal_DepthTest_LT),
        ENTRY(PF_LEQUAL, pfInternal_DepthTest_LE),
        ENTRY(PF_GREATER, pfInternal_DepthTest_GT),
        ENTRY(PF_GEQUAL, pfInternal_DepthTest_GE)
    };

    static const PFdepthfunc_simd depthTestsSimd[6] = {
        ENTRY(PF_EQUAL, pfInternal_SimdDepthTest_EQ),
        ENTRY(PF_NOTEQUAL, pfInternal_SimdDepthTest_NEQ),
        ENTRY(PF_LESS, pfInternal_SimdDepthTest_LT),
        ENTRY(PF_LEQUAL, pfInternal_SimdDepthTest_LE),
        ENTRY(PF_GREATER, pfInternal_SimdDepthTest_GT),
        ENTRY(PF_GEQUAL, pfInternal_SimdDepthTest_GE)
    };

    if (depthTest) *depthTest = depthTests[mode];
    if (depthTestSimd) *depthTestSimd = depthTestsSimd[mode];

#   undef ENTRY
}

#endif //PF_INTERNAL_DEPTH_H
