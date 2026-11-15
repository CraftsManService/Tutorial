cmake_minimum_required(VERSION 3.15)
project(Tutorial C CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(Tutorial 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stb_image_implementation.cpp
)

add_executable(2d_Physics
    ${CMAKE_CURRENT_SOURCE_DIR}/src/2d_Physics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stb_image_implementation.cpp
)

target_link_libraries(Tutorial PRIVATE OpenGL::GL glfw)
target_link_libraries(2d_Physics PRIVATE OpenGL::GL glfw)