#include "test.hpp"
#include "../src/movegen/movegen.hpp"
#include "../src/core/fen.hpp"
#include <iostream>

namespace heavensgate::test {

static bool test_startpos_move_count() {
    MoveGenerator::init();
    Board board;
    FEN::parse(StartposFEN, board);

    MoveList moves;
    MoveGenerator::generate_legal_moves(board, moves);
    return moves.size() == 20;
}

static bool test_check_detection() {
    MoveGenerator::init();
    Board board;
    // Position where White king on e1 is in check from Black queen on e5 (e-file completely open)
    std::string fen = "rnb1kbnr/pppp1ppp/8/4q3/8/8/PPP2PPP/RNBQKBNR w KQkq - 0 1";
    FEN::parse(fen, board);

    return MoveGenerator::in_check(board, Color::White) && !MoveGenerator::in_check(board, Color::Black);
}

static bool dummy_movegen_init = []() {
    register_test("MoveGen: Startpos legal move count (20)", test_startpos_move_count);
    register_test("MoveGen: In-Check detection logic", test_check_detection);
    return true;
}();

} // namespace heavensgate::test
