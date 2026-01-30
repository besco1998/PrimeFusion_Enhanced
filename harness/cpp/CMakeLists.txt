# Universal C++ Runners

# 1. Time Runner
add_executable(pf_runner_time src/runner_time.cpp)
target_link_libraries(pf_runner_time PRIVATE pf_common ${CMAKE_DL_LIBS})
target_compile_options(pf_runner_time PRIVATE -Wall -Wextra -Werror)
set_target_properties(pf_runner_time PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# 2. Memory Runner
add_executable(pf_runner_memory src/runner_memory.cpp)
target_link_libraries(pf_runner_memory PRIVATE pf_common ${CMAKE_DL_LIBS})
target_compile_options(pf_runner_memory PRIVATE -Wall -Wextra -Werror)
set_target_properties(pf_runner_memory PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
