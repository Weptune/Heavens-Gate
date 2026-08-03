#include "test.hpp"
#include "../src/search/search.hpp"
#include "../src/core/fen.hpp"
#include <iostream>

namespace heavensgate::test {

static bool test_minimax_mate_in_1() {
    MoveGenerator::init();
    Board board;
    // Scholar's mate position: White queen on f7 delivers checkmate (Qxf7#)
    std::string fen = "r1bqkb1r/pppp1ppp/2n5/4p3/2B1n3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 0 4";
    FEN::parse(fen, board);

    SearchEngine engine;
    SearchResult res = engine.search_minimax(board, 2);

    return res.best_move == Move(Square::f3, Square::f7, MoveType::Capture) && res.best_score > 20000;
}

static bool test_minimax_free_piece_capture() {
    MoveGenerator::init();
    Board board;
    // Undefended pawn on e5, White knight on f3 can capture it (Nxe5)
    std::string fen = "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 3";
    FEN::parse(fen, board);

    SearchEngine engine;
    SearchResult res = engine.search_minimax(board, 2);

    return res.best_move == Move(Square::f3, Square::e5, MoveType::Capture) && res.best_score > 0;
}

static bool test_alphabeta_eval_equivalence() {
    MoveGenerator::init();
    Board board;
    FEN::parse(StartposFEN, board);

    SearchEngine engine;
    SearchResult mm_res = engine.search_minimax(board, 3);
    SearchResult ab_res = engine.search_alphabeta(board, 3, false);

    return mm_res.best_score == ab_res.best_score && mm_res.best_move == ab_res.best_move;
}

static bool test_alphabeta_node_reduction() {
    MoveGenerator::init();
    Board board;
    FEN::parse(StartposFEN, board);

    SearchEngine engine;
    SearchResult mm_res = engine.search_minimax(board, 4);
    SearchResult ab_res = engine.search_alphabeta(board, 4, false);

    return ab_res.metrics.total_nodes < mm_res.metrics.total_nodes &&
           mm_res.best_score == ab_res.best_score;
}

static bool test_move_ordering_reduction() {
    MoveGenerator::init();
    Board board;
    FEN::parse(StartposFEN, board);

    SearchEngine engine;
    SearchResult raw_res = engine.search_alphabeta(board, 4, false);
    SearchResult ord_res = engine.search_alphabeta(board, 4, true);

    return ord_res.metrics.total_nodes < raw_res.metrics.total_nodes &&
           raw_res.best_score == ord_res.best_score;
}

static bool test_iterative_deepening_search() {
    MoveGenerator::init();
    Board board;
    FEN::parse(StartposFEN, board);

    SearchEngine engine;
    SearchResult res = engine.search_iterative_deepening(board, 4);

    return res.completed_depth == 4 && static_cast<bool>(res.best_move);
}

static bool dummy_search_init = []() {
    register_test("Search: Minimax finds Mate-in-1 (Qxf7#)", test_minimax_mate_in_1);
    register_test("Search: Minimax captures undefended piece (Nxe5)", test_minimax_free_piece_capture);
    register_test("Search: Alpha-Beta Score & Best Move Equivalence", test_alphabeta_eval_equivalence);
    register_test("Search: Alpha-Beta Node Count Reduction vs Minimax", test_alphabeta_node_reduction);
    register_test("Search: Move Ordering Node Reduction vs Raw Alpha-Beta", test_move_ordering_reduction);
    register_test("Search: Iterative Deepening Search Completion (v5.0)", test_iterative_deepening_search);
    return true;
}();

} // namespace heavensgate::test
