cmake_minimum_required(VERSION 3.15)
project(PrimeFusion_Enhanced VERSION 2.0.0 LANGUAGES CXX)

# ==============================================================================
# 0. Global Standards
# ==============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optimization Flags
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Global Compiler Flags
add_compile_options(-Wall -Wextra -Wpedantic)

# USER REQUEST: "forcely run without any compiler or system optimization"
# We define a custom "NoOpt" build or just override Release flags
add_compile_options(-O0 -g) # Disable all optimizations

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Commenting out optimizations
    # add_compile_options(-O3 -DNDEBUG) 
    # add_compile_options(-march=native)
endif()

# ==============================================================================
# 1. External Dependencies
# ==============================================================================
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)

# LibCBOR and MsgPack might be system installed or local
# We prefer PkgConfig for robustness
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCBOR libcbor)
pkg_check_modules(MSGPACK msgpack)
pkg_check_modules(RAPIDJSON RapidJSON)


# ==============================================================================
# 2. Core Modules
# ==============================================================================
# Add Common Interfaces (The "Immutable Core")
add_subdirectory(benchmarks/common)


# ==============================================================================
# 3. Automatic Format Discovery (Plugins)
# ==============================================================================
# Glob all directories in benchmarks/ and add them if they have a CMakeLists.txt
file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks benchmarks/*)
foreach(child ${children})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/${child})
        if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/${child}/CMakeLists.txt)
            if(NOT ${child} STREQUAL "common")
                message(STATUS "Found Benchmark Plugin: ${child}")
                add_subdirectory(benchmarks/${child})
            endif()
        endif()
    endif()
endforeach()

# ==============================================================================
# 4. Harness
# ==============================================================================
# We can add a C++ Harness here, or rely on Python to invoke binaries.
# For this architecture, we will build a 'runner_main' if desired, but 
# primarily we build the individual benchmark binaries from the plugins.

# Universal C++ Runner
add_subdirectory(harness/cpp)

# ==============================================================================
# 5. Testing
# ==============================================================================
enable_testing()
