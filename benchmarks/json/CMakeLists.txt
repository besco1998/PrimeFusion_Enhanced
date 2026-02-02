add_library(pf_json SHARED
    src/json_benchmark.cpp
)

add_library(pf_json_battery SHARED
    src/json_benchmark_battery.cpp
)
target_link_libraries(pf_json_battery PRIVATE pf_common ${RAPIDJSON_LIBRARIES})
target_include_directories(pf_json_battery PRIVATE ${RAPIDJSON_INCLUDE_DIRS} include)

add_library(pf_json_odometry SHARED
    src/json_benchmark_odometry.cpp
)
target_link_libraries(pf_json_odometry PRIVATE pf_common ${RAPIDJSON_LIBRARIES})
target_include_directories(pf_json_odometry PRIVATE ${RAPIDJSON_INCLUDE_DIRS} include)

add_library(pf_json_status SHARED
    src/json_benchmark_status.cpp
)
target_link_libraries(pf_json_status PRIVATE pf_common ${RAPIDJSON_LIBRARIES})
target_include_directories(pf_json_status PRIVATE ${RAPIDJSON_INCLUDE_DIRS} include)

add_library(pf_json_attitude SHARED src/json_benchmark_attitude.cpp)
target_link_libraries(pf_json_attitude PRIVATE pf_common ${RAPIDJSON_LIBRARIES})
target_include_directories(pf_json_attitude PRIVATE ${RAPIDJSON_INCLUDE_DIRS} include)

add_library(pf_json_global_position SHARED src/json_benchmark_global_position.cpp)
target_link_libraries(pf_json_global_position PRIVATE pf_common ${RAPIDJSON_LIBRARIES})
target_include_directories(pf_json_global_position PRIVATE ${RAPIDJSON_INCLUDE_DIRS} include)

add_library(pf_json_gps_block SHARED src/json_benchmark_gps_block.cpp)
target_link_libraries(pf_json_gps_block PRIVATE pf_common ${RAPIDJSON_LIBRARIES})
target_include_directories(pf_json_gps_block PRIVATE ${RAPIDJSON_INCLUDE_DIRS} include)


# Link against the Core Interface
target_link_libraries(pf_json PRIVATE pf_common)

# Link against RapidJSON
target_link_libraries(pf_json PRIVATE ${RAPIDJSON_LIBRARIES})
target_include_directories(pf_json PRIVATE ${RAPIDJSON_INCLUDE_DIRS})

# Allow including local headers
target_include_directories(pf_json PRIVATE include)

# Verify we have the required dependency
if(NOT RAPIDJSON_FOUND)
    message(FATAL_ERROR "RapidJSON not found! Please run tools/install_dependencies.sh")
endif()

# ==============================================================================
# Tests
# ==============================================================================
if(BUILD_TESTING)
    add_executable(json_integrity_test tests/test_integrity.cpp)
    target_link_libraries(json_integrity_test PRIVATE pf_json pf_common)
    add_test(NAME JsonIntegrity COMMAND json_integrity_test)
endif()
