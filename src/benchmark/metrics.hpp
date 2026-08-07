#pragma once

#include <cstdint>
#include <string>
#include <chrono>
#include <cmath>

namespace heavensgate {

struct EngineMetrics {
    std::string engine_version{"v0.0"};
    uint64_t total_nodes{0};
    uint64_t alpha_beta_cuts{0};
    double elapsed_seconds{0.0};
    double nps{0.0};
    int max_depth{0};
    double effective_branching_factor{0.0};
    double alpha_beta_cut_percentage{0.0};
    double hash_hit_rate{0.0};

    void calculate_derived();
    std::string report_markdown() const;
};

class MetricsTracker {
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    EngineMetrics current_metrics_;

public:
    void reset() { current_metrics_ = EngineMetrics{}; }
    void start_timer();
    void stop_timer();
    
    void add_nodes(uint64_t count = 1) noexcept { current_metrics_.total_nodes += count; }
    void add_cut() noexcept { current_metrics_.alpha_beta_cuts++; }
    void set_depth(int d) noexcept { current_metrics_.max_depth = d; }
    void set_version(std::string_view ver) { current_metrics_.engine_version = ver; }

    EngineMetrics get_metrics();
};

} // namespace heavensgate
