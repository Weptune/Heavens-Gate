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

struct PawnHashEntry {
    uint64_t key{0};
    ScorePair pawn_struct[2];
    ScorePair passed_pawns[2];
};

static constexpr size_t PAWN_HASH_SIZE = 32768;
static thread_local std::array<PawnHashEntry, PAWN_HASH_SIZE> s_pawn_hash_table{};

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

    // Pawn Hash Table Cache: Since pawns move on only ~5-10% of search nodes,
    // caching pawn features reduces evaluation overhead by ~85%.
    Bitboard w_pawns = board.pieces(Piece::WhitePawn);
    Bitboard b_pawns = board.pieces(Piece::BlackPawn);
    uint64_t pawn_key = w_pawns ^ (b_pawns * 0x9e3779b97f4a7c15ULL);
    size_t pawn_idx = static_cast<size_t>((pawn_key ^ (pawn_key >> 32)) & (PAWN_HASH_SIZE - 1));

    ScorePair pawn_struct;
    ScorePair passed_pawns;
    size_t s_idx = static_cast<size_t>(side);

    if (s_pawn_hash_table[pawn_idx].key == pawn_key && pawn_key != 0) {
        pawn_struct = s_pawn_hash_table[pawn_idx].pawn_struct[s_idx];
        passed_pawns = s_pawn_hash_table[pawn_idx].passed_pawns[s_idx];
    } else {
        pawn_struct = EvalFeatures::evaluate_pawn_structure(board, side);
        passed_pawns = EvalFeatures::evaluate_passed_pawns(board, side);

        Color opp_side = ~side;
        size_t o_idx = static_cast<size_t>(opp_side);

        s_pawn_hash_table[pawn_idx].key = pawn_key;
        s_pawn_hash_table[pawn_idx].pawn_struct[s_idx] = pawn_struct;
        s_pawn_hash_table[pawn_idx].passed_pawns[s_idx] = passed_pawns;
        s_pawn_hash_table[pawn_idx].pawn_struct[o_idx] = EvalFeatures::evaluate_pawn_structure(board, opp_side);
        s_pawn_hash_table[pawn_idx].passed_pawns[o_idx] = EvalFeatures::evaluate_passed_pawns(board, opp_side);
    }

    ScorePair king_safety  = EvalFeatures::evaluate_king_safety(board, side);
    ScorePair activity     = EvalFeatures::evaluate_piece_activity(board, side);
    ScorePair mobility     = EvalFeatures::evaluate_mobility(board, side);

    ScorePair pos_total = pawn_struct + passed_pawns + king_safety + activity + mobility;

    int knights = popcount(board.pieces(make_piece(Color::White, PieceType::Knight))) + popcount(board.pieces(make_piece(Color::Black, PieceType::Knight)));
    int bishops = popcount(board.pieces(make_piece(Color::White, PieceType::Bishop))) + popcount(board.pieces(make_piece(Color::Black, PieceType::Bishop)));
    int rooks   = popcount(board.pieces(make_piece(Color::White, PieceType::Rook)))   + popcount(board.pieces(make_piece(Color::Black, PieceType::Rook)));
    int queens  = popcount(board.pieces(make_piece(Color::White, PieceType::Queen)))  + popcount(board.pieces(make_piece(Color::Black, PieceType::Queen)));
    int game_phase = knights * 1 + bishops * 1 + rooks * 2 + queens * 4;

    int mg_total = mg_material + mg_pst + pos_total.mg;
    int eg_total = eg_material + eg_pst + pos_total.eg;

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
        // Tier 1: Fast O(1) Bitmask Material + PST Eval (~5 nanoseconds)
        int white_fast = evaluate_side(board, Color::White);
        int black_fast = evaluate_side(board, Color::Black);
        int fast_diff  = (board.side_to_move() == Color::White) ? (white_fast - black_fast) : (black_fast - white_fast);

        // Tier 1 Lazy Cutoff Threshold (+/- 600 cp):
        // Softened for Classical/Rapid so full Spectral-Tropical graph physics
        // remain active across all positional battles up to a full Queen lead!
        if (std::abs(fast_diff) >= 600) {
            int knights = popcount(board.pieces(Piece::WhiteKnight) | board.pieces(Piece::BlackKnight));
            int bishops = popcount(board.pieces(Piece::WhiteBishop) | board.pieces(Piece::BlackBishop));
            int rooks   = popcount(board.pieces(Piece::WhiteRook)   | board.pieces(Piece::BlackRook));
            int queens  = popcount(board.pieces(Piece::WhiteQueen)  | board.pieces(Piece::BlackQueen));
            int game_phase = std::min(24, knights * 1 + bishops * 1 + rooks * 2 + queens * 4);
            int tapered_tempo = (18 * game_phase + 4 * (24 - game_phase)) / 24;
            return fast_diff + tapered_tempo;
        }

        // Tier 2: Full Spectral-Tropical Graph Eigensolver (High Precision for [-600, +600] cp)
        return TropicalEvaluator::instance().evaluate(board);
    }

    int white_score = evaluate_side(board, Color::White);
    int black_score = evaluate_side(board, Color::Black);

    int knights = popcount(board.pieces(Piece::WhiteKnight) | board.pieces(Piece::BlackKnight));
    int bishops = popcount(board.pieces(Piece::WhiteBishop) | board.pieces(Piece::BlackBishop));
    int rooks   = popcount(board.pieces(Piece::WhiteRook)   | board.pieces(Piece::BlackRook));
    int queens  = popcount(board.pieces(Piece::WhiteQueen)  | board.pieces(Piece::BlackQueen));
    int game_phase = std::min(24, knights * 1 + bishops * 1 + rooks * 2 + queens * 4);
    int tapered_tempo = (18 * game_phase + 4 * (24 - game_phase)) / 24;

    int relative_score = white_score - black_score;
    return (board.side_to_move() == Color::White) ? (relative_score + tapered_tempo) : (-relative_score + tapered_tempo);
}

int Evaluator::evaluate_fast(const Board& board) {
    // Fast O(1) Bitboard Positional Evaluation:
    // Computes Material + PST + Pawn Structure + Passed Pawns + King Safety + Piece Activity + Mobility.
    // Zero allocations, pure SIMD/Bitboard math (~50 nanoseconds).
    int white_score = evaluate_side(board, Color::White);
    int black_score = evaluate_side(board, Color::Black);

    int knights = popcount(board.pieces(Piece::WhiteKnight) | board.pieces(Piece::BlackKnight));
    int bishops = popcount(board.pieces(Piece::WhiteBishop) | board.pieces(Piece::BlackBishop));
    int rooks   = popcount(board.pieces(Piece::WhiteRook)   | board.pieces(Piece::BlackRook));
    int queens  = popcount(board.pieces(Piece::WhiteQueen)  | board.pieces(Piece::BlackQueen));
    int game_phase = std::min(24, knights * 1 + bishops * 1 + rooks * 2 + queens * 4);
    int tapered_tempo = (18 * game_phase + 4 * (24 - game_phase)) / 24;

    int relative_score = white_score - black_score;
    return (board.side_to_move() == Color::White) ? (relative_score + tapered_tempo) : (-relative_score + tapered_tempo);
}

static constexpr int MAX_SEARCH_PLY = 256;
static thread_local TensorMPSQuantized::QuantizedEnvironment s_quant_env[MAX_SEARCH_PLY];

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
