#ifndef PF_CONFIG_H
#define PF_CONFIG_H

//#define PF_SCANLINES_RASTER_METHOD    // Performs triangle rasterization using scanline rather than barycentric method
//#define PF_SUPPORT_NO_POT_TEXTURE     // Allows fetching samples from texcoords on non-power-of-two textures
//#define PF_PHONG_REFLECTION           // Disable the Blinn-Phong reflection model for Phong
//#define PF_GOURAUD_SHADING            // Enable vertex shading for lighting instead of per-fragment shading

#ifndef PF_MAX_PROJECTION_STACK_SIZE
#   define PF_MAX_PROJECTION_STACK_SIZE 2
#endif //PF_MAX_PROJECTION_STACK_SIZE

#ifndef PF_MAX_MODELVIEW_STACK_SIZE
#   define PF_MAX_MODELVIEW_STACK_SIZE 8
#endif //PF_MAX_MODELVIEW_STACK_SIZE

#ifndef PF_MAX_LIGHT_STACK
#   define PF_MAX_LIGHT_STACK 8
#endif //PF_MAX_LIGHT_STACK

#ifndef PF_MAX_CLIPPED_POLYGON_VERTICES
#   define PF_MAX_CLIPPED_POLYGON_VERTICES 12
#endif //PF_MAX_CLIPPED_POLYGON_VERTICES

#ifndef PF_PIXELFORMAT_R5G5B5A1_ALPHA_THRESHOLD
    #define PF_PIXELFORMAT_R5G5B5A1_ALPHA_THRESHOLD 50  // Threshold over 255 to set alpha as 0
#endif //PF_PIXELFORMAT_R5G5B5A1_ALPHA_THRESHOLD

#ifndef PF_CLIP_EPSILON
#   define PF_CLIP_EPSILON 1e-5f
#endif //PF_CLIP_EPSILON

#ifdef PF_SUPPORT_OPENMP

//  Pixel threshold for parallelizing the rasterization loop
//  NOTE: In barycentric rendering method, this corresponds
//        to the area of the triangle's bounding box,
//        whereas in scanline rendering method, it
//        corresponds to the area of the triangle.
#   ifndef PF_OPENMP_RASTER_THRESHOLD_AREA
#       define PF_OPENMP_RASTER_THRESHOLD_AREA 32*32
#   endif //PF_OPENMP_RASTER_THRESHOLD_AREA

//  Buffer size threshold for parallelizing `pfClear` loops
#   ifndef PF_OPENMP_CLEAR_BUFFER_SIZE_THRESHOLD
#       define PF_OPENMP_CLEAR_BUFFER_SIZE_THRESHOLD 640*480
#   endif //PF_OPENMP_CLEAR_BUFFER_SIZE_THRESHOLD

#endif //PF_SUPPORT_OPENMP

#endif //PF_CONFIG_H