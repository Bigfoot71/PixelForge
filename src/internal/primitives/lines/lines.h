#ifndef PF_LINES_H
#define PF_LINES_H

#include "../../context.h"
#include "../../../pfm.h"

void Process_ProjectAndClipLine(PFvertex* restrict line, int_fast8_t* restrict vertexCounter, const PFMmat4 mvp);

void Rasterize_Line_NODEPTH(const PFvertex* v1, const PFvertex* v2);
void Rasterize_Line_DEPTH(const PFvertex* v1, const PFvertex* v2);

void Rasterize_Line_THICK_NODEPTH(const PFvertex* v1, const PFvertex* v2);
void Rasterize_Line_THICK_DEPTH(const PFvertex* v1, const PFvertex* v2);

#endif //PF_LINES_H