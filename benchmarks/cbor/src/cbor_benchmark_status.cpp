#include "IBenchmark.h"
#include <cbor.h>
#include <iostream>
#include <vector>
#include <cstring>

namespace pf {

class CborBenchmarkStatus : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
         // Sanity
        PayloadStatus p;
        p.severity = 5;
        strncpy(p.text, "Hello World", 50);
        auto buf = encode(&p);
        PayloadStatus d;
        memset(&d, 0, sizeof(d));
        decode(buf, &d);
        if (d.severity == 5 && strcmp(d.text, "Hello World") == 0) {
            std::cout << "[CBOR-Status] Sanity Check: PASS" << std::endl;
        } else {
            std::cerr << "[CBOR-Status] Sanity Check: FAILED" << std::endl;
            exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadStatus& m = *static_cast<const PayloadStatus*>(data);
        unsigned char buffer[1024];
        unsigned char* ptr = buffer;
        size_t buffer_size = sizeof(buffer);
        size_t n;

        n = cbor_encode_map_start(2, ptr, buffer_size); ptr += n; buffer_size -= n;
        
        // Helper
        auto encode_str = [&](const char* key, const char* str) {
            n = cbor_encode_string_start(strlen(key), ptr, buffer_size); ptr += n; buffer_size -= n;
            memcpy(ptr, key, strlen(key)); ptr += strlen(key); buffer_size -= strlen(key);
            
            size_t vlen = strlen(str);
            n = cbor_encode_string_start(vlen, ptr, buffer_size); ptr += n; buffer_size -= n;
            memcpy(ptr, str, vlen); ptr += vlen; buffer_size -= vlen;
        };

        // Sev
        n = cbor_encode_string_start(3, ptr, buffer_size); ptr += n; buffer_size -= n;
        memcpy(ptr, "sev", 3); ptr += 3; buffer_size -= 3;
        n = cbor_encode_uint8(m.severity, ptr, buffer_size); ptr += n; buffer_size -= n;

        // Txt
        encode_str("txt", m.text);

        return std::vector<uint8_t>(buffer, ptr);
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadStatus& m = *static_cast<PayloadStatus*>(out_data);
        struct cbor_load_result result;
        cbor_item_t* item = cbor_load(buffer.data(), buffer.size(), &result);
        if(!item) return;

        if(cbor_isa_map(item)) {
            cbor_pair* pairs = cbor_map_handle(item);
            size_t sz = cbor_map_size(item);
            for(size_t i=0; i<sz; i++) {
                if(!cbor_isa_string(pairs[i].key)) continue;
                std::string k((char*)cbor_string_handle(pairs[i].key), cbor_string_length(pairs[i].key));
                if (k == "sev") m.severity = cbor_get_int(pairs[i].value);
                else if (k == "txt" && cbor_isa_string(pairs[i].value)) {
                    size_t vl = cbor_string_length(pairs[i].value);
                    size_t cp = vl < 49 ? vl : 49;
                    memcpy(m.text, cbor_string_handle(pairs[i].value), cp);
                    m.text[cp] = '\0';
                }
            }
        }
        cbor_decref(&item);
    }

    void teardown() override {}
    std::string name() const override { return "CBOR-Status"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::CborBenchmarkStatus(); }
