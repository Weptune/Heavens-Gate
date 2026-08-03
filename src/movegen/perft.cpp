#include "perft.hpp"
#include "../core/fen.hpp"
#include <iostream>
#include <iomanip>

namespace heavensgate {

uint64_t Perft::perft(Board& board, int depth) {
    if (depth <= 0) return 1ULL;

    MoveList moves;
    MoveGenerator::generate_legal_moves(board, moves);

    if (depth == 1) return moves.size();

    uint64_t nodes = 0;
    for (const auto& m : moves) {
        board.make_move(m);
        nodes += perft(board, depth - 1);
        board.unmake_move(m);
    }
    return nodes;
}

PerftResults Perft::perft_detailed(Board& board, int depth) {
    auto start = std::chrono::high_resolution_clock::now();
    
    PerftResults results;
    results.nodes = perft(board, depth);

    auto end = std::chrono::high_resolution_clock::now();
    results.duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
    if (results.duration_ms > 0) {
        results.nps = (results.nodes * 1000.0) / results.duration_ms;
    }

    return results;
}

void Perft::divide(Board& board, int depth) {
    if (depth <= 0) return;

    MoveList moves;
    MoveGenerator::generate_legal_moves(board, moves);

    uint64_t total_nodes = 0;
    std::cout << "\nPerft Divide (Depth " << depth << "):\n";
    std::cout << "------------------------------------\n";

    for (const auto& m : moves) {
        board.make_move(m);
        uint64_t nodes = (depth == 1) ? 1ULL : perft(board, depth - 1);
        board.unmake_move(m);

        total_nodes += nodes;
        std::cout << move_to_uci(m) << ": " << nodes << "\n";
    }

    std::cout << "------------------------------------\n";
    std::cout << "Total Moves: " << moves.size() << "\n";
    std::cout << "Total Nodes: " << total_nodes << "\n\n";
}

const std::vector<PerftPosition>& Perft::standard_positions() {
    static const std::vector<PerftPosition> positions = {
        {
            "Initial Position",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            {20ULL, 400ULL, 8902ULL, 197281ULL, 4865609ULL, 119060324ULL}
        },
        {
            "Kiweteam Position",
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            {48ULL, 2039ULL, 97862ULL, 4085603ULL, 193690690ULL}
        },
        {
            "Position 3 (Endgame)",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            {14ULL, 191ULL, 2812ULL, 43238ULL, 674624ULL, 11030083ULL}
        },
        {
            "Position 4 (Promotions & Castling)",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            {6ULL, 264ULL, 9467ULL, 422333ULL, 15833292ULL}
        },
        {
            "Position 5 (Tactical Pins & Checks)",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            {44ULL, 1486ULL, 62379ULL, 2103487ULL, 89941194ULL}
        },
        {
            "Position 6 (Midgame Complexity)",
            "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
            {46ULL, 2079ULL, 89890ULL, 3894594ULL, 164075551ULL}
        }
    };
    return positions;
}

bool Perft::run_verification_suite(int max_depth) {
    MoveGenerator::init();
    Board board;
    bool all_passed = true;

    std::cout << "\n======================================================\n";
    std::cout << "       HEAVEN'S GATE PERFT VERIFICATION SUITE         \n";
    std::cout << "======================================================\n";

    for (const auto& pos : standard_positions()) {
        std::cout << "\n[Testing] " << pos.name << "\n";
        std::cout << "FEN: " << pos.fen << "\n";

        if (!FEN::parse(pos.fen, board)) {
            std::cout << "ERROR: Failed to parse FEN string!\n";
            all_passed = false;
            continue;
        }

        int target_depth = std::min(max_depth, static_cast<int>(pos.expected_nodes.size()));
        for (int d = 1; d <= target_depth; ++d) {
            uint64_t expected = pos.expected_nodes[d - 1];
            PerftResults res = perft_detailed(board, d);

            bool pass = (res.nodes == expected);
            std::cout << "  Depth " << d << ": " 
                      << res.nodes << " / " << expected << " nodes "
                      << "in " << std::fixed << std::setprecision(2) << res.duration_ms << " ms "
                      << "(" << static_cast<uint64_t>(res.nps) << " NPS) "
                      << "[" << (pass ? "PASSED" : "FAILED") << "]\n";

            if (!pass) {
                all_passed = false;
            }
        }
    }

    std::cout << "\n------------------------------------------------------\n";
    std::cout << "VERIFICATION SUITE RESULT: " << (all_passed ? "ALL PASSED" : "FAILED") << "\n";
    std::cout << "------------------------------------------------------\n\n";

    return all_passed;
}

} // namespace heavensgate
