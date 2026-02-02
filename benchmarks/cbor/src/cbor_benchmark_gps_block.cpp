#include "IBenchmark.h"
#include <cbor.h>
#include <iostream>
#include <vector>
#include <cstring>
#include "mavlink_types.h"

namespace pf {

class CborBenchmarkGPSBlock : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        PayloadGPSBlock p;
        for(int i=0; i<50; i++) {
            PayloadGPSRaw raw; 
            memset(&raw, 0, sizeof(raw));
            raw.timestamp = 1000 + i;
            p.messages.push_back(raw);
        }
        auto buf = encode(&p);
        PayloadGPSBlock d;
        decode(buf, &d);
        if (d.messages.size() == 50 && d.messages[0].timestamp == 1000) {
             std::cout << "[CBOR-GPSBlock] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[CBOR-GPSBlock] Sanity Check: FAILED " << d.messages.size() << std::endl;
             exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadGPSBlock& m = *static_cast<const PayloadGPSBlock*>(data);
        unsigned char buffer[8192];
        unsigned char* ptr = buffer;
        size_t buffer_size = sizeof(buffer);
        size_t n;

        // Array of N
        n = cbor_encode_array_start(m.messages.size(), ptr, buffer_size); ptr += n; buffer_size -= n;
        
        auto encode_key = [&](const char* key) {
             size_t len = strlen(key);
             size_t k = cbor_encode_string_start(len, ptr, buffer_size); ptr += k; buffer_size -= k;
             memcpy(ptr, key, len); ptr += len; buffer_size -= len;
        };

        for(const auto& r : m.messages) {
             n = cbor_encode_map_start(7, ptr, buffer_size); ptr += n; buffer_size -= n;
             
             encode_key("ts"); n = cbor_encode_uint64(r.timestamp, ptr, buffer_size); ptr += n; buffer_size -= n;
             encode_key("bn"); n = cbor_encode_uint32(r.block_number, ptr, buffer_size); ptr += n; buffer_size -= n;
             encode_key("tu"); n = cbor_encode_uint64(r.time_usec, ptr, buffer_size); ptr += n; buffer_size -= n;
             encode_key("ft"); n = cbor_encode_uint8(r.fix_type, ptr, buffer_size); ptr += n; buffer_size -= n;
             encode_key("lat"); 
             if(r.lat>=0) n=cbor_encode_uint64(r.lat, ptr, buffer_size); else n=cbor_encode_negint64(-1-r.lat, ptr, buffer_size); 
             ptr += n; buffer_size -= n;
             
             encode_key("lon");
             if(r.lon>=0) n=cbor_encode_uint64(r.lon, ptr, buffer_size); else n=cbor_encode_negint64(-1-r.lon, ptr, buffer_size);
             ptr += n; buffer_size -= n;

             encode_key("alt");
             if(r.alt>=0) n=cbor_encode_uint64(r.alt, ptr, buffer_size); else n=cbor_encode_negint64(-1-r.alt, ptr, buffer_size);
             ptr += n; buffer_size -= n;
        }

        return std::vector<uint8_t>(buffer, ptr);
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadGPSBlock& m = *static_cast<PayloadGPSBlock*>(out_data);
        struct cbor_load_result result;
        cbor_item_t* item = cbor_load(buffer.data(), buffer.size(), &result);
        if(!item) return;

        if(cbor_isa_array(item)) {
            size_t size = cbor_array_size(item);
            m.messages.resize(size);
            cbor_item_t** items = cbor_array_handle(item);
            for(size_t i=0; i<size; i++) {
                // items[i] is a Map representing one GPSRaw
                if(cbor_isa_map(items[i])) {
                    cbor_pair* pairs = cbor_map_handle(items[i]);
                    size_t map_sz = cbor_map_size(items[i]);
                    for(size_t j=0; j<map_sz; j++) {
                         std::string k((char*)cbor_string_handle(pairs[j].key), cbor_string_length(pairs[j].key));
                         cbor_item_t* v = pairs[j].value;
                         if(k=="ts") m.messages[i].timestamp = cbor_get_int(v);
                         else if(k=="bn") m.messages[i].block_number = cbor_get_int(v);
                         else if(k=="tu") m.messages[i].time_usec = cbor_get_int(v);
                         else if(k=="ft") m.messages[i].fix_type = cbor_get_int(v);
                         else if(k=="lat") {
                            if(cbor_isa_negint(v)) m.messages[i].lat = -1 - cbor_get_int(v);
                            else m.messages[i].lat = cbor_get_int(v);
                         }
                         else if(k=="lon") {
                            if(cbor_isa_negint(v)) m.messages[i].lon = -1 - cbor_get_int(v);
                            else m.messages[i].lon = cbor_get_int(v);
                         }
                         else if(k=="alt") {
                            if(cbor_isa_negint(v)) m.messages[i].alt = -1 - cbor_get_int(v);
                            else m.messages[i].alt = cbor_get_int(v);
                         }
                    }
                }
            }
        }
        cbor_decref(&item);
    }

    void teardown() override {}
    std::string name() const override { return "CBOR-GPSBlock"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::CborBenchmarkGPSBlock(); }
