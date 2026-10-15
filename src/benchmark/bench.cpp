#include "bench.hpp"
#include "../board/board.hpp"
#include "../search/search.hpp"
#include "../search/tt.hpp"
#include <array>
#include <chrono>
#include <iostream>
#include <memory>

namespace heavensgate {

static const char* const BenchFens[] = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    "4rrk1/p4p1p/1p1R2p1/2p5/2P5/2Q1PP2/PP3PqP/4K2R w K - 1 23",
    "1rbqn3/ppp3pp/2np4/4p2k/4P3/3P2RP/PPP2PP1/R1B1Q1K1 b - - 2 18",
    "r1b1kbnN/1pp4p/p5p1/8/3PpB2/8/PPP2P1P/R3K2R w KQ - 0 13",
    "1r1q1rk1/2pbbpp1/7p/1p3Q2/7P/2P3P1/1B1P1P1R/RN3K2 w - - 2 21",
    "2kr1r2/2p1b2p/p1bq2p1/1p2p3/3nP3/2NQB1P1/PPP2P1P/1R2K2R w - - 5 20",
    "8/1P6/2k5/1R3p2/3Kp3/4N3/1P5r/8 b - - 0 54",
    "8/4p2p/3p1k2/p4P1P/r5B1/6K1/8/4R3 b - - 1 44",
    "1R6/3p2pk/8/4Pp2/5P1p/6PK/2q4P/8 b - - 1 52",
    "8/p1p5/1p1p4/8/1P1kPK2/P7/2P3P1/8 b - - 7 41",
    "8/6k1/7p/1p2r3/1q6/p7/P1R3PP/5R1K w - - 2 38",
    "k1b5/1rPp4/1p1P3p/1p1N3P/p4Q2/8/2BK1R2/8 w - - 2 51",
    "8/p1Bp2kp/3P1p2/4rPp1/4P1P1/2R1KQ2/8/1q6 b - - 4 44",
    "1r3k2/8/Q3pq2/1p6/1P6/KR4b1/P1r5/8 b - - 1 44",
    "8/1p3p1k/p1p2Pp1/r1P2bPp/8/2K1B3/8/4R2B b - - 1 54",
    "4b3/5k2/3R3p/5p1P/p4P1K/Pr5P/8/8 w - - 11 53",
    "3r1k2/R4p1p/5Pp1/1p4P1/1P1NK1P1/8/1P6/8 b - - 6 49",
    "4k3/p4p1p/Bp4pP/1P1p2B1/4b3/2K1P3/8/8 b - - 2 40",
    "8/8/2k5/8/1RP2B1p/1P3P1P/7K/8 w - - 0 52",
    "2k2B2/6R1/5p2/3Bpb1R/p3P3/P1p3K1/2P5/8 b - - 1 50",
    "r7/2bk4/n3p3/3bP1p1/1p1B2P1/1p1P4/1P2K3/5R2 b - - 1 51"
};

uint64_t run_bench(int depth) {
    TranspositionTable table(16);
    auto engine = std::make_unique<SearchEngine>(&table);
    engine->set_threads(1);
    engine->set_book_enabled(false);
    engine->set_uci_output(false);
    uint64_t nodes = 0;
    const auto start = std::chrono::steady_clock::now();
    int index = 0;
    for (const char* fen : BenchFens) {
        Board board;
        board.load_fen(fen);
        table.clear();
        engine->clear();
        const auto result = engine->search_iterative_deepening(board, depth, 0.0);
        nodes += result.metrics.total_nodes;
        std::cout << "position " << ++index << ": " << result.metrics.total_nodes << " nodes, bestmove "
                  << move_to_uci(result.best_move) << "\n";
    }
    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    const uint64_t nps = seconds > 0 ? static_cast<uint64_t>(nodes / seconds) : 0;
    std::cout << nodes << " nodes " << nps << " nps" << std::endl;
    return nodes;
}

} // namespace heavensgate
