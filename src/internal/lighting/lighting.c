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

PFcolor pfInternal_LightingProcess(const PFlight* activeLights, const PFmaterial* material,
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
    for (const PFlight *light = activeLights; light != NULL; light = light->next)
    {
        // Declare the light contribution and initialize it to zero for now.
        PFubyte lR = 0, lG = 0, lB = 0;

        // Compute light direction
        PFMvec3 L;
        pfmVec3SubR(L, light->position, fragPos);

        // Compute distance from light to fragment position
        // And also normalize the light direction if necessary.
        PFfloat lightToFragPosDistSq = L[0]*L[0] + L[1]*L[1] + L[2]*L[2];

        PFfloat lightToFragPosDist = 0;
        if (lightToFragPosDistSq != 0.0f)
        {
            lightToFragPosDist = sqrtf(lightToFragPosDistSq);
            PFfloat invMag = 1.0f/lightToFragPosDist;
            for (int_fast8_t i = 0; i < 3; i++)
            {
                L[i] *= invMag;
            }
        }

        // Spotlight (soft edges)
        PFubyte intensity = 255;
        if (light->innerCutOff < M_PI)
        {
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

void pfInternal_SimdLightingProcess(PFsimd_color fragments, const PFlight* activeLights,
                                    const PFmaterial* material,
                                    const PFMsimd_vec3 viewPos,
                                    const PFMsimd_vec3 fragPos,
                                    const PFMsimd_vec3 N)
{
    // Load and normalize material colors
    PFMsimd_vec3 colDiffuse, colAmbient, colSpecular, colEmission;
    pfInternal_SimdColorUnpackedToVec(colDiffuse, fragments, 3);
    pfInternal_SimdColorSisdToVec(colAmbient, material->ambient, 3);
    pfInternal_SimdColorSisdToVec(colSpecular, material->specular, 3);
    pfInternal_SimdColorSisdToVec(colEmission, material->emission, 3);

    // Normalize ambient color by diffuse color
    pfmSimdVec3Mul(colAmbient, colAmbient, colDiffuse);

    // Compute the view direction from fragment position
    PFMsimd_vec3 V;
    pfmSimdVec3DirectionR(V, viewPos, fragPos);

    // Initialize light contribution
    PFMsimd_vec3 lightContribution;
    pfmSimdVec3Zero(lightContribution);

    // Process each light
    for (const PFlight *light = activeLights; light != NULL; light = light->next)
    {
        // Load light data
        PFMsimd_vec3 lightPos, lightDir;
        pfmSimdVec3Load(lightPos, light->position);
        pfmSimdVec3Load(lightDir, light->direction);

        PFMsimd_vec3 lightAmbient, lightDiffuse, lightSpecular;
        pfInternal_SimdColorSisdToVec(lightAmbient, light->ambient, 3);
        pfInternal_SimdColorSisdToVec(lightDiffuse, light->diffuse, 3);
        pfInternal_SimdColorSisdToVec(lightSpecular, light->specular, 3);

        // Compute the light direction from fragment position
        PFMsimd_vec3 L;
        pfmSimdVec3DirectionR(L, lightPos, fragPos);

        // Ambient component
        PFMsimd_vec3 ambient;
        pfmSimdVec3MulR(ambient, lightAmbient, colAmbient);

        // Diffuse component
        PFMsimd_vec3 diffuse;
        {
            PFMsimd_f diff = pfmSimdMax_F32(pfmSimdVec3Dot(N, L), pfmSimdSetZero_F32());
            pfmSimdVec3ScaleR(diffuse, lightDiffuse, diff);
            pfmSimdVec3Mul(diffuse, diffuse, colDiffuse);
        }

        // Specular component
        PFMsimd_vec3 specular;
        {
            PFMsimd_vec3 negL;
            pfmSimdVec3NegR(negL, L);

#       ifndef PF_PHONG_REFLECTION
            PFMsimd_vec3 halfWayDir;
            pfmSimdVec3AddR(halfWayDir, L, V);
            pfmSimdVec3Normalize(halfWayDir, halfWayDir);
            PFMsimd_f spec = pfmSimdMax_F32(pfmSimdVec3Dot(N, halfWayDir), pfmSimdSetZero_F32());
#       else
            PFMsimd_vec3 reflectDir;
            pfmSimdVec3ReflectR(reflectDir, negL, N);
            PFMsimd_f spec = pfmSimdMax_F32(pfmSimdVec3Dot(V, reflectDir), pfmSimdSetZero_F32());
#       endif

            spec = pfmSimdPow_F32(spec, material->shininess);

            pfmSimdVec3ScaleR(specular, lightSpecular, spec);
            pfmSimdVec3Mul(specular, specular, colSpecular);
        }

        // Spotlight (soft edges)
        if (light->innerCutOff < M_PI)
        {
            PFMsimd_vec3 negLightDir;
            pfmSimdVec3NegR(negLightDir, lightDir);

            PFMsimd_f theta = pfmSimdVec3Dot(L, negLightDir);
            PFMsimd_f epsilon = pfmSimdSet1_F32(light->innerCutOff - light->outerCutOff);

            PFMsimd_f intensity = pfmSimdDiv_F32(pfmSimdSub_F32(theta, pfmSimdSet1_F32(light->outerCutOff)), epsilon);
            intensity = pfmSimdClamp_F32(intensity, pfmSimdSetZero_F32(), pfmSimdSet1_F32(1.0f));

            pfmSimdVec3Scale(diffuse, diffuse, intensity);
            pfmSimdVec3Scale(specular, specular, intensity);
        }

        // Attenuation
        if (light->attLinear || light->attQuadratic)
        {
            PFMsimd_f distanceSq = pfmSimdVec3DistanceSq(lightPos, fragPos);
            PFMsimd_f distance = pfmSimdSqrt_F32(distanceSq);

            PFMsimd_f attConstant = pfmSimdSet1_F32(light->attConstant);
            PFMsimd_f attLinear = pfmSimdMul_F32(pfmSimdSet1_F32(light->attLinear), distance);
            PFMsimd_f attQuadratic = pfmSimdMul_F32(pfmSimdSet1_F32(light->attQuadratic), distanceSq);

            PFMsimd_f attenuation = pfmSimdRCP_F32(
                pfmSimdAdd_F32(attConstant, pfmSimdAdd_F32(attLinear, attQuadratic)));

            pfmSimdVec3Scale(ambient, ambient, attenuation);
            pfmSimdVec3Scale(diffuse, diffuse, attenuation);
            pfmSimdVec3Scale(specular, specular, attenuation);
        }

        // Adding light contribution
        pfmSimdVec3Add(lightContribution, lightContribution, ambient);
        pfmSimdVec3Add(lightContribution, lightContribution, diffuse);
        pfmSimdVec3Add(lightContribution, lightContribution, specular);
    }

    // Store the result
    pfInternal_SimdColorUnpackedFromVec(fragments, lightContribution, 3);
}

/*
void pfInternal_SimdLightingProcess(PFsimd_color outColors, const PFlight* activeLights,
                                    const PFmaterial* material, PFsimd_color diffuses,
                                    const PFMsimd_vec3 viewPos, const PFMsimd_vec3 fragPos,
                                    const PFMsimd_vec3 N)
{
    // load material data
    PFMsimd_vec3 colDiffuse, colAmbient, colSpecular, colEmission;
    pfInternal_SimdColorUnpackedToVec(colDiffuse, diffuses, 3);
    pfInternal_SimdColorSisdToVec(colAmbient, material->ambient, 3);
    pfInternal_SimdColorSisdToVec(colSpecular, material->specular, 3);
    pfInternal_SimdColorSisdToVec(colEmission, material->emission, 3);

    // base ambient
    pfmSimdVec3Mul(colAmbient, colAmbient, colDiffuse);

    // compute light contributions
    PFMsimd_vec3 lightContribution;
    pfmSimdVec3Zero(lightContribution);
    for (const PFlight *light = activeLights; light != NULL; light = light->next)
    {
        // load light data
        PFMsimd_vec3 lightPos, lightDir;
        pfmSimdVec3Load(lightPos, light->position);
        pfmSimdVec3Load(lightDir, light->direction);

        PFMsimd_vec3 lightAmbient, lightDiffuse, lightSpecular;
        pfInternal_SimdColorSisdToVec(lightAmbient, light->ambient, 3);
        pfInternal_SimdColorSisdToVec(lightDiffuse, light->diffuse, 3);
        pfInternal_SimdColorSisdToVec(lightSpecular, light->specular, 3);

        // ambient
        PFMsimd_vec3 ambient;
        pfmSimdVec3MulR(ambient, lightAmbient, colAmbient);

        // diffuse 
        PFMsimd_vec3 L;
        pfmSimdVec3DirectionR(L, lightPos, fragPos);
        PFMsimd_f diff = pfmSimdMax_F32(pfmSimdVec3Dot(N, L), pfmSimdSetZero_F32());

        PFMsimd_vec3 diffuse;
        pfmSimdVec3ScaleR(diffuse, lightDiffuse, diff);
        pfmSimdVec3Mul(diffuse, diffuse, colDiffuse);

        // specular
        PFMsimd_vec3 V;
        pfmSimdVec3DirectionR(V, viewPos, fragPos);

        PFMsimd_vec3 negL;
        pfmSimdVec3NegR(negL, L);

        PFMsimd_vec3 reflectDir;
        pfmSimdVec3ReflectR(reflectDir, negL, N);
        PFMsimd_f spec = pfmSimdPow_F32(pfmSimdMax_F32(pfmSimdVec3Dot(V, reflectDir), pfmSimdSetZero_F32()), material->shininess);

        PFMsimd_vec3 specular;
        pfmSimdVec3ScaleR(specular, lightSpecular, spec);
        pfmSimdVec3Mul(specular, specular, colSpecular);

        // spotlight (soft edges)
        PFMsimd_vec3 negLightDir;
        pfmSimdVec3NegR(negLightDir, lightDir);
        PFMsimd_f theta = pfmSimdVec3Dot(L, negLightDir);
        PFMsimd_f epsilon = pfmSimdSet1_F32(light->innerCutOff - light->outerCutOff);
        PFMsimd_f intensity = pfmSimdClamp_F32(
            pfmSimdDiv_F32(pfmSimdSub_F32(theta, pfmSimdSet1_F32(light->outerCutOff)), epsilon),
            pfmSimdSetZero_F32(), pfmSimdSet1_F32(1.0f));

        pfmSimdVec3Scale(diffuse, diffuse, intensity);
        pfmSimdVec3Scale(specular, specular, intensity);

        // attenuation
        PFMsimd_f distanceSq = pfmSimdVec3DistanceSq(lightPos, fragPos);
        PFMsimd_f distance = pfmSimdSqrt_F32(distanceSq);

        PFMsimd_f attConstant = pfmSimdSet1_F32(light->attConstant);
        PFMsimd_f attLinear = pfmSimdMul_F32(pfmSimdSet1_F32(light->attLinear), distance);
        PFMsimd_f attQuadratic = pfmSimdMul_F32(pfmSimdSet1_F32(light->attQuadratic), distanceSq);

        PFMsimd_f attenuation = pfmSimdRCP_F32(
            pfmSimdAdd_F32(attConstant, pfmSimdAdd_F32(attLinear, attQuadratic)));
  
        pfmSimdVec3Scale(ambient, ambient, attenuation);
        pfmSimdVec3Scale(diffuse, diffuse, attenuation);
        pfmSimdVec3Scale(specular, specular, attenuation); 

        // adding light contribution
        pfmSimdVec3Add(lightContribution, lightContribution, ambient);
        pfmSimdVec3Add(lightContribution, lightContribution, diffuse);
        pfmSimdVec3Add(lightContribution, lightContribution, specular);
    }

    pfInternal_SimdColorUnpackedFromVec(
        outColors, lightContribution, 3);
}
*/