cmake_minimum_required(VERSION 3.21)

# Path to example resources
add_definitions(-DRESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../resources/")

# Basic 2D example
add_executable(raylib_2D ${PF_ROOT_PATH}/examples/raylib/raylib_2D.c)
target_link_libraries(raylib_2D PRIVATE pixelforge raylib m)
target_include_directories(raylib_2D PRIVATE ${PF_ROOT_PATH}/src)

# Basic 3D example
add_executable(raylib_3D ${PF_ROOT_PATH}/examples/raylib/raylib_3D.c)
target_link_libraries(raylib_3D PRIVATE pixelforge raylib m)
target_include_directories(raylib_3D PRIVATE ${PF_ROOT_PATH}/src)

# Gears example
add_executable(raylib_Gears ${PF_ROOT_PATH}/examples/raylib/raylib_Gears.c)
target_link_libraries(raylib_Gears PRIVATE pixelforge raylib m)
target_include_directories(raylib_Gears PRIVATE ${PF_ROOT_PATH}/src)

# Gears example
add_executable(raylib_PostProcess ${PF_ROOT_PATH}/examples/raylib/raylib_PostProcess.c)
target_link_libraries(raylib_PostProcess PRIVATE pixelforge raylib m)
target_include_directories(raylib_PostProcess PRIVATE ${PF_ROOT_PATH}/src)

# Points example
add_executable(raylib_Points ${PF_ROOT_PATH}/examples/raylib/raylib_Points.c)
target_link_libraries(raylib_Points PRIVATE pixelforge raylib m)
target_include_directories(raylib_Points PRIVATE ${PF_ROOT_PATH}/src)

# Framebuffer example
add_executable(raylib_Framebuffer ${PF_ROOT_PATH}/examples/raylib/raylib_Framebuffer.c)
target_link_libraries(raylib_Framebuffer PRIVATE pixelforge raylib m)
target_include_directories(raylib_Framebuffer PRIVATE ${PF_ROOT_PATH}/src)

# Texture 2D example
add_executable(raylib_Texture2D ${PF_ROOT_PATH}/examples/raylib/raylib_Texture2D.c)
target_link_libraries(raylib_Texture2D PRIVATE pixelforge raylib m)
target_include_directories(raylib_Texture2D PRIVATE ${PF_ROOT_PATH}/src)

# Texture Matrix example
add_executable(raylib_TextureMatrix ${PF_ROOT_PATH}/examples/raylib/raylib_TextureMatrix.c)
target_link_libraries(raylib_TextureMatrix PRIVATE pixelforge raylib m)
target_include_directories(raylib_TextureMatrix PRIVATE ${PF_ROOT_PATH}/src)

# Textured 3D Model example
add_executable(raylib_ModelTextured ${PF_ROOT_PATH}/examples/raylib/raylib_ModelTextured.c)
target_link_libraries(raylib_ModelTextured PRIVATE pixelforge raylib m)
target_include_directories(raylib_ModelTextured PRIVATE ${PF_ROOT_PATH}/src)

# Animated 3D Model example
add_executable(raylib_ModelAnimation ${PF_ROOT_PATH}/examples/raylib/raylib_ModelAnimation.c)
target_link_libraries(raylib_ModelAnimation PRIVATE pixelforge raylib m)
target_include_directories(raylib_ModelAnimation PRIVATE ${PF_ROOT_PATH}/src)

# Wire Mode 3D example
add_executable(raylib_ModelWires ${PF_ROOT_PATH}/examples/raylib/raylib_ModelWires.c)
target_link_libraries(raylib_ModelWires PRIVATE pixelforge raylib m)
target_include_directories(raylib_ModelWires PRIVATE ${PF_ROOT_PATH}/src)

# First Person 3D example
add_executable(raylib_FirstPerson ${PF_ROOT_PATH}/examples/raylib/raylib_FirstPerson.c)
target_link_libraries(raylib_FirstPerson PRIVATE pixelforge raylib m)
target_include_directories(raylib_FirstPerson PRIVATE ${PF_ROOT_PATH}/src)