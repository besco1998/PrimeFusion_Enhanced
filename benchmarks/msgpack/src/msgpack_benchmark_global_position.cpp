#include "IBenchmark.h"
#include <msgpack.hpp>
#include <iostream>
#include <vector>
#include <cstring>

namespace pf {

class MsgPackBenchmarkGlobalPosition : public IBenchmark {
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
             std::cout << "[MsgPack-GlobalPos] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[MsgPack-GlobalPos] Sanity Check: FAILED" << std::endl;
             exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadGlobalPosition& m = *static_cast<const PayloadGlobalPosition*>(data);
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> packer(sbuf);
        packer.pack_map(9);
        packer.pack("boot"); packer.pack(m.time_boot_ms);
        packer.pack("lat"); packer.pack(m.lat);
        packer.pack("lon"); packer.pack(m.lon);
        packer.pack("alt"); packer.pack(m.alt);
        packer.pack("rel"); packer.pack(m.relative_alt);
        packer.pack("vx"); packer.pack(m.vx);
        packer.pack("vy"); packer.pack(m.vy);
        packer.pack("vz"); packer.pack(m.vz);
        packer.pack("hdg"); packer.pack(m.hdg);
        return std::vector<uint8_t>(sbuf.data(), sbuf.data() + sbuf.size());
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadGlobalPosition& m = *static_cast<PayloadGlobalPosition*>(out_data);
        msgpack::object_handle oh = msgpack::unpack((const char*)buffer.data(), buffer.size());
        msgpack::object obj = oh.get();
        if(obj.type != msgpack::type::MAP) return;
        
        auto& map = obj.via.map;
        for(size_t i=0; i<map.size; i++) {
             auto k = map.ptr[i].key.as<std::string>();
             auto& val = map.ptr[i].val;
             if (k=="boot") m.time_boot_ms = val.as<uint32_t>();
             else if (k=="lat") m.lat = val.as<int32_t>();
             else if (k=="lon") m.lon = val.as<int32_t>();
             else if (k=="alt") m.alt = val.as<int32_t>();
             else if (k=="rel") m.relative_alt = val.as<int32_t>();
             else if (k=="vx") m.vx = val.as<int16_t>();
             else if (k=="vy") m.vy = val.as<int16_t>();
             else if (k=="vz") m.vz = val.as<int16_t>();
             else if (k=="hdg") m.hdg = val.as<uint16_t>();
        }
    }

    void teardown() override {}
    std::string name() const override { return "MsgPack-GlobalPos"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::MsgPackBenchmarkGlobalPosition(); }
