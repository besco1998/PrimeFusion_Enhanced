add_library(pf_cbor SHARED
    src/cbor_benchmark.cpp
)

add_library(pf_cbor_odometry SHARED
    src/cbor_benchmark_odometry.cpp
)
target_link_libraries(pf_cbor_odometry PRIVATE pf_common ${LIBCBOR_LIBRARIES})
target_include_directories(pf_cbor_odometry PRIVATE ${LIBCBOR_INCLUDE_DIRS} include)

add_library(pf_cbor_status SHARED
    src/cbor_benchmark_status.cpp
)
target_link_libraries(pf_cbor_status PRIVATE pf_common ${LIBCBOR_LIBRARIES})
target_include_directories(pf_cbor_status PRIVATE ${LIBCBOR_INCLUDE_DIRS} include)

add_library(pf_cbor_attitude SHARED src/cbor_benchmark_attitude.cpp)
target_link_libraries(pf_cbor_attitude PRIVATE pf_common ${LIBCBOR_LIBRARIES})
target_include_directories(pf_cbor_attitude PRIVATE ${LIBCBOR_INCLUDE_DIRS} include)

add_library(pf_cbor_global_position SHARED src/cbor_benchmark_global_position.cpp)
target_link_libraries(pf_cbor_global_position PRIVATE pf_common ${LIBCBOR_LIBRARIES})
target_include_directories(pf_cbor_global_position PRIVATE ${LIBCBOR_INCLUDE_DIRS} include)

add_library(pf_cbor_gps_block SHARED src/cbor_benchmark_gps_block.cpp)
target_link_libraries(pf_cbor_gps_block PRIVATE pf_common ${LIBCBOR_LIBRARIES})
target_include_directories(pf_cbor_gps_block PRIVATE ${LIBCBOR_INCLUDE_DIRS} include)

add_library(pf_cbor_battery SHARED src/cbor_benchmark_battery.cpp)
target_link_libraries(pf_cbor_battery PRIVATE pf_common ${LIBCBOR_LIBRARIES})
target_include_directories(pf_cbor_battery PRIVATE ${LIBCBOR_INCLUDE_DIRS} include)

target_link_libraries(pf_cbor PRIVATE pf_common)
target_link_libraries(pf_cbor PRIVATE ${LIBCBOR_LIBRARIES})
target_include_directories(pf_cbor PRIVATE ${LIBCBOR_INCLUDE_DIRS})
target_include_directories(pf_cbor PRIVATE include)

if(NOT LIBCBOR_FOUND)
    message(FATAL_ERROR "LibCBOR not found! Please run tools/install_dependencies.sh")
endif()

if(BUILD_TESTING)
    add_executable(cbor_integrity_test tests/test_integrity.cpp)
    target_link_libraries(cbor_integrity_test PRIVATE pf_cbor pf_common)
    add_test(NAME CborIntegrity COMMAND cbor_integrity_test)
endif()
