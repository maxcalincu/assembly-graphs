add_catch(test_core
    test_basics.cpp
    test_sag_with_endpoints.cpp
    test_paths.cpp
    test_generators.cpp
    test_set_builders.cpp
)

target_link_libraries(test_core PUBLIC core)

add_catch(test_basics test_basics.cpp)
target_link_libraries(test_basics PUBLIC core)

add_catch(test_sag_with_endpoints test_sag_with_endpoints.cpp)
target_link_libraries(test_sag_with_endpoints PUBLIC core)

add_catch(test_paths test_paths.cpp)
target_link_libraries(test_paths PUBLIC core)

add_catch(test_generators test_generators.cpp)
target_link_libraries(test_generators PUBLIC core)

add_catch(test_set_builders test_set_builders.cpp)
target_link_libraries(test_set_builders PUBLIC core)
