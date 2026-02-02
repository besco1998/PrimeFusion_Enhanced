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

add_library(pf_msgpack_odometry SHARED
    src/msgpack_benchmark_odometry.cpp
)
target_link_libraries(pf_msgpack_odometry PRIVATE pf_common msgpack-cxx)
target_link_libraries(pf_msgpack_odometry PRIVATE pf_common msgpack-cxx)
target_include_directories(pf_msgpack_odometry PRIVATE include)

add_library(pf_msgpack_status SHARED
    src/msgpack_benchmark_status.cpp
)
target_link_libraries(pf_msgpack_status PRIVATE pf_common msgpack-cxx)
target_include_directories(pf_msgpack_status PRIVATE include)

add_library(pf_msgpack_attitude SHARED src/msgpack_benchmark_attitude.cpp)
target_link_libraries(pf_msgpack_attitude PRIVATE pf_common msgpack-cxx)
target_include_directories(pf_msgpack_attitude PRIVATE include)

add_library(pf_msgpack_global_position SHARED src/msgpack_benchmark_global_position.cpp)
target_link_libraries(pf_msgpack_global_position PRIVATE pf_common msgpack-cxx)
target_include_directories(pf_msgpack_global_position PRIVATE include)

add_library(pf_msgpack_gps_block SHARED src/msgpack_benchmark_gps_block.cpp)
target_link_libraries(pf_msgpack_gps_block PRIVATE pf_common msgpack-cxx)
target_include_directories(pf_msgpack_gps_block PRIVATE include)

add_library(pf_msgpack_battery SHARED src/msgpack_benchmark_battery.cpp)
target_link_libraries(pf_msgpack_battery PRIVATE pf_common msgpack-cxx)
target_include_directories(pf_msgpack_battery PRIVATE include)

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
