#include "IBenchmark.h"
#include <cbor.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <cmath>

namespace pf {

class CborBenchmarkAttitude : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        PayloadAttitude p;
        memset(&p, 0, sizeof(p));
        p.roll = 1.0f;
        auto buf = encode(&p);
        PayloadAttitude d; 
        memset(&d, 0, sizeof(d));
        decode(buf, &d);
        if (std::abs(d.roll - 1.0f) < 0.001f) {
             std::cout << "[CBOR-Attitude] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[CBOR-Attitude] Sanity Check: FAILED" << std::endl;
             exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadAttitude& m = *static_cast<const PayloadAttitude*>(data);
        unsigned char buffer[1024];
        unsigned char* ptr = buffer;
        size_t buffer_size = sizeof(buffer);
        size_t n;

        n = cbor_encode_map_start(7, ptr, buffer_size); ptr += n; buffer_size -= n;
        
        // Helper
        auto encode_pair_uint = [&](const char* key, unsigned val) {
            n = cbor_encode_string_start(strlen(key), ptr, buffer_size); ptr += n; buffer_size -= n;
            memcpy(ptr, key, strlen(key)); ptr += strlen(key); buffer_size -= strlen(key);
            n = cbor_encode_uint32(val, ptr, buffer_size); ptr += n; buffer_size -= n;
        };
        auto encode_pair_float = [&](const char* key, float val) {
            n = cbor_encode_string_start(strlen(key), ptr, buffer_size); ptr += n; buffer_size -= n;
            memcpy(ptr, key, strlen(key)); ptr += strlen(key); buffer_size -= strlen(key);
            n = cbor_encode_single(val, ptr, buffer_size); ptr += n; buffer_size -= n;
        };

        encode_pair_uint("boot", m.time_boot_ms);
        encode_pair_float("r", m.roll);
        encode_pair_float("p", m.pitch);
        encode_pair_float("y", m.yaw);
        encode_pair_float("rs", m.rollspeed);
        encode_pair_float("ps", m.pitchspeed);
        encode_pair_float("ys", m.yawspeed);

        return std::vector<uint8_t>(buffer, ptr);
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadAttitude& m = *static_cast<PayloadAttitude*>(out_data);
        // Correct DOM usage
        struct cbor_load_result result;
        cbor_item_t* item = cbor_load(buffer.data(), buffer.size(), &result);
        if(!item) return;

        if(cbor_isa_map(item)) {
            cbor_pair* pairs = cbor_map_handle(item);
            size_t sz = cbor_map_size(item);
            for(size_t i=0; i<sz; i++) {
                if(!cbor_isa_string(pairs[i].key)) continue;
                std::string k((char*)cbor_string_handle(pairs[i].key), cbor_string_length(pairs[i].key));
                
                cbor_item_t* val = pairs[i].value;
                if (k=="boot") m.time_boot_ms = cbor_get_int(val);
                else {
                    float f = 0;
                    if (cbor_isa_float_ctrl(val)) f = cbor_float_get_float(val);
                    if (k=="r") m.roll=f;
                    else if (k=="p") m.pitch=f;
                    else if (k=="y") m.yaw=f;
                    else if (k=="rs") m.rollspeed=f;
                    else if (k=="ps") m.pitchspeed=f;
                    else if (k=="ys") m.yawspeed=f;
                }
            }
        }
        cbor_decref(&item);
    }

    void teardown() override {}
    std::string name() const override { return "CBOR-Attitude"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::CborBenchmarkAttitude(); }
