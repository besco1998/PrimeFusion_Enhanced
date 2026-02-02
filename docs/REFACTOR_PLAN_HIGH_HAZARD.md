# High-Hazard Refactoring Plan: "Scientific Multi-Type Matrix" (Corrected)

**Objective:** Benchmark 6 Distinct MAVLink Message Types + 1 Vector Block.

## 1. Selected MAVLink Messages

| # | Type | Name | Characteristics | Scientific Goal |
| :--- | :--- | :--- | :--- | :--- |
| **1** | **Baseline** | `GPS_RAW_INT` (#24) | Integers (`int32`, `uint16`) | **Baseline.** Matches current project state. |
| **2** | **New** | `GLOBAL_POSITION_INT` (#33) | Integers (`int32`, `int16`) | **Fused Pos.** Requested 6th message. |
| **3** | **New** | `ODOMETRY` (#331) | Large Arrays (`float[21]`) | **Memory Bandwidth.** Covariance matrices test memory copies. |
| **4** | **New** | `ATTITUDE` (#30) | Pure Floats (`float`) | **FPU Handling.** Tests Float serialization/rounding. |
| **5** | **New** | `BATTERY_STATUS` (#147) | Fixed Array (`uint16[10]`) | **Array Scaling.** Tests small fixed-size array iteration. |
| **6** | **New** | `STATUSTEXT` (#253) | C-String (`char text[50]`) | **String Copying.** Tests null-termination handling. |
| **7** | **Block** | `GPS_BLOCK` | Vector (`std::vector<GPS>`) | **Amortization.** Tests scaling 1 vs 50 messages. |

## 2. Data Structures (`common/include/mavlink_types.h`)

```cpp
// 1. GPS_RAW_INT (Baseline)
struct PayloadGPSRaw {
    uint64_t time_usec;
    int32_t lat, lon, alt;
    uint16_t eph, epv, vel, cog;
    uint8_t fix_type, satellites_visible;
    // ... plus our custom fields (block_num, hash) ...
};

// 2. GLOBAL_POSITION_INT
struct PayloadGlobalPosition {
    uint32_t time_boot_ms;
    int32_t lat, lon, alt, relative_alt;
    int16_t vx, vy, vz;
    uint16_t hdg;
};

// 3. ODOMETRY
struct PayloadOdometry {
    uint64_t time_usec;
    float x, y, z;
    float q[4];
    float vx, vy, vz;
    float rollspeed, pitchspeed, yawspeed;
    float pose_covariance[21];
    float velocity_covariance[21];
};

// 4. ATTITUDE
struct PayloadAttitude {
    uint32_t time_boot_ms;
    float roll, pitch, yaw;
    float rollspeed, pitchspeed, yawspeed;
};

// 5. BATTERY_STATUS
struct PayloadBattery {
    uint16_t voltages[10];
    int16_t current_battery;
    int32_t current_consumed, energy_consumed;
    int16_t temperature;
    uint8_t battery_remaining;
};

// 6. STATUSTEXT
struct PayloadStatus {
    uint8_t severity;
    char text[50];
};

// 7. GPS_BLOCK
struct PayloadGPSBlock {
    std::vector<PayloadGPSRaw> msgs; // Size 50
};
```

## 3. Architecture: "The Matrix"

We will build **28 Independent Executables** (4 Formats x 7 Types) to ensure "No Errors" via total isolation.

*   `bin/pf_runner_gps_raw` -> `libpf_json_gps_raw.so`
*   `bin/pf_runner_odometry` -> `libpf_cbor_odometry.so`
*   ...etc.

## 4. Execution Order (Atomic Steps)
1.  **Define Types:** Create `mavlink_types.h`.
2.  **Update Protocol:** Edit `gps_beacon.proto` to add 6 new messages (preserve existing as `GPSRaw`).
3.  **Refactor Core:** Make `IBenchmark.h` use `void*`.
4.  **Create Runners:** Generate the 7 C++ runner templates.
5.  **Refactor Plugins:** Update logic for 7 variants.
6.  **Verify:** Run `runner.py`.
