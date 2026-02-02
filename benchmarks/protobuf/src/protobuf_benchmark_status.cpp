#include "IBenchmark.h"
#include "gps_beacon.pb.h"
#include <iostream>
#include <vector>
#include <cstring>

namespace pf {

class ProtobufBenchmarkStatus : public IBenchmark {
public:
    void setup(const BenchmarkConfig& config) override {
        (void)config;
        std::cout << "[Proto-Status] Setup." << std::endl;
    }

    std::vector<uint8_t> encode(const void* data) override {
        const PayloadStatus& m = *static_cast<const PayloadStatus*>(data);
        fanet::Status b;
        b.set_severity(m.severity);
        b.set_text(m.text);
        size_t size = b.ByteSizeLong();
        std::vector<uint8_t> result(size);
        b.SerializeToArray(result.data(), size);
        return result;
    }

    void decode(const std::vector<uint8_t>& buffer, void* out_data) override {
        PayloadStatus& m = *static_cast<PayloadStatus*>(out_data);
        fanet::Status b;
        if (!b.ParseFromArray(buffer.data(), buffer.size())) return;
        m.severity = b.severity();
        strncpy(m.text, b.text().c_str(), 49);
        m.text[49] = '\0';
    }

    void teardown() override {}
    std::string name() const override { return "Protobuf-Status"; }
};

} // pf
extern "C" pf::IBenchmark* create_benchmark() { return new pf::ProtobufBenchmarkStatus(); }
