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

static bool test_en_passant_horizontal_pin() {
    MoveGenerator::init();
    Board board;
    // White King on a5, White Pawn on d5, Black Pawn on c5 (just moved c7-c5, ep square c6), Black Rook on h5
    // Capturing en-passant d5xc6 is ILLEGAL because it exposes White King on a5 to the Black Rook on h5 along rank 5!
    std::string fen = "8/8/8/K1pP3r/8/8/8/k7 w - c6 0 1";
    FEN::parse(fen, board);

    MoveList moves;
    MoveGenerator::generate_legal_moves(board, moves);

    for (const auto& m : moves) {
        if (m.is_ep()) {
            return false; // En-passant capture should be illegal due to horizontal pin!
        }
    }
    return true;
}

static bool test_absolute_pin_legality() {
    MoveGenerator::init();
    Board board;
    // White King on e1, White Knight on e2, Black Queen on e8
    // The White Knight on e2 is absolutely pinned along the e-file and cannot move!
    std::string fen = "4q3/8/8/8/8/8/4N3/4K3 w - - 0 1";
    FEN::parse(fen, board);

    MoveList moves;
    MoveGenerator::generate_legal_moves(board, moves);

    for (const auto& m : moves) {
        if (m.from() == Square::e2) {
            return false; // Pinned knight cannot move!
        }
    }
    return true;
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

    std::cout << "[RUN] Movegen: En-passant horizontal pin legality ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_en_passant_horizontal_pin(), "En-passant horizontal pin test failed!");
    std::cout << "PASSED" << std::endl;

    std::cout << "[RUN] Movegen: Absolute pin legality ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_absolute_pin_legality(), "Absolute pin test failed!");
    std::cout << "PASSED" << std::endl;
}

} // namespace heavensgate
