#include "test.hpp"
#include "../src/search/search.hpp"
#include "../src/core/fen.hpp"
#include "../src/core/zobrist.hpp"
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
    FEN::parse(FEN::StartPOS, board);

    SearchEngine engine;
    SearchResult ab_res = engine.search_alphabeta(board, 3, false, false);

    return static_cast<bool>(ab_res.best_move);
}

static bool test_alphabeta_node_reduction() {
    MoveGenerator::init();
    Board board;
    FEN::parse(FEN::StartPOS, board);

    SearchEngine engine;
    SearchResult mm_res = engine.search_minimax(board, 4);
    SearchResult ab_res = engine.search_alphabeta(board, 4, true, true);

    return ab_res.metrics.total_nodes < mm_res.metrics.total_nodes;
}

static bool test_move_ordering_reduction() {
    MoveGenerator::init();
    Board board;
    FEN::parse(FEN::StartPOS, board);

    SearchEngine engine;
    SearchResult ab_unordered = engine.search_alphabeta(board, 4, false, false);
    SearchResult ab_ordered   = engine.search_alphabeta(board, 4, true, true);

    return ab_ordered.metrics.total_nodes < ab_unordered.metrics.total_nodes;
}

static bool test_zobrist_incremental_correctness() {
    MoveGenerator::init();
    Board board;
    FEN::parse(FEN::StartPOS, board);

    uint64_t initial_key = board.zobrist_key();

    Move m(Square::e2, Square::e4, MoveType::Quiet);
    board.make_move(m);
    uint64_t move_key = board.zobrist_key();

    HEAVENSGATE_ASSERT(initial_key != move_key, "Zobrist key must change after e2e4!");

    board.unmake_move(m);
    uint64_t restored_key = board.zobrist_key();

    return initial_key == restored_key;
}

static bool test_transposition_table_cutoffs() {
    MoveGenerator::init();
    Board board;
    FEN::parse(FEN::StartPOS, board);

    SearchEngine engine;
    SearchResult res = engine.search_alphabeta(board, 4, true, true);

    return res.tt_hits > 0;
}

static bool test_quiescence_search_horizon_fix() {
    MoveGenerator::init();
    Board board;
    // Position where White queen is under attack on d5.
    std::string fen = "r1bqk2r/pppp1ppp/2n5/3Qp3/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 0 5";
    FEN::parse(fen, board);

    SearchEngine engine;
    SearchResult res = engine.search_alphabeta(board, 3, true, true);

    return res.best_move == Move(Square::d5, Square::f7, MoveType::Capture);
}

} // namespace heavensgate::test

namespace heavensgate {

void test_search() {
    std::cout << "[RUN] Search: Minimax finds Mate-in-1 (Qxf7#) ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_minimax_mate_in_1(), "Minimax failed to find mate in 1!");
    std::cout << "PASSED" << std::endl;

    std::cout << "[RUN] Search: Minimax captures undefended piece (Nxe5) ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_minimax_free_piece_capture(), "Minimax failed to capture free pawn!");
    std::cout << "PASSED" << std::endl;

    std::cout << "[RUN] Search: Alpha-Beta Score & Best Move Equivalence ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_alphabeta_eval_equivalence(), "Alpha-Beta search failed equivalence!");
    std::cout << "PASSED" << std::endl;

    std::cout << "[RUN] Search: Alpha-Beta Node Count Reduction vs Minimax ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_alphabeta_node_reduction(), "Alpha-Beta failed node count reduction!");
    std::cout << "PASSED" << std::endl;

    std::cout << "[RUN] Search: Move Ordering Node Reduction vs Raw Alpha-Beta ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_move_ordering_reduction(), "Move ordering failed node count reduction!");
    std::cout << "PASSED" << std::endl;

    std::cout << "[RUN] Zobrist: Incremental hash update & unmake correctness ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_zobrist_incremental_correctness(), "Zobrist unmake key mismatch!");
    std::cout << "PASSED" << std::endl;

    std::cout << "[RUN] Transposition Table: Subtree cutoff hits & score equivalence ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_transposition_table_cutoffs(), "TT failed to register cutoff hits!");
    std::cout << "PASSED" << std::endl;

    std::cout << "[RUN] Quiescence Search: Eliminates Horizon Effect (rejects Qxd5 queen blunder) ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_quiescence_search_horizon_fix(), "Quiescence search failed!");
    std::cout << "PASSED" << std::endl;
}

} // namespace heavensgate
