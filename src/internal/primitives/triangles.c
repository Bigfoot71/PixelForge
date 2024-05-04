#ifdef PF_SCANLINES_METHOD
#   include "./triangles_scanlines.c"
#else //PF_BARYCENTRIC_RASTER_METHOD
#   include "./triangles_barycentric.c"
#endif //PF_RASTER_METHOD
