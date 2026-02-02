# MAVLink Payload Specifications (Phase 6)

**Source of Truth:** MAVLink Standard Definitions
**Architecture:** Multi-Type Benchmarking Matrix

This document defines the exact data structures used for the "Scientific Multi-Type" benchmarking phase.

---

## 1. Baseline: GPS_RAW_INT (ID 24)
*   **Status:** **Implemented** (matches existing `Payload` struct).
*   **Characteristics:** Mixed Scalar Integers.
*   **Fields:**

| Field | MAVLink Type | C++ Type | Note |
| :--- | :--- | :--- | :--- |
| `time_usec` | `uint64_t` | `uint64_t` | Timestamp |
| `fix_type` | `uint8_t` | `uint8_t` | |
| `lat` | `int32_t` | `int32_t` | deg * 1E7 |
| `lon` | `int32_t` | `int32_t` | deg * 1E7 |
| `alt` | `int32_t` | `int32_t` | mm |
| `eph` | `uint16_t` | `uint16_t` | cm |
| `epv` | `uint16_t` | `uint16_t` | cm |
| `vel` | `uint16_t` | `uint16_t` | cm/s |
| `cog` | `uint16_t` | `uint16_t` | cdeg |
| `satellites_visible` | `uint8_t` | `uint8_t` | |
| *Custom* | - | `uint32_t` | `block_number` (For verification) |
| *Custom* | - | `uint8_t[32]` | `hash` (For verification - dummy) |

---

## 2. New: GLOBAL_POSITION_INT (ID 33)
*   **Characteristics:** Fused Position (Mixed Int).
*   **Scientific Goal:** Test standard telemetry packet size.
*   **Fields:**

| Field | MAVLink Type | C++ Type |
| :--- | :--- | :--- |
| `time_boot_ms` | `uint32_t` | `uint32_t` |
| `lat` | `int32_t` | `int32_t` |
| `lon` | `int32_t` | `int32_t` |
| `alt` | `int32_t` | `int32_t` |
| `relative_alt` | `int32_t` | `int32_t` |
| `vx` | `int16_t` | `int16_t` |
| `vy` | `int16_t` | `int16_t` |
| `vz` | `int16_t` | `int16_t` |
| `hdg` | `uint16_t` | `uint16_t` |

---

## 3. New: ODOMETRY (ID 331)
*   **Characteristics:** Large Contiguous Floats.
*   **Scientific Goal:** Measure memory bandwidth (memcpy) vs serialization overhead.
*   **Fields:**

| Field | MAVLink Type | C++ Type | Size |
| :--- | :--- | :--- | :--- |
| `time_usec` | `uint64_t` | `uint64_t` | 8 |
| `frame_id` | `uint8_t` | `uint8_t` | 1 |
| `child_frame_id` | `uint8_t` | `uint8_t` | 1 |
| `x`, `y`, `z` | `float` | `float` | 12 |
| `q` | `float[4]` | `float[4]` | 16 |
| `vx`, `vy`, `vz` | `float` | `float` | 12 |
| `rollspeed`, `pitchspeed`, `yawspeed` | `float` | `float` | 12 |
| `pose_covariance` | `float[21]` | `float[21]` | 84 |
| `velocity_covariance` | `float[21]` | `float[21]` | 84 |
| **Total Payload** | | | **~230 bytes** |

---

## 4. New: ATTITUDE (ID 30)
*   **Characteristics:** Pure Floating Point.
*   **Scientific Goal:** Measure FPU serialization cost (casting/rounding).
*   **Fields:**

| Field | MAVLink Type | C++ Type |
| :--- | :--- | :--- |
| `time_boot_ms` | `uint32_t` | `uint32_t` |
| `roll` | `float` | `float` |
| `pitch` | `float` | `float` |
| `yaw` | `float` | `float` |
| `rollspeed` | `float` | `float` |
| `pitchspeed` | `float` | `float` |
| `yawspeed` | `float` | `float` |

---

## 5. New: BATTERY_STATUS (ID 147)
*   **Characteristics:** Fixed Array (`uint16`).
*   **Scientific Goal:** Test small array scaling (10 items).
*   **Fields:**

| Field | MAVLink Type | C++ Type |
| :--- | :--- | :--- |
| `id` | `uint8_t` | `uint8_t` |
| `temperature` | `int16_t` | `int16_t` |
| `voltages` | `uint16_t[10]` | `uint16_t[10]` |
| `current_battery` | `int16_t` | `int16_t` |
| `current_consumed` | `int32_t` | `int32_t` |
| `energy_consumed` | `int32_t` | `int32_t` |
| `battery_remaining` | `uint8_t` | `uint8_t` |

---

## 6. New: STATUSTEXT (ID 253)
*   **Characteristics:** String.
*   **Scientific Goal:** Test string handling (Null termination, copy overhead).
*   **Fields:**

| Field | MAVLink Type | C++ Type |
| :--- | :--- | :--- |
| `severity` | `uint8_t` | `uint8_t` |
| `text` | `char[50]` | `char[50]` |

---

## 7. New: GPS_BLOCK
*   **Characteristics:** `std::vector<GPS_RAW_INT>`.
*   **Scientific Goal:** Amortization of overhead (Size 50).
