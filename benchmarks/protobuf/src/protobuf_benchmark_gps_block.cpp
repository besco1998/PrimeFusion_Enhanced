#include "IBenchmark.h"
#include "gps_beacon.pb.h"
#include <iostream>
#include <vector>
#include <cstring>
#include "mavlink_types.h"

namespace pf {

class ProtobufBenchmarkGPSBlock : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        (void)config;
        std::cout << "[Proto-GPSBlock] Setup." << std::endl;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadGPSBlock& m = *static_cast<const PayloadGPSBlock*>(data);
        fanet::GPSBlock b;
        for(const auto& r : m.messages) {
            fanet::GPSBeacon* p = b.add_messages();
            p->set_timestamp(r.timestamp);
            p->set_block_number(r.block_number);
            p->set_time_usec(r.time_usec);
            p->set_fix_type(r.fix_type);
            p->set_lat(r.lat);
            p->set_lon(r.lon);
            p->set_alt(r.alt);
            // ... map minimal subset or all
        }
        size_t size = b.ByteSizeLong();
        std::vector<uint8_t> result(size);
        b.SerializeToArray(result.data(), size);
        return result;
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadGPSBlock& m = *static_cast<PayloadGPSBlock*>(out_data);
        fanet::GPSBlock b;
        if (!b.ParseFromArray(buffer.data(), buffer.size())) return;
        m.messages.resize(b.messages_size());
        for(int i=0; i<b.messages_size(); i++) {
            const auto& p = b.messages(i);
            m.messages[i].timestamp = p.timestamp();
            m.messages[i].block_number = p.block_number();
            m.messages[i].time_usec = p.time_usec();
            m.messages[i].fix_type = p.fix_type();
            m.messages[i].lat = p.lat();
            m.messages[i].lon = p.lon();
            m.messages[i].alt = p.alt();
        }
    }

    void teardown() override {}
    std::string name() const override { return "Protobuf-GPSBlock"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::ProtobufBenchmarkGPSBlock(); }
