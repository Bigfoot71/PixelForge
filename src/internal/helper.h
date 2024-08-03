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

#ifndef PF_INTERNAL_HELPER_H
#define PF_INTERNAL_HELPER_H

#include "context/context.h"

static inline void pfInternal_SwapVertex(PFvertex* a, PFvertex* b)
{
    PFvertex tmp = *a;
    *a = *b; *b = tmp;
}

static inline void pfInternal_SwapByte(PFubyte* a, PFubyte* b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

static inline PFvertex pfInternal_LerpVertex(const PFvertex* start, const PFvertex* end, PFfloat t)
{
    PFvertex result = { 0 };

    const PFubyte *startCol = (const PFubyte*)(&start->color);
    const PFubyte *endCol = (const PFubyte*)(&end->color);
    PFubyte *resultCol = (PFubyte*)(&result.color);
    PFubyte uT = 255*t;

#   ifdef PF_SUPPORT_OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++)
    {
        result.homogeneous[i] = start->homogeneous[i] + t*(end->homogeneous[i] - start->homogeneous[i]);
        result.position[i] = start->position[i] + t*(end->position[i] - start->position[i]);
        resultCol[i] = startCol[i] + (uT*((PFint)endCol[i] - startCol[i]))/255;

        if (i < 2) result.texcoord[i] = start->texcoord[i] + t*(end->texcoord[i] - start->texcoord[i]);
        if (i < 3) result.normal[i] = start->normal[i] + t*(end->normal[i] - start->normal[i]);
    }

    return result;
}

static inline PFcolor pfInternal_LerpColor_SMOOTH(PFcolor a, PFcolor b, PFfloat t)
{
    return (PFcolor) {
        a.r + t*(b.r - a.r),
        a.g + t*(b.g - a.g),
        a.b + t*(b.b - a.b),
        a.a + t*(b.a - a.a)
    };
}

static inline PFcolor pfInternal_LerpColor_FLAT(PFcolor v1, PFcolor v2, PFfloat t)
{
    return (t < 0.5f) ? v1 : v2;
}

static inline PFcolor pfInternal_BaryColor_SMOOTH(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    PFubyte uW1 = 255*w1;
    PFubyte uW2 = 255*w2;
    PFubyte uW3 = 255*w3;

    return (PFcolor) {
        ((uW1*v1.r) + (uW2*v2.r) + (uW3*v3.r))/255,
        ((uW1*v1.g) + (uW2*v2.g) + (uW3*v3.g))/255,
        ((uW1*v1.b) + (uW2*v2.b) + (uW3*v3.b))/255,
        ((uW1*v1.a) + (uW2*v2.a) + (uW3*v3.a))/255
    };
}

static inline PFcolor pfInternal_BaryColor_FLAT(PFcolor v1, PFcolor v2, PFcolor v3, PFfloat w1, PFfloat w2, PFfloat w3)
{
    return ((w1 > w2) & (w1 > w3)) ? v1 : (w2 >= w3) ? v2 : v3;
}


#endif //PF_INTERNAL_HELPER_H
