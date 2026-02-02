#include "IBenchmark.h"
#include <msgpack.hpp>
#include <iostream>
#include <vector>
#include <cstring>

namespace pf {

class MsgPackBenchmarkStatus : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
         // Sanity
        PayloadStatus p;
        p.severity = 5;
        strncpy(p.text, "Hello World", 50);
        auto buf = encode(&p);
        PayloadStatus d;
        memset(&d, 0, sizeof(d));
        decode(buf, &d);
        if (d.severity == 5 && strcmp(d.text, "Hello World") == 0) {
            std::cout << "[MsgPack-Status] Sanity Check: PASS" << std::endl;
        } else {
            std::cerr << "[MsgPack-Status] Sanity Check: FAILED" << std::endl;
            exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadStatus& m = *static_cast<const PayloadStatus*>(data);
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> packer(sbuf);
        packer.pack_map(2);
        packer.pack("sev"); packer.pack(m.severity);
        packer.pack("txt"); packer.pack(std::string(m.text)); // Pack as string
        return std::vector<uint8_t>(sbuf.data(), sbuf.data() + sbuf.size());
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadStatus& m = *static_cast<PayloadStatus*>(out_data);
        msgpack::object_handle oh = msgpack::unpack((const char*)buffer.data(), buffer.size());
        msgpack::object obj = oh.get();
        if(obj.type != msgpack::type::MAP) return;
        
        auto& map = obj.via.map;
        for(size_t i=0; i<map.size; i++) {
             auto k = map.ptr[i].key.as<std::string>();
             if (k == "sev") m.severity = (uint8_t)map.ptr[i].val.as<unsigned int>();
             else if (k == "txt") {
                 std::string s = map.ptr[i].val.as<std::string>();
                 strncpy(m.text, s.c_str(), 49);
                 m.text[49] = '\0';
             }
        }
    }

    void teardown() override {}
    std::string name() const override { return "MsgPack-Status"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::MsgPackBenchmarkStatus(); }
