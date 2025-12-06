add_library(core SHARED
    src/sag_with_endpoints/sag_with_endpoints.cpp
)

target_compile_features(core PUBLIC cxx_std_20)

target_include_directories(core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(src/basics)
add_subdirectory(src/sag_with_endpoints)
