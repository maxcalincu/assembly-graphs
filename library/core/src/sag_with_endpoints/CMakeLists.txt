add_library(sag_with_endpoints
    sag_with_endpoints.cpp
)

target_include_directories(sag_with_endpoints PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

add_catch(test_sag_with_endpoints test.cpp)

target_include_directories(test_sag_with_endpoints PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(test_sag_with_endpoints PUBLIC
    sag_with_endpoints
)
