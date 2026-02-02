#include "IBenchmark.h"
#include "gps_beacon.pb.h"
#include <iostream>
#include <vector>
#include <cstring>
#include <memory>

namespace pf {

class ProtobufBenchmark : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        // Validate version
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        std::cout << "[Protobuf] Setup complete. Variant: Standard" << std::endl;
        (void)config; 

        // --- Integrity Verification ---
        Payload p;
        p.timestamp = 123456789;
        p.block_number = 999;
        auto buf = encode(&p);
        Payload d;
        memset(&d, 0, sizeof(Payload));
        decode(buf, &d);
        
        if (d.timestamp == 123456789 && d.block_number == 999) {
             std::cout << "[Protobuf] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[Protobuf] Sanity Check: FAILED! Expected 123456789, Got " << d.timestamp << std::endl;
             exit(1);
        }
    }

    std::vector<uint8_t> encode(const void* data) override {
        const Payload& m = *static_cast<const Payload*>(data);
        fanet::GPSBeacon b;
        
        b.set_timestamp(m.timestamp);
        // ... (Omitting full copy for brevity, C++ will use existing implementation)
        // Actually I must include the implementation in the tool call
        b.set_block_number(m.block_number);
        b.set_hash(m.hash, 32);
        b.set_time_usec(m.time_usec);
        b.set_fix_type(m.fix_type);
        b.set_lat(m.lat);
        b.set_lon(m.lon);
        b.set_alt(m.alt);
        b.set_eph(m.eph);
        b.set_epv(m.epv);
        b.set_vel(m.vel);
        b.set_cog(m.cog);
        b.set_satellites_visible(m.satellites_visible);
        b.set_alt_ellipsoid(m.alt_ellipsoid);
        b.set_h_acc(m.h_acc);
        b.set_v_acc(m.v_acc);
        b.set_vel_acc(m.vel_acc);
        b.set_hdg_acc(m.hdg_acc);
        
        b.set_hdg_acc(m.hdg_acc);
        
        // Optimization: Use Stack Buffer (SerializeToArray) instead of Heap String (SerializeToString)
        uint8_t buffer[256]; // Sufficient for ~101 bytes
        size_t size = b.ByteSizeLong();
        if (size > sizeof(buffer)) {
             // Fallback or Error? For benchmark parity we assume it fits.
             std::cerr << "[PROTO] Buffer overflow!" << std::endl;
             return std::vector<uint8_t>();
        }

        if (!b.SerializeToArray(buffer, size)) {
             std::cerr << "[PROTO] Serialize Failed!" << std::endl;
        }
        return std::vector<uint8_t>(buffer, buffer + size); // Still one copy to vector return, but avoids intermediate string
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        Payload& m = *static_cast<Payload*>(out_data);
        fanet::GPSBeacon b;
        if (!b.ParseFromArray(buffer.data(), buffer.size())) {
            std::cerr << "[PROTO] Parse Failed!" << std::endl;
            return;
        }
        
        m.timestamp = b.timestamp();
        m.block_number = b.block_number();
        
        // Safety copy for hash
        const std::string& h = b.hash();
        if(h.size() >= 32) memcpy(m.hash, h.data(), 32);
        
        m.time_usec = b.time_usec();
        m.fix_type = b.fix_type();
        m.lat = b.lat();
        m.lon = b.lon();
        m.alt = b.alt();
        m.eph = b.eph();
        m.epv = b.epv();
        m.vel = b.vel();
        m.cog = b.cog();
        m.satellites_visible = b.satellites_visible();
        m.alt_ellipsoid = b.alt_ellipsoid();
        m.h_acc = b.h_acc();
        m.v_acc = b.v_acc();
        m.vel_acc = b.vel_acc();
        m.hdg_acc = b.hdg_acc();
    }

    void teardown() override {
        google::protobuf::ShutdownProtobufLibrary();
    }

    std::string name() const override {
        return "Protobuf-Standard";
    }
};

} // namespace pf

extern "C" pf::IBenchmark* create_benchmark() {
    return new pf::ProtobufBenchmark();
}
