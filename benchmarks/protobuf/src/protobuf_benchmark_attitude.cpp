#include "IBenchmark.h"
#include "gps_beacon.pb.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <cmath>

namespace pf {

class ProtobufBenchmarkAttitude : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        (void)config;
        std::cout << "[Proto-Attitude] Setup." << std::endl;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadAttitude& m = *static_cast<const PayloadAttitude*>(data);
        fanet::Attitude b;
        b.set_time_boot_ms(m.time_boot_ms);
        b.set_roll(m.roll);
        b.set_pitch(m.pitch);
        b.set_yaw(m.yaw);
        b.set_rollspeed(m.rollspeed);
        b.set_pitchspeed(m.pitchspeed);
        b.set_yawspeed(m.yawspeed);
        size_t size = b.ByteSizeLong();
        std::vector<uint8_t> result(size);
        b.SerializeToArray(result.data(), size);
        return result;
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadAttitude& m = *static_cast<PayloadAttitude*>(out_data);
        fanet::Attitude b;
        if (!b.ParseFromArray(buffer.data(), buffer.size())) return;
        m.time_boot_ms = b.time_boot_ms();
        m.roll = b.roll();
        m.pitch = b.pitch();
        m.yaw = b.yaw();
        m.rollspeed = b.rollspeed();
        m.pitchspeed = b.pitchspeed();
        m.yawspeed = b.yawspeed();
    }

    void teardown() override {}
    std::string name() const override { return "Protobuf-Attitude"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::ProtobufBenchmarkAttitude(); }
