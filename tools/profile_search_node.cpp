#include "core/types.hpp"
#include "core/bitwise.hpp"
#include "core/fen.hpp"
#include "core/zobrist.hpp"
#include "board/board.hpp"
#include "movegen/movegen.hpp"
#include "movegen/magic.hpp"
#include "movegen/attack_masks.hpp"
#include "evaluation/eval.hpp"
#include "search/move_picker.hpp"
#include "search/tt.hpp"
#include "search/search.hpp"
#include <iostream>
#include <chrono>
#include <vector>

using namespace heavensgate;

int main() {
    Zobrist::init();
    MoveGenerator::init();
    Evaluator::init();

    std::vector<std::string> fens = {
        std::string(StartposFEN),
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",
        "r1bqk1nr/pppp1ppp/2n5/2b1p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 4 4",
        "r1bqkb1r/pp2pppp/2n2n2/3p4/3P4/2N2N2/PPP2PPP/R1BQKB1R w KQkq - 2 6",
        "r1b2rk1/pp1nqppp/2pbpn2/3p4/2PP4/2N1PN2/PPQ1BPPP/R1B2RK1 w - - 4 10",
        "2r2rk1/1pq1bppp/p2p1n2/4p3/P3P3/1PN1BP2/1P1Q2PP/2RR2K1 w - - 1 18",
        "8/5pk1/4p1p1/7p/3r3P/5PP1/4RK2/8 w - - 0 45"
    };

    std::vector<Board> boards;
    for (const auto& fen : fens) {
        Board b;
        FEN::parse(fen, b);
        boards.push_back(b);
    }

    const int ITERS = 100000;
    std::cout << "Profiling search node operations (" << ITERS << " iters across " << boards.size() << " positions)...\n";

    // 1. in_check
    auto t0 = std::chrono::high_resolution_clock::now();
    volatile bool dummy_b = false;
    for (int i = 0; i < ITERS; ++i) {
        for (const auto& b : boards) {
            dummy_b = MoveGenerator::in_check(b, b.side_to_move());
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double in_check_ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / (ITERS * boards.size());

    // 2. generate_legal_moves
    MoveList moves;
    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        for (const auto& b : boards) {
            MoveGenerator::generate_legal_moves(b, moves);
        }
    }
    t1 = std::chrono::high_resolution_clock::now();
    double movegen_ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / (ITERS * boards.size());

    // 3. MovePicker score_and_sort_moves
    MovePicker picker;
    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        for (const auto& b : boards) {
            MoveGenerator::generate_legal_moves(b, moves);
            picker.score_and_sort_moves(b, moves, 1, Move());
        }
    }
    t1 = std::chrono::high_resolution_clock::now();
    double sort_ns = (std::chrono::duration<double, std::nano>(t1 - t0).count() / (ITERS * boards.size())) - movegen_ns;

    // 4. make_move + unmake_move
    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        for (auto& b : boards) {
            MoveGenerator::generate_legal_moves(b, moves);
            if (!moves.empty()) {
                Move m = moves[0];
                b.make_move(m);
                b.unmake_move(m);
            }
        }
    }
    t1 = std::chrono::high_resolution_clock::now();
    double make_unmake_ns = (std::chrono::duration<double, std::nano>(t1 - t0).count() / (ITERS * boards.size())) - movegen_ns;

    // 5. TT probe + store
    TranspositionTable tt;
    tt.resize(16);
    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        for (const auto& b : boards) {
            tt.probe(b.zobrist_key());
            tt.store(b.zobrist_key(), Move(), 100, 4, TTBound::Exact, 1);
        }
    }
    t1 = std::chrono::high_resolution_clock::now();
    double tt_ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / (ITERS * boards.size());

    std::cout << "Operation Timings per call:\n";
    std::cout << "  in_check:               " << in_check_ns << " ns\n";
    std::cout << "  generate_legal_moves:   " << movegen_ns << " ns\n";
    std::cout << "  score_and_sort_moves:   " << sort_ns << " ns\n";
    std::cout << "  make_move + unmake:     " << make_unmake_ns << " ns\n";
    std::cout << "  tt probe + store:       " << tt_ns << " ns\n";

    return 0;
}
