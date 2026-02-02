#include "IBenchmark.h"
#include "gps_beacon.pb.h"
#include <iostream>
#include <vector>
#include <cstring>

namespace pf {

class ProtobufBenchmarkGlobalPosition : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        (void)config;
        std::cout << "[Proto-GlobalPos] Setup." << std::endl;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadGlobalPosition& m = *static_cast<const PayloadGlobalPosition*>(data);
        fanet::GlobalPosition b;
        b.set_time_boot_ms(m.time_boot_ms);
        b.set_lat(m.lat);
        b.set_lon(m.lon);
        b.set_alt(m.alt);
        b.set_relative_alt(m.relative_alt);
        b.set_vx(m.vx);
        b.set_vy(m.vy);
        b.set_vz(m.vz);
        b.set_hdg(m.hdg);
        size_t size = b.ByteSizeLong();
        std::vector<uint8_t> result(size);
        b.SerializeToArray(result.data(), size);
        return result;
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadGlobalPosition& m = *static_cast<PayloadGlobalPosition*>(out_data);
        fanet::GlobalPosition b;
        if (!b.ParseFromArray(buffer.data(), buffer.size())) return;
        m.time_boot_ms = b.time_boot_ms();
        m.lat = b.lat();
        m.lon = b.lon();
        m.alt = b.alt();
        m.relative_alt = b.relative_alt();
        m.vx = b.vx();
        m.vy = b.vy();
        m.vz = b.vz();
        m.hdg = b.hdg();
    }

    void teardown() override {}
    std::string name() const override { return "Protobuf-GlobalPos"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::ProtobufBenchmarkGlobalPosition(); }
