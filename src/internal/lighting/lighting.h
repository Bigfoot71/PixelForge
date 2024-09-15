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

#ifndef PF_INTERNAL_LIGHTING_H
#define PF_INTERNAL_LIGHTING_H

#include "../context/context.h"

PFcolor pfiLightingProcess(const PFIlight* activeLights, const PFImaterial* material,
                                   PFcolor diffuse, const PFMvec3 viewPos,
                                   const PFMvec3 fragPos, const PFMvec3 N);

#if PF_SIMD_SUPPORT
void pfiSimdLightingProcess(PFcolor_simd fragments, const PFIlight* activeLights,
                                    const PFImaterial* material,
                                    const PFsimdv3f viewPos,
                                    const PFsimdv3f fragPos,
                                    const PFsimdv3f N);
#endif //PF_SIMD_SUPPORT

#endif //PF_INTERNAL_LIGHTING_H
