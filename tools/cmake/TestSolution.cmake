function(add_my_executable NAME)
  add_executable(${NAME} ${ARGN})
  set_target_properties(${NAME} PROPERTIES COMPILE_FLAGS "-Wall -Werror -Wextra -Wno-gnu-string-literal-operator-template -Wpedantic")
endfunction()

function(add_my_library NAME)
  add_library(${NAME} ${ARGN})
  set_target_properties(${NAME} PROPERTIES COMPILE_FLAGS "-Wall -Werror -Wextra -Wno-gnu-string-literal-operator-template -Wpedantic")
endfunction()

function(add_my_shared_library NAME)
  add_library(${NAME} SHARED ${ARGN})
  set_target_properties(${NAME} PROPERTIES COMPILE_FLAGS "-Wall -Werror -Wextra -Wno-gnu-string-literal-operator-template -Wpedantic")
endfunction()

function(add_catch TARGET)
  add_my_executable(${TARGET} ${ARGN})
  target_link_libraries(${TARGET} PRIVATE catch_main)
endfunction()
