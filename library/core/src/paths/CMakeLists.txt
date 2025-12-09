add_catch(test_paths test.cpp)

target_include_directories(test_paths PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(test_paths PUBLIC
    core
)