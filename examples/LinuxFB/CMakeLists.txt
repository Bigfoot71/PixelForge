cmake_minimum_required(VERSION 3.21)

# Path to example resources
add_definitions(-DRESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../resources/")

# Function to link all libraries accordingly to the compiler / target platform
function(link_libraries_linuxfb target)
    target_link_libraries(${target} PRIVATE pixelforge m)
endfunction()

# Get all .c files in the LinuxFB examples directory
file(GLOB LINUXFB_EXAMPLES_SOURCES "${PF_ROOT_PATH}/examples/LinuxFB/*.c")

# Loop over each .c file and create an executable for each one
foreach(SOURCE_FILE ${LINUXFB_EXAMPLES_SOURCES})
    get_filename_component(EXECUTABLE_NAME ${SOURCE_FILE} NAME_WE)
    add_executable(${EXECUTABLE_NAME} ${SOURCE_FILE})
    link_libraries_linuxfb(${EXECUTABLE_NAME})
    target_include_directories(${EXECUTABLE_NAME} PRIVATE ${PF_ROOT_PATH}/src)
endforeach()
