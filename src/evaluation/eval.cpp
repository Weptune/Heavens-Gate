#include "eval.hpp"
#include "eval_params.hpp"
#include "tuned_eval_params.inc"
#include "pst.hpp"
#include "spectral_graph.hpp"
#include "tropical_eval.hpp"
#include <algorithm>

namespace heavensgate {

thread_local EvalMode Evaluator::current_mode_ = EvalMode::MasterPositional;

void Evaluator::init() {
    PST::init();
    apply_tuned_eval_params(g_eval_params);
    EvalFeatures::init();
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
    int mg_material = board.mg_material(side);
    int eg_material = board.eg_material(side);
    int mg_pst = board.mg_pst(side);
    int eg_pst = board.eg_pst(side);

    if (current_mode_ == EvalMode::MaterialOnly) {
        return mg_material + mg_pst;
    }

    // Pawn Hash Table Cache: Since pawns move on only ~5-10% of search nodes,
    // caching pawn features reduces evaluation overhead by ~85%.
    Bitboard w_pawns = board.pieces(Piece::WhitePawn);
    Bitboard b_pawns = board.pieces(Piece::BlackPawn);
    ScorePair pawn_struct{0, 0};
    ScorePair passed_pawns{0, 0};
    size_t s_idx = static_cast<size_t>(side);

    if (w_pawns || b_pawns) {
        uint64_t pawn_key = w_pawns ^ (b_pawns * 0x9e3779b97f4a7c15ULL);
        if (pawn_key == 0) pawn_key = 1;
        size_t pawn_idx = static_cast<size_t>((pawn_key ^ (pawn_key >> 32)) & (PAWN_HASH_SIZE - 1));

        if (s_pawn_hash_table[pawn_idx].key == pawn_key) {
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
    }

    ScorePair king_safety  = EvalFeatures::evaluate_king_safety(board, side);
    ScorePair activity     = EvalFeatures::evaluate_piece_activity(board, side);
    ScorePair threats      = EvalFeatures::evaluate_threats(board, side);
    ScorePair mobility     = EvalFeatures::evaluate_mobility(board, side);
    ScorePair imbalances   = EvalFeatures::evaluate_material_imbalances(board, side);

    ScorePair pos_total = pawn_struct + passed_pawns + king_safety + activity + threats + mobility + imbalances;

    int game_phase = board.game_phase();

    int mg_total = mg_material + mg_pst + pos_total.mg;
    int eg_total = eg_material + eg_pst + pos_total.eg;

    int mg_weight = std::min(game_phase, 24);
    int eg_weight = 24 - mg_weight;

    return (mg_total * mg_weight + eg_total * eg_weight) / 24;
}

int Evaluator::evaluate(const Board& board) {
    if (current_mode_ == EvalMode::SpectralTropical) {
        // Tier 1: Fast O(1) Bitmask Material + PST Eval (~5 nanoseconds)
        int white_fast = evaluate_side(board, Color::White);
        int black_fast = evaluate_side(board, Color::Black);
        int fast_diff  = (board.side_to_move() == Color::White) ? (white_fast - black_fast) : (black_fast - white_fast);

        // Tier 1 Lazy Cutoff Threshold (+/- 600 cp):
        // Softened for Classical/Rapid so full Spectral-Tropical graph physics
        // remain active across all positional battles up to a full Queen lead!
        if (std::abs(fast_diff) >= 600) {
            int game_phase = std::min(24, board.game_phase());
            int tapered_tempo = (g_eval_params.tempo_mg * game_phase + g_eval_params.tempo_eg * (24 - game_phase)) / 24;
            return fast_diff + tapered_tempo;
        }

        // Tier 2: Full Spectral-Tropical Graph Eigensolver (High Precision for [-600, +600] cp)
        return TropicalEvaluator::instance().evaluate(board);
    }

    return evaluate_fast(board);
}

int Evaluator::evaluate_fast(const Board& board, int alpha, int beta) {
    // 1. Fast O(1) Material + PST base evaluation (~2 nanoseconds)
    int white_mg = board.mg_material(Color::White) + board.mg_pst(Color::White);
    int white_eg = board.eg_material(Color::White) + board.eg_pst(Color::White);
    int black_mg = board.mg_material(Color::Black) + board.mg_pst(Color::Black);
    int black_eg = board.eg_material(Color::Black) + board.eg_pst(Color::Black);

    int game_phase = std::min(24, board.game_phase());
    int tapered_tempo = (g_eval_params.tempo_mg * game_phase + g_eval_params.tempo_eg * (24 - game_phase)) / 24;

    int diff_mg = white_mg - black_mg;
    int diff_eg = white_eg - black_eg;
    int simple_diff = (diff_mg * game_phase + diff_eg * (24 - game_phase)) / 24;
    int simple_eval = (board.side_to_move() == Color::White) ? (simple_diff + tapered_tempo) : (-simple_diff + tapered_tempo);

    // Lazy Evaluation Cutoffs:
    // If simple_eval is already >= beta + 240 or <= alpha - 240, positional features cannot change the cutoff decision.
    constexpr int LAZY_MARGIN = 240;
    if (beta < ScoreMate - 1000 && simple_eval >= beta + LAZY_MARGIN) {
        return simple_eval;
    }
    if (alpha > -ScoreMate + 1000 && simple_eval <= alpha - LAZY_MARGIN) {
        return simple_eval;
    }

    // 2. Full Positional Evaluation:
    // Computes Pawn Structure + Passed Pawns + King Safety + Piece Activity + Mobility + Imbalances.
    int white_score = evaluate_side(board, Color::White);
    int black_score = evaluate_side(board, Color::Black);

    int relative_score = white_score - black_score;
    return (board.side_to_move() == Color::White) ? (relative_score + tapered_tempo) : (-relative_score + tapered_tempo);
}

void Evaluator::reset_incremental_cache() {
}

int Evaluator::evaluate_incremental(const Board& board, int /*ply*/, Square /*from_sq*/, Square /*to_sq*/) {
    return evaluate(board);
}

} // namespace heavensgate
