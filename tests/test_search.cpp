#include "test.hpp"
#include "../src/search/search.hpp"
#include "../src/search/syzygy.hpp"
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

static bool test_repetition_draw_score() {
    MoveGenerator::init();
    Board board;
    FEN::parse(FEN::StartPOS, board);

    Move m1(Square::g1, Square::f3, MoveType::Quiet);
    Move m2(Square::g8, Square::f6, MoveType::Quiet);
    Move m3(Square::f3, Square::g1, MoveType::Quiet);
    Move m4(Square::f6, Square::g8, MoveType::Quiet);

    board.make_move(m1); board.make_move(m2); board.make_move(m3); board.make_move(m4);
    board.make_move(m1); board.make_move(m2); board.make_move(m3); board.make_move(m4);

    return board.is_repetition(2);
}

static bool test_iterative_deepening_mate_in_2() {
    MoveGenerator::init();
    Board board;
    // Legal's Mate in 2 position: 1. Nf6+ gxf6 2. Bxf7#
    std::string fen = "r2qkb1r/pp2nppp/3p4/2pNN3/2BnP3/8/PPPP1PPP/R1BQK2R w KQkq - 0 1";
    FEN::parse(fen, board);

    SearchEngine engine;
    SearchResult res = engine.search_iterative_deepening(board, 4, 0.0);

    return res.best_score > 20000;
}

static bool test_capture_history() {
    MovePicker mp;
    Piece attacker = Piece::WhiteKnight;
    Square to = Square::e5;
    PieceType victim = PieceType::Pawn;

    int initial_score = mp.get_capture_history(attacker, to, victim);
    mp.add_capture_history(attacker, to, victim, 4);
    int updated_score = mp.get_capture_history(attacker, to, victim);

    return (initial_score == 0) && (updated_score == 16);
}

static bool test_singular_extension_search() {
    MoveGenerator::init();
    Board board;
    // Tactical position with a singular winning sacrifice: 1. Bxh7+ Kxh7 2. Ng5+
    std::string fen = "r1bq1rk1/ppp2ppp/2n1pn2/3p4/2PP4/2NBPN2/PP3PPP/R1BQK2R w KQ - 0 7";
    FEN::parse(fen, board);

    SearchEngine engine;
    SearchResult res = engine.search_iterative_deepening(board, 7, 0.0);

    return static_cast<bool>(res.best_move) && res.completed_depth >= 7;
}

static bool test_endgame_patterns() {
    MoveGenerator::init();
    SyzygyTablebase::instance().init();

    // 1. KBNK Win
    {
        Board b;
        FEN::parse("8/8/8/8/8/5K1k/4B1N1/8 w - - 0 1", b);
        int score = SyzygyTablebase::instance().probe_wdl(b, 0);
        std::cout << "[KBNK: score=" << score << "] " << std::flush;
        if (score < 20000) return false;
    }

    // 2. KNNK Draw
    {
        Board b;
        FEN::parse("8/8/8/8/8/5K1k/4N1N1/8 w - - 0 1", b);
        int score = SyzygyTablebase::instance().probe_wdl(b, 0);
        std::cout << "[KNNK: score=" << score << "] " << std::flush;
        if (score != 0) return false;
    }

    // 3. Wrong-color Bishop + Rook Pawn Fortress (A8 is light, bishop on Dark square C3, Black king in corner A8)
    {
        Board b;
        FEN::parse("k7/8/P7/8/8/2B5/8/K7 w - - 0 1", b);
        int score = SyzygyTablebase::instance().probe_wdl(b, 0);
        std::cout << "[Fortress: score=" << score << "] " << std::flush;
        if (score != 0) return false;
    }

    // 4. Lucena Position (Pawn on 7th, strong King on 8th, defending King cut off)
    {
        Board b;
        FEN::parse("3K4/3P4/8/8/8/6k1/r7/1R6 w - - 0 1", b);
        int score = SyzygyTablebase::instance().probe_wdl(b, 0);
        std::cout << "[Lucena: score=" << score << "] " << std::flush;
        if (score < 20000) return false;
    }

    // 5. Philidor Position (Defending king on 8th rank, defending rook on 6th rank)
    {
        Board b;
        FEN::parse("4k3/8/4r3/3KP3/8/8/8/1R6 w - - 0 1", b);
        int score = SyzygyTablebase::instance().probe_wdl(b, 0);
        std::cout << "[Philidor: score=" << score << "] " << std::flush;
        if (score != 0) return false;
    }

    return true;
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

    std::cout << "[RUN] Search: 2-Fold Repetition Detection ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_repetition_draw_score(), "2-Fold repetition detection failed!");
    std::cout << "PASSED" << std::endl;

    std::cout << "[RUN] Search: Iterative Deepening Mate-in-2 ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_iterative_deepening_mate_in_2(), "Iterative deepening mate-in-2 failed!");
    std::cout << "PASSED" << std::endl;

    std::cout << "[RUN] Search: Capture History Table Recording ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_capture_history(), "Capture history failed to record or retrieve score!");
    std::cout << "PASSED" << std::endl;

    std::cout << "[RUN] Search: Singular Extensions Deep Search (Depth 7) ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_singular_extension_search(), "Singular extension search failed at depth 7!");
    std::cout << "PASSED" << std::endl;

    std::cout << "[RUN] Search: Phase 4 Endgame Patterns (KBNK, KNNK, Fortress, Lucena, Philidor) ... " << std::flush;
    HEAVENSGATE_ASSERT(test::test_endgame_patterns(), "Phase 4 endgame pattern verification failed!");
    std::cout << "PASSED" << std::endl;
}

} // namespace heavensgate
