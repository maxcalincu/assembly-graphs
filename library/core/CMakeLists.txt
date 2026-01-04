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

add_catch(test_core
    src/test_basics.cpp
    src/test_sag_with_endpoints.cpp
    src/test_paths.cpp
    src/test_generators.cpp
    src/test_set_builders.cpp
)

target_link_libraries(test_core PUBLIC core)

add_catch(test_basics src/test_basics.cpp)
target_link_libraries(test_basics PUBLIC core)

add_catch(test_sag_with_endpoints src/test_sag_with_endpoints.cpp)
target_link_libraries(test_sag_with_endpoints PUBLIC core)

add_catch(test_paths src/test_paths.cpp)
target_link_libraries(test_paths PUBLIC core)

add_catch(test_generators src/test_generators.cpp)
target_link_libraries(test_generators PUBLIC core)

add_catch(test_set_builders src/test_set_builders.cpp)
target_link_libraries(test_set_builders PUBLIC core)
