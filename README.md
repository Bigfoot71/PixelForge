![PixelForge](examples/resources/images/PixelForge.png)

# PixelForge

PixelForge is a lightweight software rendering library written in standard C99, designed for versatile rendering without any external dependencies beyond the C standard library.

**Note:** Currently, PixelForge has two triangle rasterization modes, one (default) uses the [barycentric](https://en.wikipedia.org/wiki/Barycentric_coordinate_system) method, and another by [scanlines](https://en.wikipedia.org/wiki/Scanline_rendering), activatable via the `PF_SCANLINES_RASTER_METHOD` definition. The reason is that I plan to implement SIMD support for the barycentric method in the future and keep a non-SIMD-based scanline method. Currently, the performance difference between the two methods is not _significant_, although the scanline method is slightly more efficient right now. However, since the scanline method is still a work in progress, I prefer to keep the barycentric method as the default, which is the most stable. However, in the future, the scanline method may become the default, with the barycentric method as the secondary option to achieve better performance at the expense of portability.

## Features

- **OpenGL-Like API**: PixelForge offers an API reminiscent of OpenGL 1.X, facilitating easy adoption for those familiar with OpenGL.
- **Multiple Contexts**: Allows creation of multiple rendering contexts for different purposes.
- **Framebuffer Support**: Supports framebuffers, enabling rendering to various pixel buffers.
- **Texture Rendering**: Supports rendering of textures using pixel buffers.
- **Perspective Correction**: Applies perspective correction to texture coordinates during 3D rendering.
- **Primitive Rendering**: Efficiently renders points, lines, triangles, quads, strips, and fans. Additionally, it allows for adjusting point size, line width, and polygon mode, enabling rendering of triangles and quads as lines or points.
- **Pixel Formats**: Supports various commonly used pixel formats and allows users to provide their own getter/setter functions for each texture and framebuffer via function pointers.
- **Blend Modes**: Offers several blend modes for color blending, such as addition, subtraction, multiplication, simple averaging, and alpha blending. Additionally, supports custom color blending functions via function pointer.
- **Depth Testing**: Enables toggling depth testing for 3D rendering management. Several basic depth testing functions are provided, but it's also possible to supply custom functions via function pointers. Additionally, the clear depth value can be adjusted as needed.
- **Material Support**: Rendering material support through `pfMaterialf` and `pfMaterialfv`, similar to OpenGL 1.
- **Phong Lighting**: Rendering multiple lights using `pfLightfv`, with a default support for up to 8 lights, adjustable via a definition. The default supported lighting model is the **Blinn-Phong** model, but you can also activate the Phong model with perfect reflection using `PF_PHONG_REFLECTION`.
- **Gouraud Lighting**: If you find the Phong model too slow, you have the option to enable Gouraud shading via a `PF_GOURAUD_SHADING` definition, making everyone happy!
- **Face Culling**: Supports selection of face culling _(back face culling, front face culling, "no culling")_.
- **Post-Processing**: PixelForge supports post-processing effects through a customizable function pointer. Users can provide a function that takes the position (x, y, z) and color of each pixel on the screen and returns the color to be applied to that pixel. This feature enables various effects such as fog, bloom, and color grading to be implemented easily.
- **Double buffering**: In some cases, double buffering is necessary to avoid flickering during rendering, for example. You can define an auxiliary buffer and swap the buffers as needed.
- **OpenMP Support**: Added support for OpenMP to parallelize triangle rasterization loops, with verification of the number of pixels to be rasterized _(adjustable)_ to activate this parallelization, significantly increasing rasterization performance for large triangles.

## Usage

1. Clone the repository:

   ```bash
   git clone https://github.com/Bigfoot71/PixelForge.git
   ```

2. Include the PixelForge header file in your project:

   ```c
   #include "pixelforge.h"
   ```

3. Compile and link your project with PixelForge library.

## Examples

The repository contains multiple examples showcasing how to use PixelForge with SDL2, raylib, the Windows API for Windows environments, and the X11 window server for Linux. These examples include functions for drawing models in raylib, as well as primitive drawing, projection configuration, and more, all of which can be utilized across different environments.

## License

This library is released under the [Zlib License](LICENSE).

## Contribution

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

## Screenshots
![PixelForge](examples/screenshots/ModelAnimation.png)
![PixelForge](examples/screenshots/ModelTextured.png)
![PixelForge](examples/screenshots/Gears.gif)
