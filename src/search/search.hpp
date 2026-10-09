#pragma once

#include "../core/types.hpp"
#include "../core/polyglot.hpp"
#include "../core/time_budget.hpp"
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
#include <atomic>

namespace heavensgate {

namespace test { struct SearchEngineTestAccess; }

struct SearchResult {
    Move best_move;
    int best_score = 0;
    int depth = 0;
    int completed_depth = 0;
    EngineMetrics metrics;
    uint64_t tt_hits = 0;
    uint64_t tt_probes = 0;
    uint64_t q_nodes = 0;
#if defined(HG_TT_DIAGNOSTICS)
    TTDiagnostics tt_diagnostics;
#endif
    int threads_used = 1;
    PrincipalVariation pv;
};

class SearchEngine {
public:
    explicit SearchEngine(TranspositionTable* shared_tt = nullptr, MovePicker* shared_picker = nullptr);

    SearchEngine(const SearchEngine&) = delete;
    SearchEngine& operator=(const SearchEngine&) = delete;
    SearchEngine(SearchEngine&&) = default;
    SearchEngine& operator=(SearchEngine&&) = default;

    SearchResult search_minimax(Board& board, int depth, bool export_tree = false);
    SearchResult search_alphabeta(Board& board, int depth, bool use_move_ordering = true, bool use_tt = true, bool export_tree = false);
    SearchResult search_iterative_deepening(Board& board, int max_depth, double max_time_ms, uint64_t max_nodes = 0, double opt_time_ms = 0.0);
    SearchResult search_smp(Board& board, int max_depth, int num_threads = 4);

    GameTreeExporter& exporter() { return exporter_; }
    const GameTreeExporter& tree_exporter() const { return exporter_; }
    GameTreeExporter& tree_exporter() { return exporter_; }
    TranspositionTable& tt() { return *tt_ptr_; }
    const TranspositionTable& tt() const { return *tt_ptr_; }
    MovePicker& move_picker() noexcept { return *move_picker_ptr_; }
    const MovePicker& move_picker() const noexcept { return *move_picker_ptr_; }
    PolyGlotBook& polyglot_book() { return polyglot_book_; }
    const PolyGlotBook& polyglot_book() const { return polyglot_book_; }
    void stop() { time_stop_flag_.store(true, std::memory_order_relaxed); }
    // Configuration is performed while idle, never from the recursive search.
    void set_threads(int threads);
    void set_book_enabled(bool enabled) noexcept { book_enabled_ = enabled; }
    bool book_enabled() const noexcept { return book_enabled_; }
    int threads() const { return num_threads_; }
    const SearchTimingOptions& timing_options() const noexcept { return timing_options_; }
    void set_move_overhead(double ms) noexcept { timing_options_.move_overhead_ms = std::clamp(ms, 0.0, 5000.0); }
    void set_minimum_smp_time(double ms) noexcept { timing_options_.minimum_smp_time_ms = std::clamp(ms, 0.0, 1000.0); }
    void set_uci_output(bool enabled) noexcept { uci_output_ = enabled; }
    static void init_lmr_table(float divisor = 3.20f);
    bool uci_output() const noexcept { return uci_output_; }
    bool is_stopped() const noexcept {
        return time_stop_flag_.load(std::memory_order_relaxed) ||
               (master_stop_flag_ && master_stop_flag_->load(std::memory_order_relaxed));
    }
    void set_master_stop_flag(std::atomic<bool>* flag) noexcept { master_stop_flag_ = flag; }

private:
    friend struct test::SearchEngineTestAccess;
    int quiescence_search(Board& board, int alpha, int beta, int ply);
    int negamax_minimax(Board& board, int depth, int ply, SearchTreeNode* json_node);
    int negamax_alphabeta(Board& board, int depth, int ply, int alpha, int beta, bool use_move_ordering, bool use_tt, Move pv_move = Move(), SearchTreeNode* json_node = nullptr, int prev_eval = -ScoreInfinity, Move excluded_move = Move(), bool previous_was_null = false);
    void iterative_deepening_root(Board& board, int max_depth, uint64_t max_nodes, SearchResult& final_result);
    void prepare_search(double max_time_ms, double opt_time_ms);
    void finish_tt_statistics(SearchResult& result) noexcept {
        result.tt_hits = tt_statistics_.hits;
        result.tt_probes = tt_statistics_.probes;
#if defined(HG_TT_DIAGNOSTICS)
        result.tt_diagnostics = tt_statistics_.diagnostics;
#endif
        tt().publish_statistics(tt_statistics_);
    }

    bool is_time_up() {
        if (is_stopped()) return true;
        if (max_time_ms_ > 0.0) {
            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double, std::milli>(now - search_start_time_).count();
            if (elapsed >= max_time_ms_) {
                time_stop_flag_.store(true, std::memory_order_relaxed);
                return true;
            }
        }
        return false;
    }

    std::atomic<bool>* master_stop_flag_{nullptr};

    TranspositionTable* tt_ptr_{nullptr};
    TranspositionTable local_tt_;
    TTStatistics tt_statistics_;
    PolyGlotBook polyglot_book_;
    PVTable pv_table_;
    MovePicker* move_picker_ptr_{nullptr};
    MovePicker local_move_picker_;
    MovePicker& move_picker_;
    GameTreeExporter exporter_;
    MetricsTracker metrics_tracker_;
    std::array<std::array<int, 4096>, 2> corr_history_{};
    std::array<std::array<int, 4096>, 2> non_pawn_corr_history_{};
    std::array<std::array<int, 4096>, 2> major_corr_history_{};
    std::array<int, 256> eval_stack_{};
    std::array<Move, 256> move_stack_{};
    std::array<Piece, 256> piece_stack_{};
    int num_threads_{6};
    static constexpr int MaxThreads = 64;
    std::array<std::unique_ptr<SearchEngine>, MaxThreads - 1> workers_{};
    Board worker_board_;
    bool book_enabled_{true};
    bool uci_output_{false};
    SearchTimingOptions timing_options_{};
    uint64_t time_poll_mask_{2047};
    uint64_t max_nodes_{0};

public:
    void clear() noexcept {
        pv_table_.clear();
        local_move_picker_.clear();
        if (tt_ptr_ == &local_tt_) {
            local_tt_.clear();
        }
        corr_history_.fill({});
        non_pawn_corr_history_.fill({});
        major_corr_history_.fill({});
        eval_stack_.fill(-ScoreInfinity);
        move_stack_.fill(Move{});
        piece_stack_.fill(Piece::None);
        node_count_ = 0;
        q_nodes_ = 0;
        tt_statistics_.reset();
        for (auto& worker : workers_) if (worker) worker->clear();
        time_stop_flag_.store(false, std::memory_order_relaxed);
    }

    std::chrono::steady_clock::time_point search_start_time_;
    double opt_time_ms_ = 0.0;
    double max_time_ms_ = 0.0;
    std::atomic<bool> time_stop_flag_{false};
    uint64_t q_nodes_ = 0;
    uint64_t node_count_ = 0;
};

} // namespace heavensgate
