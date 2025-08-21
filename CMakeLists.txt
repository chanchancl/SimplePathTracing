cmake_minimum_required(VERSION 3.10)

project(
    test
    LANGUAGES CXX
)

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS TRUE CACHE BOOL "Suppress developer warnings")

add_subdirectory(thridparty)
add_subdirectory(source)