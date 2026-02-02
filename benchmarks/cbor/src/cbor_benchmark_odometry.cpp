#include "IBenchmark.h"
#include <cbor.h>
#include <iostream>
#include <vector>
#include <cstring>

namespace pf {

class CborBenchmarkOdometry : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        // Sanity Check
        PayloadOdometry p;
        memset(&p, 0, sizeof(p));
        p.time_usec = 1000;
        p.pose_covariance[0] = 1.23f;
        auto buf = encode(&p);
        PayloadOdometry d;
        memset(&d, 0, sizeof(d));
        decode(buf, &d);
        if (d.time_usec == 1000 && std::abs(d.pose_covariance[0] - 1.23f) < 0.001f) {
             std::cout << "[CBOR-Odometry] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[CBOR-Odometry] Sanity Check: FAILED" << std::endl;
             std::cerr << "Time: " << d.time_usec << " PCOV[0]: " << d.pose_covariance[0] << std::endl;
             exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadOdometry& m = *static_cast<const PayloadOdometry*>(data);
        unsigned char buffer[4096]; 
        unsigned char* ptr = buffer;
        size_t buffer_size = sizeof(buffer);
        size_t n;

        // Map(15 items)
        n = cbor_encode_map_start(15, ptr, buffer_size); ptr += n; buffer_size -= n;

        auto cbor_encode_text_string = [&](uint8_t* p, size_t sz, const char* str, size_t len) {
             size_t k = cbor_encode_string_start(len, p, sz);
             memcpy(p + k, str, len);
             return k + len;
        };

        auto encode_pair_uint64 = [&](const char* key, uint64_t val) {
            n = cbor_encode_text_string(ptr, buffer_size, key, strlen(key)); ptr += n; buffer_size -= n;
            n = cbor_encode_uint64(val, ptr, buffer_size); ptr += n; buffer_size -= n;
        };
        auto encode_pair_uint = [&](const char* key, unsigned val) {
            n = cbor_encode_text_string(ptr, buffer_size, key, strlen(key)); ptr += n; buffer_size -= n;
            n = cbor_encode_uint32(val, ptr, buffer_size); ptr += n; buffer_size -= n;
        };
        auto encode_pair_float = [&](const char* key, float val) {
            n = cbor_encode_text_string(ptr, buffer_size, key, strlen(key)); ptr += n; buffer_size -= n;
            n = cbor_encode_single(val, ptr, buffer_size); ptr += n; buffer_size -= n;
        };
        
        encode_pair_uint64("time", m.time_usec);
        encode_pair_uint("frame", m.frame_id);
        encode_pair_uint("child", m.child_frame_id);
        
        encode_pair_float("x", m.x);
        encode_pair_float("y", m.y);
        encode_pair_float("z", m.z);
        
        // Quaternions
        n = cbor_encode_text_string(ptr, buffer_size, "q", 1); ptr += n; buffer_size -= n;
        n = cbor_encode_array_start(4, ptr, buffer_size); ptr += n; buffer_size -= n;
        for(int i=0; i<4; i++) {
             n = cbor_encode_single(m.q[i], ptr, buffer_size); ptr += n; buffer_size -= n;
        }
        
        encode_pair_float("vx", m.vx);
        encode_pair_float("vy", m.vy);
        encode_pair_float("vz", m.vz);
        
        encode_pair_float("rs", m.rollspeed);
        encode_pair_float("ps", m.pitchspeed);
        encode_pair_float("ys", m.yawspeed);
        
        // Pose Cov
        n = cbor_encode_text_string(ptr, buffer_size, "pcov", 4); ptr += n; buffer_size -= n;
        n = cbor_encode_array_start(21, ptr, buffer_size); ptr += n; buffer_size -= n;
        for(int i=0; i<21; i++) {
             n = cbor_encode_single(m.pose_covariance[i], ptr, buffer_size); ptr += n; buffer_size -= n;
        }

        // Vel Cov
        n = cbor_encode_text_string(ptr, buffer_size, "vcov", 4); ptr += n; buffer_size -= n;
        n = cbor_encode_array_start(21, ptr, buffer_size); ptr += n; buffer_size -= n;
        for(int i=0; i<21; i++) {
             n = cbor_encode_single(m.velocity_covariance[i], ptr, buffer_size); ptr += n; buffer_size -= n;
        }

        return std::vector<uint8_t>(buffer, ptr);
    }

    // --- Streaming Decoder Context & Callbacks ---
    struct DecodeContext {
        PayloadOdometry* m;
        std::string current_key;
        bool waiting_for_value = false;
        enum State { NONE, Q, PCOV, VCOV };
        State state = NONE;
        int idx = 0;
    };

    static void on_string(void* ctx, cbor_data data, size_t len) {
        DecodeContext* c = (DecodeContext*)ctx;
        if (!c->waiting_for_value) {
            c->current_key.assign((const char*)data, len);
            c->waiting_for_value = true;
            
            if (c->current_key == "q") c->state = DecodeContext::Q;
            else if (c->current_key == "pcov") c->state = DecodeContext::PCOV;
            else if (c->current_key == "vcov") c->state = DecodeContext::VCOV;
            else c->state = DecodeContext::NONE;
            
            c->idx = 0;
        }
    }

    static void handle_float(DecodeContext* c, float val) {
        if (c->state == DecodeContext::Q) {
            if(c->idx < 4) c->m->q[c->idx++] = val;
            if(c->idx == 4) { c->state = DecodeContext::NONE; c->waiting_for_value = false; }
        } else if (c->state == DecodeContext::PCOV) {
            if(c->idx < 21) c->m->pose_covariance[c->idx++] = val;
            if(c->idx == 21) { c->state = DecodeContext::NONE; c->waiting_for_value = false; }
        } else if (c->state == DecodeContext::VCOV) {
            if(c->idx < 21) c->m->velocity_covariance[c->idx++] = val;
            if(c->idx == 21) { c->state = DecodeContext::NONE; c->waiting_for_value = false; }
        } else if (c->waiting_for_value) {
            if (c->current_key == "x") c->m->x = val;
            else if (c->current_key == "y") c->m->y = val;
            else if (c->current_key == "z") c->m->z = val;
            else if (c->current_key == "vx") c->m->vx = val;
            else if (c->current_key == "vy") c->m->vy = val;
            else if (c->current_key == "vz") c->m->vz = val;
            else if (c->current_key == "rs") c->m->rollspeed = val;
            else if (c->current_key == "ps") c->m->pitchspeed = val;
            else if (c->current_key == "ys") c->m->yawspeed = val;
            c->waiting_for_value = false; 
        }
    }
    
    static void handle_int(DecodeContext* c, uint64_t val) {
         if (c->waiting_for_value) {
             if (c->current_key == "time") c->m->time_usec = val;
             else if (c->current_key == "frame") c->m->frame_id = (uint8_t)val;
             else if (c->current_key == "child") c->m->child_frame_id = (uint8_t)val;
             c->waiting_for_value = false;
         }
    }

    static void on_float(void* ctx, float val) { handle_float((DecodeContext*)ctx, val); }
    static void on_double(void* ctx, double val) { handle_float((DecodeContext*)ctx, (float)val); }
    static void on_uint8(void* ctx, uint8_t val) { handle_int((DecodeContext*)ctx, val); }
    static void on_uint16(void* ctx, uint16_t val) { handle_int((DecodeContext*)ctx, val); }
    static void on_uint32(void* ctx, uint32_t val) { handle_int((DecodeContext*)ctx, val); }
    static void on_uint64(void* ctx, uint64_t val) { handle_int((DecodeContext*)ctx, val); }


    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadOdometry& m = *static_cast<PayloadOdometry*>(out_data);
        
        struct cbor_callbacks callbacks = cbor_empty_callbacks;
        callbacks.string = on_string;
         
        callbacks.float2 = on_float; 
        callbacks.float4 = on_float;
        callbacks.float8 = on_double;
        callbacks.uint8 = on_uint8;
        callbacks.uint16 = on_uint16;
        callbacks.uint32 = on_uint32;
        callbacks.uint64 = on_uint64;
        
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
    std::string name() const override { return "CBOR-Odometry"; }
};

} // namespace pf

extern "C" pf::IBenchmark* create_benchmark() { return new pf::CborBenchmarkOdometry(); }
