#ifndef PF_POINTS_H
#define PF_POINTS_H

#include "../../context.h"
#include "../../../pfm.h"

PFboolean Process_ProjectPoint(PFvertex* restrict v, const PFMmat4 mvp);
void Rasterize_Point_NODEPTH(const PFvertex* point);
void Rasterize_Point_DEPTH(const PFvertex* point);

#endif //PF_POINTS_H