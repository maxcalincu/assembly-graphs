# add_library(basics
    # basics.cpp
# )

# target_include_directories(basics PUBLIC
#   ${CMAKE_CURRENT_SOURCE_DIR}/../../include
# )

add_catch(test_basics test.cpp)

target_include_directories(test_basics PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# target_link_libraries(test_basics PUBLIC
#     basics
# )
