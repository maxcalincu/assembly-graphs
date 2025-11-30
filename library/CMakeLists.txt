add_library(assembly-graphs)

add_subdirectory(core)
add_subdirectory(graphics)

target_link_libraries(assembly-graphs
    PUBLIC
        core
    PRIVATE
        graphics
)
