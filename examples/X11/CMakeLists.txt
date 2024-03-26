cmake_minimum_required(VERSION 3.21)

# Path to example resources
add_definitions(-DRESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../resources/")

# Basic 2D example
add_executable(X11_2D ${PF_ROOT_PATH}/examples/X11/X11_2D.c)
target_link_libraries(X11_2D PRIVATE pixelforge_static X11 m)
target_include_directories(X11_2D PRIVATE ${PF_ROOT_PATH}/src)

# Basic 3D example
add_executable(X11_3D ${PF_ROOT_PATH}/examples/X11/X11_3D.c)
target_link_libraries(X11_3D PRIVATE pixelforge_static X11 m)
target_include_directories(X11_3D PRIVATE ${PF_ROOT_PATH}/src)
