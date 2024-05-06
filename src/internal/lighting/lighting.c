#include "./lighting.h"
#include <stdint.h>

PFcolor Process_Lights(const PFlight* activeLights, const PFmaterial* material, PFcolor diffuse, const PFMvec3 viewPos, const PFMvec3 fragPos, const PFMvec3 fragNormal)
{
    // Final color
    // Calculate the emission component of the final color
    PFubyte R = material->emission.r;
    PFubyte G = material->emission.g;
    PFubyte B = material->emission.b;

    // Ambient component
    // Calculate the ambient lighting contribution
    PFubyte aR = (material->ambient.r*diffuse.r)/255;
    PFubyte aG = (material->ambient.g*diffuse.g)/255;
    PFubyte aB = (material->ambient.b*diffuse.b)/255;

    // Compute the view direction from fragment position
    PFMvec3 viewDir;
    pfmVec3Sub(viewDir, viewPos, fragPos);
    pfmVec3Normalize(viewDir, viewDir);

    // Specular properties
    // Retrieve material's shininess and specular color
    PFfloat shininess = material->shininess;
    PFcolor specular = material->specular;

    // Loop through active lights
    for (const PFlight *light = activeLights; light != NULL; light = light->next)
    {
        // Declare the light contribution and initialize it to zero for now.
        PFubyte lR = 0, lG = 0, lB = 0;

        // Compute light direction
        PFMvec3 lightDir;
        pfmVec3Sub(lightDir, light->position, fragPos);

        // Compute distance from light to fragment position
        // And also normalize the light direction if necessary.
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

        // Spotlight (soft edges)
        PFubyte intensity = 255;
        if (light->innerCutOff < M_PI)
        {
            PFMvec3 negLightDir;
            pfmVec3Neg(negLightDir, light->direction);

            PFfloat theta = pfmVec3Dot(lightDir, negLightDir);
            PFfloat epsilon = light->innerCutOff - light->outerCutOff;
            intensity = CLAMP((PFint)(255*(theta - light->outerCutOff)/epsilon), 0, 255);

            if (intensity == 0)
                goto apply_light_contribution;
        }

        // Attenuation
        PFubyte attenuation = 255;
        if (light->attLinear || light->attQuadratic)
        {
            attenuation = 255/(light->attConstant +
                light->attLinear*lightToFragPosDist +
                light->attQuadratic*lightToFragPosDistSq);

            if (attenuation == 0)
                goto apply_light_contribution;
        }

        // Factor used to scale the final color
        PFubyte factor = (intensity*attenuation)/255;

        // Diffuse component
        // Calculate the diffuse reflection of the light
        PFubyte diff = MAX((PFint)(255*pfmVec3Dot(fragNormal, lightDir)), 0);
        lR = MIN_255(lR + (diffuse.r*light->diffuse.r*diff)/(255*255));
        lG = MIN_255(lG + (diffuse.g*light->diffuse.g*diff)/(255*255));
        lB = MIN_255(lB + (diffuse.b*light->diffuse.b*diff)/(255*255));

        // Specular component
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

        // Apply spotlight soft edges and distance attenuation
        lR = (lR*factor)/255;
        lG = (lG*factor)/255;
        lB = (lB*factor)/255;

        // Add ambient contribution of the light
        // Then add the light's contribution to the final color
        apply_light_contribution:
        R = MIN_255(R + lR + (aR*light->ambient.r)/255);
        G = MIN_255(G + lG + (aG*light->ambient.g)/255);
        B = MIN_255(B + lB + (aB*light->ambient.b)/255);
    }

    // Return the final computed color
    return (PFcolor) { R, G, B, diffuse.a };
}
