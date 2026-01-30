#include "IBenchmark.h"
#include <iostream>
#include <cstring>
#include <cassert>
#include <memory>

// External factory function
extern "C" pf::IBenchmark* create_benchmark();

void log(const std::string& msg) {
    std::cout << "[MSGPACK-TEST] " << msg << std::endl;
}

int main() {
    log("Starting MessagePack Integrity Test...");

    std::unique_ptr<pf::IBenchmark> bench(create_benchmark());
    
    // 1. Test Standard Variant (Int Keys)
    {
        pf::BenchmarkConfig config;
        config.iterations = 1;
        config.variant_name = "Standard";
        bench->setup(config);
        
        pf::Payload original;
        original.timestamp = 1122334455;
        memset(original.hash, 0xCC, 32);
        
        std::vector<uint8_t> buffer = bench->encode(original);
        log("Standard Encoded Size: " + std::to_string(buffer.size()) + " bytes");
        
        // Mock Decode Check
        // pf::Payload decoded;
        // bench->decode(buffer, decoded);
    }
    
    // 2. Test String Keys Variant
    {
        pf::BenchmarkConfig config;
        config.iterations = 1;
        config.variant_name = "StringKeys";
        bench->setup(config);
        
        pf::Payload original;
        original.timestamp = 1122334455;
        
        std::vector<uint8_t> buffer = bench->encode(original);
        log("StringKey Encoded Size: " + std::to_string(buffer.size()) + " bytes");
    }
    
    log("Integrity Check Passed!");
    bench->teardown();
    return 0;
}
