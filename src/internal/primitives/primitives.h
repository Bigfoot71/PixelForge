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

#ifndef PF_PRIMITIVES_H
#define PF_PRIMITIVES_H

#include "../../pixelforge.h"

void pfiProcessRasterize_POINT(void);
void pfiProcessRasterize_POLY_POINTS(int_fast8_t vertexCount);

void pfiProcessRasterize_LINE(void);
void pfiProcessRasterize_POLY_LINES(int_fast8_t vertexCount);

void pfiProcessRasterize_TRIANGLE(PFface faceToRender);
void pfiProcessRasterize_TRIANGLE_FAN(PFface faceToRender, int_fast8_t numTriangles);
void pfiProcessRasterize_TRIANGLE_STRIP(PFface faceToRender, int_fast8_t numTriangles);

#endif //PF_PRIMITIVES_H
