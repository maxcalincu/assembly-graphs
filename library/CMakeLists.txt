add_library(assembly-graphs INTERFACE)

target_compile_features(assembly-graphs INTERFACE cxx_std_20)

add_subdirectory(core)

target_link_libraries(assembly-graphs
    INTERFACE
        core
)
