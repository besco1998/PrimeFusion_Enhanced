#include "IBenchmark.h"
#include <cbor.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <cmath>

namespace pf {

class CborBenchmarkBattery : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        // Sanity Check
        PayloadBattery p;
        memset(&p, 0, sizeof(p));
        p.id = 1;
        p.voltages[0] = 4200;
        auto buf = encode(&p);
        PayloadBattery d;
        memset(&d, 0, sizeof(d));
        decode(buf, &d);
        if (d.id == 1 && d.voltages[0] == 4200) {
             // std::cout << "[CBOR-Battery] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[CBOR-Battery] Sanity Check: FAILED" << std::endl;
             exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadBattery& m = *static_cast<const PayloadBattery*>(data);
        unsigned char buffer[1024]; 
        unsigned char* ptr = buffer;
        size_t buffer_size = sizeof(buffer);
        size_t n;

        // Map(9 items)
        n = cbor_encode_map_start(9, ptr, buffer_size); ptr += n; buffer_size -= n;

        auto cbor_encode_text_string = [&](uint8_t* p, size_t sz, const char* str, size_t len) {
             size_t k = cbor_encode_string_start(len, p, sz);
             memcpy(p + k, str, len);
             return k + len;
        };

        auto encode_pair_uint = [&](const char* key, unsigned val) {
            n = cbor_encode_text_string(ptr, buffer_size, key, strlen(key)); ptr += n; buffer_size -= n;
            n = cbor_encode_uint32(val, ptr, buffer_size); ptr += n; buffer_size -= n;
        };
        
        auto encode_pair_int = [&](const char* key, int val) {
            n = cbor_encode_text_string(ptr, buffer_size, key, strlen(key)); ptr += n; buffer_size -= n;
            if(val >= 0) n = cbor_encode_uint32(val, ptr, buffer_size);
            else n = cbor_encode_negint(-1 - val, ptr, buffer_size); 
            ptr += n; buffer_size -= n;
        };
        
        encode_pair_uint("id", m.id);
        encode_pair_uint("func", m.battery_function);
        encode_pair_uint("type", m.type);
        encode_pair_int("temp", m.temperature);
        
        // Voltages Array
        n = cbor_encode_text_string(ptr, buffer_size, "volt", 4); ptr += n; buffer_size -= n;
        n = cbor_encode_array_start(10, ptr, buffer_size); ptr += n; buffer_size -= n;
        for(int i=0; i<10; i++) {
             n = cbor_encode_uint16(m.voltages[i], ptr, buffer_size); ptr += n; buffer_size -= n;
        }
        
        encode_pair_int("current", m.current_battery);
        encode_pair_int("cons", m.current_consumed);
        encode_pair_int("energy", m.energy_consumed);
        encode_pair_int("rem", m.battery_remaining);

        return std::vector<uint8_t>(buffer, ptr);
    }

    // Streaming Decoder
    struct DecodeContext {
        PayloadBattery* m;
        std::string current_key;
        bool waiting_for_value = false;
        bool in_voltages = false;
        int idx = 0;
    };

    static void on_string(void* ctx, cbor_data data, size_t len) {
        DecodeContext* c = (DecodeContext*)ctx;
        if (!c->waiting_for_value) {
            c->current_key.assign((const char*)data, len);
            c->waiting_for_value = true;
            if(c->current_key == "volt") { c->in_voltages = true; c->idx = 0; }
            else c->in_voltages = false;
        }
    }

    static void handle_int(DecodeContext* c, int64_t val) {
         if (c->in_voltages) {
             if(c->idx < 10) c->m->voltages[c->idx++] = (uint16_t)val;
             if(c->idx == 10) { c->in_voltages = false; c->waiting_for_value = false; }
         } else if (c->waiting_for_value) {
             if (c->current_key == "id") c->m->id = (uint8_t)val;
             else if (c->current_key == "func") c->m->battery_function = (uint8_t)val;
             else if (c->current_key == "type") c->m->type = (uint8_t)val;
             else if (c->current_key == "temp") c->m->temperature = (int16_t)val;
             else if (c->current_key == "current") c->m->current_battery = (int16_t)val;
             else if (c->current_key == "cons") c->m->current_consumed = (int32_t)val;
             else if (c->current_key == "energy") c->m->energy_consumed = (int32_t)val;
             else if (c->current_key == "rem") c->m->battery_remaining = (int8_t)val;
             c->waiting_for_value = false;
         }
    }

    static void on_uint8(void* ctx, uint8_t val) { handle_int((DecodeContext*)ctx, val); }
    static void on_uint16(void* ctx, uint16_t val) { handle_int((DecodeContext*)ctx, val); }
    static void on_uint32(void* ctx, uint32_t val) { handle_int((DecodeContext*)ctx, val); }
    static void on_uint64(void* ctx, uint64_t val) { handle_int((DecodeContext*)ctx, val); }
    static void on_negint64(void* ctx, uint64_t val) { handle_int((DecodeContext*)ctx, -1 - (int64_t)val); }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadBattery& m = *static_cast<PayloadBattery*>(out_data);
        
        struct cbor_callbacks callbacks = cbor_empty_callbacks;
        callbacks.string = on_string;
        callbacks.uint8 = on_uint8;
        callbacks.uint16 = on_uint16;
        callbacks.uint32 = on_uint32;
        callbacks.uint64 = on_uint64;
        callbacks.negint64 = on_negint64;
        
        DecodeContext ctx;
        ctx.m = &m;
        
        size_t offset = 0;
        while(offset < buffer.size()) {
             cbor_decoder_result res = cbor_stream_decode(buffer.data() + offset, buffer.size() - offset, &callbacks, &ctx);
             if (res.read == 0) break;
             offset += res.read;
             if (res.status != CBOR_DECODER_FINISHED) break; 
        }
    }

    void teardown() override {}
    std::string name() const override { return "CBOR-Battery"; }
};

} // namespace pf

extern "C" pf::IBenchmark* create_benchmark() { return new pf::CborBenchmarkBattery(); }
