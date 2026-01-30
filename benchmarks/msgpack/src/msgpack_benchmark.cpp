#include "IBenchmark.h"
#include <msgpack.hpp>
#include <iostream>
#include <vector>
#include <cstring>
#include <memory>
#include <sstream>

namespace pf {

class MsgPackBenchmark : public IBenchmark {
public:
    enum Variant { STANDARD, STRING_KEYS };
    Variant variant_ = STANDARD;

    void setup(const BenchmarkConfig& config) override {
        if (config.variant_name == "StringKeys") variant_ = STRING_KEYS;
        else variant_ = STANDARD;
        std::cout << "[MsgPack] Setup complete. Variant: " << config.variant_name << std::endl;

        // --- Integrity Verification ---
        Payload p;
        p.timestamp = 123456789;
        p.block_number = 999;
        auto buf = encode(p);
        Payload d;
        memset(&d, 0, sizeof(Payload));
        decode(buf, d);
        
        if (d.timestamp == 123456789 && d.block_number == 999) {
             std::cout << "[MsgPack] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[MsgPack] Sanity Check: FAILED! Expected 123456789, Got " << d.timestamp << std::endl;
             exit(1);
        }
    }

    std::vector<uint8_t> encode(const Payload& m) override {
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> packer(sbuf);

        // Map size = 18 fields
        packer.pack_map(18);

        if (variant_ == STRING_KEYS) {
            // String Keys
            packer.pack("timestamp"); packer.pack(m.timestamp);
            packer.pack("block_number"); packer.pack(m.block_number);
            packer.pack("hash"); packer.pack_bin(32); packer.pack_bin_body((const char*)m.hash, 32);
            packer.pack("time_usec"); packer.pack(m.time_usec);
            packer.pack("fix_type"); packer.pack(m.fix_type);
            packer.pack("lat"); packer.pack(m.lat);
            packer.pack("lon"); packer.pack(m.lon);
            packer.pack("alt"); packer.pack(m.alt);
            packer.pack("eph"); packer.pack(m.eph);
            packer.pack("epv"); packer.pack(m.epv);
            packer.pack("vel"); packer.pack(m.vel);
            packer.pack("cog"); packer.pack(m.cog);
            packer.pack("satellites_visible"); packer.pack(m.satellites_visible);
            packer.pack("alt_ellipsoid"); packer.pack(m.alt_ellipsoid);
            packer.pack("h_acc"); packer.pack(m.h_acc);
            packer.pack("v_acc"); packer.pack(m.v_acc);
            packer.pack("vel_acc"); packer.pack(m.vel_acc);
            packer.pack("hdg_acc"); packer.pack(m.hdg_acc);
        } else {
            // Integer Keys (0..17)
            packer.pack(0); packer.pack(m.timestamp);
            packer.pack(1); packer.pack(m.block_number);
            packer.pack(2); packer.pack_bin(32); packer.pack_bin_body((const char*)m.hash, 32);
            packer.pack(3); packer.pack(m.time_usec);
            packer.pack(4); packer.pack(m.fix_type);
            packer.pack(5); packer.pack(m.lat);
            packer.pack(6); packer.pack(m.lon);
            packer.pack(7); packer.pack(m.alt);
            packer.pack(8); packer.pack(m.eph);
            packer.pack(9); packer.pack(m.epv);
            packer.pack(10); packer.pack(m.vel);
            packer.pack(11); packer.pack(m.cog);
            packer.pack(12); packer.pack(m.satellites_visible);
            packer.pack(13); packer.pack(m.alt_ellipsoid);
            packer.pack(14); packer.pack(m.h_acc);
            packer.pack(15); packer.pack(m.v_acc);
            packer.pack(16); packer.pack(m.vel_acc);
            packer.pack(17); packer.pack(m.hdg_acc);
        }

        std::vector<uint8_t> result(sbuf.data(), sbuf.data() + sbuf.size());
        return result;
    }

    // --- SAX Visitor for MsgPack ---
    struct PayloadVisitor : msgpack::null_visitor {
        Payload* m;
        Variant variant;
        std::string key;
        
        PayloadVisitor(Payload* p, Variant v) : m(p), variant(v) {}

        bool visit_str(const char* v, uint32_t size) {
            key.assign(v, size);
            return true;
        }

        bool visit_bin(const char* v, uint32_t size) {
            if ((variant == STRING_KEYS && key == "hash") ||
                (variant == STANDARD && key.empty() /* Integer Key 2 logic needed? Visitor doesn't track parent */)) {
                // Wait. Visitor is purely sequential. We need a state machine to know "Value for Key X".
                // MsgPack visitor doesn't give us context easily unless we track it.
                // Assuming standard Map traversal: Key -> Value -> Key -> Value.
            }
            // For simplicity in this quick refactor, we stick to "Key" string tracking.
            // But for Integer Keys, we need to track the integer key.
            return true;
        }
        
        // This is getting complex to implement perfectly correct SAX for MsgPack in one shot.
        // Reason: MsgPack Visitor calls `visit_positive_integer` for both Keys (if int) and Values.
        // We need `bool expecting_value`.
        
        bool expecting_value = false;
        uint64_t current_int_key = 0;

        bool visit_positive_integer(uint64_t v) {
            if (!expecting_value) {
                // Context: It's a Key (for Integer Variant)
                current_int_key = v;
                expecting_value = true;
                return true;
            }
            
            // Context: It's a Value
            if (variant == STRING_KEYS) {
                if (key == "timestamp") m->timestamp = v;
                else if (key == "block_number") m->block_number = (uint32_t)v;
                // ... (abbreviated for safety, full list below)
            } else {
                 switch(current_int_key) {
                     case 0: m->timestamp = v; break;
                     // ...
                 }
            }
            expecting_value = false;
            return true;
        }
        
        // ... (We need safe fallback to DOM for MsgPack given complexity and time?)
        // actually, let's defer MsgPack SAX. It's notoriously hard to get right without a full state machine class.
        // The `object_handle` approach is the "standard" way people use msgpack-cxx.
        // Unlike RapidJSON/LibCBOR where SAX is common, MsgPack C++ users usually imply DOM.
        // Honest optimization: Keep DOM but pre-allocate? No.
        // I will revert to sticking with the CLEAN DOM implementation I just fixed in the previous step, 
        // OR try to just fix JSON.
        // User asked: "is there any honest optimization for all".
        // Use the DOM implementation but ensure it's tight.
        // Actually, I'll stick to DOM for MsgPack to avoid breakage, but state that SAX is possible but complex.
    };

    void decode(const std::vector<uint8_t>& buffer, Payload& m) override {
        // Reverting to the robust DOM implementation we just verified.
        // MsgPack DOM is reasonably fast (Unpack + Iterate).
        msgpack::object_handle oh = msgpack::unpack((const char*)buffer.data(), buffer.size());
        msgpack::object obj = oh.get();
        
        if (obj.type != msgpack::type::MAP) return;
        
        size_t map_size = obj.via.map.size;
        msgpack::object_kv* kv = obj.via.map.ptr;
        
        for(size_t i=0; i<map_size; i++) {
            msgpack::object& key = kv[i].key;
            msgpack::object& val = kv[i].val;
            
            if (variant_ == STRING_KEYS) {
                 if (key.type == msgpack::type::STR) {
                     std::string k = key.as<std::string>();
                     if (k == "timestamp") m.timestamp = val.as<uint64_t>();
                     else if (k == "block_number") m.block_number = val.as<uint32_t>();
                     else if (k == "hash" && val.type == msgpack::type::BIN) {
                         const char* data = val.via.bin.ptr;
                         if (val.via.bin.size == 32) memcpy(m.hash, data, 32);
                     }
                     else if (k == "time_usec") m.time_usec = val.as<uint64_t>();
                     else if (k == "fix_type") m.fix_type = val.as<uint8_t>();
                     else if (k == "lat") m.lat = val.as<int32_t>();
                     else if (k == "lon") m.lon = val.as<int32_t>();
                     else if (k == "alt") m.alt = val.as<int32_t>();
                     else if (k == "eph") m.eph = val.as<uint16_t>();
                     else if (k == "epv") m.epv = val.as<uint16_t>();
                     else if (k == "vel") m.vel = val.as<uint16_t>();
                     else if (k == "cog") m.cog = val.as<uint16_t>();
                     else if (k == "satellites_visible") m.satellites_visible = val.as<uint8_t>();
                     else if (k == "alt_ellipsoid") m.alt_ellipsoid = val.as<int32_t>();
                     else if (k == "h_acc") m.h_acc = val.as<uint32_t>();
                     else if (k == "v_acc") m.v_acc = val.as<uint32_t>();
                     else if (k == "vel_acc") m.vel_acc = val.as<uint32_t>();
                     else if (k == "hdg_acc") m.hdg_acc = val.as<uint32_t>();
                 }
            } else {
                 // Integer Keys
                 if (key.type == msgpack::type::POSITIVE_INTEGER) {
                     uint64_t k = key.as<uint64_t>();
                     switch(k) {
                         case 0: m.timestamp = val.as<uint64_t>(); break;
                         case 1: m.block_number = val.as<uint32_t>(); break;
                         case 2: if(val.type == msgpack::type::BIN) memcpy(m.hash, val.via.bin.ptr, 32); break;
                         case 3: m.time_usec = val.as<uint64_t>(); break;
                         case 4: m.fix_type = val.as<uint8_t>(); break;
                         case 5: m.lat = val.as<int32_t>(); break;
                         case 6: m.lon = val.as<int32_t>(); break;
                         case 7: m.alt = val.as<int32_t>(); break;
                         case 8: m.eph = val.as<uint16_t>(); break;
                         case 9: m.epv = val.as<uint16_t>(); break;
                         case 10: m.vel = val.as<uint16_t>(); break;
                         case 11: m.cog = val.as<uint16_t>(); break;
                         case 12: m.satellites_visible = val.as<uint8_t>(); break;
                         case 13: m.alt_ellipsoid = val.as<int32_t>(); break;
                         case 14: m.h_acc = val.as<uint32_t>(); break;
                         case 15: m.v_acc = val.as<uint32_t>(); break;
                         case 16: m.vel_acc = val.as<uint32_t>(); break;
                         case 17: m.hdg_acc = val.as<uint32_t>(); break;
                     }
                 }
            }
        }
    }

    void teardown() override {}

    std::string name() const override {
        return (variant_ == STRING_KEYS) ? "MsgPack-StringKeys" : "MsgPack-Standard";
    }
};

} // namespace pf

extern "C" pf::IBenchmark* create_benchmark() {
    return new pf::MsgPackBenchmark();
}
