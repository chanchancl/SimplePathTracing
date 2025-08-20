set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE sourceFiles CONFIGURE_DEPENDS src/*.cpp)

add_executable(test ${sourceFiles})

target_include_directories(test PUBLIC include)

target_link_libraries(test glm)
