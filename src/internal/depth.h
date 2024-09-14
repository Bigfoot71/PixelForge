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
#include "./simd.h"

/* SISD Depth testing functions */

static inline PFboolean
pfiDepthTest_EQ(PFfloat src, PFfloat dst)
{
    return (src == dst);
}

static inline PFboolean
pfiDepthTest_NEQ(PFfloat src, PFfloat dst)
{
    return (src != dst);
}

static inline PFboolean
pfiDepthTest_LT(PFfloat src, PFfloat dst)
{
    return (src < dst);
}

static inline PFboolean
pfiDepthTest_LE(PFfloat src, PFfloat dst)
{
    return (src <= dst);
}

static inline PFboolean
pfiDepthTest_GT(PFfloat src, PFfloat dst)
{
    return (src > dst);
}

static inline PFboolean
pfiDepthTest_GE(PFfloat src, PFfloat dst)
{
    return (src >= dst);
}

#define ENTRY(MODE, FUNC) [MODE] = FUNC
static const PFdepthfunc GC_depthTestFuncs[6] = {
    ENTRY(PF_EQUAL, pfiDepthTest_EQ),
    ENTRY(PF_NOTEQUAL, pfiDepthTest_NEQ),
    ENTRY(PF_LESS, pfiDepthTest_LT),
    ENTRY(PF_LEQUAL, pfiDepthTest_LE),
    ENTRY(PF_GREATER, pfiDepthTest_GT),
    ENTRY(PF_GEQUAL, pfiDepthTest_GE)
};
#undef ENTRY

/* SIMD Depth testing functions */

#if PF_SIMD_SUPPORT

static inline PFsimdvf
pfiDepthTest_EQ_simd(PFsimdvf src, PFsimdvf dst)
{
    return pfiSimdCmpEQ_F32(src, dst);
}

static inline PFsimdvf
pfiDepthTest_NEQ_simd(PFsimdvf src, PFsimdvf dst)
{
    return pfiSimdCmpEQ_F32(src, dst);
}

static inline PFsimdvf
pfiDepthTest_LT_simd(PFsimdvf src, PFsimdvf dst)
{
    return pfiSimdCmpLT_F32(src, dst);
}

static inline PFsimdvf
pfiDepthTest_LE_simd(PFsimdvf src, PFsimdvf dst)
{
    return pfiSimdCmpLE_F32(src, dst);
}

static inline PFsimdvf
pfiDepthTest_GT_simd(PFsimdvf src, PFsimdvf dst)
{
    return pfiSimdCmpGT_F32(src, dst);
}

static inline PFsimdvf
pfiDepthTest_GE_simd(PFsimdvf src, PFsimdvf dst)
{
    return pfiSimdCmpGE_F32(src, dst);
}

#define ENTRY(MODE, FUNC) [MODE] = FUNC
static const PFdepthfunc_simd GC_depthTestFuncs_simd[6] = {
    ENTRY(PF_EQUAL, pfiDepthTest_EQ_simd),
    ENTRY(PF_NOTEQUAL, pfiDepthTest_NEQ_simd),
    ENTRY(PF_LESS, pfiDepthTest_LT_simd),
    ENTRY(PF_LEQUAL, pfiDepthTest_LE_simd),
    ENTRY(PF_GREATER, pfiDepthTest_GT_simd),
    ENTRY(PF_GEQUAL, pfiDepthTest_GE_simd)
};
#undef ENTRY

#endif //PF_SIMD_SUPPORT

/* Helper Functions */

static inline PFboolean
pfiIsDepthModeValid(PFdepthmode mode)
{
    return (mode >= PF_EQUAL && mode <= PF_GEQUAL);
}

#endif //PF_INTERNAL_DEPTH_H
