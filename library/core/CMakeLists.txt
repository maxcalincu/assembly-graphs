add_library(core SHARED
    src/basics.cpp
    src/sag_with_endpoints.cpp
    src/paths.cpp
    src/generators.cpp
    src/set_builders.cpp
)

target_compile_features(core PUBLIC cxx_std_20)
target_include_directories(core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(tools)
add_subdirectory(tests)