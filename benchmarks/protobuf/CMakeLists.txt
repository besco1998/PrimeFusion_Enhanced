find_package(Protobuf REQUIRED)

# Generate C++ from Proto
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS src/gps_beacon.proto)

add_library(pf_protobuf SHARED
    src/protobuf_benchmark.cpp
    ${PROTO_SRCS}
)

target_link_libraries(pf_protobuf PRIVATE pf_common)
target_link_libraries(pf_protobuf PRIVATE ${Protobuf_LIBRARIES})
target_include_directories(pf_protobuf PRIVATE ${Protobuf_INCLUDE_DIRS})
target_include_directories(pf_protobuf PRIVATE ${CMAKE_CURRENT_BINARY_DIR}) # For generated headers

if(BUILD_TESTING)
    add_executable(protobuf_integrity_test tests/test_integrity.cpp)
    target_link_libraries(protobuf_integrity_test PRIVATE pf_protobuf pf_common)
    add_test(NAME ProtobufIntegrity COMMAND protobuf_integrity_test)
endif()
