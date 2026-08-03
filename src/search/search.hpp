#pragma once

#include "../board/board.hpp"
#include "../movegen/movegen.hpp"
#include "../evaluation/eval.hpp"
#include "../benchmark/metrics.hpp"
#include "../visualization/exporter.hpp"
#include "pv.hpp"
#include "move_picker.hpp"

namespace heavensgate {

struct SearchResult {
    Move best_move;
    int best_score{0};
    EngineMetrics metrics;
    PrincipalVariation pv;
};

class SearchEngine {
private:
    MetricsTracker metrics_tracker_;
    PVTable pv_table_;
    TreeExporter exporter_;
    MovePicker move_picker_;

    int negamax_minimax(Board& board, int depth, int ply, TreeNodeJSON* json_node);
    int negamax_alphabeta(Board& board, int depth, int ply, int alpha, int beta, bool use_move_ordering, TreeNodeJSON* json_node);

public:
    SearchResult search_minimax(Board& board, int depth, bool export_tree = false);
    SearchResult search_alphabeta(Board& board, int depth, bool use_move_ordering = true, bool export_tree = false);

    const TreeExporter& tree_exporter() const { return exporter_; }
};

} // namespace heavensgate
