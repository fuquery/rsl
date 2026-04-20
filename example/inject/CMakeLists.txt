
add_executable(injector injection_impl.cpp)
target_link_libraries(injector PRIVATE rsl-util)


set(all_injected_file "${CMAKE_CURRENT_SOURCE_DIR}/injected.cpp")
add_custom_command(
    OUTPUT ${all_injected_file}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E touch ${all_injected_file}
    COMMAND ${CMAKE_COMMAND} -E echo "Running injector from source dir..."
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:injector>
)
add_library(injected_lib injected.cpp)
add_dependencies(injected_lib injector)

add_executable(injection_example main.cpp)
target_link_libraries(injection_example PRIVATE injected_lib)
