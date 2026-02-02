#include "runner_template.hpp"

int main(int argc, char** argv) {
    return pf::run_benchmark_template<pf::PayloadGPSRaw>(argc, argv);
}
