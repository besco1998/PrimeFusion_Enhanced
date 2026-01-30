#include <iostream>
#include <vector>
#include <string>
#include <dlfcn.h>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <malloc.h>
#include "IBenchmark.h"

typedef pf::IBenchmark* (*CreateBenchmarkFunc)();

// Helper: Get Peak RSS
size_t get_peak_rss() {
    FILE* fp = fopen("/proc/self/status", "r");
    if (!fp) return 0;
    char line[128];
    size_t result = 0;
    while (fgets(line, 128, fp)) {
        if (strncmp(line, "VmPeak:", 7) == 0) {
            sscanf(line + 7, "%lu", &result);
            break;
        }
    }
    fclose(fp);
    return result; 
}

// Helper: Get Mallinfo (Heap)
size_t get_allocated_mem() {
    struct mallinfo2 mi = mallinfo2();
    return mi.uordblks;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <plugin_path> <variant_name> <iterations>" << std::endl;
        return 1;
    }

    std::string plugin_path = argv[1];
    std::string variant_name = argv[2];
    size_t iterations = std::stoull(argv[3]);
    // Note: output_dir is not used by C++ runner anymore, Python handles CSV

    // Load Plugin
    void* handle = dlopen(plugin_path.c_str(), RTLD_LAZY);
    if (!handle) { std::cerr << dlerror() << std::endl; return 1; }

    CreateBenchmarkFunc create = (CreateBenchmarkFunc) dlsym(handle, "create_benchmark");
    if (!create) { std::cerr << dlerror() << std::endl; return 1; }

    std::unique_ptr<pf::IBenchmark> bench(create());
    pf::BenchmarkConfig config;
    config.iterations = iterations;
    config.variant_name = variant_name;
    config.warm_up = true;
    bench->setup(config);

    // Payload Initialization (Deterministic)
    pf::Payload payload;
    payload.timestamp = 1600000000ULL;
    payload.block_number = 12345;
    // Hash: 0x00, 0x01, ... 0x1F
    for(int i=0; i<32; i++) payload.hash[i] = (uint8_t)i;
    
    payload.time_usec = 987654321ULL;
    payload.fix_type = 3; // 3D Fix
    payload.lat = 37774929; // Micro-degrees
    payload.lon = -122419416;
    payload.alt = 10000; // mm
    payload.eph = 1500;
    payload.epv = 2000;
    payload.vel = 1234;
    payload.cog = 5678;
    payload.satellites_visible = 12;
    payload.alt_ellipsoid = 11500;
    payload.h_acc = 500;
    payload.v_acc = 400;
    payload.vel_acc = 100;
    payload.hdg_acc = 200;

    // 1. Cold Start Measurement (First Call)
    size_t cold_start_heap = get_allocated_mem();
    {
        auto b = bench->encode(payload);
        pf::Payload d;
        bench->decode(b, d);
    }
    size_t cold_end_heap = get_allocated_mem();
    long cold_delta = (long)cold_end_heap - (long)cold_start_heap; // Can be negative in weird libc caching cases, effectively 0

    // 2. Warmup (Stabilize Pools)
    for(int i=0; i<100; i++) {
        auto b = bench->encode(payload);
        pf::Payload d;
        bench->decode(b, d);
    }

    // 3. Warm Measurement (N Iterations)
    size_t warm_start_heap = get_allocated_mem();
    
    std::vector<uint8_t> buffer;
    for(size_t i=0; i<iterations; i++) {
        buffer = bench->encode(payload);
        pf::Payload d;
        bench->decode(buffer, d);
    }
    
    size_t warm_end_heap = get_allocated_mem();
    // Verify average per iteration
    long warm_total_delta = (long)warm_end_heap - (long)warm_start_heap;
    
    size_t peak_rss = get_peak_rss();
    size_t ser_size = buffer.size();

    // Output Metrics to STDOUT
    std::cout << "PEAK_RSS_KB=" << peak_rss << std::endl;
    std::cout << "MALLOC_DELTA_COLD=" << cold_delta << std::endl;
    std::cout << "MALLOC_DELTA_WARM=" << warm_total_delta << std::endl;
    std::cout << "SERIALIZED_SIZE=" << ser_size << std::endl;

    bench->teardown();
    bench.reset(); // Destroy object before unloading library
    dlclose(handle);
    return 0;
}
