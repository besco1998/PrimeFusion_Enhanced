#include "IBenchmark.h"
#include <cbor.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <memory>

// RAII Wrapper for cbor_item_t
struct CborDeleter {
    void operator()(cbor_item_t* item) const {
        if(item) cbor_decref(&item);
    }
};
using UniqueCborItem = std::unique_ptr<cbor_item_t, CborDeleter>;

namespace pf {

class CborBenchmark : public IBenchmark {
public:
    enum Variant { STANDARD, STRING_KEYS };
    Variant variant_ = STANDARD;

    void setup(const BenchmarkConfig& config) override {
        if (config.variant_name == "StringKeys") variant_ = STRING_KEYS;
        else variant_ = STANDARD;
        std::cout << "[CBOR] Setup complete. Variant: " << config.variant_name << std::endl;

        // --- Integrity Verification ---
        Payload p;
        p.timestamp = 123456789;
        p.block_number = 999;
        // ... set a few sanity fields
        
        auto buf = encode(p);
        Payload d;
        // Zero out d to be sure
        memset(&d, 0, sizeof(Payload));
        
        decode(buf, d);
        
        if (d.timestamp == 123456789 && d.block_number == 999) {
             std::cout << "[CBOR] Sanity Check: PASS (Decode is working accurately)" << std::endl;
        } else {
             std::cerr << "[CBOR] Sanity Check: FAILED! Decode is broken/skipping." << std::endl;
             std::cerr << "   Expected: 123456789, Got: " << d.timestamp << std::endl;
             // Force exit or throw to alert user immediately
             exit(1); 
        }
    }

    std::vector<uint8_t> encode(const Payload& m) override {
        // High-Performance Streaming Implementation (Stack-based, No Malloc)
        unsigned char buffer[2048]; // Sufficient for 18 fields
        unsigned char* ptr = buffer;
        size_t buffer_size = sizeof(buffer);
        size_t n;

        // Encode Map Header (18 items)
        n = cbor_encode_map_start(18, ptr, buffer_size);
        ptr += n; buffer_size -= n;

        // Helper: Encode Text String (optimized with explicit length)
        auto encode_string_len = [&](const char* str, size_t len) {
            size_t n = cbor_encode_string_start(len, ptr, buffer_size);
            ptr += n; buffer_size -= n;
            memcpy(ptr, str, len);
            ptr += len; buffer_size -= len;
        };
        
        // Helper: Encode Byte String
        auto encode_bytes = [&](const uint8_t* data, size_t len) {
            size_t n = cbor_encode_bytestring_start(len, ptr, buffer_size);
            ptr += n; buffer_size -= n;
            memcpy(ptr, data, len);
            ptr += len; buffer_size -= len;
        };

        // Helper: Encode Adaptive Integer (Size Optimization)
        auto encode_uint = [&](uint64_t val) {
            if (val <= 23) {
                // Inline tiny int (0-23)
                size_t n = cbor_encode_uint8((uint8_t)val, ptr, buffer_size); 
                ptr += n; buffer_size -= n;
            } else if (val <= 0xFF) {
                size_t n = cbor_encode_uint8((uint8_t)val, ptr, buffer_size);
                ptr += n; buffer_size -= n;
            } else if (val <= 0xFFFF) {
                size_t n = cbor_encode_uint16((uint16_t)val, ptr, buffer_size);
                ptr += n; buffer_size -= n;
            } else if (val <= 0xFFFFFFFF) {
                size_t n = cbor_encode_uint32((uint32_t)val, ptr, buffer_size);
                ptr += n; buffer_size -= n;
            } else {
                size_t n = cbor_encode_uint64(val, ptr, buffer_size);
                ptr += n; buffer_size -= n;
            }
        };

        // Helper Lambda for K/V pair
        // Note: We use cbor_encode_uint8 for keys to ensure 1-byte encoding (manual optimization)
        auto encode_pair = [&](uint8_t key, uint64_t val) {
            size_t k_len = cbor_encode_uint8(key, ptr, buffer_size);
            ptr += k_len; buffer_size -= k_len;
            
            encode_uint(val); // Adaptive value
        };

        auto encode_str_pair = [&](const char* key, size_t key_len, uint64_t val) {
             encode_string_len(key, key_len);
             encode_uint(val);
        };

        if (variant_ == STRING_KEYS) {
            // Manual length passing to avoid strlen overhead
            encode_str_pair("timestamp", 9, m.timestamp);
            encode_str_pair("block_number", 12, m.block_number);
            
            // Hash ByteString
            encode_string_len("hash", 4);
            encode_bytes(m.hash, 32);

            encode_str_pair("time_usec", 9, m.time_usec);
            encode_str_pair("fix_type", 8, m.fix_type);
            encode_str_pair("lat", 3, (uint64_t)m.lat);
            encode_str_pair("lon", 3, (uint64_t)m.lon);
            encode_str_pair("alt", 3, (uint64_t)m.alt);
            encode_str_pair("eph", 3, m.eph);
            encode_str_pair("epv", 3, m.epv);
            encode_str_pair("vel", 3, m.vel);
            encode_str_pair("cog", 3, m.cog);
            encode_str_pair("satellites_visible", 18, m.satellites_visible);
            encode_str_pair("alt_ellipsoid", 13, (uint64_t)m.alt_ellipsoid);
            encode_str_pair("h_acc", 5, m.h_acc);
            encode_str_pair("v_acc", 5, m.v_acc);
            encode_str_pair("vel_acc", 7, m.vel_acc);
            encode_str_pair("hdg_acc", 7, m.hdg_acc);
            
        } else {
            // Integer Keys
            encode_pair(0, m.timestamp);
            encode_pair(1, m.block_number);
            
            // Hash (Key 2)
            n = cbor_encode_uint8(2, ptr, buffer_size); ptr += n; buffer_size -= n;
            encode_bytes(m.hash, 32);
            
            encode_pair(3, m.time_usec);
            encode_pair(4, m.fix_type);
            encode_pair(5, (uint64_t)m.lat);
            encode_pair(6, (uint64_t)m.lon);
            encode_pair(7, (uint64_t)m.alt);
            encode_pair(8, m.eph);
            encode_pair(9, m.epv);
            encode_pair(10, m.vel);
            encode_pair(11, m.cog);
            encode_pair(12, m.satellites_visible);
            encode_pair(13, (uint64_t)m.alt_ellipsoid);
            encode_pair(14, m.h_acc);
            encode_pair(15, m.v_acc);
            encode_pair(16, m.vel_acc);
            encode_pair(17, m.hdg_acc);
        }

        return std::vector<uint8_t>(buffer, ptr);
    }

    // --- Streaming Decoder Context & Callbacks ---
    struct DecodeContext {
        Payload* m;
        Variant variant;
        std::string current_key_str;
        int64_t current_key_int = -1;
        bool waiting_for_value = false; // Next item is a value
    };

    static void on_string(void* ctx, cbor_data data, size_t len) {
        DecodeContext* c = (DecodeContext*)ctx;
        if (!c->waiting_for_value) {
            c->current_key_str.assign((const char*)data, len);
            c->waiting_for_value = true;
        } else {
            // No string values in Payload, ignore or handle if needed
        }
    }

    static void on_byte_string(void* ctx, cbor_data data, size_t len) {
        DecodeContext* c = (DecodeContext*)ctx;
        if (c->waiting_for_value) {
             // Only 'hash' or int key 2 is bytes
             if ((c->variant == STRING_KEYS && c->current_key_str == "hash") || 
                 (c->variant == STANDARD && c->current_key_int == 2)) {
                 if (len == 32) memcpy(c->m->hash, data, 32);
             }
             c->waiting_for_value = false;
        }
    }

    static void handle_int_value(DecodeContext* c, uint64_t val) {
        if (!c->waiting_for_value) {
            // It's a key (Integer Variant)
            c->current_key_int = val;
            c->waiting_for_value = true;
            return;
        }

        // It's a value, assign based on key
        if (c->variant == STRING_KEYS) {
            const std::string& k = c->current_key_str;
            if (k == "timestamp") c->m->timestamp = val;
            else if (k == "block_number") c->m->block_number = val;
            else if (k == "time_usec") c->m->time_usec = val;
            else if (k == "fix_type") c->m->fix_type = val;
            else if (k == "lat") c->m->lat = (int32_t)val;
            else if (k == "lon") c->m->lon = (int32_t)val;
            else if (k == "alt") c->m->alt = (int32_t)val;
            else if (k == "eph") c->m->eph = val;
            else if (k == "epv") c->m->epv = val;
            else if (k == "vel") c->m->vel = val;
            else if (k == "cog") c->m->cog = val;
            else if (k == "satellites_visible") c->m->satellites_visible = (uint8_t)val;
            else if (k == "alt_ellipsoid") c->m->alt_ellipsoid = (int32_t)val;
            else if (k == "h_acc") c->m->h_acc = (uint32_t)val;
            else if (k == "v_acc") c->m->v_acc = (uint32_t)val;
            else if (k == "vel_acc") c->m->vel_acc = (uint32_t)val;
            else if (k == "hdg_acc") c->m->hdg_acc = (uint32_t)val;
        } else {
            // Integer Keys
            switch(c->current_key_int) {
                case 0: c->m->timestamp = val; break;
                case 1: c->m->block_number = (uint32_t)val; break;
                // case 2 is hash (bytes), handled in on_byte_string
                case 3: c->m->time_usec = val; break;
                case 4: c->m->fix_type = (uint8_t)val; break;
                case 5: c->m->lat = (int32_t)val; break;
                case 6: c->m->lon = (int32_t)val; break;
                case 7: c->m->alt = (int32_t)val; break;
                case 8: c->m->eph = (uint16_t)val; break;
                case 9: c->m->epv = (uint16_t)val; break;
                case 10: c->m->vel = (uint16_t)val; break;
                case 11: c->m->cog = (uint16_t)val; break;
                case 12: c->m->satellites_visible = (uint8_t)val; break;
                case 13: c->m->alt_ellipsoid = (int32_t)val; break;
                case 14: c->m->h_acc = (uint32_t)val; break;
                case 15: c->m->v_acc = (uint32_t)val; break;
                case 16: c->m->vel_acc = (uint32_t)val; break;
                case 17: c->m->hdg_acc = (uint32_t)val; break;
            }
        }
        c->waiting_for_value = false;
    }

    static void on_uint8(void* ctx, uint8_t val) { handle_int_value((DecodeContext*)ctx, val); }
    static void on_uint16(void* ctx, uint16_t val) { handle_int_value((DecodeContext*)ctx, val); }
    static void on_uint32(void* ctx, uint32_t val) { handle_int_value((DecodeContext*)ctx, val); }
    static void on_uint64(void* ctx, uint64_t val) { handle_int_value((DecodeContext*)ctx, val); }

    void decode(const std::vector<uint8_t>& buffer, Payload& m) override {
        struct cbor_callbacks callbacks = cbor_empty_callbacks;
        callbacks.uint8 = on_uint8;
        callbacks.uint16 = on_uint16;
        callbacks.uint32 = on_uint32;
        callbacks.uint64 = on_uint64;
        callbacks.string = on_string;
        callbacks.byte_string = on_byte_string;
        // Map start/end ignored, we just process flow
        
        DecodeContext ctx;
        ctx.m = &m;
        ctx.variant = variant_;
        
        size_t offset = 0;
        while(offset < buffer.size()) {
            cbor_decoder_result res = cbor_stream_decode(buffer.data() + offset, buffer.size() - offset, &callbacks, &ctx);
            if (res.read == 0) break;
            offset += res.read;
            
            if (res.status != CBOR_DECODER_FINISHED) {
                 break;
            }
        }
    }

    void teardown() override {}

    std::string name() const override {
        return (variant_ == STRING_KEYS) ? "CBOR-StringKeys" : "CBOR-Standard";
    }
};

} // namespace pf

extern "C" pf::IBenchmark* create_benchmark() {
    return new pf::CborBenchmark();
}
