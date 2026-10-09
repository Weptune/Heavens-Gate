#include "test.hpp"
#include "../src/evaluation/eval.hpp"
#include "../src/evaluation/eval_params.hpp"
#include "../src/core/fen.hpp"

namespace heavensgate {
static Board color_mirror(const Board& board) {
    Board mirror;
    for (int i = 0; i < 64; ++i) {
        const auto p = board.piece_at(static_cast<Square>(i));
        if (p != Piece::None) mirror.set_piece(static_cast<Square>(i ^ 56), make_piece(~color_of(p), piece_type_of(p)));
    }
    mirror.set_side_to_move(~board.side_to_move());
    const auto rights = board.castling_rights();
    mirror.set_castling_rights(static_cast<CastlingRights>(((rights & 3) << 2) | ((rights & 12) >> 2)));
    mirror.set_en_passant_sq(board.ep_square() == Square::None ? Square::None : static_cast<Square>(static_cast<int>(board.ep_square()) ^ 56));
    mirror.set_halfmove_clock(board.halfmove_clock());
    mirror.set_fullmove_number(board.fullmove_number());
    mirror.recalculate_zobrist_key();
    return mirror;
}

void test_eval() {
    Evaluator::init();
    Evaluator::set_mode(EvalMode::MasterPositional);
    Board board;
    board.reset();
    HEAVENSGATE_ASSERT(g_eval_params.tempo_mg == 15, "Current tuned move-1 tempo changed");
    HEAVENSGATE_ASSERT(Evaluator::evaluate(board) == g_eval_params.tempo_mg, "Startpos must equal configured tempo");
    HEAVENSGATE_ASSERT(board.game_phase() == 24, "Startpos phase must be 24");

    for (const char* fen : {
        FEN::StartPOS.data(),
        "4k3/8/4p3/3N4/3P4/8/8/4K3 w - - 0 1",
        "k7/8/8/8/8/8/7P/5K1R w - - 0 1",
        "4k3/rr6/8/8/8/8/8/3QK3 b - - 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"}) {
        board.load_fen(fen);
        const auto mirror = color_mirror(board);
        HEAVENSGATE_ASSERT(Evaluator::evaluate(board) == Evaluator::evaluate(mirror), "Color-reflected STM scores differ");
        for (const auto feature : {EvalFeatures::evaluate_pawn_structure, EvalFeatures::evaluate_passed_pawns,
                                  EvalFeatures::evaluate_piece_activity, EvalFeatures::evaluate_material_imbalances,
                                  EvalFeatures::evaluate_king_safety, EvalFeatures::evaluate_threats, EvalFeatures::evaluate_mobility}) {
            for (Color side : {Color::White, Color::Black}) {
                const auto a = feature(board, side), b = feature(mirror, ~side);
                HEAVENSGATE_ASSERT(a.mg == b.mg && a.eg == b.eg, "Feature is not color symmetric");
            }
        }
    }

    Board blocked, open;
    blocked.load_fen("4k3/8/4p3/3N4/3P4/8/8/4K3 w - - 0 1");
    open.load_fen("4k3/8/4p3/8/3P4/7N/8/4K3 w - - 0 1");
    const auto a = EvalFeatures::evaluate_pawn_structure(blocked, Color::White);
    const auto b = EvalFeatures::evaluate_pawn_structure(open, Color::White);
    HEAVENSGATE_ASSERT(a.mg == b.mg && a.eg == b.eg, "Pawn cache feature depends on non-pawn blockers");
    Evaluator::reset_incremental_cache();
    const int cold_blocked = Evaluator::evaluate(blocked);
    const int warm_open = Evaluator::evaluate(open);
    Evaluator::reset_incremental_cache();
    HEAVENSGATE_ASSERT(Evaluator::evaluate(open) == warm_open, "Open lever cold/warm cache mismatch");
    HEAVENSGATE_ASSERT(Evaluator::evaluate(blocked) == cold_blocked, "Blocked lever cold/warm cache mismatch");

    board.load_fen("k7/8/8/8/8/8/7P/5K1R w - - 0 1");
    const auto boxed = EvalFeatures::evaluate_piece_activity(board, Color::White);
    board.load_fen("k7/8/8/8/8/8/7P/4K2R w - - 0 1");
    const auto free = EvalFeatures::evaluate_piece_activity(board, Color::White);
    HEAVENSGATE_ASSERT(free.mg - boxed.mg == 45 && free.eg == boxed.eg, "Boxed rook penalty is unreachable");

    board.load_fen("4k3/rr6/8/8/8/8/8/3QK3 w - - 0 1");
    const auto queen = EvalFeatures::evaluate_material_imbalances(board, Color::White);
    const auto rooks = EvalFeatures::evaluate_material_imbalances(board, Color::Black);
    HEAVENSGATE_ASSERT(queen.mg == 15 && queen.eg == -25 && rooks.mg == 0 && rooks.eg == 12,
                      "Queen-vs-rooks interaction must be counted once, separate from rook pair");
    std::cout << "[RUN] Classical eval: tempo, color symmetry, pawn-cache coherence, boxed rooks, single imbalance ... PASSED\n";
}
} // namespace heavensgate
