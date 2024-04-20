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

// Macros for preparing rendering areas as well as
// barycentric coordinates and their incrementation steps

#define PF_PREPARE_TRIANGLE_FRONT_2D() \
    /*
        Get integer 2D position coordinates
    */ \
    const PFint x1 = (PFint)v1->screen[0], y1 = (PFint)v1->screen[1]; \
    const PFint x2 = (PFint)v2->screen[0], y2 = (PFint)v2->screen[1]; \
    const PFint x3 = (PFint)v3->screen[0], y3 = (PFint)v3->screen[1]; \
    /*
        Check if vertices are in anti-clockwise order or degenerate,
        in which case the triangle cannot be rendered
    */ \
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) >= 0) return; \
    /*
        Calculate the 2D bounding box of the triangle clamped to the viewport dimensions
    */ \
    const PFuint xMin = (PFuint)CLAMP(MIN(x1, MIN(x2, x3)), (PFint)currentCtx->viewportX, (PFint)(currentCtx->viewportX + currentCtx->viewportW)); \
    const PFuint yMin = (PFuint)CLAMP(MIN(y1, MIN(y2, y3)), (PFint)currentCtx->viewportY, (PFint)(currentCtx->viewportY + currentCtx->viewportH)); \
    const PFuint xMax = (PFuint)CLAMP(MAX(x1, MAX(x2, x3)), (PFint)currentCtx->viewportX, (PFint)(currentCtx->viewportX + currentCtx->viewportW)); \
    const PFuint yMax = (PFuint)CLAMP(MAX(y1, MAX(y2, y3)), (PFint)currentCtx->viewportY, (PFint)(currentCtx->viewportY + currentCtx->viewportH)); \
    /*
        If triangle is entirely outside the viewport we can stop now
    */ \
    if (xMin == xMax && yMin == yMax) return; \
    /*
        Calculate weight increment steps for each edge
    */ \
    const PFint stepWX1 = y3 - y2, stepWY1 = x2 - x3; \
    const PFint stepWX2 = y1 - y3, stepWY2 = x3 - x1; \
    const PFint stepWX3 = y2 - y1, stepWY3 = x1 - x2; \
    /*
        Calculate original edge weights relative to bounds.min
        Will be used to obtain barycentric coordinates by incrementing then
        NOTE: The type of face rendered and the order of the vertices (FRONT/BACK) is dependent on these calculations.
    */ \
    PFint w1Row = (xMin - x2)*stepWX1 + stepWY1*(yMin - y2); \
    PFint w2Row = (xMin - x3)*stepWX2 + stepWY2*(yMin - y3); \
    PFint w3Row = (xMin - x1)*stepWX3 + stepWY3*(yMin - y1); \
    /*
        Finally, we calculate the inverse of the sum of
        the barycentric coordinates for the top-left point; this
        sum always remains the same, regardless of the coordinate
        within the triangle.
    */ \
    const PFfloat invWSum = 1.0f/(w1Row + w2Row + w3Row);


#define PF_PREPARE_TRIANGLE_FRONT_3D() \
    /*
        Get integer 2D position coordinates
    */ \
    const PFint x1 = (PFint)v1->screen[0], y1 = (PFint)v1->screen[1]; \
    const PFint x2 = (PFint)v2->screen[0], y2 = (PFint)v2->screen[1]; \
    const PFint x3 = (PFint)v3->screen[0], y3 = (PFint)v3->screen[1]; \
    /*
        Check if vertices are in anti-clockwise order or degenerate,
        in which case the triangle cannot be rendered
    */ \
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) >= 0) return; \
    /*
        Calculate the 2D bounding box of the triangle
    */ \
    const PFuint xMin = MIN(x1, MIN(x2, x3)); \
    const PFuint yMin = MIN(y1, MIN(y2, y3)); \
    const PFuint xMax = MAX(x1, MAX(x2, x3)); \
    const PFuint yMax = MAX(y1, MAX(y2, y3)); \
    /*
        Calculate weight increment steps for each edge
    */ \
    const PFint stepWX1 = y3 - y2, stepWY1 = x2 - x3; \
    const PFint stepWX2 = y1 - y3, stepWY2 = x3 - x1; \
    const PFint stepWX3 = y2 - y1, stepWY3 = x1 - x2; \
    /*
        Calculate original edge weights relative to bounds.min
        Will be used to obtain barycentric coordinates by incrementing then
        NOTE: The type of face rendered and the order of the vertices (FRONT/BACK) is dependent on these calculations.
    */ \
    PFint w1Row = (xMin - x2)*stepWX1 + stepWY1*(yMin - y2); \
    PFint w2Row = (xMin - x3)*stepWX2 + stepWY2*(yMin - y3); \
    PFint w3Row = (xMin - x1)*stepWX3 + stepWY3*(yMin - y1); \
    /*
        Finally, we calculate the inverse of the sum of
        the barycentric coordinates for the top-left point; this
        sum always remains the same, regardless of the coordinate
        within the triangle.
    */ \
    const PFfloat invWSum = 1.0f/(w1Row + w2Row + w3Row);


#define PF_PREPARE_TRIANGLE_BACK_3D() \
    /*
        Get integer 2D position coordinates
    */ \
    const PFint x1 = (PFint)v1->screen[0], y1 = (PFint)v1->screen[1]; \
    const PFint x2 = (PFint)v2->screen[0], y2 = (PFint)v2->screen[1]; \
    const PFint x3 = (PFint)v3->screen[0], y3 = (PFint)v3->screen[1]; \
    /*
        Check if vertices are in clockwise order or degenerate,
        in which case the triangle cannot be rendered
    */ \
    if ((x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1) <= 0) return; \
    /*
        Calculate the 2D bounding box of the triangle
    */ \
    const PFuint xMin = MIN(x1, MIN(x2, x3)); \
    const PFuint yMin = MIN(y1, MIN(y2, y3)); \
    const PFuint xMax = MAX(x1, MAX(x2, x3)); \
    const PFuint yMax = MAX(y1, MAX(y2, y3)); \
    /*
        Calculate weight increment steps for each edge
        NOTE: The steps are reversed when we render the back faces
    */ \
    const PFint stepWX1 = y2 - y3, stepWY1 = x3 - x2; \
    const PFint stepWX2 = y3 - y1, stepWY2 = x1 - x3; \
    const PFint stepWX3 = y1 - y2, stepWY3 = x2 - x1; \
    /*
        Calculate original edge weights relative to bounds.min
        Will be used to obtain barycentric coordinates by incrementing then
        NOTE: The type of face rendered and the order of the vertices (FRONT/BACK) is dependent on these calculations.
    */ \
    PFint w1Row = (yMin - y2)*stepWY1 + stepWX1*(xMin - x2); \
    PFint w2Row = (yMin - y3)*stepWY2 + stepWX2*(xMin - x3); \
    PFint w3Row = (yMin - y1)*stepWY3 + stepWX3*(xMin - x1); \
    /*
        Finally, we calculate the inverse of the sum of
        the barycentric coordinates for the top-left point; this
        sum always remains the same, regardless of the coordinate
        within the triangle.
    */ \
    const PFfloat invWSum = 1.0f/(w1Row + w2Row + w3Row);


// Rasterization loop implementation

#ifndef PF_USE_OPENMP

// Begin/End flat triangle rasterizer macros

#define PF_BEGIN_TRIANGLE_FLAT_LOOP() \
    PFtexture *texTarget = &currentCtx->currentFramebuffer->texture; \
    for (PFuint y = yMin; y <= yMax; y++) \
    { \
        const PFuint yOffset = y*texTarget->width; \
        PFint w1 = w1Row, w2 = w2Row, w3 = w3Row; \
        for (PFuint x = xMin; x <= xMax; x++) \
        { \
            if ((w1 | w2 | w3) >= 0) \
            { \
                PFcolor finalColor; \
                const PFuint xyOffset = yOffset + x; \
                const PFfloat aW1 = w1*invWSum, aW2 = w2*invWSum, aW3 = w3*invWSum; \
                const PFfloat z = 1.0f/(aW1*v1->homogeneous[2] + aW2*v2->homogeneous[2] + aW3*v3->homogeneous[2]);

#define PF_END_TRIANGLE_FLAT_LOOP() \
                texTarget->pixelSetter(texTarget->pixels, xyOffset, finalColor); \
                currentCtx->currentFramebuffer->zbuffer[xyOffset] = z; \
            } \
            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3; \
        } \
        w1Row += stepWY1, w2Row += stepWY2, w3Row += stepWY3; \
    }


// Begin/End depth triangle rasterizer macros

#define PF_BEGIN_TRIANGLE_DEPTH_LOOP() \
    PFtexture *texTarget = &currentCtx->currentFramebuffer->texture; \
    for (PFuint y = yMin; y <= yMax; y++) \
    { \
        const PFuint yOffset = y*texTarget->width; \
        PFint w1 = w1Row, w2 = w2Row, w3 = w3Row; \
        for (PFuint x = xMin; x <= xMax; x++) \
        { \
            if ((w1 | w2 | w3) >= 0) \
            { \
                PFcolor finalColor; \
                const PFuint xyOffset = yOffset + x; \
                const PFfloat aW1 = w1*invWSum, aW2 = w2*invWSum, aW3 = w3*invWSum; \
                const PFfloat z = 1.0f/(aW1*v1->homogeneous[2] + aW2*v2->homogeneous[2] + aW3*v3->homogeneous[2]); \
                if (z < currentCtx->currentFramebuffer->zbuffer[xyOffset]) \
                {

#define PF_END_TRIANGLE_DEPTH_LOOP() \
                    texTarget->pixelSetter(texTarget->pixels, xyOffset, finalColor); \
                    currentCtx->currentFramebuffer->zbuffer[xyOffset] = z; \
                } \
            } \
            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3; \
        } \
        w1Row += stepWY1, w2Row += stepWY2, w3Row += stepWY3; \
    }

// Begin/End flat triangle light rasterizer macros

#define PF_BEGIN_TRIANGLE_FLAT_LIGHT_LOOP(material) \
    for (int i = 0; i <= currentCtx->lastActiveLight; i++) \
    { \
        const PFlight* light = &currentCtx->lights[i]; \
        if (!light->active) continue; \
        const PFcolor ambient = pfBlendMultiplicative(light->ambient, (material).ambient); \
        PF_BEGIN_TRIANGLE_FLAT_LOOP(); \

#define PF_END_TRIANGLE_FLAT_LIGHT_LOOP() \
        PF_END_TRIANGLE_FLAT_LOOP(); \
    }


// Begin/End depth triangle light rasterizer macros

#define PF_BEGIN_TRIANGLE_DEPTH_LIGHT_LOOP(material) \
    for (int i = 0; i <= currentCtx->lastActiveLight; i++) \
    { \
        const PFlight* light = &currentCtx->lights[i]; \
        if (!light->active) continue; \
        const PFcolor ambient = pfBlendMultiplicative(light->ambient, (material).ambient); \
        PFtexture *texTarget = &currentCtx->currentFramebuffer->texture; \
        for (PFuint y = yMin; y <= yMax; y++) \
        { \
            const PFuint yOffset = y*texTarget->width; \
            PFint w1 = w1Row, w2 = w2Row, w3 = w3Row; \
            for (PFuint x = xMin; x <= xMax; x++) \
            { \
                if ((w1 | w2 | w3) >= 0) \
                { \
                    PFcolor finalColor; \
                    const PFuint xyOffset = yOffset + x; \
                    const PFfloat aW1 = w1*invWSum, aW2 = w2*invWSum, aW3 = w3*invWSum; \
                    const PFfloat z = 1.0f/(aW1*v1->homogeneous[2] + aW2*v2->homogeneous[2] + aW3*v3->homogeneous[2]); \
                    if (z < currentCtx->currentFramebuffer->zbuffer[xyOffset]) \
                    {

#define PF_END_TRIANGLE_DEPTH_LIGHT_LOOP() \
                        texTarget->pixelSetter(texTarget->pixels, xyOffset, finalColor); \
                        currentCtx->currentFramebuffer->zbuffer[xyOffset] = z; \
                    } \
                } \
                w1 += stepWX1, w2 += stepWX2, w3 += stepWX3; \
            } \
            w1Row += stepWY1, w2Row += stepWY2, w3Row += stepWY3; \
        } \
    }


#else //PF_USE_OPENMP

// Begin/End flat triangle rasterizer macros

#define PF_BEGIN_TRIANGLE_FLAT_LOOP() \
    PFtexture *texTarget = &currentCtx->currentFramebuffer->texture; \
    _Pragma("omp parallel for if((yMax - yMin)*(xMax - xMin) >= PF_OPENMP_PIXEL_RASTER_THRESHOLD)") \
    for (PFuint y = yMin; y <= yMax; y++) \
    { \
        const PFuint yOffset = y*texTarget->width; \
        PFint i = y - yMin; \
        PFint w1 = w1Row + i*stepWY1; \
        PFint w2 = w2Row + i*stepWY2; \
        PFint w3 = w3Row + i*stepWY3; \
        for (PFuint x = xMin; x <= xMax; x++) \
        { \
            if ((w1 | w2 | w3) >= 0) \
            { \
                PFcolor finalColor; \
                const PFuint xyOffset = yOffset + x; \
                const PFfloat aW1 = w1*invWSum, aW2 = w2*invWSum, aW3 = w3*invWSum; \
                const PFfloat z = 1.0f/(aW1*v1->homogeneous[2] + aW2*v2->homogeneous[2] + aW3*v3->homogeneous[2]);

#define PF_END_TRIANGLE_FLAT_LOOP() \
                texTarget->pixelSetter(texTarget->pixels, xyOffset, finalColor); \
                currentCtx->currentFramebuffer->zbuffer[xyOffset] = z; \
            } \
            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3; \
        } \
    }


// Begin/End depth triangle rasterizer macros

#define PF_BEGIN_TRIANGLE_DEPTH_LOOP() \
    PFtexture *texTarget = &currentCtx->currentFramebuffer->texture; \
    _Pragma("omp parallel for if((yMax - yMin)*(xMax - xMin) >= PF_OPENMP_PIXEL_RASTER_THRESHOLD)") \
    for (PFuint y = yMin; y <= yMax; y++) \
    { \
        const PFuint yOffset = y*texTarget->width; \
        PFint i = y - yMin; \
        PFint w1 = w1Row + i*stepWY1; \
        PFint w2 = w2Row + i*stepWY2; \
        PFint w3 = w3Row + i*stepWY3; \
        for (PFuint x = xMin; x <= xMax; x++) \
        { \
            if ((w1 | w2 | w3) >= 0) \
            { \
                PFcolor finalColor; \
                const PFuint xyOffset = yOffset + x; \
                const PFfloat aW1 = w1*invWSum, aW2 = w2*invWSum, aW3 = w3*invWSum; \
                const PFfloat z = 1.0f/(aW1*v1->homogeneous[2] + aW2*v2->homogeneous[2] + aW3*v3->homogeneous[2]); \
                if (z < currentCtx->currentFramebuffer->zbuffer[xyOffset]) \
                {

#define PF_END_TRIANGLE_DEPTH_LOOP() \
                    texTarget->pixelSetter(texTarget->pixels, xyOffset, finalColor); \
                    currentCtx->currentFramebuffer->zbuffer[xyOffset] = z; \
                } \
            } \
            w1 += stepWX1, w2 += stepWX2, w3 += stepWX3; \
        } \
    }

// Begin/End flat triangle light rasterizer macros

#define PF_BEGIN_TRIANGLE_FLAT_LIGHT_LOOP(material) \
    for (int i = 0; i <= currentCtx->lastActiveLight; i++) \
    { \
        const PFlight* light = &currentCtx->lights[i]; \
        if (!light->active) continue; \
        const PFcolor ambient = pfBlendMultiplicative(light->ambient, (material).ambient); \
        PF_BEGIN_TRIANGLE_FLAT_LOOP(); \

#define PF_END_TRIANGLE_FLAT_LIGHT_LOOP() \
        PF_END_TRIANGLE_FLAT_LOOP(); \
    }


// Begin/End depth triangle light rasterizer macros

#define PF_BEGIN_TRIANGLE_DEPTH_LIGHT_LOOP(material) \
    for (int i = 0; i <= currentCtx->lastActiveLight; i++) \
    { \
        const PFlight* light = &currentCtx->lights[i]; \
        if (!light->active) continue; \
        const PFcolor ambient = pfBlendMultiplicative(light->ambient, (material).ambient); \
        PFtexture *texTarget = &currentCtx->currentFramebuffer->texture; \
        _Pragma("omp parallel for if((yMax - yMin)*(xMax - xMin) >= PF_OPENMP_PIXEL_RASTER_THRESHOLD)") \
        for (PFuint y = yMin; y <= yMax; y++) \
        { \
            const PFuint yOffset = y*texTarget->width; \
            PFint i = y - yMin; \
            PFint w1 = w1Row + i*stepWY1; \
            PFint w2 = w2Row + i*stepWY2; \
            PFint w3 = w3Row + i*stepWY3; \
            for (PFuint x = xMin; x <= xMax; x++) \
            { \
                if ((w1 | w2 | w3) >= 0) \
                { \
                    PFcolor finalColor; \
                    const PFuint xyOffset = yOffset + x; \
                    const PFfloat aW1 = w1*invWSum, aW2 = w2*invWSum, aW3 = w3*invWSum; \
                    const PFfloat z = 1.0f/(aW1*v1->homogeneous[2] + aW2*v2->homogeneous[2] + aW3*v3->homogeneous[2]); \
                    if (z < currentCtx->currentFramebuffer->zbuffer[xyOffset]) \
                    {

#define PF_END_TRIANGLE_DEPTH_LIGHT_LOOP() \
                        texTarget->pixelSetter(texTarget->pixels, xyOffset, finalColor); \
                        currentCtx->currentFramebuffer->zbuffer[xyOffset] = z; \
                    } \
                } \
                w1 += stepWX1, w2 += stepWX2, w3 += stepWX3; \
            } \
        } \
    }


#endif //PF_USE_OPENMP