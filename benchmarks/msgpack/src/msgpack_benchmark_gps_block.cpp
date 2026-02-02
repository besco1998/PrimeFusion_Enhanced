#include "IBenchmark.h"
#include <msgpack.hpp>
#include <iostream>
#include <vector>
#include <cstring>
#include "mavlink_types.h"

namespace pf {

class MsgPackBenchmarkGPSBlock : public IBenchmark {
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
             std::cout << "[MsgPack-GPSBlock] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[MsgPack-GPSBlock] Sanity Check: FAILED" << std::endl;
             exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadGPSBlock& m = *static_cast<const PayloadGPSBlock*>(data);
        msgpack::sbuffer sbuf;
        msgpack::packer<msgpack::sbuffer> packer(sbuf);
        
        packer.pack_array(m.messages.size());
        for(const auto& r : m.messages) {
            packer.pack_map(7);
            packer.pack("ts"); packer.pack(r.timestamp);
            packer.pack("bn"); packer.pack(r.block_number);
            packer.pack("tu"); packer.pack(r.time_usec);
            packer.pack("ft"); packer.pack(r.fix_type);
            packer.pack("lat"); packer.pack(r.lat);
            packer.pack("lon"); packer.pack(r.lon);
            packer.pack("alt"); packer.pack(r.alt);
        }
        return std::vector<uint8_t>(sbuf.data(), sbuf.data() + sbuf.size());
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadGPSBlock& m = *static_cast<PayloadGPSBlock*>(out_data);
        msgpack::object_handle oh = msgpack::unpack((const char*)buffer.data(), buffer.size());
        msgpack::object obj = oh.get();
        if(obj.type != msgpack::type::ARRAY) return;
        
        auto& arr = obj.via.array;
        m.messages.resize(arr.size);
        for(size_t i=0; i<arr.size; i++) {
             msgpack::object& item = arr.ptr[i];
             if(item.type == msgpack::type::MAP) {
                 auto& map = item.via.map;
                 for(size_t j=0; j<map.size; j++) {
                     auto k = map.ptr[j].key.as<std::string>();
                     auto& v = map.ptr[j].val;
                     if(k=="ts") m.messages[i].timestamp = v.as<uint64_t>();
                     else if(k=="bn") m.messages[i].block_number = v.as<uint32_t>();
                     else if(k=="tu") m.messages[i].time_usec = v.as<uint64_t>();
                     else if(k=="ft") m.messages[i].fix_type = v.as<uint8_t>();
                     else if(k=="lat") m.messages[i].lat = v.as<int32_t>();
                     else if(k=="lon") m.messages[i].lon = v.as<int32_t>();
                     else if(k=="alt") m.messages[i].alt = v.as<int32_t>();
                 }
             }
        }
    }

    void teardown() override {}
    std::string name() const override { return "MsgPack-GPSBlock"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::MsgPackBenchmarkGPSBlock(); }
