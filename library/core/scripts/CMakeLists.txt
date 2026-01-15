find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(construct_set construct_set.cpp)

target_link_libraries(construct_set PRIVATE
    core
    Boost::program_options
)