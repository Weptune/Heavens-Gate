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
    FEN::parse(FEN::StartPOS, board);

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

static bool test_magic_bitboard_consistency() {
    Magic::init();
    Board board;
    board.reset();

    Square sq = Square::d4;
    Bitboard occ = board.occupied();

    Bitboard b_attacks = Magic::get_bishop_attacks(sq, occ);
    Bitboard r_attacks = Magic::get_rook_attacks(sq, occ);

    return (b_attacks != EmptyBB) && (r_attacks != EmptyBB);
}

} // namespace heavensgate::test

namespace heavensgate {

void test_movegen() {
    std::cout << "[RUN] Movegen: Startpos legal move count (20 moves) ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_startpos_movecount(), "Startpos legal move count must be exactly 20!");
    std::cout << "PASSED" << std::endl;

    std::cout << "[RUN] Movegen: Check detection ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_in_check_detection(), "Check detection failed!");
    std::cout << "PASSED" << std::endl;

    std::cout << "[RUN] Movegen: Magic Bitboard sliding attacks ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_magic_bitboard_consistency(), "Magic bitboard attacks failed!");
    std::cout << "PASSED" << std::endl;
}

} // namespace heavensgate
