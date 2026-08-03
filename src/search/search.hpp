#pragma once

#include "../board/board.hpp"
#include "../movegen/movegen.hpp"
#include "../evaluation/eval.hpp"
#include "../benchmark/metrics.hpp"
#include "../visualization/exporter.hpp"
#include "pv.hpp"
#include "move_picker.hpp"
#include <chrono>

namespace heavensgate {

struct SearchResult {
    Move best_move;
    int best_score{0};
    EngineMetrics metrics;
    PrincipalVariation pv;
    int completed_depth{0};
};

class SearchEngine {
private:
    MetricsTracker metrics_tracker_;
    PVTable pv_table_;
    TreeExporter exporter_;
    MovePicker move_picker_;

    std::chrono::high_resolution_clock::time_point search_start_time_;
    double max_time_ms_{0.0};
    bool time_stop_flag_{false};

    bool is_time_up() noexcept {
        if (time_stop_flag_) return true;
        if (max_time_ms_ <= 0.0) return false;
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - search_start_time_).count();
        if (elapsed >= max_time_ms_) {
            time_stop_flag_ = true;
            return true;
        }
        return false;
    }

    int negamax_minimax(Board& board, int depth, int ply, TreeNodeJSON* json_node);
    int negamax_alphabeta(Board& board, int depth, int ply, int alpha, int beta, bool use_move_ordering, Move pv_move, TreeNodeJSON* json_node);

public:
    SearchResult search_minimax(Board& board, int depth, bool export_tree = false);
    SearchResult search_alphabeta(Board& board, int depth, bool use_move_ordering = true, bool export_tree = false);
    SearchResult search_iterative_deepening(Board& board, int max_depth, double max_time_ms = 0.0);

    const TreeExporter& tree_exporter() const { return exporter_; }
};

} // namespace heavensgate
