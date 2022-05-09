cmake_minimum_required(VERSION 3.23.1)

project(OpenCL-Graphics)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} SDL2main SDL2 OpenCL)
