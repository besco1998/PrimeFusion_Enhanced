add_library(pf_common INTERFACE)

target_include_directories(pf_common INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# We might add common utilities later (Timer, Stats, etc.)
# target_sources(pf_common PRIVATE src/Timer.cpp)
