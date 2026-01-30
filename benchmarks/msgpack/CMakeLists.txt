include(FetchContent)

message(STATUS "Fetching msgpack-cxx (cpp-6.1.0)...")
FetchContent_Declare(
    msgpack-cxx
    GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
    GIT_TAG        cpp-6.1.0
)
FetchContent_MakeAvailable(msgpack-cxx)

add_library(pf_msgpack SHARED
    src/msgpack_benchmark.cpp
)

target_link_libraries(pf_msgpack PRIVATE pf_common)
# msgpack-cxx is a header-only library, usually target name is msgpack-cxx
target_link_libraries(pf_msgpack PRIVATE msgpack-cxx)

# We might need to add the include directory manually if not exported perfectly by the legacy fetch
# but typically modern cmake does it. Let's rely on the target.

if(BUILD_TESTING)
    add_executable(msgpack_integrity_test tests/test_integrity.cpp)
    target_link_libraries(msgpack_integrity_test PRIVATE pf_msgpack pf_common)
    add_test(NAME MsgPackIntegrity COMMAND msgpack_integrity_test)
endif()
