#include "./lighting.h"

PFcolor Process_Light(const PFlight* light, PFcolor ambient, PFcolor texel, const PFMvec3 viewPosition, const PFMvec3 position, const PFMvec3 normal, PFfloat shininess)
{
    // get view direction for this fragment position **(can be optimized)**
    PFMvec3 viewDir;
    pfmVec3Sub(viewDir, viewPosition, position);
    pfmVec3Normalize(viewDir, viewDir);

    // Compute ambient lighting contribution
    ambient = pfBlendMultiplicative(texel, ambient);

    // diffuse
    PFMvec3 lightFragPosDt;
    pfmVec3Sub(lightFragPosDt, light->position, position);

    PFMvec3 lightDir;
    pfmVec3Normalize(lightDir, lightFragPosDt);

    PFfloat diff = fmaxf(pfmVec3Dot(normal, lightDir), 0.0f);

    PFcolor diffuse = pfBlendMultiplicative(light->diffuse, texel);
    diffuse.r = (PFubyte)((PFfloat)diffuse.r*diff);
    diffuse.g = (PFubyte)((PFfloat)diffuse.g*diff);
    diffuse.b = (PFubyte)((PFfloat)diffuse.b*diff);

    // specular
#ifndef PF_PHONG_REFLECTION
    // Blinn-Phong
    PFMvec3 halfWayDir;
    pfmVec3Add(halfWayDir, lightDir, viewDir);
    pfmVec3Normalize(halfWayDir, halfWayDir);
    PFfloat spec = powf(fmaxf(pfmVec3Dot(normal, halfWayDir), 0.0f), shininess);
#else
    // Phong
    PFMvec3 reflectionDir, negLightDir;
    pfmVec3Neg(negLightDir, lightDir);
    pfmVec3Reflect(reflectionDir, negLightDir, normal);
    PFfloat spec = powf(fmaxf(pfmVec3Dot(reflectionDir, viewDir), 0.0f), shininess);
#endif

    const PFcolor specular = {
        (PFubyte)((PFfloat)light->specular.r*spec),
        (PFubyte)((PFfloat)light->specular.g*spec),
        (PFubyte)((PFfloat)light->specular.b*spec),
        255
    };

    // spotlight (soft edges)
    PFfloat intensity = 1.0f;
    if (light->cutoff < M_PI)
    {
        PFMvec3 negLightDir;
        pfmVec3Neg(negLightDir, light->direction);

        PFfloat theta = pfmVec3Dot(lightDir, negLightDir);
        PFfloat epsilon = light->cutoff - light->outerCutoff;
        intensity = CLAMP((theta - light->outerCutoff) / epsilon, 0.0f, 1.0f);
    }

    // attenuation
    PFfloat attenuation = 1.0f;
    if (light->attLinear || light->attQuadratic)
    {
        PFfloat distanceSq = lightFragPosDt[0]*lightFragPosDt[0] +
                             lightFragPosDt[1]*lightFragPosDt[1] +
                             lightFragPosDt[2]*lightFragPosDt[2];

        PFfloat distance = sqrtf(distanceSq);

        attenuation = 1.0f/(light->attConstant + light->attLinear*distance + light->attQuadratic*distanceSq);
    }

    // add final light color
    PFcolor finalColor = pfBlendAdditive(diffuse, specular);
    PFfloat factor = intensity*attenuation;

    finalColor.r = (PFubyte)((PFfloat)finalColor.r*factor);
    finalColor.g = (PFubyte)((PFfloat)finalColor.g*factor);
    finalColor.b = (PFubyte)((PFfloat)finalColor.b*factor);

    return pfBlendAdditive(ambient, finalColor);
}

#ifdef PF_GOURAUD_SHADING
PFcolor Process_Gouraud(const PFctx* ctx, const PFvertex* v, const PFMvec3 viewPosition, const PFmaterial* material)
{
    PFcolor finalColor = { 0 };

    for (PFint i = 0; i <= ctx->lastActiveLight; i++)
    {
        const PFlight *light = &ctx->lights[i];
        if (!light->active) continue;

        const PFcolor ambient = pfBlendMultiplicative(light->ambient, material->ambient);
        PFcolor color = Process_Light(light, ambient, v->color, viewPosition, v->position, v->normal, material->shininess);
        color = pfBlendAdditive(color, material->emission);

        finalColor = pfBlendAdditive(finalColor, color);
    }

    return finalColor;
}
#endif //PF_GOURAUD_SHADING
