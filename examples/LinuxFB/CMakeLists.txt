cmake_minimum_required(VERSION 3.21)

# Path to example resources
add_definitions(-DRESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../resources/")

# Basic 2D example
add_executable(LinuxFB_2D ${PF_ROOT_PATH}/examples/LinuxFB/LinuxFB_2D.c)
target_link_libraries(LinuxFB_2D PRIVATE pixelforge m)
target_include_directories(LinuxFB_2D PRIVATE ${PF_ROOT_PATH}/src)

# Basic 3D example
add_executable(LinuxFB_3D ${PF_ROOT_PATH}/examples/LinuxFB/LinuxFB_3D.c)
target_link_libraries(LinuxFB_3D PRIVATE pixelforge m)
target_include_directories(LinuxFB_3D PRIVATE ${PF_ROOT_PATH}/src)

# Gears example
add_executable(LinuxFB_Gears ${PF_ROOT_PATH}/examples/LinuxFB/LinuxFB_Gears.c)
target_link_libraries(LinuxFB_Gears PRIVATE pixelforge m)
target_include_directories(LinuxFB_Gears PRIVATE ${PF_ROOT_PATH}/src)
