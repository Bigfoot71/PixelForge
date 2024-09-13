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

#include "./lighting.h"

PFcolor pfiLightingProcess(const PFlight* activeLights, const PFmaterial* material,
                                   PFcolor diffuse, const PFMvec3 viewPos,
                                   const PFMvec3 fragPos, const PFMvec3 N)
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
    PFMvec3 V;
    pfmVec3DirectionR(V, viewPos, fragPos);

    // Specular properties
    // Retrieve material's shininess and specular color
    PFfloat shininess = material->shininess;
    PFcolor specular = material->specular;

    // Loop through active lights
    for (const PFlight *light = activeLights; light != NULL; light = light->next) {
        // Declare the light contribution and initialize it to zero for now.
        PFubyte lR = 0, lG = 0, lB = 0;

        // Compute light direction
        PFMvec3 L;
        pfmVec3SubR(L, light->position, fragPos);

        // Compute distance from light to fragment position
        // And also normalize the light direction if necessary.
        PFfloat lightToFragPosDistSq = L[0]*L[0] + L[1]*L[1] + L[2]*L[2];

        PFfloat lightToFragPosDist = 0;
        if (lightToFragPosDistSq != 0.0f) {
            lightToFragPosDist = sqrtf(lightToFragPosDistSq);
            PFfloat invMag = 1.0f/lightToFragPosDist;
            for (int_fast8_t i = 0; i < 3; i++) {
                L[i] *= invMag;
            }
        }

        // Spotlight (soft edges)
        PFubyte intensity = 255;
        if (light->innerCutOff < M_PI) {
            PFMvec3 negLightDir;
            pfmVec3NegR(negLightDir, light->direction);

            PFfloat theta = pfmVec3Dot(L, negLightDir);
            PFfloat epsilon = light->innerCutOff - light->outerCutOff;
            intensity = CLAMP((PFint)(255*(theta - light->outerCutOff)/epsilon), 0, 255);

            if (intensity == 0)
                goto apply_light_contribution;
        }

        // Attenuation
        PFubyte attenuation = 255;
        if (light->attLinear || light->attQuadratic) {
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
        PFubyte diff = MAX((PFint)(255*pfmVec3Dot(N, L)), 0);
        lR = MIN_255(lR + (diffuse.r*light->diffuse.r*diff)/(255*255));
        lG = MIN_255(lG + (diffuse.g*light->diffuse.g*diff)/(255*255));
        lB = MIN_255(lB + (diffuse.b*light->diffuse.b*diff)/(255*255));

        // Specular component
#       ifndef PF_PHONG_REFLECTION
            // Blinn-Phong
            PFMvec3 halfWayDir;
            pfmVec3AddR(halfWayDir, L, V);
            pfmVec3Normalize(halfWayDir, halfWayDir);
            PFubyte spec = 255*powf(fmaxf(pfmVec3Dot(N, halfWayDir), 0.0f), shininess);
#       else
            // Phong
            PFMvec3 reflectDir;
            PFMvec3 negLightDir;
            pfmVec3NegR(negLightDir, L);
            pfmVec3ReflectR(reflectDir, negLightDir, N);
            PFubyte spec = 255*powf(fmaxf(pfmVec3Dot(reflectDir, V), 0.0f), shininess);
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

void pfiSimdLightingProcess(PFsimd_color fragments, const PFlight* activeLights,
                                    const PFmaterial* material,
                                    const PFsimdv3f viewPos,
                                    const PFsimdv3f fragPos,
                                    const PFsimdv3f N)
{
    // Load and normalize material colors
    PFsimdv3f colDiffuse, colAmbient, colSpecular, colEmission;
    pfiColorUnpackedToVec_simd(colDiffuse, fragments, 3);
    pfiColorSisdToVec_simd(colAmbient, material->ambient, 3);
    pfiColorSisdToVec_simd(colSpecular, material->specular, 3);
    pfiColorSisdToVec_simd(colEmission, material->emission, 3);

    // Normalize ambient color by diffuse color
    pfiVec3Mul_simd(colAmbient, colAmbient, colDiffuse);

    // Compute the view direction from fragment position
    PFsimdv3f V;
    pfiVec3DirectionR_simd(V, viewPos, fragPos);

    // Initialize light contribution
    PFsimdv3f lightContribution;
    pfiVec3Zero_simd(lightContribution);

    // Process each light
    for (const PFlight *light = activeLights; light != NULL; light = light->next) {
        // Load light data
        PFsimdv3f lightPos, lightDir;
        pfiVec3Load_simd(lightPos, light->position);
        pfiVec3Load_simd(lightDir, light->direction);

        PFsimdv3f lightAmbient, lightDiffuse, lightSpecular;
        pfiColorSisdToVec_simd(lightAmbient, light->ambient, 3);
        pfiColorSisdToVec_simd(lightDiffuse, light->diffuse, 3);
        pfiColorSisdToVec_simd(lightSpecular, light->specular, 3);

        // Compute the light direction from fragment position
        PFsimdv3f L;
        pfiVec3DirectionR_simd(L, lightPos, fragPos);

        // Ambient component
        PFsimdv3f ambient;
        pfiVec3MulR_simd(ambient, lightAmbient, colAmbient);

        // Diffuse component
        PFsimdv3f diffuse; {
            PFsimdvf diff = pfiSimdMax_F32(pfiVec3Dot_simd(N, L), pfiSimdSetZero_F32());
            pfiVec3ScaleR_simd(diffuse, lightDiffuse, diff);
            pfiVec3Mul_simd(diffuse, diffuse, colDiffuse);
        }

        // Specular component
        PFsimdv3f specular; {
            PFsimdv3f negL;
            pfiVec3NegR_simd(negL, L);

#       ifndef PF_PHONG_REFLECTION
            PFsimdv3f halfWayDir;
            pfiVec3AddR_simd(halfWayDir, L, V);
            pfiVec3Normalize_simd(halfWayDir, halfWayDir);
            PFsimdvf spec = pfiSimdMax_F32(pfiVec3Dot_simd(N, halfWayDir), pfiSimdSetZero_F32());
#       else
            PFsimdv3f reflectDir;
            pfiVec3ReflectR_simd(reflectDir, negL, N);
            PFsimdvf spec = pfiSimdMax_F32(pfiVec3Dot_simd(V, reflectDir), pfiSimdSetZero_F32());
#       endif

            spec = pfiSimdPow_F32(spec, material->shininess);

            pfiVec3ScaleR_simd(specular, lightSpecular, spec);
            pfiVec3Mul_simd(specular, specular, colSpecular);
        }

        // Spotlight (soft edges)
        if (light->innerCutOff < M_PI) {
            PFsimdv3f negLightDir;
            pfiVec3NegR_simd(negLightDir, lightDir);

            PFsimdvf theta = pfiVec3Dot_simd(L, negLightDir);
            PFsimdvf epsilon = pfiSimdSet1_F32(light->innerCutOff - light->outerCutOff);

            PFsimdvf intensity = pfiSimdDiv_F32(pfiSimdSub_F32(theta, pfiSimdSet1_F32(light->outerCutOff)), epsilon);
            intensity = pfiSimdClamp_F32(intensity, *(PFsimdvf*)GC_simd_f32_0, *(PFsimdvf*)GC_simd_f32_1);

            pfiVec3Scale_simd(diffuse, diffuse, intensity);
            pfiVec3Scale_simd(specular, specular, intensity);
        }

        // Attenuation
        if (light->attLinear || light->attQuadratic) {
            PFsimdvf distanceSq = pfiVec3DistanceSq_simd(lightPos, fragPos);
            PFsimdvf distance = pfiSimdSqrt_F32(distanceSq);

            PFsimdvf attConstant = pfiSimdSet1_F32(light->attConstant);
            PFsimdvf attLinear = pfiSimdMul_F32(pfiSimdSet1_F32(light->attLinear), distance);
            PFsimdvf attQuadratic = pfiSimdMul_F32(pfiSimdSet1_F32(light->attQuadratic), distanceSq);

            PFsimdvf attenuation = pfiSimdRCP_F32(
                pfiSimdAdd_F32(attConstant, pfiSimdAdd_F32(attLinear, attQuadratic)));

            pfiVec3Scale_simd(ambient, ambient, attenuation);
            pfiVec3Scale_simd(diffuse, diffuse, attenuation);
            pfiVec3Scale_simd(specular, specular, attenuation);
        }

        // Adding light contribution
        pfiVec3Add_simd(lightContribution, lightContribution, ambient);
        pfiVec3Add_simd(lightContribution, lightContribution, diffuse);
        pfiVec3Add_simd(lightContribution, lightContribution, specular);
    }

    // Store the result
    pfiColorUnpackedFromVec_simd(fragments, lightContribution, 3);
}
