cmake_minimum_required(VERSION 3.13)

project(sprite_paint)
add_subdirectory(raylib)
include_directories(includes)

add_executable(sprite_paint sprite_paint.cpp common.cpp ui.cpp)

target_link_libraries(sprite_paint raylib "-static-libstdc++")
