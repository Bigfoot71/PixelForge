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

    PFubyte diff = 255*fmaxf(pfmVec3Dot(normal, lightDir), 0.0f);
    PFcolor diffuse = light->diffuse;

    diffuse.r = (diffuse.r*texel.r*diff)/(255*255);
    diffuse.g = (diffuse.g*texel.g*diff)/(255*255);
    diffuse.b = (diffuse.b*texel.b*diff)/(255*255);
    diffuse.a = texel.a;

    // specular
#ifndef PF_PHONG_REFLECTION
    // Blinn-Phong
    PFMvec3 halfWayDir;
    pfmVec3Add(halfWayDir, lightDir, viewDir);
    pfmVec3Normalize(halfWayDir, halfWayDir);
    PFubyte spec = 255*powf(fmaxf(pfmVec3Dot(normal, halfWayDir), 0.0f), shininess);
#else
    // Phong
    PFMvec3 reflectionDir, negLightDir;
    pfmVec3Neg(negLightDir, lightDir);
    pfmVec3Reflect(reflectionDir, negLightDir, normal);
    PFubyte spec = 255*powf(fmaxf(pfmVec3Dot(reflectionDir, viewDir), 0.0f), shininess);
#endif

    PFcolor specular = light->specular;
    specular.r = (specular.r*spec)/255;
    specular.g = (specular.g*spec)/255;
    specular.b = (specular.b*spec)/255;

    // spotlight (soft edges)
    PFubyte intensity = 255;
    if (light->cutoff < M_PI)
    {
        PFMvec3 negLightDir;
        pfmVec3Neg(negLightDir, light->direction);

        PFfloat theta = pfmVec3Dot(lightDir, negLightDir);
        PFfloat epsilon = light->cutoff - light->outerCutoff;
        intensity = 255*CLAMP((theta - light->outerCutoff) / epsilon, 0.0f, 1.0f);
    }

    // attenuation
    PFubyte attenuation = 255;
    if (light->attLinear || light->attQuadratic)
    {
        PFfloat distanceSq =
            lightFragPosDt[0]*lightFragPosDt[0] +
            lightFragPosDt[1]*lightFragPosDt[1] +
            lightFragPosDt[2]*lightFragPosDt[2];

        attenuation = 255/(
            light->attConstant +
            light->attLinear*sqrtf(distanceSq) +
            light->attQuadratic*distanceSq);
    }

    // add final light color
    PFcolor finalColor = pfBlendAdditive(diffuse, specular);
    PFubyte factor = (intensity*attenuation)/255;

    finalColor.r = (finalColor.r*factor)/255;
    finalColor.g = (finalColor.g*factor)/255;
    finalColor.b = (finalColor.b*factor)/255;

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
