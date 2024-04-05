![PixelForge](examples/resources/images/PixelForge.png)

# PixelForge

PixelForge is a lightweight software rendering library written in standard C99, designed for versatile rendering without any external dependencies beyond the C standard library.

## Features

- **OpenGL-Like API**: PixelForge offers an API reminiscent of OpenGL 1, facilitating easy adoption for those familiar with OpenGL.
- **Multiple Contexts**: Allows creation of multiple rendering contexts for different purposes.
- **Framebuffer Support**: Supports framebuffers, enabling rendering to various pixel buffers.
- **Texture Rendering**: Supports rendering of textures using pixel buffers.
- **Perspective Correction**: Applies perspective correction to texture coordinates during 3D rendering.
- **Primitive Rendering**: Capable of rendering lines, triangles, and quads efficiently.
- **Pixel Formats**: Provides support for various commonly used pixel formats, and allows users to supply their own getter/setter functions for each texture and framebuffer, enabling flexibility and customization.
- **Blend Modes**: Offers several blend modes for color blending, such as addition, subtraction, multiplication, simple averaging, and alpha blending. Additionally, supports custom color blending functions.
- **Material Support**: Rendering material support through `pfMaterialf` and `pfMaterialfv`, similar to OpenGL 1.
- **Blinn-Phong Lighting**: Rendering multiple lights using `pfLightfv`, with a default support for up to 8 lights, adjustable via a macro. Additionally, support for Phong lighting with perfect reflection can be activated through another macro.

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

The repository contains multiple examples demonstrating the usage of PixelForge with SDL2, raylib, and even with X11 window server for Linux. These examples include functions for drawing models in raylib, as well as primitive drawing, projection configuration, and more, all of which can be utilized across different environments.

## License

This library is released under the [Zlib License](LICENSE).

## Contribution

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

## Screenshots
![PixelForge](examples/screenshots/ModelAnimation.png)
![PixelForge](examples/screenshots/ModelTextured.png)
![PixelForge](examples/screenshots/2D.png)
