set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE sourceFiles CONFIGURE_DEPENDS src/*.cpp)

add_executable(test ${sourceFiles})

target_include_directories(test PUBLIC include)
target_compile_definitions(test PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_FORCE_RADIANS GLM_FORCE_LEFT_HANDED)

target_link_libraries(test glm)
