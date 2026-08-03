#include "test.hpp"
#include "../src/evaluation/eval.hpp"
#include "../src/evaluation/pst.hpp"
#include "../src/core/fen.hpp"
#include <iostream>

namespace heavensgate::test {

static bool test_material_eval_startpos() {
    Board board;
    FEN::parse(StartposFEN, board);
    int eval = Evaluator::evaluate_material(board);
    return eval == 0; // Equal material at startpos
}

static bool test_material_eval_advantage() {
    Board board;
    // White is up a Queen (Q on e4)
    std::string fen = "rnb1kbnr/pppp1ppp/8/8/4Q3/8/PPPP1PPP/RNB1KBNR w KQkq - 0 1";
    FEN::parse(fen, board);

    int eval_white = Evaluator::evaluate_material(board);
    
    board.set_side_to_move(Color::Black);
    int eval_black = Evaluator::evaluate_material(board);

    return eval_white > 0 && eval_black < 0 && eval_white == -eval_black;
}

static bool test_pst_center_pawn_bonus() {
    Board board;
    // Position 1: White pawn on e4 (center control)
    std::string fen_e4 = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1";
    // Position 2: White pawn on h3 (edge push)
    std::string fen_h3 = "rnbqkbnr/pppppppp/8/8/7P/8/PPPPPPP1/RNBQKBNR w KQkq - 0 1";

    Board board_e4, board_h3;
    FEN::parse(fen_e4, board_e4);
    FEN::parse(fen_h3, board_h3);

    int eval_e4 = Evaluator::evaluate(board_e4);
    int eval_h3 = Evaluator::evaluate(board_h3);

    // Central e4 pawn push must evaluate higher than edge h3 pawn push
    return eval_e4 > eval_h3;
}

static bool test_game_phase_calculation() {
    Board board;
    FEN::parse(StartposFEN, board);
    int phase = Evaluator::calculate_game_phase(board);
    return phase == 24; // Full Midgame phase at startpos
}

static bool dummy_eval_init = []() {
    register_test("Eval: Startpos material symmetry (0 cp)", test_material_eval_startpos);
    register_test("Eval: Material advantage calculation & color symmetry", test_material_eval_advantage);
    register_test("Eval: Piece-Square Table center pawn bonus (e4 > h3)", test_pst_center_pawn_bonus);
    register_test("Eval: Game phase calculation (Startpos = 24)", test_game_phase_calculation);
    return true;
}();

} // namespace heavensgate::test
