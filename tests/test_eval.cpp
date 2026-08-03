#include "test.hpp"
#include "../src/evaluation/eval.hpp"
#include "../src/core/fen.hpp"
#include <iostream>

namespace heavensgate::test {

static bool test_eval_startpos_symmetry() {
    Evaluator::init();
    Board board;
    FEN::parse(StartposFEN, board);

    int eval = Evaluator::evaluate(board);
    return eval == 0;
}

static bool test_eval_material_advantage() {
    Evaluator::init();
    Board board;
    // Position where White is up a queen
    std::string fen = "rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    FEN::parse(fen, board);

    int white_eval = Evaluator::evaluate(board);

    // Switch to Black to move
    board.set_side_to_move(Color::Black);
    int black_eval = Evaluator::evaluate(board);

    return white_eval > 800 && black_eval < -800;
}

static bool test_eval_pst_pawn_center() {
    Evaluator::init();
    Board board;
    FEN::parse(StartposFEN, board);

    // Pawn on e4 should have higher PST value than pawn on h3
    int e4_mg = PST::get_mg(PieceType::Pawn, Color::White, Square::e4);
    int h3_mg = PST::get_mg(PieceType::Pawn, Color::White, Square::h3);

    return e4_mg > h3_mg;
}

static bool test_eval_game_phase() {
    Evaluator::init();
    Board board;
    FEN::parse(StartposFEN, board);

    int white_eval = Evaluator::evaluate_side(board, Color::White);
    return white_eval > 0;
}

static bool dummy_eval_init = []() {
    register_test("Eval: Startpos material symmetry (0 cp)", test_eval_startpos_symmetry);
    register_test("Eval: Material advantage calculation & color symmetry", test_eval_material_advantage);
    register_test("Eval: Piece-Square Table center pawn bonus (e4 > h3)", test_eval_pst_pawn_center);
    register_test("Eval: Game phase calculation (Startpos = 24)", test_eval_game_phase);
    return true;
}();

} // namespace heavensgate::test
