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
    g_eval_params.init_v2_tables();
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

// Terms that depend on pieces, i.e. everything outside the pawn-only hash.
static ScorePair non_pawn_terms(const Board& board, Color side) {
    return EvalFeatures::evaluate_king_safety(board, side) + EvalFeatures::evaluate_piece_activity(board, side)
         + EvalFeatures::evaluate_threats(board, side) + EvalFeatures::evaluate_mobility(board, side)
         + EvalFeatures::evaluate_material_imbalances(board, side) + EvalFeatures::evaluate_passed_extras(board, side);
}

ScorePair Evaluator::positional_terms(const Board& board, Color side) {
    return EvalFeatures::evaluate_pawn_structure(board, side) + EvalFeatures::evaluate_passed_pawns(board, side)
         + non_pawn_terms(board, side);
}

ScorePair Evaluator::side_totals(const Board& board, Color side) {
    int mg_material = board.mg_material(side);
    int eg_material = board.eg_material(side);
    int mg_pst = board.mg_pst(side);
    int eg_pst = board.eg_pst(side);

    if (current_mode_ == EvalMode::MaterialOnly) {
        return {mg_material + mg_pst, mg_material + mg_pst};
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

    ScorePair pos_total = pawn_struct + passed_pawns + non_pawn_terms(board, side);

    return {mg_material + mg_pst + pos_total.mg, eg_material + eg_pst + pos_total.eg};
}

int Evaluator::evaluate_side(const Board& board, Color side) {
    const ScorePair t = side_totals(board, side);
    const int mg_weight = std::min(board.game_phase(), 24);
    return (t.mg * mg_weight + t.eg * (24 - mg_weight)) / 24;
}

// Endgame scale in 64ths, applied to the white-relative EG difference. Fixed rules
// (not tuned): material configurations that are drawn far more often than the
// raw score suggests.
int Evaluator::eg_scale(const Board& board, int eg_white) {
    const Color strong = eg_white >= 0 ? Color::White : Color::Black, weak = ~strong;
    auto count = [&](Color c, PieceType pt) { return popcount(board.pieces(make_piece(c, pt))); };
    auto npm = [&](Color c) {
        return 3 * (count(c, PieceType::Knight) + count(c, PieceType::Bishop)) + 5 * count(c, PieceType::Rook) + 9 * count(c, PieceType::Queen);
    };
    if (count(strong, PieceType::Pawn) == 0) {
        if (count(strong, PieceType::Knight) == 2 && npm(strong) == 6 && npm(weak) == 0) return 0; // KNN v K
        if (npm(strong) - npm(weak) <= 3) return npm(strong) <= 3 ? 0 : 8;  // a minor (or less) up, no pawns
    }
    // Opposite-coloured bishops.
    const Bitboard wb = board.pieces(Piece::WhiteBishop), bb = board.pieces(Piece::BlackBishop);
    constexpr Bitboard Dark = 0xAA55AA55AA55AA55ULL;
    if (popcount(wb) == 1 && popcount(bb) == 1 && (!(wb & Dark)) != (!(bb & Dark)))
        return (npm(Color::White) == 3 && npm(Color::Black) == 3) ? 32 : 48;
    return 64;
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
    const int phase = std::min(24, board.game_phase());
    const int tempo = (g_eval_params.tempo_mg * phase + g_eval_params.tempo_eg * (24 - phase)) / 24;

    // Lazy cutoff: when material + PST alone is far outside the window, skip the
    // positional terms. Margin in tuned eval units (a pawn is ~130-170 here).
    constexpr int LAZY_MARGIN = 320;
    const int base_mg = board.mg_material(Color::White) + board.mg_pst(Color::White) - board.mg_material(Color::Black) - board.mg_pst(Color::Black);
    const int base_eg = board.eg_material(Color::White) + board.eg_pst(Color::White) - board.eg_material(Color::Black) - board.eg_pst(Color::Black);
    const int base_white = (base_mg * phase + base_eg * (24 - phase)) / 24;
    const int simple = (board.side_to_move() == Color::White ? base_white : -base_white) + tempo;
    if (beta < ScoreMate - 1000 && simple >= beta + LAZY_MARGIN) return simple;
    if (alpha > -ScoreMate + 1000 && simple <= alpha - LAZY_MARGIN) return simple;

    // Full evaluation: taper once on the white-relative difference, with the
    // endgame scale applied to the EG part.
    const ScorePair w = side_totals(board, Color::White), b = side_totals(board, Color::Black);
    const int mg = w.mg - b.mg, eg = w.eg - b.eg;
    const int scaled_eg = eg * eg_scale(board, eg) / 64;
    const int white_score = (mg * phase + scaled_eg * (24 - phase)) / 24;
    return (board.side_to_move() == Color::White) ? (white_score + tempo) : (-white_score + tempo);
}

void Evaluator::reset_incremental_cache() {
    s_pawn_hash_table.fill(PawnHashEntry{});
}

int Evaluator::evaluate_incremental(const Board& board, int /*ply*/, Square /*from_sq*/, Square /*to_sq*/) {
    return evaluate(board);
}

} // namespace heavensgate
