
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE sourceFiles CONFIGURE_DEPENDS src/*.cpp)

add_executable(SimpleTracingRenderer ${sourceFiles})

target_include_directories(SimpleTracingRenderer PUBLIC include)
target_compile_definitions(SimpleTracingRenderer PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_FORCE_RADIANS GLM_FORCE_LEFT_HANDED)

target_compile_definitions(SimpleTracingRenderer PUBLIC WITH_DEBUG_INFO)

target_link_libraries(SimpleTracingRenderer PRIVATE glm fast_obj_lib)
