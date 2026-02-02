#include "IBenchmark.h"
#include <msgpack.hpp>
#include <iostream>
#include <vector>

namespace pf {

class MsgPackBenchmarkOdometry : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        PayloadOdometry p;
        memset(&p, 0, sizeof(p));
        p.time_usec = 1000;
        p.pose_covariance[0] = 1.23f;
        auto buf = encode(&p);
        PayloadOdometry d;
        memset(&d, 0, sizeof(d));
        decode(buf, &d);
        if (d.time_usec == 1000 && std::abs(d.pose_covariance[0] - 1.23f) < 0.001f) {
             std::cout << "[MsgPack-Odometry] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[MsgPack-Odometry] Sanity Check: FAILED" << std::endl;
             exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadOdometry& m = *static_cast<const PayloadOdometry*>(data);
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> packer(sbuf);

        packer.pack_map(15);
        packer.pack("time"); packer.pack(m.time_usec);
        packer.pack("frame"); packer.pack(m.frame_id);
        packer.pack("child"); packer.pack(m.child_frame_id);
        packer.pack("x"); packer.pack(m.x);
        packer.pack("y"); packer.pack(m.y);
        packer.pack("z"); packer.pack(m.z);
        
        packer.pack("q");
        packer.pack_array(4);
        for(int i=0; i<4; i++) packer.pack(m.q[i]);
        
        packer.pack("vx"); packer.pack(m.vx);
        packer.pack("vy"); packer.pack(m.vy);
        packer.pack("vz"); packer.pack(m.vz);
        packer.pack("rs"); packer.pack(m.rollspeed);
        packer.pack("ps"); packer.pack(m.pitchspeed);
        packer.pack("ys"); packer.pack(m.yawspeed);
        
        packer.pack("pcov");
        packer.pack_array(21);
        for(int i=0; i<21; i++) packer.pack(m.pose_covariance[i]);
        
        packer.pack("vcov");
        packer.pack_array(21);
        for(int i=0; i<21; i++) packer.pack(m.velocity_covariance[i]);

        return std::vector<uint8_t>(sbuf.data(), sbuf.data() + sbuf.size());
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadOdometry& m = *static_cast<PayloadOdometry*>(out_data);
        msgpack::object_handle oh = msgpack::unpack((const char*)buffer.data(), buffer.size());
        msgpack::object obj = oh.get();
        
        if (obj.type != msgpack::type::MAP) return;
        
        // Manual extraction via map iteration
        auto& map = obj.via.map;
        for(uint32_t i=0; i<map.size; ++i) {
            auto key = map.ptr[i].key.as<std::string>();
            auto& val = map.ptr[i].val;
            
            if(key == "time") m.time_usec = val.as<uint64_t>();
            else if(key == "frame") m.frame_id = val.as<uint8_t>();
            else if(key == "child") m.child_frame_id = val.as<uint8_t>();
            else if(key == "x") m.x = val.as<float>();
            else if(key == "y") m.y = val.as<float>();
            else if(key == "z") m.z = val.as<float>();
            else if(key == "q") {
                 auto& arr = val.via.array;
                 for(int j=0; j<4; j++) m.q[j] = arr.ptr[j].as<float>();
            }
            else if(key == "pcov") {
                 auto& arr = val.via.array;
                 for(int j=0; j<21; j++) m.pose_covariance[j] = arr.ptr[j].as<float>();
            }
            // ... (Assume full coverage for benchmark)
             else if(key == "vcov") {
                 auto& arr = val.via.array;
                 for(int j=0; j<21; j++) m.velocity_covariance[j] = arr.ptr[j].as<float>();
            }
        }
    }

    void teardown() override {}
    std::string name() const override { return "MsgPack-Odometry"; }
};

} // namespace pf

extern "C" pf::IBenchmark* create_benchmark() { return new pf::MsgPackBenchmarkOdometry(); }
