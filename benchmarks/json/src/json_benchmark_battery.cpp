#include "IBenchmark.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <vector>
#include <cstring>

namespace pf {

class JsonBenchmarkBattery : public IBenchmark {
public:
    enum Variant { STANDARD, SHORT };
    Variant variant_ = STANDARD;

    void setup(const BenchmarkConfig& config) override {
        // Battery doesn't really have "Canonical" variants yet, just keep Standard
        variant_ = STANDARD; 
        std::cout << "[JSON-Battery] Setup complete." << std::endl;

        // Integrity Verification
        PayloadBattery p;
        p.id = 1;
        p.battery_function = 1;
        p.voltages[0] = 4200;
        p.voltages[9] = 3500;
        
        auto buf = encode(&p);
        PayloadBattery d;
        memset(&d, 0, sizeof(PayloadBattery));
        decode(buf, &d);
        
        if (d.id == 1 && d.voltages[0] == 4200 && d.voltages[9] == 3500) {
             std::cout << "[JSON-Battery] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[JSON-Battery] Sanity Check: FAILED!" << std::endl;
             exit(1);
        }
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadBattery& m = *static_cast<const PayloadBattery*>(data);
        
        rapidjson::StringBuffer sb(0, 1024); 
        rapidjson::Writer<rapidjson::StringBuffer> w(sb);
        w.StartObject();

        w.Key("id"); w.Uint(m.id);
        w.Key("func"); w.Uint(m.battery_function);
        w.Key("type"); w.Uint(m.type);
        w.Key("temp"); w.Int(m.temperature);
        
        w.Key("voltages");
        w.StartArray();
        for(int i=0; i<10; i++) w.Uint(m.voltages[i]);
        w.EndArray();

        w.Key("current"); w.Int(m.current_battery);
        w.Key("consumed"); w.Int(m.current_consumed);
        w.Key("energy"); w.Int(m.energy_consumed); // Fixed typo energy_consumed
        w.Key("pct"); w.Int(m.battery_remaining);
        
        w.EndObject();
        const char* s = sb.GetString();
        return std::vector<uint8_t>(s, s + sb.GetSize());
    }

    struct PayloadHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, PayloadHandler> {
        PayloadBattery* m;
        std::string key;
        bool in_voltages = false;
        int voltage_idx = 0;

        PayloadHandler(PayloadBattery* p) : m(p) {}

        bool Key(const char* str, rapidjson::SizeType length, bool) {
            key.assign(str, length);
            if (key == "voltages") {
                in_voltages = true;
                voltage_idx = 0;
            } else {
                in_voltages = false;
            }
            return true;
        }

        bool Uint(unsigned u) { return Int(u); }
        bool Int(int i) { 
            if (in_voltages) {
                if (voltage_idx < 10) {
                    m->voltages[voltage_idx++] = (uint16_t)i;
                }
            } else {
                if (key == "id") m->id = (uint8_t)i;
                else if (key == "func") m->battery_function = (uint8_t)i;
                else if (key == "type") m->type = (uint8_t)i;
                else if (key == "temp") m->temperature = (int16_t)i;
                else if (key == "current") m->current_battery = (int16_t)i;
                else if (key == "consumed") m->current_consumed = i;
                else if (key == "energy") m->energy_consumed = i;
                else if (key == "pct") m->battery_remaining = (int8_t)i;
            }
            return true; 
        }
    };

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadBattery& m = *static_cast<PayloadBattery*>(out_data);
        rapidjson::Reader reader;
        rapidjson::MemoryStream ss((const char*)buffer.data(), buffer.size());
        PayloadHandler handler(&m);
        reader.Parse(ss, handler);
    }

    void teardown() override {}
    std::string name() const override { return "JSON-Battery"; }
};

} // namespace pf

extern "C" pf::IBenchmark* create_benchmark() {
    return new pf::JsonBenchmarkBattery();
}
