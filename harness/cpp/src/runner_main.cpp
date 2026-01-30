#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <dlfcn.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <malloc.h> // For mallinfo2 (Linux specific, but that's the requirement)
#include <cstring>
#include "IBenchmark.h"

using namespace std::chrono;

// Type alias for factory function
typedef pf::IBenchmark* (*CreateBenchmarkFunc)();

struct Metrics {
    double total_time_ms;
    double avg_encode_time_us;
    double avg_decode_time_us;
    size_t peak_rss_kb;
    size_t serialized_size;
    size_t malloc_delta;
};

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
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <plugin_path> <variant_name> <iterations> <output_dir>" << std::endl;
        return 1;
    }

    std::string plugin_path = argv[1];
    std::string variant_name = argv[2];
    size_t iterations = std::stoull(argv[3]);
    std::string output_dir = argv[4];

    // 1. Load Plugin
    void* handle = dlopen(plugin_path.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "Cannot open library: " << dlerror() << std::endl;
        return 1;
    }

    // 2. Load Factory
    CreateBenchmarkFunc create = (CreateBenchmarkFunc) dlsym(handle, "create_benchmark");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        std::cerr << "Cannot load symbol 'create_benchmark': " << dlsym_error << std::endl;
        dlclose(handle);
        return 1;
    }

    // 3. Initialize Benchmark
    std::unique_ptr<pf::IBenchmark> bench(create());
    pf::BenchmarkConfig config;
    config.iterations = iterations;
    config.variant_name = variant_name;
    config.warm_up = true;
    
    bench->setup(config);
    std::cout << "[Runner] Loaded " << bench->name() << " from " << plugin_path << std::endl;

    // 4. Prepare Payload
    pf::Payload payload;
    payload.timestamp = 1600000000;
    payload.block_number = 12345;
    // ... fill other fields ... 
    
    // 5. Warmup
    for(int i=0; i<100; i++) {
        auto b = bench->encode(payload);
        pf::Payload d;
        bench->decode(b, d);
    }

    // 6. Measurement Loop
    size_t baseline_mem = get_allocated_mem();
    auto start_total = high_resolution_clock::now();
    
    std::vector<uint8_t> encoded_buffer;
    double total_encode_us = 0;
    double total_decode_us = 0;

    for(size_t i=0; i<iterations; i++) {
        // Measure Encode
        auto t1 = high_resolution_clock::now();
        encoded_buffer = bench->encode(payload);
        auto t2 = high_resolution_clock::now();
        total_encode_us += duration_cast<nanoseconds>(t2 - t1).count() / 1000.0;
        
        // Measure Decode
        pf::Payload decoded_out;
        auto t3 = high_resolution_clock::now();
        bench->decode(encoded_buffer, decoded_out);
        auto t4 = high_resolution_clock::now();
        total_decode_us += duration_cast<nanoseconds>(t4 - t3).count() / 1000.0;
    }
    
    auto end_total = high_resolution_clock::now();
    size_t final_mem = get_allocated_mem();

    // 7. Collect Metrics
    Metrics m;
    m.total_time_ms = duration_cast<milliseconds>(end_total - start_total).count();
    m.avg_encode_time_us = total_encode_us / iterations;
    m.avg_decode_time_us = total_decode_us / iterations;
    m.serialized_size = encoded_buffer.size();
    m.peak_rss_kb = get_peak_rss();
    m.malloc_delta = (final_mem > baseline_mem) ? (final_mem - baseline_mem) : 0; // Simple delta

    // 8. Output CSV
    // Filename: <output_dir>/<TIMESTAMP>_<NAME>_results.csv
    // But we append to a single file per run usually.
    // Let's write to <output_dir>/results.csv (Header handled by python or check existence)
    
    std::string csv_path = output_dir + "/raw_results.csv";
    bool file_exists = (access(csv_path.c_str(), F_OK) != -1);
    
    std::ofstream processed_file;
    processed_file.open(csv_path, std::ios::app);
    
    if (!file_exists) {
        processed_file << "Format,Variant,Iterations,TotalTime(ms),AvgEncode(us),AvgDecode(us),Size(bytes),PeakRSS(KB),MallocDelta(bytes)\n";
    }
    
    processed_file << bench->name() << "," 
                  << variant_name << "," 
                  << iterations << "," 
                  << m.total_time_ms << "," 
                  << m.avg_encode_time_us << ","
                  << m.avg_decode_time_us << "," 
                  << m.serialized_size << "," 
                  << m.peak_rss_kb << ","
                  << m.malloc_delta << "\n";
                  
    processed_file.close();

    std::cout << "[Runner] Finished. Results appended to " << csv_path << std::endl;

    bench->teardown();
    dlclose(handle);
    return 0;
}
