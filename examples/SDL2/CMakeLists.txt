cmake_minimum_required(VERSION 3.21)

# Path to example resources
add_definitions(-DRESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../resources/")

# Basic 2D example
add_executable(SDL2_2D ${PF_ROOT_PATH}/examples/SDL2/SDL2_2D.c)
target_link_libraries(SDL2_2D PRIVATE pixelforge SDL2 m)
target_include_directories(SDL2_2D PRIVATE ${PF_ROOT_PATH}/src)

# Basic 3D example
add_executable(SDL2_3D ${PF_ROOT_PATH}/examples/SDL2/SDL2_3D.c)
target_link_libraries(SDL2_3D PRIVATE pixelforge SDL2 m)
target_include_directories(SDL2_3D PRIVATE ${PF_ROOT_PATH}/src)

# Gears example
add_executable(SDL2_Gears ${PF_ROOT_PATH}/examples/SDL2/SDL2_Gears.c)
target_link_libraries(SDL2_Gears PRIVATE pixelforge SDL2 m)
target_include_directories(SDL2_Gears PRIVATE ${PF_ROOT_PATH}/src)
