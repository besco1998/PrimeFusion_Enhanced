add_library(pf_cbor SHARED
    src/cbor_benchmark.cpp
)

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
