#include "IBenchmark.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <cmath>

namespace pf {

class JsonBenchmarkAttitude : public IBenchmark {
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
             std::cout << "[JSON-Attitude] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[JSON-Attitude] Sanity Check: FAILED" << std::endl;
             exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadAttitude& m = *static_cast<const PayloadAttitude*>(data);
        rapidjson::StringBuffer sb(0, 1024);
        rapidjson::Writer<rapidjson::StringBuffer> w(sb);
        w.StartObject();
        w.Key("boot"); w.Uint(m.time_boot_ms);
        w.Key("r"); w.Double(m.roll);
        w.Key("p"); w.Double(m.pitch);
        w.Key("y"); w.Double(m.yaw);
        w.Key("rs"); w.Double(m.rollspeed);
        w.Key("ps"); w.Double(m.pitchspeed);
        w.Key("ys"); w.Double(m.yawspeed);
        w.EndObject();
        const char* s = sb.GetString();
        return std::vector<uint8_t>(s, s + sb.GetSize());
    }

    struct PayloadHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, PayloadHandler> {
        PayloadAttitude* m;
        std::string key;
        PayloadHandler(PayloadAttitude* p) : m(p) {}
        bool Key(const char* str, rapidjson::SizeType len, bool) { key.assign(str, len); return true; }
        bool Uint(unsigned u) { if(key=="boot") m->time_boot_ms = u; return true; }
        bool Double(double d) {
            float f = (float)d;
            if(key=="r") m->roll=f;
            else if(key=="p") m->pitch=f;
            else if(key=="y") m->yaw=f;
            else if(key=="rs") m->rollspeed=f;
            else if(key=="ps") m->pitchspeed=f;
            else if(key=="ys") m->yawspeed=f;
            return true;
        }
    };

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadAttitude& m = *static_cast<PayloadAttitude*>(out_data);
        rapidjson::Reader reader;
        rapidjson::MemoryStream ss((const char*)buffer.data(), buffer.size());
        PayloadHandler handler(&m);
        reader.Parse(ss, handler);
    }

    void teardown() override {}
    std::string name() const override { return "JSON-Attitude"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::JsonBenchmarkAttitude(); }
