cmake_minimum_required(VERSION 3.16)

project(RayTray LANGUAGES CXX)

add_executable(raytracer main.cpp)

target_compile_features(raytracer PRIVATE cxx_std_11)

if (MSVC)
    target_compile_options(raytracer PRIVATE /W4)
else()
    target_compile_options(raytracer PRIVATE -Wall -Wextra -pedantic)
endif()
