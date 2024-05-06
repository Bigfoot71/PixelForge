#ifndef PF_INTERNAL_LIGHTING_H
#define PF_INTERNAL_LIGHTING_H

#include "../context.h"

PFcolor Process_Lights(const PFlight* activeLights, const PFmaterial* material,
    PFcolor diffuse, const PFMvec3 viewPos, const PFMvec3 fragPos, const PFMvec3 fragNormal);

#endif //PF_INTERNAL_LIGHTING_H