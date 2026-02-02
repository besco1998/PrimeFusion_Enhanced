#include "IBenchmark.h"
#include <msgpack.hpp>
#include <iostream>
#include <vector>

namespace pf {

class MsgpackBenchmarkBattery : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadBattery& m = *static_cast<const PayloadBattery*>(data);
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> packer(sbuf);

        packer.pack_map(9);
        packer.pack("id"); packer.pack(m.id);
        packer.pack("func"); packer.pack(m.battery_function);
        packer.pack("type"); packer.pack(m.type);
        packer.pack("temp"); packer.pack(m.temperature);
        packer.pack("volt"); 
        packer.pack_array(10);
        for(int i=0; i<10; i++) packer.pack(m.voltages[i]);
        
        packer.pack("current"); packer.pack(m.current_battery);
        packer.pack("cons"); packer.pack(m.current_consumed);
        packer.pack("energy"); packer.pack(m.energy_consumed);
        packer.pack("rem"); packer.pack(m.battery_remaining);

        return std::vector<uint8_t>(sbuf.data(), sbuf.data() + sbuf.size());
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadBattery& m = *static_cast<PayloadBattery*>(out_data);
        msgpack::object_handle oh = msgpack::unpack((const char*)buffer.data(), buffer.size());
        msgpack::object obj = oh.get();
        
        if (obj.type != msgpack::type::MAP) return;
        
        // Manual iteration
        size_t size = obj.via.map.size;
        for(size_t i=0; i<size; i++) {
             std::string key = obj.via.map.ptr[i].key.as<std::string>();
             msgpack::object& val = obj.via.map.ptr[i].val;
             
             if(key == "id") m.id = (uint8_t)val.as<unsigned>();
             else if(key == "func") m.battery_function = (uint8_t)val.as<unsigned>();
             else if(key == "type") m.type = (uint8_t)val.as<unsigned>();
             else if(key == "temp") m.temperature = (int16_t)val.as<int>();
             else if(key == "current") m.current_battery = (int16_t)val.as<int>();
             else if(key == "cons") m.current_consumed = (int32_t)val.as<int>();
             else if(key == "energy") m.energy_consumed = (int32_t)val.as<int>();
             else if(key == "rem") m.battery_remaining = (int8_t)val.as<int>();
             else if(key == "volt" && val.type == msgpack::type::ARRAY) {
                 size_t arr_size = val.via.array.size;
                 for(size_t j=0; j<arr_size && j<10; j++) {
                     m.voltages[j] = (uint16_t)val.via.array.ptr[j].as<unsigned>();
                 }
             }
        }
    }

    void teardown() override {}
    std::string name() const override { return "MsgPack-Battery"; }
};

} // namespace pf

extern "C" pf::IBenchmark* create_benchmark() { return new pf::MsgpackBenchmarkBattery(); }
