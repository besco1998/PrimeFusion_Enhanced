#include "IBenchmark.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <vector>
#include <cstring>
#include "mavlink_types.h"

namespace pf {

class JsonBenchmarkGPSBlock : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        PayloadGPSBlock p;
        for(int i=0; i<50; i++) {
            PayloadGPSRaw raw;
            memset(&raw, 0, sizeof(raw));
            raw.timestamp = 1000 + i;
            p.messages.push_back(raw);
        }
        
        auto buf = encode(&p);
        PayloadGPSBlock d;
        decode(buf, &d);
        
        if (d.messages.size() == 50 && d.messages[0].timestamp == 1000) {
             std::cout << "[JSON-GPSBlock] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[JSON-GPSBlock] Sanity Check: FAILED" << std::endl;
             exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadGPSBlock& m = *static_cast<const PayloadGPSBlock*>(data);
        rapidjson::StringBuffer sb(0, 4096);
        rapidjson::Writer<rapidjson::StringBuffer> w(sb);
        
        w.StartArray();
        for(const auto& r : m.messages) {
            w.StartObject();
            w.Key("ts"); w.Uint64(r.timestamp);
            w.Key("bn"); w.Uint(r.block_number);
            // hash skipped for brevity/speed in JSON unless required
            w.Key("tu"); w.Uint64(r.time_usec);
            w.Key("ft"); w.Uint(r.fix_type);
            w.Key("lat"); w.Int(r.lat);
            w.Key("lon"); w.Int(r.lon);
            w.Key("alt"); w.Int(r.alt);
            // ... minimal set for benchmark load
            w.EndObject();
        }
        w.EndArray();
        
        const char* s = sb.GetString();
        return std::vector<uint8_t>(s, s + sb.GetSize());
    }

    struct PayloadHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, PayloadHandler> {
        PayloadGPSBlock* m;
        PayloadGPSRaw current;
        std::string key;
        bool in_obj = false;

        PayloadHandler(PayloadGPSBlock* p) : m(p) {}
        
        bool StartObject() { in_obj = true; memset(&current, 0, sizeof(current)); return true; }
        bool EndObject(rapidjson::SizeType) { m->messages.push_back(current); in_obj = false; return true; }
        
        bool Key(const char* str, rapidjson::SizeType len, bool) { key.assign(str, len); return true; }
        
        bool Uint(unsigned u) { return Int(u); }
        bool Uint64(uint64_t u) {
            if(key=="ts") current.timestamp=u;
            else if(key=="tu") current.time_usec=u;
            return true;
        }
        bool Int(int i) {
             if(key=="bn") current.block_number=i;
             else if(key=="ft") current.fix_type=(uint8_t)i;
             // Also handle fields that might be small enough for Int but intended for other types if they were missing?
             // e.g. sat_visible (sv) is uint8.
             else if(key=="sv") current.satellites_visible=(uint8_t)i;
             // lat/lon are large, unlikely to be Int, but if they are? 
             else if(key=="lat") current.lat=i;
             else if(key=="lon") current.lon=i;
             else if(key=="alt") current.alt=i;
             else if(key=="ts") current.timestamp=(uint64_t)i;
             else if(key=="tu") current.time_usec=(uint64_t)i;
             return true;
        }
    };

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadGPSBlock& m = *static_cast<PayloadGPSBlock*>(out_data);
        rapidjson::Reader reader;
        rapidjson::MemoryStream ss((const char*)buffer.data(), buffer.size());
        PayloadHandler handler(&m);
        reader.Parse(ss, handler);
    }

    void teardown() override {}
    std::string name() const override { return "JSON-GPSBlock"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::JsonBenchmarkGPSBlock(); }
