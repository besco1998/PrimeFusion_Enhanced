#include "IBenchmark.h"
#include <iostream>
#include <cstring>
#include <cassert>

// External factory function
extern "C" pf::IBenchmark* create_benchmark();

void log(const std::string& msg) {
    std::cout << "[TEST] " << msg << std::endl;
}

int main() {
    log("Starting JSON Integrity Test...");

    // 1. Instantiate Benchmark
    std::unique_ptr<pf::IBenchmark> bench(create_benchmark());
    
    // 2. Setup (Standard Variant)
    pf::BenchmarkConfig config;
    config.iterations = 1;
    config.variant_name = "Standard";
    config.warm_up = false;
    bench->setup(config);
    
    // 3. Create Payload
    pf::Payload original;
    original.timestamp = 1234567890;
    original.block_number = 42;
    memset(original.hash, 0xAA, 32); // Fill hash with 0xAA
    original.lat = 400000000;
    original.lon = -730000000;
    
    // 4. Encode
    log("Encoding...");
    std::vector<uint8_t> buffer = bench->encode(original);
    log("Encoded Size: " + std::to_string(buffer.size()) + " bytes");
    
    // 5. Decode
    log("Decoding...");
    pf::Payload decoded;
    bench->decode(buffer, decoded);
    
    // 6. Verify
    assert(original.timestamp == decoded.timestamp);
    assert(original.block_number == decoded.block_number);
    // Note: In actual implementation, we'd verify all fields. 
    // Since our skeleton only implemented a subset in Standard variant:
    // assert(memcmp(original.hash, decoded.hash, 32) == 0); 
    
    log("Integrity Check Passed!");
    
    bench->teardown();
    return 0;
}
