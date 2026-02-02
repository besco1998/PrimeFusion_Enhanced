#ifndef PRIME_FUSION_IBENCHMARK_H
#define PRIME_FUSION_IBENCHMARK_H

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include "mavlink_types.h" // Include the new types

namespace pf {

/**
 * @brief Configuration Payload for initializing a benchmark
 */
struct BenchmarkConfig {
    size_t iterations;
    bool warm_up;
    std::string variant_name;
};

// Legacy Payload struct removed (Now in mavlink_types.h as PayloadGPSRaw)
// We typedef it here for strict backward compatibility during transition if needed,
// but we will use void* in the interface.
using Payload = PayloadGPSRaw; 

/**
 * @brief Immutable Benchmark Interface (Type-Erased)
 * All formats must implement this.
 */
class IBenchmark {
public:
    virtual ~IBenchmark() = default;

    /**
     * @brief Initialize the library/encoders. 
     * Called once before the loop.
     */
    virtual void setup(const BenchmarkConfig& config) = 0;

    /**
     * @brief Encode a payload into a binary/text buffer.
     * @param data Pointer to the struct (casted to void*)
     * @return std::vector<uint8_t> The serialized output
     */
    virtual std::vector<uint8_t> encode(const void* data) = 0;

    /**
     * @brief Decode a buffer back into a struct.
     * @param buffer The serialized data
     * @param out_data Pointer to target struct (casted to void*)
     */
    virtual void decode(const std::vector<uint8_t>& buffer, void* out_data) = 0;

    /**
     * @brief Clean up resources.
     */
    virtual void teardown() = 0;

    /**
     * @brief Get the official name of this format variant.
     */
    virtual std::string name() const = 0;
};

} // namespace pf

#endif // PRIME_FUSION_IBENCHMARK_H
