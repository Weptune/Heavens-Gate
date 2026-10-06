#include "core/types.hpp"
#include "core/bitwise.hpp"
#include "core/fen.hpp"
#include "core/zobrist.hpp"
#include "board/board.hpp"
#include "movegen/movegen.hpp"
#include "movegen/attack_masks.hpp"
#include "evaluation/eval.hpp"
#include "evaluation/eval_features.hpp"
#include "search/syzygy.hpp"
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
    std::cout << "Profiling " << ITERS << " iterations across " << boards.size() << " positions...\n";

    // 1. Total evaluate_fast
    auto t0 = std::chrono::high_resolution_clock::now();
    volatile int dummy = 0;
    for (int i = 0; i < ITERS; ++i) {
        for (const auto& b : boards) {
            dummy += Evaluator::evaluate_fast(b);
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    double total_ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / (ITERS * boards.size());
    std::cout << "Total evaluate_fast: " << total_ns << " ns / call\n";

    // 2. Breakdown of components for White
    double pawn_ns = 0, king_ns = 0, act_ns = 0, threat_ns = 0, mob_ns = 0, imb_ns = 0;

    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        for (const auto& b : boards) {
            ScorePair p = EvalFeatures::evaluate_pawn_structure(b, Color::White) + EvalFeatures::evaluate_passed_pawns(b, Color::White);
            dummy += p.mg;
        }
    }
    t1 = std::chrono::high_resolution_clock::now();
    pawn_ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / (ITERS * boards.size());

    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        for (const auto& b : boards) {
            ScorePair k = EvalFeatures::evaluate_king_safety(b, Color::White);
            dummy += k.mg;
        }
    }
    t1 = std::chrono::high_resolution_clock::now();
    king_ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / (ITERS * boards.size());

    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        for (const auto& b : boards) {
            ScorePair a = EvalFeatures::evaluate_piece_activity(b, Color::White);
            dummy += a.mg;
        }
    }
    t1 = std::chrono::high_resolution_clock::now();
    act_ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / (ITERS * boards.size());

    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        for (const auto& b : boards) {
            ScorePair th = EvalFeatures::evaluate_threats(b, Color::White);
            dummy += th.mg;
        }
    }
    t1 = std::chrono::high_resolution_clock::now();
    threat_ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / (ITERS * boards.size());

    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        for (const auto& b : boards) {
            ScorePair m = EvalFeatures::evaluate_mobility(b, Color::White);
            dummy += m.mg;
        }
    }
    t1 = std::chrono::high_resolution_clock::now();
    mob_ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / (ITERS * boards.size());

    t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERS; ++i) {
        for (const auto& b : boards) {
            ScorePair im = EvalFeatures::evaluate_material_imbalances(b, Color::White);
            dummy += im.mg;
        }
    }
    t1 = std::chrono::high_resolution_clock::now();
    imb_ns = std::chrono::duration<double, std::nano>(t1 - t0).count() / (ITERS * boards.size());

    std::cout << "\nSingle-side component breakdown:\n";
    std::cout << "  Pawn Structure + Passed: " << pawn_ns << " ns\n";
    std::cout << "  King Safety:             " << king_ns << " ns\n";
    std::cout << "  Piece Activity:          " << act_ns << " ns\n";
    std::cout << "  Threats:                 " << threat_ns << " ns\n";
    std::cout << "  Mobility:                " << mob_ns << " ns\n";
    std::cout << "  Material Imbalances:     " << imb_ns << " ns\n";

    return 0;
}
