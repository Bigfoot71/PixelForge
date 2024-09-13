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

static inline void pfiSwapVertex(PFvertex* a, PFvertex* b)
{
    PFvertex tmp = *a;
    *a = *b; *b = tmp;
}

static inline void pfiSwapByte(PFubyte* a, PFubyte* b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

static inline PFvertex pfiLerpVertex(const PFvertex* start, const PFvertex* end, PFfloat t)
{
    PFvertex result = { 0 };

    const PFubyte *startCol = (const PFubyte*)(&start->color);
    const PFubyte *endCol = (const PFubyte*)(&end->color);
    PFubyte *resultCol = (PFubyte*)(&result.color);
    PFubyte uT = 255*t;

#   ifdef _OPENMP
#       pragma omp simd
#   endif
    for (int_fast8_t i = 0; i < 4; i++) {
        result.homogeneous[i] = start->homogeneous[i] + t*(end->homogeneous[i] - start->homogeneous[i]);
        result.position[i] = start->position[i] + t*(end->position[i] - start->position[i]);
        resultCol[i] = startCol[i] + (uT*((PFint)endCol[i] - startCol[i]))/255;

        if (i < 2) result.texcoord[i] = start->texcoord[i] + t*(end->texcoord[i] - start->texcoord[i]);
        if (i < 3) result.normal[i] = start->normal[i] + t*(end->normal[i] - start->normal[i]);
    }

    return result;
}

#endif //PF_INTERNAL_HELPER_H
