target_sources(rsl-util-test PRIVATE
    assign.cpp
    constructor.cpp
    empty.cpp
    structured_bindings.cpp
    tie.cpp
    make_tuple.cpp
)

add_subdirectory(tuple.get)
add_subdirectory(extensions)
