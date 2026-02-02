#include "IBenchmark.h"
#include <msgpack.hpp>
#include <iostream>
#include <vector>
#include <cstring>
#include <cmath>

namespace pf {

class MsgPackBenchmarkAttitude : public IBenchmark {
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
             std::cout << "[MsgPack-Attitude] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[MsgPack-Attitude] Sanity Check: FAILED" << std::endl;
             exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadAttitude& m = *static_cast<const PayloadAttitude*>(data);
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> packer(sbuf);
        packer.pack_map(7);
        packer.pack("boot"); packer.pack(m.time_boot_ms);
        packer.pack("r"); packer.pack(m.roll);
        packer.pack("p"); packer.pack(m.pitch);
        packer.pack("y"); packer.pack(m.yaw);
        packer.pack("rs"); packer.pack(m.rollspeed);
        packer.pack("ps"); packer.pack(m.pitchspeed);
        packer.pack("ys"); packer.pack(m.yawspeed);
        return std::vector<uint8_t>(sbuf.data(), sbuf.data() + sbuf.size());
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadAttitude& m = *static_cast<PayloadAttitude*>(out_data);
        msgpack::object_handle oh = msgpack::unpack((const char*)buffer.data(), buffer.size());
        msgpack::object obj = oh.get();
        if(obj.type != msgpack::type::MAP) return;
        
        auto& map = obj.via.map;
        for(size_t i=0; i<map.size; i++) {
             auto k = map.ptr[i].key.as<std::string>();
             auto& val = map.ptr[i].val;
             if (k=="boot") m.time_boot_ms = val.as<uint32_t>();
             else if (k=="r") m.roll = val.as<float>();
             else if (k=="p") m.pitch = val.as<float>();
             else if (k=="y") m.yaw = val.as<float>();
             else if (k=="rs") m.rollspeed = val.as<float>();
             else if (k=="ps") m.pitchspeed = val.as<float>();
             else if (k=="ys") m.yawspeed = val.as<float>();
        }
    }

    void teardown() override {}
    std::string name() const override { return "MsgPack-Attitude"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::MsgPackBenchmarkAttitude(); }
