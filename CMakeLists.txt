cmake_minimum_required(VERSION 3.12)

project(sprite_paint)
add_subdirectory(raylib)
include_directories(includes)

add_executable(sprite_paint sprite_paint.cpp common.cpp ui.cpp)
set_property(TARGET sprite_paint PROPERTY CXX_STANDARD 23)

target_link_libraries(sprite_paint raylib)
