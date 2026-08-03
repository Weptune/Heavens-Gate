#include "metrics.hpp"
#include <sstream>
#include <iomanip>

namespace heavensgate {

void EngineMetrics::calculate_derived() {
    if (elapsed_seconds > 0.0) {
        nps = total_nodes / elapsed_seconds;
    } else {
        nps = 0.0;
    }

    if (total_nodes > 0) {
        alpha_beta_cut_percentage = (static_cast<double>(alpha_beta_cuts) / total_nodes) * 100.0;
    } else {
        alpha_beta_cut_percentage = 0.0;
    }

    if (max_depth > 0 && total_nodes > 0) {
        effective_branching_factor = std::pow(static_cast<double>(total_nodes), 1.0 / max_depth);
    } else {
        effective_branching_factor = 0.0;
    }
}

std::string EngineMetrics::report_markdown() const {
    std::stringstream ss;
    ss << "### Benchmark Report - Engine " << engine_version << "\n";
    ss << "| Metric | Value |\n";
    ss << "| :--- | :--- |\n";
    ss << "| Total Nodes | " << total_nodes << " |\n";
    ss << "| Time Elapsed | " << std::fixed << std::setprecision(4) << elapsed_seconds << " s |\n";
    ss << "| Nodes Per Second | " << static_cast<uint64_t>(nps) << " NPS |\n";
    ss << "| Max Search Depth | " << max_depth << " |\n";
    if (effective_branching_factor > 0.0) {
        ss << "| Effective Branching Factor (EBF) | " << std::fixed << std::setprecision(2) << effective_branching_factor << " |\n";
    }
    if (alpha_beta_cuts > 0) {
        ss << "| Alpha-Beta Cutoffs | " << alpha_beta_cuts << " |\n";
        ss << "| Cutoff Rate | " << std::fixed << std::setprecision(1) << alpha_beta_cut_percentage << "% |\n";
    }
    return ss.str();
}

void MetricsTracker::start_timer() {
    current_metrics_.total_nodes = 0;
    current_metrics_.alpha_beta_cuts = 0;
    start_time_ = std::chrono::high_resolution_clock::now();
}

void MetricsTracker::stop_timer() {
    auto end = std::chrono::high_resolution_clock::now();
    current_metrics_.elapsed_seconds = std::chrono::duration<double>(end - start_time_).count();
    current_metrics_.calculate_derived();
}

EngineMetrics MetricsTracker::get_metrics() {
    return current_metrics_;
}

} // namespace heavensgate
