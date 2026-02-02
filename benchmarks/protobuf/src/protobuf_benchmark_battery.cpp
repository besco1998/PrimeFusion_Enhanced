#include "IBenchmark.h"
#include "gps_beacon.pb.h"
#include <iostream>
#include <vector>

namespace pf {

class ProtobufBenchmarkBattery : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        // Verify Proto
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadBattery& m = *static_cast<const PayloadBattery*>(data);
        fanet::Battery proto;
        
        proto.set_id(m.id);
        proto.set_battery_function(m.battery_function);
        proto.set_type(m.type);
        proto.set_temperature(m.temperature);
        for(int i=0; i<10; i++) proto.add_voltages(m.voltages[i]);
        proto.set_current_battery(m.current_battery);
        proto.set_current_consumed(m.current_consumed);
        proto.set_energy_consumed(m.energy_consumed);
        proto.set_battery_remaining(m.battery_remaining);
        
        #if 1
        // Optimized: Stack Array
        uint8_t buffer[1024]; 
        size_t size = proto.ByteSizeLong(); 
        proto.SerializeToArray(buffer, size);
        return std::vector<uint8_t>(buffer, buffer + size);
        #else
        std::string s;
        proto.SerializeToString(&s);
        return std::vector<uint8_t>(s.begin(), s.end());
        #endif
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadBattery& m = *static_cast<PayloadBattery*>(out_data);
        fanet::Battery proto;
        if (!proto.ParseFromArray(buffer.data(), buffer.size())) return;
        
        m.id = (uint8_t)proto.id();
        m.battery_function = (uint8_t)proto.battery_function();
        m.type = (uint8_t)proto.type();
        m.temperature = (int16_t)proto.temperature();
        for(int i=0; i<proto.voltages_size() && i<10; i++) {
            m.voltages[i] = (uint16_t)proto.voltages(i);
        }
        m.current_battery = (int16_t)proto.current_battery();
        m.current_consumed = (int32_t)proto.current_consumed();
        m.energy_consumed = (int32_t)proto.energy_consumed();
        m.battery_remaining = (int8_t)proto.battery_remaining();
    }

    void teardown() override {
        google::protobuf::ShutdownProtobufLibrary();
    }
    std::string name() const override { return "Protobuf-Battery"; }
};

} // namespace pf

extern "C" pf::IBenchmark* create_benchmark() { return new pf::ProtobufBenchmarkBattery(); }
