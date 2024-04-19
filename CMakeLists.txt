cmake_minimum_required(VERSION 3.21)
project(PixelForge)

# C standard definition
set(CMAKE_C_STANDARD 99)

# Definitions of construction constants
set(PF_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(PF_IS_MAIN ${PROJECT_IS_TOP_LEVEL})

# Setting build options
option(PF_INSTALL "Install PixelForge library and headers" OFF)
option(PF_BUILD_STATIC "Build PixelForge as a static library" ON)
option(PF_BUILD_SHARED "Build PixelForge as a shared library" OFF)
option(PF_BUILD_EXAMPLES_RAYLIB "Build PixelForge examples for raylib" ${PF_IS_MAIN})
option(PF_BUILD_EXAMPLES_SDL2 "Build PixelForge examples for SDL2" ${PF_IS_MAIN})
option(PF_BUILD_EXAMPLES_X11 "Build PixelForge examples for X11" ${PF_IS_MAIN})
option(PF_USE_OPENMP "Enable support for OpenMP parallelization" OFF)

# Defining source files
file(GLOB SRCS
    ${PF_ROOT_PATH}/src/blend.c
    ${PF_ROOT_PATH}/src/render.c
    ${PF_ROOT_PATH}/src/texture.c
    ${PF_ROOT_PATH}/src/framebuffer.c
)
file(GLOB HDRS
    ${PF_ROOT_PATH}/src/*.h
)

# Common compilation options
add_compile_options(-Wall -Wextra -pedantic)

# Adding source files to static target
if(PF_BUILD_STATIC)
    add_library(pixelforge_static STATIC ${SRCS} ${HDRS})
    target_compile_definitions(pixelforge_static PUBLIC PF_BUILD_STATIC)
endif()

# Adding source files to the dynamic target
if(PF_BUILD_SHARED)
    add_library(pixelforge_dynamic SHARED ${SRCS} ${HDRS})
    target_compile_definitions(pixelforge_dynamic PUBLIC PF_BUILD_SHARED)
    set_property(TARGET pixelforge_dynamic PROPERTY POSITION_INDEPENDENT_CODE ON)   # Add -fPIC
endif()

# Enable OpenMP support if PF_USE_OPENMP is ON
if(PF_USE_OPENMP)
    find_package(OpenMP)
    if (OPENMP_FOUND)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
        if(PF_BUILD_STATIC)
            target_compile_definitions(pixelforge_static PUBLIC PF_USE_OPENMP)
        endif()
        if(PF_BUILD_SHARED)
            target_compile_definitions(pixelforge_dynamic PUBLIC PF_USE_OPENMP)
        endif()
    endif()
endif()

# Installation if option is enabled
if(PF_INSTALL)
    # Installing static libraries if the STATIC option is enabled
    if(PF_BUILD_STATIC)
        install(TARGETS pixelforge_static ARCHIVE DESTINATION /usr/local/lib)
		set_target_properties(pixelforge_static PROPERTIES OUTPUT_NAME pixelforge)
    endif()

    # Installing shared libraries if the SHARED option is enabled
    if(PF_BUILD_SHARED)
        install(TARGETS pixelforge_dynamic LIBRARY DESTINATION /usr/local/lib)
		set_target_properties(pixelforge_shared PROPERTIES OUTPUT_NAME pixelforge)
    endif()

    # Installing header files
    install(FILES ${HDRS} DESTINATION /usr/local/include)
endif()

# Examples
include(examples/CMakeLists.txt)