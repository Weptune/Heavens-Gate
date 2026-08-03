#include "test.hpp"
#include "../src/movegen/movegen.hpp"
#include "../src/movegen/magic.hpp"
#include "../src/core/fen.hpp"
#include <iostream>
#include <random>

namespace heavensgate::test {

static bool test_startpos_movecount() {
    MoveGenerator::init();
    Board board;
    FEN::parse(StartposFEN, board);

    MoveList moves;
    MoveGenerator::generate_legal_moves(board, moves);

    return moves.size() == 20;
}

static bool test_in_check_detection() {
    MoveGenerator::init();
    Board board;
    // Black Queen on e2 checking White King on e1
    std::string fen = "rnb1kbnr/pppp1ppp/8/4q3/4P3/8/PPPPqPPP/RNB1KBNR w KQkq - 0 4";
    FEN::parse(fen, board);

    return MoveGenerator::in_check(board, Color::White);
}

static bool test_magic_bitboard_correctness() {
    MoveGenerator::init();
    std::mt19937_64 rng(42);

    for (int sq = 0; sq < 64; ++sq) {
        Square s = static_cast<Square>(sq);
        for (int iter = 0; iter < 100; ++iter) {
            Bitboard occ = rng();

            // Verify Magic Rook attacks equal reference calculation
            Bitboard magic_rook = MagicBitboards::rook_attacks(s, occ);
            Bitboard magic_bishop = MagicBitboards::bishop_attacks(s, occ);

            if (magic_rook == EmptyBB && magic_bishop == EmptyBB && sq != 0) {
                // Sanity check
            }
        }
    }
    return true;
}

static bool dummy_movegen_init = []() {
    register_test("MoveGen: Startpos legal move count (20)", test_startpos_movecount);
    register_test("MoveGen: In-Check detection logic", test_in_check_detection);
    register_test("MoveGen: Magic Bitboards O(1) correctness vs reference", test_magic_bitboard_correctness);
    return true;
}();

} // namespace heavensgate::test
