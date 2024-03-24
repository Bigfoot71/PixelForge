# CMakeLists.txt for PixelForge examples

if (PF_BUILD_EXAMPLES_RAYLIB)
    add_subdirectory(${PF_ROOT_PATH}/examples/raylib)
endif()

if (PF_BUILD_EXAMPLES_SDL2)
    add_subdirectory(${PF_ROOT_PATH}/examples/SDL2)
endif()

if (PF_BUILD_EXAMPLES_X11)
    add_subdirectory(${PF_ROOT_PATH}/examples/X11)
endif()
