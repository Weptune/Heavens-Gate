#include "test.hpp"
#include "../src/core/fen.hpp"
#include "../src/core/zobrist.hpp"
#include "../src/core/match_clock.hpp"
#include "../src/search/search.hpp"
#include "../src/search/search_params.hpp"
#include "../src/search/syzygy.hpp"
#include <atomic>
#include <cstdlib>
#include <new>
#if defined(_WIN32)
#include <malloc.h>
#endif
#if defined(_OPENMP)
#include <omp.h>
#endif

namespace {
std::atomic<bool> count_allocations{false};
std::atomic<size_t> allocation_count{0};
void note_allocation() noexcept {
    if (count_allocations.load(std::memory_order_relaxed)) allocation_count.fetch_add(1, std::memory_order_relaxed);
}
}
void* operator new(size_t n) {
    note_allocation();
    if (void* p = std::malloc(n ? n : 1)) return p;
    throw std::bad_alloc();
}
void* operator new[](size_t n) { return ::operator new(n); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, size_t) noexcept { std::free(p); }
void operator delete[](void* p, size_t) noexcept { std::free(p); }
void* operator new(size_t n, std::align_val_t alignment) {
    note_allocation();
    const auto a = static_cast<size_t>(alignment);
#if defined(_WIN32)
    void* p = _aligned_malloc(n ? n : 1, a);
#else
    void* p = std::aligned_alloc(a, ((n ? n : 1) + a - 1) / a * a);
#endif
    if (!p) throw std::bad_alloc();
    return p;
}
void* operator new[](size_t n, std::align_val_t a) { return ::operator new(n, a); }
void operator delete(void* p, std::align_val_t) noexcept {
#if defined(_WIN32)
    _aligned_free(p);
#else
    std::free(p);
#endif
}
void operator delete[](void* p, std::align_val_t a) noexcept { ::operator delete(p, a); }
void operator delete(void* p, size_t, std::align_val_t a) noexcept { ::operator delete(p, a); }
void operator delete[](void* p, size_t, std::align_val_t a) noexcept { ::operator delete(p, a); }

namespace heavensgate::test {
static bool bounded_history() {
    Board board;
    board.reset();
    const auto fen = FEN::to_string(board);
    const auto key = board.zobrist_key();
    for (size_t i = 0; i < Board::HistoryCapacity - 1; ++i) board.make_null_move();
    if (board.can_push_history() || board.is_repetition(2)) return false;
    for (size_t i = 0; i < Board::HistoryCapacity - 1; ++i) board.unmake_null_move();
    if (board.history_ply() || FEN::to_string(board) != fen || board.zobrist_key() != key) return false;
    const std::array<Move, 4> cycle = {Move(Square::g1, Square::f3), Move(Square::g8, Square::f6),
                                      Move(Square::f3, Square::g1), Move(Square::f6, Square::g8)};
    for (auto move : cycle) board.make_move(move);
    if (!board.is_repetition(2) || board.fullmove_number() != 3) return false;
    for (auto it = cycle.rbegin(); it != cycle.rend(); ++it) board.unmake_move(*it);
    return FEN::to_string(board) == fen && board.zobrist_key() == key;
}

static bool special_move_roundtrips() {
    for (const char* fen : {
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
        "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 12",
        "4k3/P7/8/8/8/8/8/4K3 w - - 0 9"}) {
        Board board; board.load_fen(fen);
        const auto original = FEN::to_string(board), key = std::to_string(board.zobrist_key());
        MoveList moves; MoveGenerator::generate_legal_moves(board, moves);
        for (auto move : moves) {
            const auto before = board;
            board.make_move(move);
            if (board.zobrist_key() != Zobrist::compute_hash(board)) return false;
            board.unmake_move(move);
            if (FEN::to_string(board) != original || std::to_string(board.zobrist_key()) != key) return false;
            for (Color side : {Color::White, Color::Black}) {
                if (board.mg_material(side) != before.mg_material(side) || board.eg_material(side) != before.eg_material(side) ||
                    board.mg_pst(side) != before.mg_pst(side) || board.eg_pst(side) != before.eg_pst(side)) return false;
            }
            if (board.game_phase() != before.game_phase()) return false;
        }
    }
    return true;
}

static bool clock_contract() {
    const auto bank = TournamentTimeControl::parse(1, 0);
    const auto movetime = TournamentTimeControl::parse(1, 0, true);
    if (bank.bank_ms != 1000 || bank.movetime_ms || movetime.bank_ms || movetime.movetime_ms != 1000) return false;
    MatchClock late{100};
    if (late.finish_move(100, 1000) || late.remaining_ms != 0) return false;
    MatchClock valid{1000};
    return valid.finish_move(125.5, 200) && valid.remaining_ms == 1074.5 && pgn_clock(valid.remaining_ms) == "0:00:01.074";
}

static bool exact_endgame_contract() {
    auto& tb = SyzygyTablebase::instance();
    tb.init();
    for (const char* fen : {
        "7k/5K2/6Q1/8/8/8/8/8 b - - 0 1", // stalemate
        "8/8/8/8/8/6q1/5k2/7K w - - 0 1", // color mirror
        "8/7k/6R1/8/8/8/8/K7 b - - 0 1", // Kxg6
        "k7/8/8/8/8/6r1/7K/8 w - - 0 1",
        "8/8/8/8/8/8/4k3/K7 w - - 0 1"}) {
        Board board; board.load_fen(fen);
        if (tb.probe_wdl(board, 0) != ScoreDraw) return false;
    }
    Board board;
    board.load_fen("7k/8/8/8/8/8/Q7/K7 w - - 99 1");
    if (tb.probe_wdl(board, 0) != SyzygyTablebase::NO_SCORE) return false;
    board.set_halfmove_clock(100);
    if (tb.probe_wdl(board, 0) != ScoreDraw) return false;
    board.set_halfmove_clock(0); // Same Zobrist key, different rule-50 clock.
    if (tb.probe_wdl(board, 0) != SyzygyTablebase::NO_SCORE) return false;
    board.load_fen("7k/6Q1/5K2/8/8/8/8/8 b - - 100 1");
    return tb.probe_wdl(board, 3) == -ScoreMate + 3;
}

static bool concurrent_tt() {
    TranspositionTable table(1);
    std::atomic<int> errors{0};
    std::array<TTStatistics, 4> statistics{};
    // The second pass deliberately maps all keys to the same four-entry bucket.
    for (bool collide : {false, true}) {
#if defined(_OPENMP)
    #pragma omp parallel for num_threads(4)
#endif
    for (int t = 0; t < 4; ++t) {
        for (int i = 0; i < 20000; ++i) {
            const uint64_t key = collide
                ? 0x123400000ULL + ((i + t) % 32) * (table.capacity() / 4)
                : 0x123400000ULL + t * 1024 + i % 256;
            const int score = static_cast<int>(key % 900), depth = static_cast<int>(key % 32);
            table.store(key, Move(Square::e2, Square::e4), score, depth, TTBound::Exact, 0, static_cast<int>(key % 100));
            TTEntry entry;
            if (table.probe(key, entry, statistics[t]) && (entry.key != key || entry.score != score || entry.depth != depth ||
                entry.bound != TTBound::Exact || entry.rule50_clock != key % 100)) ++errors;
            if (i % 100 == 0) table.hashfull();
        }
    }
    }
    TTStatistics total;
    for (const auto& worker : statistics) total.add(worker);
    if (errors.load() || total.probes != 160000 || !total.hits || total.hits > total.probes || table.probes()) return false;
    table.publish_statistics(total);
    if (table.probes() != total.probes || table.hits() != total.hits) return false;
    table.clear();
    table.store(123, Move(Square::e2, Square::e4), ScoreMate - 7, 12, TTBound::Lower, 5);
    TTEntry entry;
    if (!table.probe(123, entry) || entry.score != ScoreMate - 2 || table.probes()) return false;
    table.store(124, Move(Square::e7, Square::e5), -ScoreMate + 7, 12, TTBound::Upper, 5);
    if (!table.probe(124, entry) || entry.score != -ScoreMate + 2) return false;
    TranspositionTable disabled(0);
    TTStatistics empty;
    return !disabled.probe(123, entry, empty) && !empty.probes && !empty.hits;
}

static bool allocation_free_search() {
    auto engine = std::make_unique<SearchEngine>();
    engine->set_book_enabled(false);
    Board board; board.load_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    const auto root = FEN::to_string(board), key = std::to_string(board.zobrist_key());
    for (bool contexts : {false, true}) {
    engine->tt().set_clock_contexts(contexts);
    for (int threads : {1, 6}) {
        engine->set_threads(threads);
        engine->clear();
        allocation_count.store(0);
        count_allocations.store(true);
        const auto result = engine->search_iterative_deepening(board, 4, 0);
        count_allocations.store(false);
        if (allocation_count.load() || result.completed_depth != 4 || !result.best_move ||
            result.metrics.total_nodes != engine->node_count_ || result.q_nodes > result.metrics.total_nodes ||
            !result.tt_hits || result.tt_hits > result.tt_probes || result.tt_probes > result.metrics.total_nodes ||
            result.tt_hits != engine->tt().hits() || result.tt_probes != engine->tt().probes() ||
            !result.q_nodes || FEN::to_string(board) != root || std::to_string(board.zobrist_key()) != key) return false;
    }
    }
    engine->tt().set_clock_contexts(false);
    engine->set_threads(1);
    engine->clear();
    // Timed fallback must not allocate even when six workers are configured.
    engine->set_threads(6);
    allocation_count.store(0);
    count_allocations.store(true);
    const auto timed = engine->search_iterative_deepening(board, 64, 1.0);
    count_allocations.store(false);
    if (allocation_count.load() || !timed.best_move || timed.threads_used != 1) return false;
    engine->set_threads(1);
    engine->clear();
    allocation_count.store(0);
    count_allocations.store(true);
    const auto trace_result = engine->search_minimax(board, 3, true);
    count_allocations.store(false);
    if (allocation_count.load() || !trace_result.best_move || !engine->exporter().trace_truncated()) return false;
    if (engine->exporter().to_json_string().find("\"trace_truncated\":true") == std::string::npos) return false;
    engine->clear();
    const auto first = engine->search_iterative_deepening(board, 4, 0);
    engine->clear();
    engine->node_count_ = 123456789;
    engine->q_nodes_ = 123456789;
    engine->tt().publish_statistics(TTStatistics{123456789, 123456789});
    const auto second = engine->search_iterative_deepening(board, 4, 0);
    if (first.metrics.total_nodes != second.metrics.total_nodes || first.q_nodes != second.q_nodes ||
        first.tt_hits != second.tt_hits || first.tt_probes != second.tt_probes) return false;
    Board terminal; terminal.load_fen("7k/6Q1/5K2/8/8/8/8/8 b - - 0 1");
    const auto stopped = engine->search_iterative_deepening(terminal, 4, 0);
    if (stopped.tt_hits || stopped.tt_probes || engine->tt().hits() || engine->tt().probes()) return false;
    engine->tt().publish_statistics(TTStatistics{123456789, 123456789});
    const auto minimax = engine->search_minimax(terminal, 2);
    return !minimax.tt_hits && !minimax.tt_probes && !engine->tt().hits() && !engine->tt().probes();
}

static bool guarded_allocation_free_search() {
    struct Restore { SearchParams value = g_search_params; ~Restore() { g_search_params = value; } } restore;
    auto engine = std::make_unique<SearchEngine>();
    engine->set_book_enabled(false);
    Board board; board.load_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    g_search_params.enable_nmp_guards = true;
    for (int threads : {1, 6}) {
        engine->set_threads(threads);
        engine->clear();
        allocation_count.store(0);
        count_allocations.store(true);
        const auto result = engine->search_iterative_deepening(board, 6, 0);
        count_allocations.store(false);
        if (allocation_count.load() || result.completed_depth != 6 || !result.best_move) return false;
    }
    return true;
}
static const bool guarded_allocation_registered = register_test("Experimental NMP guards: allocation-free serial/SMP search", guarded_allocation_free_search);
static const bool history_registered = register_test("Bounded history and null repetition boundary", bounded_history);
static const bool moves_registered = register_test("Castling/EP/promotion incremental state roundtrips", special_move_roundtrips);
static const bool clock_registered = register_test("Bank parsing and flag-before-increment clock contract", clock_contract);
static const bool tb_registered = register_test("Rule-aware endgame: stalemate, en-prise, rule50, mate precedence", exact_endgame_contract);
static const bool tt_registered = register_test("Concurrent TT snapshots, collision stress, private statistics and mate scores", concurrent_tt);
static const bool allocation_registered = register_test("No C++ heap allocations in serial/SMP go; current-move node counters", allocation_free_search);
} // namespace heavensgate::test
