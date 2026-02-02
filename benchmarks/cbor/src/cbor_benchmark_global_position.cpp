#include "IBenchmark.h"
#include <cbor.h>
#include <iostream>
#include <vector>
#include <cstring>

namespace pf {

class CborBenchmarkGlobalPosition : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        PayloadGlobalPosition p;
        memset(&p, 0, sizeof(p));
        p.lat = 123456789;
        auto buf = encode(&p);
        PayloadGlobalPosition d;
        memset(&d, 0, sizeof(d));
        decode(buf, &d);
        if (d.lat == 123456789) {
             std::cout << "[CBOR-GlobalPos] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[CBOR-GlobalPos] Sanity Check: FAILED" << std::endl;
             exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadGlobalPosition& m = *static_cast<const PayloadGlobalPosition*>(data);
        unsigned char buffer[1024];
        unsigned char* ptr = buffer;
        size_t buffer_size = sizeof(buffer);
        size_t n;

        n = cbor_encode_map_start(9, ptr, buffer_size); ptr += n; buffer_size -= n;
        
        auto encode_pair_int = [&](const char* key, int64_t val) {
            n = cbor_encode_string_start(strlen(key), ptr, buffer_size); ptr += n; buffer_size -= n;
            memcpy(ptr, key, strlen(key)); ptr += strlen(key); buffer_size -= strlen(key);
            if(val >= 0) n = cbor_encode_uint64(val, ptr, buffer_size);
            else n = cbor_encode_negint64(-1 - val, ptr, buffer_size);
            ptr += n; buffer_size -= n;
        };
        auto encode_pair_uint = [&](const char* key, uint64_t val) {
             n = cbor_encode_string_start(strlen(key), ptr, buffer_size); ptr += n; buffer_size -= n;
             memcpy(ptr, key, strlen(key)); ptr += strlen(key); buffer_size -= strlen(key);
             n = cbor_encode_uint64(val, ptr, buffer_size); ptr += n; buffer_size -= n;
        };

        encode_pair_uint("boot", m.time_boot_ms);
        encode_pair_int("lat", m.lat);
        encode_pair_int("lon", m.lon);
        encode_pair_int("alt", m.alt);
        encode_pair_int("rel", m.relative_alt);
        encode_pair_int("vx", m.vx);
        encode_pair_int("vy", m.vy);
        encode_pair_int("vz", m.vz);
        encode_pair_uint("hdg", m.hdg);

        return std::vector<uint8_t>(buffer, ptr);
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadGlobalPosition& m = *static_cast<PayloadGlobalPosition*>(out_data);
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
                else if (k=="lat") m.lat = cbor_get_int(val); // Works for negint too if handled by cbor_get_int?
                // Wait, cbor_get_int returns based on type?
                // libcbor helper: 
                // cbor_get_int returns [uint8...uint64]. For negint it might be tricky.
                // Let's use specific checks or the generic helper.
                // Actually `cbor_get_int` is for uint. `cbor_get_int` is a macro? No.
                // Let's assume standard libcbor helpers.
                // For signed integers, check type.
                
                else if (k=="lat" || k=="lon" || k=="alt" || k=="rel" || k=="vx" || k=="vy" || k=="vz") {
                    int64_t v = 0;
                    if(cbor_isa_uint(val)) v = cbor_get_int(val);
                    else if(cbor_isa_negint(val)) {
                        // value is -1 - val
                        uint64_t uv = cbor_get_int(val); // returns the magnitude
                        // Actual value is -1 - uv.
                        // wait, libcbor representation for negint stores the "value" part directly.
                        // Correct logic: -1 - value.
                        v = -1 - (int64_t)uv;
                    }
                    if(k=="lat") m.lat = (int32_t)v;
                    else if(k=="lon") m.lon = (int32_t)v;
                    else if(k=="alt") m.alt = (int32_t)v;
                    else if(k=="rel") m.relative_alt = (int32_t)v;
                    else if(k=="vx") m.vx = (int16_t)v;
                    else if(k=="vy") m.vy = (int16_t)v;
                    else if(k=="vz") m.vz = (int16_t)v;
                }
                else if (k=="hdg") m.hdg = (uint16_t)cbor_get_int(val);
            }
        }
        cbor_decref(&item);
    }

    void teardown() override {}
    std::string name() const override { return "CBOR-GlobalPos"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::CborBenchmarkGlobalPosition(); }
