#ifndef RUNNER_TEMPLATE_HPP
#define RUNNER_TEMPLATE_HPP

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <dlfcn.h>
#include <memory>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <malloc.h>
#include <cstdint>
#include <random>
#include "IBenchmark.h"
#include "mavlink_types.h"

using namespace std::chrono;

typedef pf::IBenchmark* (*CreateBenchmarkFunc)();

namespace pf {

static std::mt19937 gen(42); // Fixed seed for reproducibility

// Helper: Random Data Generation (Template to be specialized)
template <typename T>
T generate_random_data() {
    T data;
    memset(&data, 0, sizeof(T));
    return data; 
}

// Specializations for our types
template <>
PayloadGPSRaw generate_random_data<PayloadGPSRaw>() {
    PayloadGPSRaw p;
    memset(&p, 0, sizeof(p));
    // Use true randomness to test variable parsing paths
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
    std::uniform_int_distribution<int32_t> dist32(INT32_MIN, INT32_MAX);
    
    p.timestamp = dist(gen);
    p.block_number = (uint32_t)dist(gen);
    for(int i=0; i<32; i++) p.hash[i] = (uint8_t)dist(gen);
    p.time_usec = dist(gen);
    p.fix_type = (uint8_t)(dist(gen) % 5); 
    p.lat = dist32(gen); 
    p.lon = dist32(gen);
    p.alt = dist32(gen); 
    p.eph = (uint16_t)dist(gen);
    p.epv = (uint16_t)dist(gen);
    p.vel = (uint16_t)dist(gen);
    p.cog = (uint16_t)dist(gen);
    p.satellites_visible = (uint8_t)(dist(gen) % 20);
    p.alt_ellipsoid = dist32(gen);
    p.h_acc = (uint32_t)dist(gen);
    p.v_acc = (uint32_t)dist(gen);
    p.vel_acc = (uint32_t)dist(gen);
    p.hdg_acc = (uint32_t)dist(gen);
    return p;
}

template <>
PayloadBattery generate_random_data<PayloadBattery>() {
    PayloadBattery p;
    memset(&p, 0, sizeof(p));
    std::uniform_int_distribution<int> dist(0, 10000);
    p.id = dist(gen) % 255;
    p.battery_function = dist(gen) % 3;
    p.type = dist(gen) % 4;
    p.temperature = dist(gen);
    for(int i=0; i<10; i++) p.voltages[i] = (uint16_t)dist(gen);
    p.current_battery = (int16_t)dist(gen);
    p.current_consumed = (int32_t)dist(gen);
    p.energy_consumed = (int32_t)dist(gen);
    p.battery_remaining = (int8_t)(dist(gen) % 100);
    return p;
}

template <>
PayloadOdometry generate_random_data<PayloadOdometry>() {
    PayloadOdometry p;
    memset(&p, 0, sizeof(p));
    std::uniform_real_distribution<float> distf(-100.0f, 100.0f);
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
    
    p.time_usec = dist(gen);
    p.frame_id = (uint8_t)(dist(gen) % 20);
    p.child_frame_id = (uint8_t)(dist(gen) % 20);
    p.x = distf(gen); p.y = distf(gen); p.z = distf(gen);
    for(int i=0; i<4; i++) p.q[i] = distf(gen); // Not normalized, fine for benchmark
    p.vx = distf(gen); p.vy = distf(gen); p.vz = distf(gen);
    p.rollspeed = distf(gen); p.pitchspeed = distf(gen); p.yawspeed = distf(gen);
    for(int i=0; i<21; i++) {
        p.pose_covariance[i] = distf(gen);
        p.velocity_covariance[i] = distf(gen);
    }
    return p;
}

template <>
PayloadAttitude generate_random_data<PayloadAttitude>() {
    PayloadAttitude p;
    memset(&p, 0, sizeof(p));
    std::uniform_real_distribution<float> distf(-3.14f, 3.14f);
    std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);
    p.time_boot_ms = dist(gen);
    p.roll = distf(gen);
    p.pitch = distf(gen);
    p.yaw = distf(gen);
    p.rollspeed = distf(gen);
    p.pitchspeed = distf(gen);
    p.yawspeed = distf(gen);
    return p;
}

template <>
PayloadGlobalPosition generate_random_data<PayloadGlobalPosition>() {
    PayloadGlobalPosition p;
    memset(&p, 0, sizeof(p));
    std::uniform_int_distribution<int32_t> dist32(INT32_MIN, INT32_MAX);
    std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);
    
    p.time_boot_ms = dist(gen);
    p.lat = dist32(gen);
    p.lon = dist32(gen);
    p.alt = dist32(gen);
    p.relative_alt = dist32(gen);
    p.vx = (int16_t)(dist(gen) % 2000);
    p.vy = (int16_t)(dist(gen) % 2000);
    p.vz = (int16_t)(dist(gen) % 2000);
    p.hdg = (uint16_t)(dist(gen) % 36000);
    return p;
}

template <>
PayloadStatus generate_random_data<PayloadStatus>() {
    PayloadStatus p;
    memset(&p, 0, sizeof(p));
    p.severity = (uint8_t)(gen() % 8);
    // Generate random string
    std::uniform_int_distribution<int> char_dist(32, 126);
    int len = 10 + (gen() % 39);
    for(int i=0; i<len; i++) p.text[i] = (char)char_dist(gen);
    p.text[len] = 0;
    return p;
}

template <>
PayloadGPSBlock generate_random_data<PayloadGPSBlock>() {
    PayloadGPSBlock p;
    int count = 10 + (gen() % 40); // 10 to 50 items
    for(int i=0; i<count; i++) {
        PayloadGPSRaw m = generate_random_data<PayloadGPSRaw>();
        m.block_number = i; 
        p.messages.push_back(m);
    }
    return p;
}

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

// Global Pool Strategy to simulate real data entropy
const int POOL_SIZE = 127; // Prime-ish to avoid alignment artifacts

template <typename PayloadT>
int run_time_benchmark(int argc, char** argv) {
     if (argc < 4) { 
        std::cerr << "Usage: " << argv[0] << " <plugin_path> <variant_name> <iterations>" << std::endl;
        return 1;
    }

    std::string plugin_path = argv[1];
    std::string variant_name = argv[2];
    size_t iterations = std::stoull(argv[3]);

    void* handle = dlopen(plugin_path.c_str(), RTLD_LAZY);
    if (!handle) { std::cerr << "DLOPEN ERR: " << dlerror() << std::endl; return 1; }

    CreateBenchmarkFunc create = (CreateBenchmarkFunc) dlsym(handle, "create_benchmark");
    if (!create) { std::cerr << "DLSYM ERR: " << dlerror() << std::endl; return 1; }

    std::unique_ptr<pf::IBenchmark> bench(create());
    pf::BenchmarkConfig config;
    config.iterations = iterations;
    config.variant_name = variant_name;
    config.warm_up = true;
    bench->setup(config);

    // 1. Generate Data Pool
    std::vector<PayloadT> pool(POOL_SIZE);
    for(int i=0; i<POOL_SIZE; i++) pool[i] = generate_random_data<PayloadT>();

    // 2. Warmup
    for(int i=0; i<100; i++) {
        auto b = bench->encode(&pool[i % POOL_SIZE]);
        PayloadT d;
        bench->decode(b, &d);
    }

    // 3. Encode Latency (Batch Timing)
    double total_encode_us = 0;
    std::vector<uint8_t> last_buffer; // Keep alive to prevent optimization
    
    // We can't store ALL buffers for millions of iterations (RAM limit).
    // But we need to time the loop.
    // We will overwrite a small set of scratch buffers?
    // Actually, std::vector return is by value.
    // We just sink it.
    
    volatile size_t sink = 0;
    
    auto t1 = high_resolution_clock::now();
    for(size_t i=0; i<iterations; i++) {
        std::vector<uint8_t> buf = bench->encode(&pool[i % POOL_SIZE]);
        sink += buf.size();
        if (i == 0) last_buffer = buf; // Keep one for size check
    }
    auto t2 = high_resolution_clock::now();
    total_encode_us = duration_cast<nanoseconds>(t2 - t1).count() / 1000.0;

    // 4. Decode Latency (Batch Timing)
    // Pre-encode the pool so we have valid inputs
    std::vector<std::vector<uint8_t>> encoded_pool(POOL_SIZE);
    for(int i=0; i<POOL_SIZE; i++) encoded_pool[i] = bench->encode(&pool[i]);

    auto t3 = high_resolution_clock::now();
    for(size_t i=0; i<iterations; i++) {
        PayloadT d;
        bench->decode(encoded_pool[i % POOL_SIZE], &d);
        // sink += d.timestamp; // We can't access generic fields easily. 
        // Logic relies on side-effects or volatile. 
        // Decoder generally writes to `d`. Constructor/Destructor of d runs.
    }
    auto t4 = high_resolution_clock::now();
    double total_decode_us = duration_cast<nanoseconds>(t4 - t3).count() / 1000.0;

    double total_wall_ms = duration_cast<milliseconds>(t4 - t1).count();

    std::cout << "TOTAL_TIME_MS=" << total_wall_ms << std::endl;
    std::cout << "AVG_ENCODE_US=" << (total_encode_us/iterations) << std::endl;
    std::cout << "AVG_DECODE_US=" << (total_decode_us/iterations) << std::endl;
    std::cout << "SERIALIZED_SIZE=" << encoded_pool[0].size() << std::endl; // Sample

    bench->teardown();
    bench.reset();
    dlclose(handle);
    return 0;
}

template <typename PayloadT>
int run_memory_benchmark(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " --memory <plugin_path> <variant_name> <iterations>" << std::endl;
        return 1;
    }
    std::string plugin_path = argv[1];
    std::string variant_name = argv[2];
    size_t iterations = std::stoull(argv[3]);

    void* handle = dlopen(plugin_path.c_str(), RTLD_LAZY);
    if (!handle) { std::cerr << "DLOPEN ERR: " << dlerror() << std::endl; return 1; }
    CreateBenchmarkFunc create = (CreateBenchmarkFunc) dlsym(handle, "create_benchmark");
    if (!create) { std::cerr << "DLSYM ERR: " << dlerror() << std::endl; return 1; }

    std::unique_ptr<pf::IBenchmark> bench(create());
    pf::BenchmarkConfig config;
    config.iterations = iterations;
    config.variant_name = variant_name;
    config.warm_up = true;
    bench->setup(config);

    // Pool
    std::vector<PayloadT> pool(POOL_SIZE);
    for(int i=0; i<POOL_SIZE; i++) pool[i] = generate_random_data<PayloadT>();

    // 1. Cold Start
    size_t cold_start_heap = get_allocated_mem();
    {
        auto b = bench->encode(&pool[0]);
        PayloadT d;
        bench->decode(b, &d);
    }
    size_t cold_end_heap = get_allocated_mem();
    long cold_delta = (long)cold_end_heap - (long)cold_start_heap;

    // 2. Warmup
    for(int i=0; i<100; i++) {
        auto b = bench->encode(&pool[i % POOL_SIZE]);
        PayloadT d;
        bench->decode(b, &d);
    }

    // 3. Warm Measurement
    size_t warm_start_heap = get_allocated_mem();
    std::vector<uint8_t> buffer;
    for(size_t i=0; i<iterations; i++) {
        buffer = bench->encode(&pool[i % POOL_SIZE]);
        PayloadT d;
        bench->decode(buffer, &d);
    }
    size_t warm_end_heap = get_allocated_mem();
    long warm_total_delta = (long)warm_end_heap - (long)warm_start_heap;
    
    size_t peak_rss = get_peak_rss();
    size_t ser_size = buffer.size();

    std::cout << "PEAK_RSS_KB=" << peak_rss << std::endl;
    std::cout << "MALLOC_DELTA_COLD=" << cold_delta << std::endl;
    std::cout << "MALLOC_DELTA_WARM=" << warm_total_delta << std::endl;
    std::cout << "SERIALIZED_SIZE=" << ser_size << std::endl;

    bench->teardown();
    bench.reset();
    dlclose(handle);
    return 0;
} 

template <typename PayloadT>
int run_benchmark_template(int argc, char** argv) {
    if (argc > 1 && std::string(argv[1]) == "--memory") {
        return run_memory_benchmark<PayloadT>(argc - 1, argv + 1);
    } else {
        return run_time_benchmark<PayloadT>(argc, argv);
    }
}

} // namespace pf

#endif // RUNNER_TEMPLATE_HPP
