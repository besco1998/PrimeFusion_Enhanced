find_package(Protobuf REQUIRED)

# Generate C++ from Proto
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS src/gps_beacon.proto)

add_library(pf_protobuf SHARED
    src/protobuf_benchmark.cpp
    gps_beacon.pb.cc
)

add_library(pf_protobuf_odometry SHARED
    src/protobuf_benchmark_odometry.cpp
    gps_beacon.pb.cc
)
target_link_libraries(pf_protobuf_odometry PRIVATE pf_common ${Protobuf_LIBRARIES})
target_include_directories(pf_protobuf_odometry PRIVATE ${Protobuf_INCLUDE_DIRS} include ${CMAKE_BINARY_DIR}/benchmarks/protobuf)

add_library(pf_protobuf_status SHARED
    src/protobuf_benchmark_status.cpp
    gps_beacon.pb.cc
)
target_link_libraries(pf_protobuf_status PRIVATE pf_common ${Protobuf_LIBRARIES})
target_include_directories(pf_protobuf_status PRIVATE ${Protobuf_INCLUDE_DIRS} include ${CMAKE_BINARY_DIR}/benchmarks/protobuf)

add_library(pf_protobuf_attitude SHARED src/protobuf_benchmark_attitude.cpp gps_beacon.pb.cc)
target_link_libraries(pf_protobuf_attitude PRIVATE pf_common ${Protobuf_LIBRARIES})
target_include_directories(pf_protobuf_attitude PRIVATE ${Protobuf_INCLUDE_DIRS} include ${CMAKE_BINARY_DIR}/benchmarks/protobuf)

add_library(pf_protobuf_global_position SHARED src/protobuf_benchmark_global_position.cpp gps_beacon.pb.cc)
target_link_libraries(pf_protobuf_global_position PRIVATE pf_common ${Protobuf_LIBRARIES})
target_include_directories(pf_protobuf_global_position PRIVATE ${Protobuf_INCLUDE_DIRS} include ${CMAKE_BINARY_DIR}/benchmarks/protobuf)

add_library(pf_protobuf_gps_block SHARED src/protobuf_benchmark_gps_block.cpp gps_beacon.pb.cc)
target_link_libraries(pf_protobuf_gps_block PRIVATE pf_common ${Protobuf_LIBRARIES})
target_include_directories(pf_protobuf_gps_block PRIVATE ${Protobuf_INCLUDE_DIRS} include ${CMAKE_BINARY_DIR}/benchmarks/protobuf)

add_library(pf_protobuf_battery SHARED src/protobuf_benchmark_battery.cpp gps_beacon.pb.cc)
target_link_libraries(pf_protobuf_battery PRIVATE pf_common ${Protobuf_LIBRARIES})
target_include_directories(pf_protobuf_battery PRIVATE ${Protobuf_INCLUDE_DIRS} include ${CMAKE_BINARY_DIR}/benchmarks/protobuf)

target_link_libraries(pf_protobuf PRIVATE pf_common)
target_link_libraries(pf_protobuf PRIVATE ${Protobuf_LIBRARIES})
target_include_directories(pf_protobuf PRIVATE ${Protobuf_INCLUDE_DIRS})
target_include_directories(pf_protobuf PRIVATE ${CMAKE_CURRENT_BINARY_DIR}) # For generated headers

if(BUILD_TESTING)
    add_executable(protobuf_integrity_test tests/test_integrity.cpp)
    target_link_libraries(protobuf_integrity_test PRIVATE pf_protobuf pf_common)
    add_test(NAME ProtobufIntegrity COMMAND protobuf_integrity_test)
endif()
