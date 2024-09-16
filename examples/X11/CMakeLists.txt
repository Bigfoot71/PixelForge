cmake_minimum_required(VERSION 3.21)

# Path to example resources
add_definitions(-DRESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../resources/")

# Function to link all libraries accordingly to the compiler / target platform
function(link_libraries_x11 target)
    target_link_libraries(${target} PRIVATE pixelforge X11)
    if(NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
        target_link_libraries(${target} PRIVATE m)
    endif()
endfunction()

# Get all .c files in the X11 examples directory
file(GLOB X11_EXAMPLES_SOURCES "${PF_ROOT_PATH}/examples/X11/*.c")

# Loop over each .c file and create an executable for each one
foreach(SOURCE_FILE ${X11_EXAMPLES_SOURCES})
    get_filename_component(EXECUTABLE_NAME ${SOURCE_FILE} NAME_WE)
    add_executable(${EXECUTABLE_NAME} ${SOURCE_FILE})
    link_libraries_x11(${EXECUTABLE_NAME})
    target_include_directories(${EXECUTABLE_NAME} PRIVATE ${PF_ROOT_PATH}/src)
endforeach()
