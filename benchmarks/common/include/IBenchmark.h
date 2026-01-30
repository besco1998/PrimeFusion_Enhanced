#ifndef PRIME_FUSION_IBENCHMARK_H
#define PRIME_FUSION_IBENCHMARK_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>

namespace pf {

/**
 * @brief Configuration Payload for initializing a benchmark
 */
struct BenchmarkConfig {
    size_t iterations;
    bool warm_up;
    std::string variant_name;
};

/**
 * @brief Generic Payload Data Structure (Mirror of GPSBeacon)
 */
struct Payload {
    uint64_t timestamp;
    uint32_t block_number;
    uint8_t hash[32];
    uint64_t time_usec;
    uint8_t fix_type;
    int32_t lat;
    int32_t lon;
    int32_t alt;
    uint16_t eph;
    uint16_t epv;
    uint16_t vel;
    uint16_t cog;
    uint8_t satellites_visible;
    int32_t alt_ellipsoid;
    uint32_t h_acc;
    uint32_t v_acc;
    uint32_t vel_acc;
    uint32_t hdg_acc;
};

/**
 * @brief Immutable Benchmark Interface
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
     * @param data The canonical input struct
     * @return std::vector<uint8_t> The serialized output
     */
    virtual std::vector<uint8_t> encode(const Payload& data) = 0;

    /**
     * @brief Decode a buffer back into a struct.
     * @param buffer The serialized data
     * @param out_data Reference to target struct
     */
    virtual void decode(const std::vector<uint8_t>& buffer, Payload& out_data) = 0;

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
