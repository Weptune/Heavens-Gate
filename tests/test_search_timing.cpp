#include "test.hpp"
#include "../src/search/search.hpp"
#include "../src/core/fen.hpp"
#include <iostream>

namespace heavensgate::test {
static bool low_clock_search() {
    auto engine = std::make_unique<SearchEngine>();
    engine->set_book_enabled(false);
    engine->set_threads(6);
    Board board; board.load_fen("8/8/8/1R6/K2k4/8/1r6/8 b - - 36 105");
    const auto root = FEN::to_string(board);
    const auto key = board.zobrist_key();
    const Move unused_history(Square::b1, Square::a3);
    engine->move_picker().add_history_score(Color::White, unused_history, 4);
    const auto original_history = engine->move_picker().get_history_score(Color::White, unused_history);
    std::array<double, 30> times{};
    int completed = 0;
    for (size_t i = 0; i < times.size(); ++i) {
        const auto budget = bank_search_budget(3.943, 0, 105);
        const auto start = std::chrono::steady_clock::now();
        const auto result = engine->search_iterative_deepening(board, 64, budget.maximum_ms, 0, budget.optimum_ms);
        times[i] = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
        MoveList legal; MoveGenerator::generate_legal_moves(board, legal);
        bool found = false;
        for (auto move : legal) found |= move == result.best_move;
        if (!found || result.threads_used != 1 || !result.metrics.total_nodes ||
            FEN::to_string(board) != root || board.zobrist_key() != key ||
            result.metrics.elapsed_seconds * 1000.0 > times[i] + 0.1 ||
            result.tt_hits != engine->tt().hits() || result.tt_probes != engine->tt().probes()) return false;
        completed += result.completed_depth > 0;
    }
    if (engine->move_picker().get_history_score(Color::White, unused_history) != original_history) return false;
    std::sort(times.begin(), times.end());
    std::cout << "[1ms budget / 3.943ms bank: median=" << times[15] << "ms p95=" << times[28]
              << "ms completed=" << completed << "/30 threads=1] ";
    // Timing is evidence, not a strict scheduler guarantee/flaky wall-clock assertion.
    const auto fixed = engine->search_iterative_deepening(board, 3, 0);
    if (fixed.completed_depth != 3 || fixed.threads_used != 6 ||
        engine->move_picker().get_history_score(Color::White, unused_history) != original_history / 2) return false;
    Board complex; complex.load_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    const auto interrupted = engine->search_iterative_deepening(complex, 64, 1.0);
    if (!interrupted.best_move || interrupted.threads_used != 1 || interrupted.completed_depth > 10) return false;
    // Explicitly retain fractional budgets; they must not become infinite search.
    const auto fraction = engine->search_iterative_deepening(board, 64, 0.05);
    if (!fraction.best_move || fraction.threads_used != 1 || fraction.metrics.total_nodes == 0) return false;

    // The ablation harness needs a real, repeatable node ceiling. A go nodes
    // limit also forces serial search so each variant receives the same budget.
    Board diagnostic; diagnostic.load_fen("r2q1rk1/pbn1p1bp/1p2Ppp1/3p4/3N1B1P/1PN2P2/PP2Q1P1/3RK2R b K h3 0 16");
    for (const char* child : {"", "a8c8", "f6f5"}) {
        Board position = diagnostic;
        if (*child) {
            MoveList legal; MoveGenerator::generate_legal_moves(position, legal);
            bool found = false;
            for (auto move : legal) if (move_to_uci(move) == child) {
                position.make_move(move); found = true; break;
            }
            if (!found) return false;
        }
        for (uint64_t cap : {1ULL, 2ULL, 25000ULL, 100000ULL}) {
            const auto capped = engine->search_iterative_deepening(position, 64, 0.0, cap);
            if (!capped.best_move || capped.threads_used != 1 ||
                !capped.metrics.total_nodes || capped.metrics.total_nodes > cap) return false;
        }
    }
    return true;
}
static const bool registered = register_test("Low-clock searched moves, serial fallback, preparation accounting and deferred history aging", low_clock_search);
} // namespace heavensgate::test
