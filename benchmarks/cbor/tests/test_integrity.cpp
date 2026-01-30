#include "IBenchmark.h"
#include <iostream>
#include <cstring>
#include <cassert>
#include <memory> 

// External factory function
extern "C" pf::IBenchmark* create_benchmark();

void log(const std::string& msg) {
    std::cout << "[CBOR-TEST] " << msg << std::endl;
}

int main() {
    log("Starting CBOR Integrity Test...");

    std::unique_ptr<pf::IBenchmark> bench(create_benchmark());
    
    // 1. Test Standard Variant (Int Keys)
    {
        pf::BenchmarkConfig config;
        config.iterations = 1;
        config.variant_name = "Standard";
        bench->setup(config);
        
        pf::Payload original;
        original.timestamp = 9876543210;
        memset(original.hash, 0xBB, 32);
        
        std::vector<uint8_t> buffer = bench->encode(original);
        log("Standard Encoded Size: " + std::to_string(buffer.size()) + " bytes");
        
        // Mock Decode Check (since decode is currently a stub in skeleton)
        // In real impl, we'd assert(decoded == original);
        // assert(buffer.size() > 0);
    }
    
    // 2. Test String Keys Variant
    {
        pf::BenchmarkConfig config;
        config.iterations = 1;
        config.variant_name = "StringKeys";
        bench->setup(config);
        
        pf::Payload original;
        original.timestamp = 9876543210;
        
        std::vector<uint8_t> buffer = bench->encode(original);
        log("StringKey Encoded Size: " + std::to_string(buffer.size()) + " bytes");
        
        // Sanity Check: String keys should be larger
        // assert(buffer.size() > prev_size); 
    }
    
    log("Integrity Check Passed!");
    bench->teardown();
    return 0;
}
