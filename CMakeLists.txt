cmake_minimum_required(VERSION 3.10)

project(
    SimpleTracingRenderer
    LANGUAGES CXX
)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS TRUE CACHE BOOL "Suppress developer warnings")

set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(thirdparty)
add_subdirectory(source)