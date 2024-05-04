#ifdef PF_BRESENHAM_RASTER_METHOD
#   include "./triangles_bresenham.c"
#else //PF_BARYCENTRIC_RASTER_METHOD
#   include "./triangles_barycentric.c"
#endif //PF_RASTER_METHOD
