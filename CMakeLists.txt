cmake_minimum_required(VERSION 3.16)

project(RayTray LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenMP)

# Serial renderer; it does not depend on OpenMP.
add_executable(raytracer main_serial.cpp)
target_compile_features(raytracer PRIVATE cxx_std_17)
if(MSVC)
    target_compile_options(raytracer PRIVATE /W4)
else()
    target_compile_options(raytracer PRIVATE -Wall -Wextra -pedantic -O3)
endif()

if(OpenMP_CXX_FOUND)
    # The parallel target is available when a C++ OpenMP runtime is installed.
    add_executable(raytracer_parallel main_parallel.cpp)
    target_compile_features(raytracer_parallel PRIVATE cxx_std_17)
    target_link_libraries(raytracer_parallel OpenMP::OpenMP_CXX)
    if(MSVC)
        target_compile_options(raytracer_parallel PRIVATE /W4)
    else()
        target_compile_options(raytracer_parallel PRIVATE -Wall -Wextra -pedantic -O3)
    endif()
else()
    message(STATUS "OpenMP C++ runtime not found; building only the serial renderer.")
endif()
