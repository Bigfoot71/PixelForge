cmake_minimum_required(VERSION 3.21)

# Path to example resources
add_definitions(-DRESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/../resources/")

# Basic 2D example
add_executable(WinAPI_2D ${PF_ROOT_PATH}/examples/WinAPI/WinAPI_2D.c)
target_link_libraries(WinAPI_2D PRIVATE pixelforge use_R_32 gdi32 m)
target_include_directories(WinAPI_2D PRIVATE ${PF_ROOT_PATH}/src)

# Basic 3D example
add_executable(WinAPI_3D ${PF_ROOT_PATH}/examples/WinAPI/WinAPI_3D.c)
target_link_libraries(WinAPI_3D PRIVATE pixelforge use_R_32 gdi32 m)
target_include_directories(WinAPI_3D PRIVATE ${PF_ROOT_PATH}/src)

# Gears example
add_executable(WinAPI_Gears ${PF_ROOT_PATH}/examples/WinAPI/WinAPI_Gears.c)
target_link_libraries(WinAPI_Gears PRIVATE pixelforge use_R_32 gdi32 m)
target_include_directories(WinAPI_Gears PRIVATE ${PF_ROOT_PATH}/src)
