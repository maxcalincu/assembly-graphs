add_catch(test_generators test.cpp)

target_include_directories(test_generators PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(test_generators PUBLIC
    core
)