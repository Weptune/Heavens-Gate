#include "test.hpp"
#include "../src/evaluation/eval.hpp"
#include "../src/core/fen.hpp"
#include <iostream>

namespace heavensgate::test {

static bool test_material_eval_startpos() {
    Board board;
    FEN::parse(StartposFEN, board);
    int eval = Evaluator::evaluate(board);
    return eval == 0; // Equal material at startpos
}

static bool test_material_eval_advantage() {
    Board board;
    // White is up a Queen (Q on e4)
    std::string fen = "rnb1kbnr/pppp1ppp/8/8/4Q3/8/PPPP1PPP/RNB1KBNR w KQkq - 0 1";
    FEN::parse(fen, board);

    int eval_white = Evaluator::evaluate(board);
    
    board.set_side_to_move(Color::Black);
    int eval_black = Evaluator::evaluate(board);

    return eval_white > 0 && eval_black < 0 && eval_white == -eval_black;
}

static bool dummy_eval_init = []() {
    register_test("Eval: Startpos material symmetry (0 cp)", test_material_eval_startpos);
    register_test("Eval: Material advantage calculation & color symmetry", test_material_eval_advantage);
    return true;
}();

} // namespace heavensgate::test
