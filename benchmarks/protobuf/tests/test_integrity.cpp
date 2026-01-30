#include "IBenchmark.h"
#include <iostream>
#include <cstring>
#include <cassert>
#include <memory>

// External factory function
extern "C" pf::IBenchmark* create_benchmark();

void log(const std::string& msg) {
    std::cout << "[PROTO-TEST] " << msg << std::endl;
}

int main() {
    log("Starting Protobuf Integrity Test...");

    std::unique_ptr<pf::IBenchmark> bench(create_benchmark());
    
    // 1. Test Standard Variant
    {
        pf::BenchmarkConfig config;
        config.iterations = 1;
        config.variant_name = "Standard";
        bench->setup(config);
        
        pf::Payload original;
        original.timestamp = 5555555;
        memset(original.hash, 0xEE, 32);
        
        std::vector<uint8_t> buffer = bench->encode(original);
        log("Encoded Size: " + std::to_string(buffer.size()) + " bytes");
        
        // Mock Decode Check
        pf::Payload decoded;
        bench->decode(buffer, decoded);
        
        // Since Protobuf decode is fully implemented in our skeleton, we CAN verify!
        assert(decoded.timestamp == original.timestamp);
        assert(memcmp(decoded.hash, original.hash, 32) == 0);
    }
    
    log("Integrity Check Passed!");
    bench->teardown();
    return 0;
}
