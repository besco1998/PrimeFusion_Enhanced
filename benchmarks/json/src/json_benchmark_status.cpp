#include "IBenchmark.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <vector>
#include <cstring>

namespace pf {

class JsonBenchmarkStatus : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        std::cout << "[JSON-Status] Setup." << std::endl;
        PayloadStatus p;
        p.severity = 5;
        strncpy(p.text, "Hello World", 50);
        auto buf = encode(&p);
        PayloadStatus d;
        memset(&d, 0, sizeof(d));
        decode(buf, &d);
        if (d.severity == 5 && strcmp(d.text, "Hello World") == 0) {
            std::cout << "[JSON-Status] Sanity Check: PASS" << std::endl;
        } else {
            std::cerr << "[JSON-Status] Sanity Check: FAILED" << std::endl;
            exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadStatus& m = *static_cast<const PayloadStatus*>(data);
        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> w(sb);
        w.StartObject();
        w.Key("sev"); w.Uint(m.severity);
        w.Key("txt"); w.String(m.text);
        w.EndObject();
        const char* s = sb.GetString();
        return std::vector<uint8_t>(s, s + sb.GetSize());
    }

    struct PayloadHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, PayloadHandler> {
        PayloadStatus* m;
        bool in_txt = false;

        PayloadHandler(PayloadStatus* p) : m(p) {}
        bool Key(const char* str, rapidjson::SizeType len, bool) {
            if (len == 3 && strncmp(str, "txt", 3) == 0) in_txt = true;
            else in_txt = false;
            return true;
        }
        bool Uint(unsigned u) { if (!in_txt) m->severity = (uint8_t)u; return true; }
        bool String(const char* str, rapidjson::SizeType len, bool) {
            if (in_txt) {
                size_t copy_len = len < 49 ? len : 49;
                memcpy(m->text, str, copy_len);
                m->text[copy_len] = '\0';
            }
            return true;
        }
    };

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadStatus& m = *static_cast<PayloadStatus*>(out_data);
        rapidjson::Reader reader;
        rapidjson::MemoryStream ss((const char*)buffer.data(), buffer.size());
        PayloadHandler handler(&m);
        reader.Parse(ss, handler);
    }

    void teardown() override {}
    std::string name() const override { return "JSON-Status"; }
};

} // namespace pf

extern "C" pf::IBenchmark* create_benchmark() { return new pf::JsonBenchmarkStatus(); }
