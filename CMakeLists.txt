cmake_minimum_required(VERSION 3.25)

project(
    clox
    VERSION 0.0.1
    LANGUAGES C
)

add_executable(
    clox
    "${CMAKE_CURRENT_SOURCE_DIR}/src/build.c"
)

target_include_directories(clox PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

set_property(TARGET clox PROPERTY C_STANDARD 11)
