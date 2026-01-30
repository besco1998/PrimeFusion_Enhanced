#ifndef PRIME_FUSION_IMETRIC_H
#define PRIME_FUSION_IMETRIC_H

#include <string>
#include <map>
#include <vector>

namespace pf {

/**
 * @brief Metric Observer Interface
 * Decouples "What we measure" from "How we encode".
 */
class IMetric {
public:
    virtual ~IMetric() = default;

    /**
     * @brief Called immediately before the benchmark loop/operation.
     */
    virtual void start() = 0;

    /**
     * @brief Called immediately after the benchmark loop/operation.
     */
    virtual void stop() = 0;

    /**
     * @brief Retrieve the measured result.
     * @return map of MetricName -> Value
     */
    virtual std::map<std::string, double> get_result() const = 0;

    /**
     * @brief Name of this metric (e.g., "CPU_Cycles", "Heap_Delta")
     */
    virtual std::string name() const = 0;
    
    /**
     * @brief Reset internal state for next run.
     */
    virtual void reset() = 0;
};

} // namespace pf

#endif // PRIME_FUSION_IMETRIC_H
