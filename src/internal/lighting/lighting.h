#ifndef PF_INTERNAL_LIGHTING_H
#define PF_INTERNAL_LIGHTING_H

#include "../context.h"

PFcolor Process_Light(const PFlight* light,
    PFcolor ambient, PFcolor texel,
    const PFMvec3 viewPosition,
    const PFMvec3 position,
    const PFMvec3 normal,
    PFfloat shininess);

#ifdef PF_GOURAUD_SHADING
PFcolor Process_Gouraud(const PFctx* ctx,
    const PFvertex* v, const PFMvec3 viewPos,
    const PFmaterial* material);
#endif //PF_GOURAUD_SHADING


#endif //PF_INTERNAL_LIGHTING_H