#include "./lighting.h"
#include <stdint.h>

PFcolor Process_Lights(const PFlight* activeLights, const PFmaterial* material, PFcolor diffuse, const PFMvec3 viewPos, const PFMvec3 fragPos, const PFMvec3 fragNormal)
{
    // TODO COMMENT (final color)
    PFubyte R = material->emission.r;
    PFubyte G = material->emission.g;
    PFubyte B = material->emission.b;

    // TODO COMMENT
    PFubyte aR = (material->ambient.r*diffuse.r)/255;
    PFubyte aG = (material->ambient.g*diffuse.g)/255;
    PFubyte aB = (material->ambient.b*diffuse.b)/255;

    // TODO COMMENT
    PFMvec3 viewDir;
    pfmVec3Sub(viewDir, viewPos, fragPos);
    pfmVec3Normalize(viewDir, viewDir);

    // TODO COMMENT
    PFfloat shininess = material->shininess;
    PFcolor specular = material->specular;

    // TODO COMMENT
    for (const PFlight *light = activeLights; light != NULL; light = light->next)
    {
        // TODO COMMENT
        PFMvec3 lightDir;
        pfmVec3Sub(lightDir, light->position, fragPos);

        PFfloat lightToFragPosDistSq =
            lightDir[0]*lightDir[0] +
            lightDir[1]*lightDir[1] +
            lightDir[2]*lightDir[2];

        PFfloat lightToFragPosDist = 0;
        if (lightToFragPosDistSq != 0.0f)
        {
            lightToFragPosDist = sqrtf(lightToFragPosDistSq);
            PFfloat invMag = 1.0f/lightToFragPosDist;
            for (int_fast8_t i = 0; i < 3; i++)
            {
                lightDir[i] *= invMag;
            }
        }

        // spotlight (soft edges)
        PFubyte intensity = 255;
        if (light->innerCutOff < M_PI)
        {
            PFMvec3 negLightDir;
            pfmVec3Neg(negLightDir, light->direction);

            PFfloat theta = pfmVec3Dot(lightDir, negLightDir);
            PFfloat epsilon = light->innerCutOff - light->outerCutOff;
            intensity = CLAMP((PFint)(255*(theta - light->outerCutOff)/epsilon), 0, 255);

            if (intensity == 0) continue;
        }

        // attenuation
        PFubyte attenuation = 255;
        if (light->attLinear || light->attQuadratic)
        {
            attenuation = 255/(light->attConstant +
                light->attLinear*lightToFragPosDist +
                light->attQuadratic*lightToFragPosDistSq);

            if (attenuation == 0) continue;
        }

        // TODO COMMENT (utilisé pour multiplié la couleur finale)
        PFubyte factor = (intensity*attenuation)/255;

        // TODO COMMENT
        PFubyte lR = (aR*light->ambient.r)/255;
        PFubyte lG = (aG*light->ambient.g)/255;
        PFubyte lB = (aB*light->ambient.b)/255;

        // diffuse
        PFubyte diff = MAX((PFint)(255*pfmVec3Dot(fragNormal, lightDir)), 0);
        lR = MIN_255(lR + (diffuse.r*light->diffuse.r*diff)/(255*255));
        lG = MIN_255(lG + (diffuse.g*light->diffuse.g*diff)/(255*255));
        lB = MIN_255(lB + (diffuse.b*light->diffuse.b*diff)/(255*255));

        // specular
#       ifndef PF_PHONG_REFLECTION
            // Blinn-Phong
            PFMvec3 halfWayDir;
            pfmVec3Add(halfWayDir, lightDir, viewDir);
            pfmVec3Normalize(halfWayDir, halfWayDir);
            PFubyte spec = 255*powf(fmaxf(pfmVec3Dot(fragNormal, halfWayDir), 0.0f), shininess);
#       else
            // Phong
            PFMvec3 reflectDir, negLightDir;
            pfmVec3Neg(negLightDir, lightDir);
            pfmVec3Reflect(reflectDir, negLightDir, fragNormal);
            PFubyte spec = 255*powf(fmaxf(pfmVec3Dot(reflectDir, viewDir), 0.0f), shininess);
#       endif

        lR = MIN_255(lR + (specular.r*light->specular.r*spec)/(255*255));
        lG = MIN_255(lG + (specular.g*light->specular.g*spec)/(255*255));
        lB = MIN_255(lB + (specular.b*light->specular.b*spec)/(255*255));

        // TODO COMMENT
        R = MIN_255(R + (lR*factor)/255);
        G = MIN_255(G + (lG*factor)/255);
        B = MIN_255(B + (lB*factor)/255);
    }

    // Final color
    return (PFcolor) { R, G, B, diffuse.a };
}

