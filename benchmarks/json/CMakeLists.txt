add_library(pf_json SHARED
    src/json_benchmark.cpp
)

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
