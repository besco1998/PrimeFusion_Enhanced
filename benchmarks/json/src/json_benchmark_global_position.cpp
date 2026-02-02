#include "IBenchmark.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <vector>
#include <cstring>

namespace pf {

class JsonBenchmarkGlobalPosition : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        PayloadGlobalPosition p;
        memset(&p, 0, sizeof(p));
        p.lat = 123456789;
        auto buf = encode(&p);
        PayloadGlobalPosition d;
        memset(&d, 0, sizeof(d));
        decode(buf, &d);
        if (d.lat == 123456789) {
             std::cout << "[JSON-GlobalPos] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[JSON-GlobalPos] Sanity Check: FAILED" << std::endl;
             exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadGlobalPosition& m = *static_cast<const PayloadGlobalPosition*>(data);
        rapidjson::StringBuffer sb(0, 1024);
        rapidjson::Writer<rapidjson::StringBuffer> w(sb);
        w.StartObject();
        w.Key("boot"); w.Uint(m.time_boot_ms);
        w.Key("lat"); w.Int(m.lat);
        w.Key("lon"); w.Int(m.lon);
        w.Key("alt"); w.Int(m.alt);
        w.Key("rel"); w.Int(m.relative_alt);
        w.Key("vx"); w.Int(m.vx);
        w.Key("vy"); w.Int(m.vy);
        w.Key("vz"); w.Int(m.vz);
        w.Key("hdg"); w.Uint(m.hdg);
        w.EndObject();
        const char* s = sb.GetString();
        return std::vector<uint8_t>(s, s + sb.GetSize());
    }

    struct PayloadHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, PayloadHandler> {
        PayloadGlobalPosition* m;
        std::string key;
        PayloadHandler(PayloadGlobalPosition* p) : m(p) {}
        bool Key(const char* str, rapidjson::SizeType len, bool) { key.assign(str, len); return true; }
        bool Uint(unsigned u) { 
            if(key=="boot") m->time_boot_ms = u;
            else if(key=="hdg") m->hdg = (uint16_t)u;
            else return Int((int)u); // Fallback to Int logic for positive integers like lat/lon
            return true; 
        }
        bool Int(int i) {
            if(key=="lat") m->lat = i;
            else if(key=="lon") m->lon = i;
            else if(key=="alt") m->alt = i;
            else if(key=="rel") m->relative_alt = i;
            else if(key=="vx") m->vx = (int16_t)i;
            else if(key=="vy") m->vy = (int16_t)i;
            else if(key=="vz") m->vz = (int16_t)i;
            return true;
        }
    };

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadGlobalPosition& m = *static_cast<PayloadGlobalPosition*>(out_data);
        rapidjson::Reader reader;
        rapidjson::MemoryStream ss((const char*)buffer.data(), buffer.size());
        PayloadHandler handler(&m);
        reader.Parse(ss, handler);
    }

    void teardown() override {}
    std::string name() const override { return "JSON-GlobalPos"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::JsonBenchmarkGlobalPosition(); }
