#include "IBenchmark.h"
#include "gps_beacon.pb.h" // Includes all messages now
#include <vector>

namespace pf {

class ProtobufBenchmarkOdometry : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        (void)config;
        std::cout << "[Proto-Odometry] Setup." << std::endl;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadOdometry& m = *static_cast<const PayloadOdometry*>(data);
        fanet::Odometry b;
        
        b.set_time_usec(m.time_usec);
        b.set_frame_id(m.frame_id);
        b.set_child_frame_id(m.child_frame_id);
        b.set_x(m.x); b.set_y(m.y); b.set_z(m.z);
        
        for(int i=0; i<4; i++) b.add_q(m.q[i]);
        
        b.set_vx(m.vx); b.set_vy(m.vy); b.set_vz(m.vz);
        b.set_rollspeed(m.rollspeed);
        b.set_pitchspeed(m.pitchspeed);
        b.set_yawspeed(m.yawspeed);
        
        for(int i=0; i<21; i++) b.add_pose_covariance(m.pose_covariance[i]);
        for(int i=0; i<21; i++) b.add_velocity_covariance(m.velocity_covariance[i]);
        
        size_t size = b.ByteSizeLong();
        std::vector<uint8_t> result(size);
        b.SerializeToArray(result.data(), size);
        return result;
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadOdometry& m = *static_cast<PayloadOdometry*>(out_data);
        fanet::Odometry b;
        if (!b.ParseFromArray(buffer.data(), buffer.size())) return;
        
        m.time_usec = b.time_usec();
        m.frame_id = b.frame_id();
        m.child_frame_id = b.child_frame_id();
        m.x = b.x(); m.y = b.y(); m.z = b.z();
        
        for(int i=0; i<b.q_size() && i<4; i++) m.q[i] = b.q(i);
        
        m.vx = b.vx(); m.vy = b.vy(); m.vz = b.vz();
        m.rollspeed = b.rollspeed();
        m.pitchspeed = b.pitchspeed();
        m.yawspeed = b.yawspeed();
        
        for(int i=0; i<b.pose_covariance_size() && i<21; i++) 
            m.pose_covariance[i] = b.pose_covariance(i);
            
        for(int i=0; i<b.velocity_covariance_size() && i<21; i++) 
            m.velocity_covariance[i] = b.velocity_covariance(i);
    }

    void teardown() override {}
    std::string name() const override { return "Protobuf-Odometry"; }
};

} // namespace pf

extern "C" pf::IBenchmark* create_benchmark() { return new pf::ProtobufBenchmarkOdometry(); }
