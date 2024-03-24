cmake_minimum_required(VERSION 3.21)

# Path to example resources
add_definitions(-DRESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../resources/")

# Basic 3D example
add_executable(SDL2_3D ${PF_ROOT_PATH}/examples/SDL2/SDL2_3D.c)
target_link_libraries(SDL2_3D PRIVATE pixelforge_static SDL2 m)
target_include_directories(SDL2_3D PRIVATE ${PF_ROOT_PATH}/src)
