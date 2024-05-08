#ifndef PF_TRIANGLES_H
#define PF_TRIANGLES_H

#include "../../context.h"
#include "../../config.h"

typedef enum {
    RASTER_TEXTURE  = 0x01,
    RASTER_DEPTH    = 0x02,
    RASTER_LIGHT    = 0x04,
    RASTER_FRONT    = 0x08,
    RASTER_3D       = 0x10
} TriangleRasterMode;

PFboolean Process_ProjectAndClipTriangle(PFvertex* restrict polygon, int_fast8_t* restrict vertexCounter, const PFMmat4 mvp);
void Rasterize_Triangle(PFface faceToRender, PFboolean is3D, const PFvertex* v1, const PFvertex* v2, const PFvertex* v3, const PFMvec3 viewPos);

#endif //PF_TRIANGLES_H
