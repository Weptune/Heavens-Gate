#pragma once

#include "../core/types.hpp"
#include "../core/polyglot.hpp"
#include "../board/board.hpp"
#include "../movegen/movegen.hpp"
#include "../evaluation/eval.hpp"
#include "pv.hpp"
#include "move_picker.hpp"
#include "tt.hpp"
#include "../visualization/exporter.hpp"
#include "../benchmark/metrics.hpp"
#include <chrono>
#include <vector>

namespace heavensgate {

struct SearchResult {
    Move best_move;
    int best_score = 0;
    int depth = 0;
    int completed_depth = 0;
    EngineMetrics metrics;
    uint64_t tt_hits = 0;
    uint64_t q_nodes = 0;
    std::vector<Move> pv;
};

class SearchEngine {
public:
    explicit SearchEngine(TranspositionTable* shared_tt = nullptr)
        : tt_ptr_(shared_tt ? shared_tt : &local_tt_) {}

    SearchEngine(const SearchEngine&) = delete;
    SearchEngine& operator=(const SearchEngine&) = delete;
    SearchEngine(SearchEngine&&) = default;
    SearchEngine& operator=(SearchEngine&&) = default;

    SearchResult search_minimax(Board& board, int depth, bool export_tree = false);
    SearchResult search_alphabeta(Board& board, int depth, bool use_move_ordering = true, bool use_tt = true, bool export_tree = false);
    SearchResult search_iterative_deepening(Board& board, int max_depth, double max_time_ms, uint64_t max_nodes = 0);
    SearchResult search_smp(Board& board, int max_depth, int num_threads = 4);

    GameTreeExporter& exporter() { return exporter_; }
    const GameTreeExporter& tree_exporter() const { return exporter_; }
    GameTreeExporter& tree_exporter() { return exporter_; }
    TranspositionTable& tt() { return *tt_ptr_; }
    const TranspositionTable& tt() const { return *tt_ptr_; }
    PolyGlotBook& polyglot_book() { return polyglot_book_; }
    const PolyGlotBook& polyglot_book() const { return polyglot_book_; }
    void stop() { time_stop_flag_ = true; }
    void set_threads(int threads) { num_threads_ = std::max(1, threads); }
    int threads() const { return num_threads_; }

private:
    int quiescence_search(Board& board, int alpha, int beta, int ply);
    int negamax_minimax(Board& board, int depth, int ply, TreeNodeJSON* json_node);
    int negamax_alphabeta(Board& board, int depth, int ply, int alpha, int beta, bool use_move_ordering, bool use_tt, Move pv_move = Move(), Move prev_move = Move(), Move prev2_move = Move(), TreeNodeJSON* json_node = nullptr, int prev_eval = -ScoreInfinity);

    bool is_time_up() {
        if (time_stop_flag_) return true;
        if (max_time_ms_ > 0.0) {
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double, std::milli>(now - search_start_time_).count();
            if (elapsed >= max_time_ms_) {
                time_stop_flag_ = true;
                return true;
            }
        }
        return false;
    }

    TranspositionTable* tt_ptr_{nullptr};
    TranspositionTable local_tt_;
    PolyGlotBook polyglot_book_;
    PVTable pv_table_;
    MovePicker move_picker_;
    GameTreeExporter exporter_;
    MetricsTracker metrics_tracker_;
    std::array<std::array<int, 4096>, 2> corr_history_{};
    std::array<std::array<int, 4096>, 2> non_pawn_corr_history_{};
    int num_threads_{6};

public:
    void clear() noexcept {
        pv_table_.clear();
        move_picker_.clear();
        if (tt_ptr_ == &local_tt_) {
            local_tt_.clear();
        }
        corr_history_.fill({});
        non_pawn_corr_history_.fill({});
        node_count_ = 0;
    }

    std::chrono::high_resolution_clock::time_point search_start_time_;
    double max_time_ms_ = 0.0;
    bool time_stop_flag_ = false;
    uint64_t q_nodes_ = 0;
    uint64_t node_count_ = 0;
};

} // namespace heavensgate
