#include "eval.hpp"
#include "pst.hpp"
#include "nnue.hpp"
#include "tensor_eval.hpp"
#include "tensor_quant.hpp"
#include "tensor_nnue.hpp"
#include "spectral_graph.hpp"
#include "tropical_eval.hpp"
#include <algorithm>

namespace heavensgate {

thread_local EvalMode Evaluator::current_mode_ = EvalMode::SpectralTropical;

void Evaluator::init() {
    PST::init();
    EvalFeatures::init();
    NNUEEvaluator::init();
    TensorMPS::instance().load_weights("heavensgate.tnw");
    TensorMPSQuantized::instance().quantize_from(TensorMPS::instance());
    TensorNNUE::instance().load_weights("heavensgate_tnnue.nnue");
    TropicalEvaluator::instance().load_weights("heavensgate_tropical.trm");
}

int Evaluator::evaluate_side(const Board& board, Color side) {
    int mg_material = 0;
    int eg_material = 0;

    int mg_pst = 0;
    int eg_pst = 0;

    auto score_piece = [&](PieceType pt, int mg_val, int eg_val) {
        Piece p = make_piece(side, pt);
        Bitboard bb = board.pieces(p);

        while (bb) {
            Square sq = pop_lsb(bb);
            mg_material += mg_val;
            eg_material += eg_val;

            mg_pst += PST::get_mg(pt, side, sq);
            eg_pst += PST::get_eg(pt, side, sq);
        }
    };

    score_piece(PieceType::Pawn,   100, 120);
    score_piece(PieceType::Knight, 320, 310);
    score_piece(PieceType::Bishop, 330, 340);
    score_piece(PieceType::Rook,   500, 530);
    score_piece(PieceType::Queen,  900, 950);
    score_piece(PieceType::King,     0,   0);

    if (current_mode_ == EvalMode::MaterialOnly) {
        return mg_material + mg_pst;
    }

    // Master Positional Features
    int pawn_struct  = EvalFeatures::evaluate_pawn_structure(board, side);
    int passed_pawns = EvalFeatures::evaluate_passed_pawns(board, side);
    int king_safety  = EvalFeatures::evaluate_king_safety(board, side);
    int activity     = EvalFeatures::evaluate_piece_activity(board, side);
    int mobility     = EvalFeatures::evaluate_mobility(board, side);

    int pos_total = pawn_struct + passed_pawns + king_safety + activity + mobility;

    int knights = popcount(board.pieces(make_piece(side, PieceType::Knight)));
    int bishops = popcount(board.pieces(make_piece(side, PieceType::Bishop)));
    int rooks   = popcount(board.pieces(make_piece(side, PieceType::Rook)));
    int queens  = popcount(board.pieces(make_piece(side, PieceType::Queen)));
    int game_phase = knights * 1 + bishops * 1 + rooks * 2 + queens * 4;

    int mg_total = mg_material + mg_pst + pos_total;
    int eg_total = eg_material + eg_pst + pos_total;

    int mg_weight = std::min(game_phase, 24);
    int eg_weight = 24 - mg_weight;

    return (mg_total * mg_weight + eg_total * eg_weight) / 24;
}

int Evaluator::evaluate(const Board& board) {
    if (current_mode_ == EvalMode::NNUE) {
        return NNUEEvaluator::evaluate(board, const_cast<Board&>(board).accumulator());
    }

    if (current_mode_ == EvalMode::TensorNetwork) {
        return TensorNNUE::instance().evaluate(board);
    }

    if (current_mode_ == EvalMode::SpectralTropical) {
        return TropicalEvaluator::instance().evaluate(board);
    }

    int white_score = evaluate_side(board, Color::White);
    int black_score = evaluate_side(board, Color::Black);

    int relative_score = white_score - black_score;
    return (board.side_to_move() == Color::White) ? relative_score : -relative_score;
}

static constexpr int MAX_SEARCH_PLY = 256;
static TensorMPSQuantized::QuantizedEnvironment s_quant_env[MAX_SEARCH_PLY];

void Evaluator::reset_incremental_cache() {
    for (int i = 0; i < MAX_SEARCH_PLY; i++) {
        s_quant_env[i].valid_up_to = -1;
    }
}

int Evaluator::evaluate_incremental(const Board& board, int ply, Square from_sq, Square to_sq) {
    if (current_mode_ != EvalMode::TensorNetwork) {
        return evaluate(board);
    }

    int idx = std::max(0, std::min(MAX_SEARCH_PLY - 1, ply));

    if (idx == 0 || from_sq == Square::None || to_sq == Square::None || s_quant_env[idx - 1].valid_up_to < 0) {
        s_quant_env[idx].valid_up_to = -1;
        return TensorMPSQuantized::instance().evaluate_incremental(board, s_quant_env[idx]);
    }

    const auto& inv_hilbert = HilbertCurve::inverse_order();
    int site_from = inv_hilbert[static_cast<int>(from_sq)];
    int site_to   = inv_hilbert[static_cast<int>(to_sq)];
    int min_site  = std::min(site_from, site_to);

    s_quant_env[idx] = s_quant_env[idx - 1];
    s_quant_env[idx].valid_up_to = min_site;

    return TensorMPSQuantized::instance().evaluate_incremental(board, s_quant_env[idx]);
}

} // namespace heavensgate
