cmake_minimum_required(VERSION 3.21)
project(pixelforge)

# C standard definition
set(CMAKE_C_STANDARD 99)

# Definitions of construction constants
set(PF_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(PF_IS_MAIN ${PROJECT_IS_TOP_LEVEL})

# Set build options
option(PF_INSTALL "Install PixelForge library and headers" OFF)
option(PF_BUILD_SHARED "Build PixelForge as a shared library" OFF)

# Set example builds
option(PF_BUILD_EXAMPLES_LINUX_FB "Build PixelForge examples for Linux '/dev/fb0'" OFF)
option(PF_BUILD_EXAMPLES_RAYLIB "Build PixelForge examples for raylib" OFF)
option(PF_BUILD_EXAMPLES_WINAPI "Build PixelForge examples for WINAPI" OFF)
option(PF_BUILD_EXAMPLES_SDL2 "Build PixelForge examples for SDL2" OFF)
option(PF_BUILD_EXAMPLES_X11 "Build PixelForge examples for X11" OFF)

# Defining source files
file(GLOB_RECURSE SRCS ${PF_ROOT_PATH}/src/*.c)
file(GLOB HDRS ${PF_ROOT_PATH}/src/*.h)

# Common compilation options
add_compile_options(-Wall -Wextra -pedantic)

# Adding source files to the target based on the chosen option (shared or static)
if(PF_BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED ${SRCS} ${HDRS})
    target_compile_definitions(${PROJECT_NAME} PUBLIC PF_BUILD_SHARED)
    set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)   # Add -fPIC
else()
    add_library(${PROJECT_NAME} STATIC ${SRCS} ${HDRS})
    target_compile_definitions(${PROJECT_NAME} PUBLIC PF_BUILD_STATIC)
endif()

# Check for SIMD support
include(CheckCCompilerFlag)
check_c_compiler_flag(-mavx2 HAVE_AVX2)
if(HAVE_AVX2)
    message(STATUS "AVX2 support detected")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx2")
else()
    # Check for SSE4.1 support
    check_c_compiler_flag(-msse4.1 HAVE_SSE4_1)
    if(HAVE_SSE4_1)
        message(STATUS "SSE4.1 support detected")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse4")
    else()
        # Check for SSE2 support
        check_c_compiler_flag(-msse2 HAVE_SSE2)
        if(HAVE_SSE2)
            message(STATUS "SSE2 support detected")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2")
        endif()
    endif()
endif()

# Check for OpenMP support
find_package(OpenMP)
if (OPENMP_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_C)
endif()

# Installation if option is enabled
if(PF_INSTALL)
    # Installing the library and headers
    install(TARGETS ${PROJECT_NAME}
            ARCHIVE DESTINATION /usr/local/lib
            LIBRARY DESTINATION /usr/local/lib
            RUNTIME DESTINATION /usr/local/bin
    )
    install(FILES ${HDRS} DESTINATION /usr/local/include)
endif()

# Examples
include(examples/CMakeLists.txt)