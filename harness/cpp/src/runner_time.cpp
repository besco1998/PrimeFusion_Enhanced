#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <dlfcn.h>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include "IBenchmark.h"

using namespace std::chrono;

typedef pf::IBenchmark* (*CreateBenchmarkFunc)();

int main(int argc, char** argv) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <plugin_path> <variant_name> <iterations> <output_dir>" << std::endl;
        return 1;
    }

    std::string plugin_path = argv[1];
    std::string variant_name = argv[2];
    size_t iterations = std::stoull(argv[3]);
    std::string output_dir = argv[4];

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

    // Payload
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

    // Warmup
    for(int i=0; i<100; i++) {
        auto b = bench->encode(payload);
        pf::Payload d;
        bench->decode(b, d);
    }

    // Critical Section: Time Measurement
    double total_encode_us = 0;
    double total_decode_us = 0;
    std::vector<uint8_t> buffer;

    auto t_start = high_resolution_clock::now();
    
    for(size_t i=0; i<iterations; i++) {
        auto t1 = high_resolution_clock::now();
        buffer = bench->encode(payload);
        auto t2 = high_resolution_clock::now();
        total_encode_us += duration_cast<nanoseconds>(t2 - t1).count() / 1000.0;
        
        pf::Payload d;
        auto t3 = high_resolution_clock::now();
        bench->decode(buffer, d);
        auto t4 = high_resolution_clock::now();
        total_decode_us += duration_cast<nanoseconds>(t4 - t3).count() / 1000.0;
    }

    auto t_end = high_resolution_clock::now();
    double total_wall_ms = duration_cast<milliseconds>(t_end - t_start).count();

    // Output Metrics to STDOUT for Python Harness to capture
    std::cout << "TOTAL_TIME_MS=" << total_wall_ms << std::endl;
    std::cout << "AVG_ENCODE_US=" << (total_encode_us/iterations) << std::endl;
    std::cout << "AVG_DECODE_US=" << (total_decode_us/iterations) << std::endl;
    
    bench->teardown();
    bench.reset(); // Destroy object before unloading library
    dlclose(handle);
    return 0;
}
