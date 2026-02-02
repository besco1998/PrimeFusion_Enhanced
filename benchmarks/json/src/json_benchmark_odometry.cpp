#include "IBenchmark.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <iostream>
#include <vector>
#include <cstring>

namespace pf {

class JsonBenchmarkOdometry : public IBenchmark {
public:
    enum Variant { STANDARD };
    Variant variant_ = STANDARD;

    void setup(const BenchmarkConfig& config) override {
        variant_ = STANDARD; 
        std::cout << "[JSON-Odometry] Setup complete." << std::endl;

        // Integrity Verification
        PayloadOdometry p;
        memset(&p, 0, sizeof(p));
        p.time_usec = 1000;
        p.pose_covariance[0] = 1.23f;
        p.pose_covariance[20] = 4.56f;
        
        auto buf = encode(&p);
        PayloadOdometry d;
        memset(&d, 0, sizeof(PayloadOdometry));
        decode(buf, &d);
        
        // Float comparison with epsilon
        bool match = (d.time_usec == 1000) && 
                     (std::abs(d.pose_covariance[0] - 1.23f) < 0.001f) &&
                     (std::abs(d.pose_covariance[20] - 4.56f) < 0.001f);

        if (match) {
             std::cout << "[JSON-Odometry] Sanity Check: PASS" << std::endl;
        } else {
             std::cerr << "[JSON-Odometry] Sanity Check: FAILED!" << std::endl;
             std::cerr << "Exp CV[0]: 1.23 Got: " << d.pose_covariance[0] << std::endl;
             std::cerr << "Exp CV[20]: 4.56 Got: " << d.pose_covariance[20] << std::endl;
             std::cerr << "Exp Time: 1000 Got: " << d.time_usec << std::endl;
             exit(1);
        }
        (void)config;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadOdometry& m = *static_cast<const PayloadOdometry*>(data);
        
        rapidjson::StringBuffer sb(0, 2048); // Larger buffer for floats
        rapidjson::Writer<rapidjson::StringBuffer> w(sb);
        w.StartObject();

        w.Key("time"); w.Uint64(m.time_usec);
        w.Key("frame"); w.Uint(m.frame_id);
        w.Key("child"); w.Uint(m.child_frame_id);
        
        w.Key("x"); w.Double(m.x);
        w.Key("y"); w.Double(m.y);
        w.Key("z"); w.Double(m.z);
        
        w.Key("q");
        w.StartArray();
        for(int i=0; i<4; i++) w.Double(m.q[i]);
        w.EndArray();

        w.Key("vx"); w.Double(m.vx);
        w.Key("vy"); w.Double(m.vy);
        w.Key("vz"); w.Double(m.vz);
        
        w.Key("rs"); w.Double(m.rollspeed);
        w.Key("ps"); w.Double(m.pitchspeed);
        w.Key("ys"); w.Double(m.yawspeed);

        w.Key("pcov");
        w.StartArray();
        for(int i=0; i<21; i++) w.Double(m.pose_covariance[i]); // Use Double for float precision in JSON
        w.EndArray();

        w.Key("vcov");
        w.StartArray();
        for(int i=0; i<21; i++) w.Double(m.velocity_covariance[i]);
        w.EndArray();
        
        w.EndObject();
        const char* s = sb.GetString();
        return std::vector<uint8_t>(s, s + sb.GetSize());
    }

    struct PayloadHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, PayloadHandler> {
        PayloadOdometry* m;
        std::string key;
        
        enum ArrayState { NONE, Q, PCOV, VCOV };
        ArrayState array_state = NONE;
        int idx = 0;

        PayloadHandler(PayloadOdometry* p) : m(p) {}

        bool Key(const char* str, rapidjson::SizeType length, bool) {
            key.assign(str, length);
            if (key == "q") { array_state = Q; idx = 0; }
            else if (key == "pcov") { array_state = PCOV; idx = 0; }
            else if (key == "vcov") { array_state = VCOV; idx = 0; }
            else { array_state = NONE; }
            return true;
        }

        bool Uint64(uint64_t u) {
             if (key == "time") m->time_usec = u;
             return true;
        }

        bool Uint(unsigned u) { return Int(u); }
        bool Int(int i) {
             if (array_state == PCOV) {
                 if (idx < 21) m->pose_covariance[idx++] = (float)i;
             }
             else if (array_state == VCOV) {
                 if (idx < 21) m->velocity_covariance[idx++] = (float)i;
             }
             else if (key == "frame") m->frame_id = (uint8_t)i;
             else if (key == "child") m->child_frame_id = (uint8_t)i;
             else if (key == "time") m->time_usec = (uint64_t)i;
             return true;
        }
        
        bool Double(double d) {
            float f = (float)d;
            if (array_state == Q) {
                if (idx < 4) m->q[idx++] = f;
            } else if (array_state == PCOV) {
                if (idx < 21) m->pose_covariance[idx++] = f;
            } else if (array_state == VCOV) {
                if (idx < 21) m->velocity_covariance[idx++] = f;
            } else {
                if (key == "x") m->x = f;
                else if (key == "y") m->y = f;
                else if (key == "z") m->z = f;
                else if (key == "vx") m->vx = f;
                else if (key == "vy") m->vy = f;
                else if (key == "vz") m->vz = f;
                else if (key == "rs") m->rollspeed = f;
                else if (key == "ps") m->pitchspeed = f;
                else if (key == "ys") m->yawspeed = f;
            }
            return true;
        }
    };

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadOdometry& m = *static_cast<PayloadOdometry*>(out_data);
        rapidjson::Reader reader;
        rapidjson::MemoryStream ss((const char*)buffer.data(), buffer.size());
        PayloadHandler handler(&m);
        reader.Parse(ss, handler);
    }

    void teardown() override {}
    std::string name() const override { return "JSON-Odometry"; }
};

} // namespace pf

extern "C" pf::IBenchmark* create_benchmark() {
    return new pf::JsonBenchmarkOdometry();
}
