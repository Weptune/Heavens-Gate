#pragma once

#include "../board/board.hpp"
#include "movegen.hpp"
#include <cstdint>
#include <vector>
#include <string>
#include <chrono>

namespace heavensgate {

struct PerftResults {
    uint64_t nodes{0};
    uint64_t captures{0};
    uint64_t en_passants{0};
    uint64_t castles{0};
    uint64_t promotions{0};
    uint64_t checks{0};
    double duration_ms{0.0};
    double nps{0.0};
};

struct PerftPosition {
    std::string name;
    std::string fen;
    std::vector<uint64_t> expected_nodes; // Indexed by (depth - 1)
};

class Perft {
public:
    static uint64_t perft(Board& board, int depth);
    static PerftResults perft_detailed(Board& board, int depth);
    static void divide(Board& board, int depth);
    static bool run_verification_suite(int max_depth = 4);

    static const std::vector<PerftPosition>& standard_positions();
};

} // namespace heavensgate
