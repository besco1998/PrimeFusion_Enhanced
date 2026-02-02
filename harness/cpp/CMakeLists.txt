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

# 3. Battery Runner (Multi-Type Pilot)
add_executable(pf_runner_battery src/runner_battery.cpp)
target_link_libraries(pf_runner_battery PRIVATE pf_common ${CMAKE_DL_LIBS})
target_compile_options(pf_runner_battery PRIVATE -Wall -Wextra -Werror)
set_target_properties(pf_runner_battery PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# 4. Odometry Runner
add_executable(pf_runner_odometry src/runner_odometry.cpp)
target_link_libraries(pf_runner_odometry PRIVATE pf_common ${CMAKE_DL_LIBS})
target_compile_options(pf_runner_odometry PRIVATE -Wall -Wextra -Werror)
set_target_properties(pf_runner_odometry PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# 5. Attitude Runner
add_executable(pf_runner_attitude src/runner_attitude.cpp)
target_link_libraries(pf_runner_attitude PRIVATE pf_common ${CMAKE_DL_LIBS})
target_compile_options(pf_runner_attitude PRIVATE -Wall -Wextra -Werror)
set_target_properties(pf_runner_attitude PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# 6. GlobalPosition Runner
add_executable(pf_runner_global_position src/runner_global_position.cpp)
target_link_libraries(pf_runner_global_position PRIVATE pf_common ${CMAKE_DL_LIBS})
target_compile_options(pf_runner_global_position PRIVATE -Wall -Wextra -Werror)
set_target_properties(pf_runner_global_position PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# 7. Status Runner
add_executable(pf_runner_status src/runner_status.cpp)
target_link_libraries(pf_runner_status PRIVATE pf_common ${CMAKE_DL_LIBS})
target_compile_options(pf_runner_status PRIVATE -Wall -Wextra -Werror)
set_target_properties(pf_runner_status PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# 8. GPS Block Runner
add_executable(pf_runner_gps_block src/runner_gps_block.cpp)
target_link_libraries(pf_runner_gps_block PRIVATE pf_common ${CMAKE_DL_LIBS})

add_executable(pf_runner_gps_raw src/runner_gps_raw.cpp)
target_link_libraries(pf_runner_gps_raw PRIVATE pf_common ${CMAKE_DL_LIBS})
target_compile_options(pf_runner_gps_raw PRIVATE -Wall -Wextra -Werror)
set_target_properties(pf_runner_gps_raw PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
target_compile_options(pf_runner_gps_block PRIVATE -Wall -Wextra -Werror)
set_target_properties(pf_runner_gps_block PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


