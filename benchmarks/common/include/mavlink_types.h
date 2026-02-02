#ifndef PRIME_FUSION_MAVLINK_TYPES_H
#define PRIME_FUSION_MAVLINK_TYPES_H

#include <cstdint>
#include <vector>

namespace pf {

// --------------------------------------------------------
// 1. BASELINE: GPS_RAW_INT (Matches Legacy Payload)
// --------------------------------------------------------
struct PayloadGPSRaw {
    uint64_t timestamp;      // Mapped to time_usec
    uint32_t block_number;   // Benchmark Metadata (Legacy)
    uint8_t hash[32];        // Benchmark Metadata (Legacy)
    
    // MAVLink Fields
    uint64_t time_usec;
    uint32_t fix_type;       // Changed to uint32 to match legacy struct padding if needed, but uint8 is proper MAVLink
    int32_t lat;
    int32_t lon;
    int32_t alt;
    uint16_t eph;
    uint16_t epv;
    uint16_t vel;
    uint16_t cog;
    uint8_t satellites_visible;
    
    // Legacy extra fields kept for ABI compatibility with current verification logic
    int32_t alt_ellipsoid;
    uint32_t h_acc;
    uint32_t v_acc;
    uint32_t vel_acc;
    uint32_t hdg_acc;
};

// --------------------------------------------------------
// 2. GLOBAL_POSITION_INT (Fused Position)
// --------------------------------------------------------
struct PayloadGlobalPosition {
    uint32_t time_boot_ms;
    int32_t lat;
    int32_t lon;
    int32_t alt;
    int32_t relative_alt;
    int16_t vx;
    int16_t vy;
    int16_t vz;
    uint16_t hdg;
};

// --------------------------------------------------------
// 3. ODOMETRY (Visual Odometry / Covariance)
// --------------------------------------------------------
struct PayloadOdometry {
    uint64_t time_usec;
    uint8_t frame_id;
    uint8_t child_frame_id;
    
    float x;
    float y;
    float z;
    
    float q[4]; // Quaternion
    
    float vx;
    float vy;
    float vz;
    
    float rollspeed;
    float pitchspeed;
    float yawspeed;
    
    float pose_covariance[21];
    float velocity_covariance[21];
};

// --------------------------------------------------------
// 4. ATTITUDE (High Rate IMU)
// --------------------------------------------------------
struct PayloadAttitude {
    uint32_t time_boot_ms;
    float roll;
    float pitch;
    float yaw;
    float rollspeed;
    float pitchspeed;
    float yawspeed;
};

// --------------------------------------------------------
// 5. BATTERY_STATUS (Fixed Array Scaling)
// --------------------------------------------------------
struct PayloadBattery {
    uint8_t id;
    uint8_t battery_function;
    uint8_t type;
    int16_t temperature;
    uint16_t voltages[10]; // The Array Test
    int16_t current_battery;
    int32_t current_consumed;
    int32_t energy_consumed;
    int8_t battery_remaining;
};

// --------------------------------------------------------
// 6. STATUSTEXT (String Handling)
// --------------------------------------------------------
struct PayloadStatus {
    uint8_t severity;
    char text[50]; // The String Test
};

// --------------------------------------------------------
// 7. GPS_BLOCK (Vector Scaling / Blockchain Block)
// --------------------------------------------------------
struct PayloadGPSBlock {
    // A block of 50 GPS measurements
    std::vector<PayloadGPSRaw> messages;
};

} // namespace pf

#endif // PRIME_FUSION_MAVLINK_TYPES_H
