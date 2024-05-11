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

#ifndef PF_LINES_H
#define PF_LINES_H

#include "../../context.h"
#include "../../../pfm.h"

void Process_ProjectAndClipLine(PFvertex* restrict line, int_fast8_t* restrict vertexCounter);

void Rasterize_Line_NODEPTH(const PFvertex* v1, const PFvertex* v2);
void Rasterize_Line_DEPTH(const PFvertex* v1, const PFvertex* v2);

void Rasterize_Line_THICK_NODEPTH(const PFvertex* v1, const PFvertex* v2);
void Rasterize_Line_THICK_DEPTH(const PFvertex* v1, const PFvertex* v2);

#endif //PF_LINES_H