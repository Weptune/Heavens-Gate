#include "eval_features.hpp"
#include "eval_params.hpp"
#include "../core/bitwise.hpp"
#include "../movegen/attack_masks.hpp"
#include <cmath>
#include <algorithm>

namespace heavensgate {

std::array<Bitboard, 64> EvalFeatures::PassedPawnMask[2]{};
std::array<Bitboard, 64> EvalFeatures::OutpostMask[2]{};
std::array<Bitboard, 8>  EvalFeatures::IsolatedPawnMask{};
std::array<int, 32>      EvalFeatures::KingDangerTable{};
std::array<Bitboard, 64> EvalFeatures::KingShieldMask[2]{};
std::array<Bitboard, 64> EvalFeatures::KingAdjacentFilesMask{};

void EvalFeatures::init() {
    // 1. Isolated Pawn Masks (adjacent files)
    for (int f = 0; f < 8; ++f) {
        File file_enum = static_cast<File>(f);
        Bitboard mask = EmptyBB;
        if (f > 0) mask |= file_bb(static_cast<File>(f - 1));
        if (f < 7) mask |= file_bb(static_cast<File>(f + 1));
        IsolatedPawnMask[static_cast<size_t>(file_enum)] = mask;
    }

    // 2. Passed Pawn & Outpost Masks
    for (int s_idx = 0; s_idx < 64; ++s_idx) {
        Square sq = static_cast<Square>(s_idx);
        File f = file_of(sq);
        Rank r = rank_of(sq);

        Bitboard w_mask = EmptyBB;
        Bitboard b_mask = EmptyBB;
        Bitboard w_outpost = EmptyBB;
        Bitboard b_outpost = EmptyBB;

        for (int rank_idx = static_cast<int>(r) + 1; rank_idx < 8; ++rank_idx) {
            Rank r_enum = static_cast<Rank>(rank_idx);
            w_mask |= square_bb(make_square(f, r_enum));
            if (static_cast<int>(f) > 0) {
                w_mask |= square_bb(make_square(static_cast<File>(static_cast<int>(f) - 1), r_enum));
                w_outpost |= square_bb(make_square(static_cast<File>(static_cast<int>(f) - 1), r_enum));
            }
            if (static_cast<int>(f) < 7) {
                w_mask |= square_bb(make_square(static_cast<File>(static_cast<int>(f) + 1), r_enum));
                w_outpost |= square_bb(make_square(static_cast<File>(static_cast<int>(f) + 1), r_enum));
            }
        }

        for (int rank_idx = static_cast<int>(r) - 1; rank_idx >= 0; --rank_idx) {
            Rank r_enum = static_cast<Rank>(rank_idx);
            b_mask |= square_bb(make_square(f, r_enum));
            if (static_cast<int>(f) > 0) {
                b_mask |= square_bb(make_square(static_cast<File>(static_cast<int>(f) - 1), r_enum));
                b_outpost |= square_bb(make_square(static_cast<File>(static_cast<int>(f) - 1), r_enum));
            }
            if (static_cast<int>(f) < 7) {
                b_mask |= square_bb(make_square(static_cast<File>(static_cast<int>(f) + 1), r_enum));
                b_outpost |= square_bb(make_square(static_cast<File>(static_cast<int>(f) + 1), r_enum));
            }
        }

        PassedPawnMask[0][static_cast<size_t>(sq)] = w_mask;
        PassedPawnMask[1][static_cast<size_t>(sq)] = b_mask;
        OutpostMask[0][static_cast<size_t>(sq)] = w_outpost;
        OutpostMask[1][static_cast<size_t>(sq)] = b_outpost;
    }

    // 3. King Danger Table (quadratic scaling for king attack count)
    for (size_t i = 0; i < 32; ++i) {
        KingDangerTable[i] = static_cast<int>(i * i * 3);
    }

    // 4. Precomputed King Shield Masks & Adjacent Files Mask
    for (int s_idx = 0; s_idx < 64; ++s_idx) {
        Square sq = static_cast<Square>(s_idx);
        File kf_enum = file_of(sq);
        Rank kr_enum = rank_of(sq);

        Bitboard adj_files = EmptyBB;
        for (int df = -1; df <= 1; ++df) {
            int f_idx = static_cast<int>(kf_enum) + df;
            if (f_idx >= 0 && f_idx < 8) {
                adj_files |= file_bb(static_cast<File>(f_idx));
            }
        }
        KingAdjacentFilesMask[static_cast<size_t>(s_idx)] = adj_files;

        Bitboard w_shield = EmptyBB;
        if (kr_enum <= Rank::Rank3) {
            for (int df = -1; df <= 1; ++df) {
                int f_idx = static_cast<int>(kf_enum) + df;
                if (f_idx >= 0 && f_idx < 8) {
                    w_shield |= square_bb(make_square(static_cast<File>(f_idx), Rank::Rank2));
                    w_shield |= square_bb(make_square(static_cast<File>(f_idx), Rank::Rank3));
                }
            }
        }
        KingShieldMask[0][static_cast<size_t>(s_idx)] = w_shield;

        Bitboard b_shield = EmptyBB;
        if (kr_enum >= Rank::Rank6) {
            for (int df = -1; df <= 1; ++df) {
                int f_idx = static_cast<int>(kf_enum) + df;
                if (f_idx >= 0 && f_idx < 8) {
                    b_shield |= square_bb(make_square(static_cast<File>(f_idx), Rank::Rank7));
                    b_shield |= square_bb(make_square(static_cast<File>(f_idx), Rank::Rank6));
                }
            }
        }
        KingShieldMask[1][static_cast<size_t>(s_idx)] = b_shield;
    }
}

ScorePair EvalFeatures::evaluate_pawn_structure(const Board& board, Color side) {
    ScorePair score{0, 0};

    Piece pawn_piece = make_piece(side, PieceType::Pawn);
    Bitboard my_pawns = board.pieces(pawn_piece);

    Piece opp_pawn_piece = make_piece(~side, PieceType::Pawn);
    Bitboard opp_pawns = board.pieces(opp_pawn_piece);

    // 1. Doubled Pawns (-14 cp MG, -24 cp EG per extra pawn on same file)
    for (int f = 0; f < 8; ++f) {
        File file_enum = static_cast<File>(f);
        Bitboard file_pawns = my_pawns & file_bb(file_enum);
        int count = popcount(file_pawns);
        if (count > 1) {
            score.mg -= (count - 1) * g_eval_params.doubled_mg;
            score.eg -= (count - 1) * g_eval_params.doubled_eg;
        }
    }

    Bitboard not_file_a = ~file_bb(File::FileA);
    Bitboard not_file_h = ~file_bb(File::FileH);

    // 2. Connected & Phalanx Pawns (+10 MG, +16 EG for connected; +12 MG, +18 EG for phalanx)
    Bitboard my_pawn_attacks = (side == Color::White)
        ? (((my_pawns & not_file_a) << 7) | ((my_pawns & not_file_h) << 9))
        : (((my_pawns & not_file_a) >> 9) | ((my_pawns & not_file_h) >> 7));

    Bitboard connected_pawns = my_pawns & my_pawn_attacks;
    score.mg += popcount(connected_pawns) * g_eval_params.connected_mg;
    score.eg += popcount(connected_pawns) * g_eval_params.connected_eg;

    Bitboard phalanx_pawns = my_pawns & (((my_pawns & not_file_a) >> 1) | ((my_pawns & not_file_h) << 1));
    score.mg += (popcount(phalanx_pawns) / 2) * g_eval_params.phalanx_mg;
    score.eg += (popcount(phalanx_pawns) / 2) * g_eval_params.phalanx_eg;

    // 3. Isolated Pawns (-16 cp MG, -26 cp EG if no friendly pawns on adjacent files)
    Bitboard pawns_copy = my_pawns;
    while (pawns_copy) {
        Square sq = pop_lsb(pawns_copy);
        File f = file_of(sq);
        Bitboard adj_mask = IsolatedPawnMask[static_cast<size_t>(f)];

        if ((my_pawns & adj_mask) == EmptyBB) {
            score.mg -= g_eval_params.isolated_mg;
            score.eg -= g_eval_params.isolated_eg;
        }
    }

    // 4. Backward Pawns (-12 cp MG, -22 cp EG)
    Bitboard opp_attacks = (side == Color::White)
        ? (((opp_pawns & not_file_a) >> 9) | ((opp_pawns & not_file_h) >> 7))
        : (((opp_pawns & not_file_a) << 7) | ((opp_pawns & not_file_h) << 9));

    pawns_copy = my_pawns;
    while (pawns_copy) {
        Square sq = pop_lsb(pawns_copy);
        File f = file_of(sq);
        Rank r = rank_of(sq);

        Bitboard adj_mask = IsolatedPawnMask[static_cast<size_t>(f)];

        bool is_behind = true;
        Bitboard adj_friendly = my_pawns & adj_mask;

        while (adj_friendly) {
            Square adj_sq = pop_lsb(adj_friendly);
            Rank adj_r = rank_of(adj_sq);
            if (side == Color::White && adj_r <= r) is_behind = false;
            if (side == Color::Black && adj_r >= r) is_behind = false;
        }

        if (is_behind && (my_pawns & adj_mask) != EmptyBB) {
            Square stop_sq = make_square(f, (side == Color::White) ? static_cast<Rank>(static_cast<int>(r) + 1) : static_cast<Rank>(static_cast<int>(r) - 1));
            if (stop_sq != Square::None && (opp_attacks & square_bb(stop_sq))) {
                score.mg -= g_eval_params.backward_mg;
                score.eg -= g_eval_params.backward_eg;
            }
        }
    }

    // Active existing pawn tension (pawns attacking each other)
    Bitboard current_tension = my_pawn_attacks & opp_pawns;
    if (current_tension) {
        int tension_count = popcount(current_tension);
        score.mg += tension_count * g_eval_params.pawn_tension_mg;
        score.eg += tension_count * g_eval_params.pawn_tension_eg;
    }

    return score;
}

ScorePair EvalFeatures::evaluate_passed_pawns(const Board& board, Color side) {
    ScorePair score{0, 0};

    Piece pawn_piece = make_piece(side, PieceType::Pawn);
    Bitboard my_pawns = board.pieces(pawn_piece);

    Piece opp_pawn_piece = make_piece(~side, PieceType::Pawn);
    Bitboard opp_pawns = board.pieces(opp_pawn_piece);

    size_t pers_idx = (side == Color::White) ? 0 : 1;


    while (my_pawns) {
        Square sq = pop_lsb(my_pawns);
        Bitboard mask = PassedPawnMask[pers_idx][static_cast<size_t>(sq)];

        if ((opp_pawns & mask) == EmptyBB) {
            Rank r = rank_of(sq);
            int rank_idx = (side == Color::White) ? static_cast<int>(r) : (7 - static_cast<int>(r));
            if (rank_idx >= 1 && rank_idx <= 6) {
                score.mg += g_eval_params.passed_mg[static_cast<size_t>(rank_idx - 1)];
                score.eg += g_eval_params.passed_eg[static_cast<size_t>(rank_idx - 1)];
            }

            // Protected passed pawn bonus (+20 cp MG, +40 cp EG)
            Bitboard friendly_defenders = board.pieces(pawn_piece);
            Bitboard pawn_def_mask = AttackMasks::pawn_attacks(~side, sq);
            if (friendly_defenders & pawn_def_mask) {
                score.mg += g_eval_params.protected_passed_mg;
                score.eg += g_eval_params.protected_passed_eg;
            }
        }
    }

    return score;
}

// Attack maps of both colours, computed once per position and reused by the king
// safety and threat terms (each eval used to regenerate them ~8 times per side).
// Keyed by Zobrist key and occupancy; attacks do not depend on any parameter.
struct AttackMaps {
    uint64_t key{0};
    Bitboard occ{0};
    bool valid{false};
    Bitboard by_type[2][6]{};   // [colour][PieceType]
    Bitboard all[2]{}, no_king[2]{};
};

static const AttackMaps& attack_maps(const Board& board) noexcept {
    thread_local AttackMaps maps;
    const Bitboard occ = board.occupied();
    if (maps.valid && maps.key == board.zobrist_key() && maps.occ == occ) return maps;
    const Bitboard not_a = ~file_bb(File::FileA), not_h = ~file_bb(File::FileH);
    for (Color c : {Color::White, Color::Black}) {
        const size_t ci = static_cast<size_t>(c);
        auto& t = maps.by_type[ci];
        const Bitboard pawns = board.pieces(make_piece(c, PieceType::Pawn));
        t[0] = c == Color::White ? (((pawns & not_a) << 7) | ((pawns & not_h) << 9))
                                 : (((pawns & not_a) >> 9) | ((pawns & not_h) >> 7));
        t[1] = t[2] = t[3] = t[4] = t[5] = EmptyBB;
        for (Bitboard b = board.pieces(make_piece(c, PieceType::Knight)); b;) t[1] |= AttackMasks::knight_attacks(pop_lsb(b));
        for (Bitboard b = board.pieces(make_piece(c, PieceType::Bishop)); b;) t[2] |= AttackMasks::bishop_attacks(pop_lsb(b), occ);
        for (Bitboard b = board.pieces(make_piece(c, PieceType::Rook)); b;) t[3] |= AttackMasks::rook_attacks(pop_lsb(b), occ);
        for (Bitboard b = board.pieces(make_piece(c, PieceType::Queen)); b;) t[4] |= AttackMasks::queen_attacks(pop_lsb(b), occ);
        const Square k = board.king_square(c);
        if (k != Square::None) t[5] = AttackMasks::king_attacks(k);
        maps.no_king[ci] = t[0] | t[1] | t[2] | t[3] | t[4];
        maps.all[ci] = maps.no_king[ci] | t[5];
    }
    maps.key = board.zobrist_key(); maps.occ = occ; maps.valid = true;
    return maps;
}

static Bitboard attacks_of(const Board& board, Color c, Bitboard /*occ*/) noexcept {
    return attack_maps(board).all[static_cast<size_t>(c)];
}

static Bitboard attacks_without_king(const Board& board, Color c, Bitboard /*occ*/) noexcept {
    return attack_maps(board).no_king[static_cast<size_t>(c)];
}

ScorePair EvalFeatures::evaluate_king_safety(const Board& board, Color side) {
    ScorePair score{0, 0};
    const auto& p = g_eval_params;

    Square ksq = board.king_square(side);
    if (ksq == Square::None) return score;

    // Without queens the king is a piece: its placement is the king PST and the
    // passed-pawn king-distance terms. Safety terms apply only with queens on.
    if (popcount(board.pieces(Piece::WhiteQueen) | board.pieces(Piece::BlackQueen)) == 0) return score;

    const Color opp = ~side;
    const int kr = static_cast<int>(rank_of(ksq));
    const int kf = static_cast<int>(file_of(ksq));

    // 0. Uncastled exposed king.
    if ((side == Color::White && kr <= 3 && kf >= 2 && kf <= 5) ||
        (side == Color::Black && kr >= 4 && kf >= 2 && kf <= 5)) {
        score.mg -= p.uncastled_king_mg;
        score.eg -= p.uncastled_king_eg;
    }

    // 1. Pawn shield.
    const size_t side_idx = side == Color::White ? 0 : 1;
    const Bitboard my_pawns = board.pieces(make_piece(side, PieceType::Pawn));
    score.mg += popcount(my_pawns & KingShieldMask[side_idx][static_cast<size_t>(ksq)]) * p.pawn_shield_mg;

    // 2. Pawn storm: closest enemy pawn on each king-adjacent file, by distance.
    const Bitboard opp_pawns = board.pieces(make_piece(opp, PieceType::Pawn));
    const Bitboard adj_opp_pawns = opp_pawns & KingAdjacentFilesMask[static_cast<size_t>(ksq)];
    for (int df = -1; df <= 1 && adj_opp_pawns; ++df) {
        const int f_idx = kf + df;
        if (f_idx < 0 || f_idx > 7) continue;
        const Bitboard file_opp_pawns = adj_opp_pawns & file_bb(static_cast<File>(f_idx));
        if (!file_opp_pawns) continue;
        // Relative rank of the most advanced enemy pawn (from our side): 4 = rank 5, 3 = rank 4, 2 = rank 3.
        const int rel = side == Color::White ? static_cast<int>(rank_of(lsb(file_opp_pawns)))
                                             : 7 - static_cast<int>(rank_of(msb(file_opp_pawns)));
        if (rel >= 2 && rel <= 4) score.mg -= p.pawn_storm_mg[static_cast<size_t>(4 - rel)];
    }

    // 3. Enemy rooks/queens on open or semi-open files next to the king.
    const Bitboard enemy_majors = (board.pieces(make_piece(opp, PieceType::Rook)) | board.pieces(make_piece(opp, PieceType::Queen)))
                                  & KingAdjacentFilesMask[static_cast<size_t>(ksq)];
    if (enemy_majors) {
        const Bitboard all_pawns = board.pieces(Piece::WhitePawn) | board.pieces(Piece::BlackPawn);
        for (int df = -1; df <= 1; ++df) {
            const int f_idx = kf + df;
            if (f_idx < 0 || f_idx > 7) continue;
            const Bitboard f_mask = file_bb(static_cast<File>(f_idx));
            const int majors_on_file = popcount(enemy_majors & f_mask);
            if (!majors_on_file) continue;
            if ((all_pawns & f_mask) == EmptyBB) {
                score.mg -= majors_on_file * p.king_open_file_mg;
                score.eg -= majors_on_file * p.king_open_file_eg;
            } else if ((my_pawns & f_mask) == EmptyBB) {
                score.mg -= majors_on_file * p.king_semi_open_file_mg;
                score.eg -= majors_on_file * p.king_semi_open_file_eg;
            }
        }
    }

    // 4. King-zone attackers: weighted units index a tuned table, only with >= 2 attackers.
    const Bitboard king_zone = AttackMasks::king_attacks(ksq) | square_bb(ksq);
    const Bitboard occ = board.occupied();
    int attacker_weight = 0, attackers = 0;
    auto count_attackers = [&](PieceType pt, int weight, auto attack_fn) {
        for (Bitboard b = board.pieces(make_piece(opp, pt)); b;) {
            const Square psq = pop_lsb(b);
            if (attack_fn(psq, occ) & king_zone) { attacker_weight += weight; ++attackers; }
        }
    };
    count_attackers(PieceType::Knight, 3, [](Square s, Bitboard) { return AttackMasks::knight_attacks(s); });
    count_attackers(PieceType::Bishop, 3, [](Square s, Bitboard o) { return AttackMasks::bishop_attacks(s, o); });
    count_attackers(PieceType::Rook,   5, [](Square s, Bitboard o) { return AttackMasks::rook_attacks(s, o); });
    count_attackers(PieceType::Queen,  8, [](Square s, Bitboard o) { return AttackMasks::queen_attacks(s, o); });
    if (attackers >= 2) {
        const size_t idx = std::min<size_t>(static_cast<size_t>(attacker_weight), 31);
        score.mg -= p.king_danger_mg[idx];
        score.eg -= p.king_danger_eg[idx];
    }

    // 5. Safe checks: squares where an enemy piece can check us and we do not defend.
    const Bitboard ours = attacks_of(board, side, occ);
    const Bitboard safe = ~ours & ~board.pieces(opp);
    const Bitboard knight_checks = AttackMasks::knight_attacks(ksq) & safe;
    const Bitboard bishop_checks = AttackMasks::bishop_attacks(ksq, occ) & safe;
    const Bitboard rook_checks = AttackMasks::rook_attacks(ksq, occ) & safe;
    const auto& opp_maps = attack_maps(board).by_type[static_cast<size_t>(opp)];
    const Bitboard n_att = opp_maps[1], b_att = opp_maps[2], r_att = opp_maps[3], q_att = opp_maps[4];
    const int nc = popcount(knight_checks & n_att), bc = popcount(bishop_checks & b_att);
    const int rc = popcount(rook_checks & r_att), qc = popcount((bishop_checks | rook_checks) & q_att);
    score.mg -= nc * p.safe_check_knight_mg + bc * p.safe_check_bishop_mg + rc * p.safe_check_rook_mg + qc * p.safe_check_queen_mg;
    score.eg -= nc * p.safe_check_knight_eg + bc * p.safe_check_bishop_eg + rc * p.safe_check_rook_eg + qc * p.safe_check_queen_eg;

    // 6. King-zone pressure: squares the enemy attacks, and those only our king defends.
    const Bitboard theirs = attacks_of(board, opp, occ);
    const Bitboard ours_no_king = attacks_without_king(board, side, occ);
    const int ring = popcount(king_zone & theirs), weak = popcount(king_zone & theirs & ~ours_no_king);
    score.mg -= ring * p.king_ring_attacks_mg + weak * p.weak_king_squares_mg;
    score.eg -= ring * p.king_ring_attacks_eg + weak * p.weak_king_squares_eg;

    return score;
}

// Passed-pawn terms that depend on pieces (kept out of the pawn-only hash).
ScorePair EvalFeatures::evaluate_passed_extras(const Board& board, Color side) {
    ScorePair score{0, 0};
    const auto& p = g_eval_params;
    const Bitboard opp_pawns = board.pieces(make_piece(~side, PieceType::Pawn));
    const Square own_k = board.king_square(side), opp_k = board.king_square(~side);
    const Color opp = ~side;
    const bool opp_has_only_pawns = !(board.pieces(make_piece(opp, PieceType::Knight)) | board.pieces(make_piece(opp, PieceType::Bishop)) |
                                      board.pieces(make_piece(opp, PieceType::Rook)) | board.pieces(make_piece(opp, PieceType::Queen)));
    const size_t pers_idx = side == Color::White ? 0 : 1;
    auto dist = [](Square a, Square b) {
        return std::max(std::abs(static_cast<int>(file_of(a)) - static_cast<int>(file_of(b))),
                        std::abs(static_cast<int>(rank_of(a)) - static_cast<int>(rank_of(b))));
    };
    for (Bitboard pawns = board.pieces(make_piece(side, PieceType::Pawn)); pawns;) {
        const Square sq = pop_lsb(pawns);
        if (opp_pawns & PassedPawnMask[pers_idx][static_cast<size_t>(sq)]) continue;
        const int rel = side == Color::White ? static_cast<int>(rank_of(sq)) : 7 - static_cast<int>(rank_of(sq));
        if (rel < 1 || rel > 6) continue;
        const size_t r = static_cast<size_t>(rel - 1);
        const Square stop = static_cast<Square>(static_cast<int>(sq) + (side == Color::White ? 8 : -8));
        if (board.piece_at(stop) != Piece::None) { score.mg += p.passed_blocked_mg[r]; score.eg += p.passed_blocked_eg[r]; }
        // Squares from the pawn to its promotion square.
        const Bitboard path = side == Color::White ? (file_bb(file_of(sq)) & ~((square_bb(sq) << 1) - 1) & ~square_bb(sq))
                                                   : (file_bb(file_of(sq)) & ((square_bb(sq)) - 1));
        if (!(path & board.occupied())) {
            score.mg += p.passed_free_mg[r]; score.eg += p.passed_free_eg[r];
            // Square rule: the enemy has only king and pawns and its king cannot catch the pawn.
            if (opp_has_only_pawns && opp_k != Square::None) {
                const Square promo = make_square(file_of(sq), side == Color::White ? Rank::Rank8 : Rank::Rank1);
                int steps = 7 - rel;
                if (rel == 1) --steps; // Double push from the start rank.
                const int king_steps = dist(opp_k, promo) - (board.side_to_move() == ~side ? 1 : 0);
                if (king_steps > steps) score.eg += p.unstoppable_passer_eg;
            }
        }
        if (own_k != Square::None) {
            const int d = dist(own_k, stop);
            score.mg += d * p.passed_own_king_mg[r]; score.eg += d * p.passed_own_king_eg[r];
        }
        if (opp_k != Square::None) {
            const int d = dist(opp_k, stop);
            score.mg += d * p.passed_opp_king_mg[r]; score.eg += d * p.passed_opp_king_eg[r];
        }
    }
    return score;
}

ScorePair EvalFeatures::evaluate_piece_activity(const Board& board, Color side) {
    ScorePair score{0, 0};

    Bitboard knights = board.pieces(make_piece(side, PieceType::Knight));
    Bitboard bishops = board.pieces(make_piece(side, PieceType::Bishop));
    Bitboard rooks   = board.pieces(make_piece(side, PieceType::Rook));
    Bitboard friendly_pawns = board.pieces(make_piece(side, PieceType::Pawn));
    Bitboard opp_pawns = board.pieces(make_piece(~side, PieceType::Pawn));

    // Existing lever feature depends on NON-pawn blockers, so it must live
    // outside the pawn-only hash. Coefficients and cold-eval semantics unchanged.
    constexpr Bitboard not_file_a = ~file_bb(File::FileA);
    constexpr Bitboard not_file_h = ~file_bb(File::FileH);
    const Bitboard pushes = side == Color::White
        ? (friendly_pawns << 8) & ~board.occupied()
        : (friendly_pawns >> 8) & ~board.occupied();
    const Bitboard attacks = side == Color::White
        ? ((pushes & not_file_a) << 7) | ((pushes & not_file_h) << 9)
        : ((pushes & not_file_a) >> 9) | ((pushes & not_file_h) >> 7);
    const Bitboard central = file_bb(File::FileC) | file_bb(File::FileD) |
                             file_bb(File::FileE) | file_bb(File::FileF);
    const int levers = popcount(attacks & opp_pawns & central);
    score.mg += levers * g_eval_params.central_lever_mg;
    score.eg += levers * g_eval_params.central_lever_eg;

    // 1. Bishop Pair Bonus (+32 cp MG, +52 cp EG in open endgames)
    if (popcount(bishops) >= 2) {
        score.mg += g_eval_params.bishop_pair_mg;
        score.eg += g_eval_params.bishop_pair_eg;
    }

    // 2. Minor Piece Development: Penalize sleeping minors on starting squares in MG (-15 cp MG, 0 EG)
    if (side == Color::White) {
        Bitboard home_minors = (knights & (square_bb(Square::b1) | square_bb(Square::g1))) |
                               (bishops & (square_bb(Square::c1) | square_bb(Square::f1)));
        score.mg -= popcount(home_minors) * g_eval_params.minor_undeveloped_mg;
    } else {
        Bitboard home_minors = (knights & (square_bb(Square::b8) | square_bb(Square::g8))) |
                               (bishops & (square_bb(Square::c8) | square_bb(Square::f8)));
        score.mg -= popcount(home_minors) * g_eval_params.minor_undeveloped_mg;
    }

    // 3. True Knight & Bishop Outposts
    size_t s_idx = (side == Color::White) ? 0 : 1;
    Bitboard central_mask = square_bb(Square::d4) | square_bb(Square::e4) | square_bb(Square::d5) | square_bb(Square::e5);

    Bitboard knights_copy = knights;
    while (knights_copy) {
        Square nsq = pop_lsb(knights_copy);
        Rank nr = rank_of(nsq);
        bool is_advanced = (side == Color::White) ? (nr >= Rank::Rank4 && nr <= Rank::Rank6) : (nr >= Rank::Rank3 && nr <= Rank::Rank5);
        if (is_advanced) {
            bool defended = (friendly_pawns & AttackMasks::pawn_attacks(~side, nsq)) != EmptyBB;
            bool immune = (opp_pawns & OutpostMask[s_idx][static_cast<size_t>(nsq)]) == EmptyBB;
            if (defended && immune) {
                score.mg += g_eval_params.knight_outpost_mg;
                score.eg += g_eval_params.knight_outpost_eg;
                if (square_bb(nsq) & central_mask) {
                    score.mg += g_eval_params.central_knight_outpost_mg;
                    score.eg += g_eval_params.central_knight_outpost_eg;
                }
            }
        }
    }

    Bitboard bishops_copy = bishops;
    while (bishops_copy) {
        Square bsq = pop_lsb(bishops_copy);
        Rank br = rank_of(bsq);
        bool is_advanced = (side == Color::White) ? (br >= Rank::Rank4 && br <= Rank::Rank6) : (br >= Rank::Rank3 && br <= Rank::Rank5);
        if (is_advanced) {
            bool defended = (friendly_pawns & AttackMasks::pawn_attacks(~side, bsq)) != EmptyBB;
            bool immune = (opp_pawns & OutpostMask[s_idx][static_cast<size_t>(bsq)]) == EmptyBB;
            if (defended && immune) {
                score.mg += g_eval_params.bishop_outpost_mg;
                score.eg += g_eval_params.bishop_outpost_eg;
            }
        }
    }

    // 4. Rooks on Open File Bonus & 7th Rank Bonus
    Bitboard all_pawns = board.pieces(Piece::WhitePawn) | board.pieces(Piece::BlackPawn);
    Rank SeventhRank = (side == Color::White) ? Rank::Rank7 : Rank::Rank2;
    Bitboard rooks_copy = rooks;
    while (rooks_copy) {
        Square rsq = pop_lsb(rooks_copy);
        File f = file_of(rsq);
        Rank r = rank_of(rsq);
        if (r == SeventhRank) {
            score.mg += g_eval_params.rook_7th_mg;
            score.eg += g_eval_params.rook_7th_eg; // Rook on 7th Rank!
        }
        if ((all_pawns & file_bb(f)) == EmptyBB) {
            score.mg += g_eval_params.rook_open_mg;
            score.eg += g_eval_params.rook_open_eg; // Open file
        } else if ((board.pieces(make_piece(side, PieceType::Pawn)) & file_bb(f)) == EmptyBB) {
            score.mg += g_eval_params.rook_semi_open_mg;
            score.eg += g_eval_params.rook_semi_open_eg; // Semi-open file
        }
    }

    // 5. Trapped Pieces (Bishops, Knights, Boxed-in Rooks)
    // Trapped Bishops on the rim
    if (side == Color::White) {
        if (board.piece_at(Square::a7) == Piece::WhiteBishop && (opp_pawns & square_bb(Square::b6))) {
            score.mg -= g_eval_params.trapped_bishop_deep_mg; score.eg -= g_eval_params.trapped_bishop_deep_eg;
        }
        if (board.piece_at(Square::h7) == Piece::WhiteBishop && (opp_pawns & square_bb(Square::g6))) {
            score.mg -= g_eval_params.trapped_bishop_deep_mg; score.eg -= g_eval_params.trapped_bishop_deep_eg;
        }
        if (board.piece_at(Square::b8) == Piece::WhiteBishop && (opp_pawns & square_bb(Square::c7))) {
            score.mg -= g_eval_params.trapped_bishop_mg; score.eg -= g_eval_params.trapped_bishop_eg;
        }
        if (board.piece_at(Square::g8) == Piece::WhiteBishop && (opp_pawns & square_bb(Square::f7))) {
            score.mg -= g_eval_params.trapped_bishop_mg; score.eg -= g_eval_params.trapped_bishop_eg;
        }
    } else {
        if (board.piece_at(Square::a2) == Piece::BlackBishop && (opp_pawns & square_bb(Square::b3))) {
            score.mg -= g_eval_params.trapped_bishop_deep_mg; score.eg -= g_eval_params.trapped_bishop_deep_eg;
        }
        if (board.piece_at(Square::h2) == Piece::BlackBishop && (opp_pawns & square_bb(Square::g3))) {
            score.mg -= g_eval_params.trapped_bishop_deep_mg; score.eg -= g_eval_params.trapped_bishop_deep_eg;
        }
        if (board.piece_at(Square::b1) == Piece::BlackBishop && (opp_pawns & square_bb(Square::c2))) {
            score.mg -= g_eval_params.trapped_bishop_mg; score.eg -= g_eval_params.trapped_bishop_eg;
        }
        if (board.piece_at(Square::g1) == Piece::BlackBishop && (opp_pawns & square_bb(Square::f2))) {
            score.mg -= g_eval_params.trapped_bishop_mg; score.eg -= g_eval_params.trapped_bishop_eg;
        }
    }

    // Boxed-in Uncastled Rooks trapped by King
    Square ksq = board.king_square(side);
    if (ksq != Square::None) {
        if (side == Color::White) {
            if ((ksq == Square::f1 || ksq == Square::g1) && (rooks & (square_bb(Square::h1) | square_bb(Square::h2)))) {
                if ((all_pawns & file_bb(File::FileH)) != EmptyBB) {
                    score.mg -= g_eval_params.trapped_rook_mg; // Trapped Kingside Rook
                }
            }
            if ((ksq == Square::c1 || ksq == Square::d1) && (rooks & (square_bb(Square::a1) | square_bb(Square::a2)))) {
                if ((all_pawns & file_bb(File::FileA)) != EmptyBB) {
                    score.mg -= g_eval_params.trapped_rook_mg; // Trapped Queenside Rook
                }
            }
        } else {
            if ((ksq == Square::f8 || ksq == Square::g8) && (rooks & (square_bb(Square::h8) | square_bb(Square::h7)))) {
                if ((all_pawns & file_bb(File::FileH)) != EmptyBB) {
                    score.mg -= g_eval_params.trapped_rook_mg; // Trapped Kingside Rook
                }
            }
            if ((ksq == Square::c8 || ksq == Square::d8) && (rooks & (square_bb(Square::a8) | square_bb(Square::a7)))) {
                if ((all_pawns & file_bb(File::FileA)) != EmptyBB) {
                    score.mg -= g_eval_params.trapped_rook_mg; // Trapped Queenside Rook
                }
            }
        }
    }

    // 6. Bishops hampered by own pawns on their square colour.
    constexpr Bitboard DarkSquares = 0xAA55AA55AA55AA55ULL;
    for (Bitboard b = bishops; b;) {
        const Square bsq = pop_lsb(b);
        const Bitboard same = (square_bb(bsq) & DarkSquares) ? DarkSquares : ~DarkSquares;
        const int n = popcount(friendly_pawns & same);
        score.mg += n * g_eval_params.bishop_pawns_mg;
        score.eg += n * g_eval_params.bishop_pawns_eg;
    }

    // 7. Minor pieces near their own king (distance in king steps).
    if (ksq != Square::None) {
        for (Bitboard b = knights | bishops; b;) {
            const Square m = pop_lsb(b);
            const int d = std::max(std::abs(static_cast<int>(file_of(m)) - static_cast<int>(file_of(ksq))),
                                   std::abs(static_cast<int>(rank_of(m)) - static_cast<int>(rank_of(ksq))));
            score.mg += d * g_eval_params.minor_king_distance_mg;
            score.eg += d * g_eval_params.minor_king_distance_eg;
        }
    }

    // 8. Rooks behind own passed pawns (same file, on the far side from promotion).
    for (Bitboard r = rooks; r;) {
        const Square rsq = pop_lsb(r);
        for (Bitboard pw = friendly_pawns & file_bb(file_of(rsq)); pw;) {
            const Square psq = pop_lsb(pw);
            const bool behind = side == Color::White ? rank_of(rsq) < rank_of(psq) : rank_of(rsq) > rank_of(psq);
            if (behind && !(opp_pawns & PassedPawnMask[s_idx][static_cast<size_t>(psq)])) {
                score.mg += g_eval_params.rook_behind_passer_mg;
                score.eg += g_eval_params.rook_behind_passer_eg;
            }
        }
    }

    return score;
}

ScorePair EvalFeatures::evaluate_threats(const Board& board, Color side) {
    ScorePair score{0, 0};

    Color opp = ~side;
    Bitboard occ = board.occupied();

    Bitboard my_pawns   = board.pieces(make_piece(side, PieceType::Pawn));

    Bitboard opp_rooks  = board.pieces(make_piece(opp, PieceType::Rook));
    Bitboard opp_queens = board.pieces(make_piece(opp, PieceType::Queen));
    Bitboard opp_minors = board.pieces(make_piece(opp, PieceType::Knight)) | board.pieces(make_piece(opp, PieceType::Bishop));

    // 1. Minor piece attacking enemy major pieces
    const auto& my_maps = attack_maps(board).by_type[static_cast<size_t>(side)];
    const Bitboard minor_attacks = my_maps[1] | my_maps[2];

    int minor_on_rook = popcount(minor_attacks & opp_rooks);
    score.mg += minor_on_rook * g_eval_params.minor_threat_rook_mg;
    score.eg += minor_on_rook * g_eval_params.minor_threat_rook_eg;

    int minor_on_queen = popcount(minor_attacks & opp_queens);
    score.mg += minor_on_queen * g_eval_params.minor_threat_queen_mg;
    score.eg += minor_on_queen * g_eval_params.minor_threat_queen_eg;

    // 2. Rook attacking enemy queen
    const Bitboard rook_attacks = my_maps[3];
    int rook_on_queen = popcount(rook_attacks & opp_queens);
    score.mg += rook_on_queen * g_eval_params.rook_threat_queen_mg;
    score.eg += rook_on_queen * g_eval_params.rook_threat_queen_eg;

    // 3. Pawn push threats attacking enemy minor/major pieces
    Bitboard not_file_a = ~file_bb(File::FileA);
    Bitboard not_file_h = ~file_bb(File::FileH);

    Bitboard single_pushes = (side == Color::White)
        ? ((my_pawns << 8) & ~occ)
        : ((my_pawns >> 8) & ~occ);

    Bitboard push_attacks = (side == Color::White)
        ? (((single_pushes & not_file_a) << 7) | ((single_pushes & not_file_h) << 9))
        : (((single_pushes & not_file_a) >> 9) | ((single_pushes & not_file_h) >> 7));

    int pawn_push_threats = popcount(push_attacks & (opp_minors | opp_rooks | opp_queens));
    score.mg += pawn_push_threats * g_eval_params.pawn_push_threat_mg;
    score.eg += pawn_push_threats * g_eval_params.pawn_push_threat_eg;

    // 4. Pawns attacking enemy pieces.
    const Bitboard pawn_attacks = (side == Color::White)
        ? (((my_pawns & not_file_a) << 7) | ((my_pawns & not_file_h) << 9))
        : (((my_pawns & not_file_a) >> 9) | ((my_pawns & not_file_h) >> 7));
    const int pawn_on_minor = popcount(pawn_attacks & opp_minors);
    const int pawn_on_major = popcount(pawn_attacks & (opp_rooks | opp_queens));
    score.mg += pawn_on_minor * g_eval_params.pawn_threat_minor_mg + pawn_on_major * g_eval_params.pawn_threat_major_mg;
    score.eg += pawn_on_minor * g_eval_params.pawn_threat_minor_eg + pawn_on_major * g_eval_params.pawn_threat_major_eg;

    // 4b. Our minors attacking enemy minors that no enemy pawn defends.
    const Bitboard opp_pawn_defence = (opp == Color::White)
        ? (((board.pieces(make_piece(opp, PieceType::Pawn)) & not_file_a) << 7) | ((board.pieces(make_piece(opp, PieceType::Pawn)) & not_file_h) << 9))
        : (((board.pieces(make_piece(opp, PieceType::Pawn)) & not_file_a) >> 9) | ((board.pieces(make_piece(opp, PieceType::Pawn)) & not_file_h) >> 7));
    const int minor_on_minor = popcount(minor_attacks & opp_minors & ~opp_pawn_defence);
    score.mg += minor_on_minor * g_eval_params.minor_threat_minor_mg;
    score.eg += minor_on_minor * g_eval_params.minor_threat_minor_eg;

    // 5. Hanging enemy pieces: attacked by us and not defended.
    const Bitboard ours = attacks_of(board, side, occ), theirs = attacks_of(board, opp, occ);
    const int hanging = popcount((opp_minors | opp_rooks | opp_queens) & ours & ~theirs);
    score.mg += hanging * g_eval_params.hanging_mg;
    score.eg += hanging * g_eval_params.hanging_eg;

    return score;
}

ScorePair EvalFeatures::evaluate_mobility(const Board& board, Color side) {
    ScorePair score{0, 0};
    Bitboard occ = board.occupied();
    Bitboard my_pieces = board.pieces(side);

    // Safe Mobility: Exclude squares controlled by opponent pawns
    Bitboard not_file_a = ~file_bb(File::FileA);
    Bitboard not_file_h = ~file_bb(File::FileH);
    Bitboard opp_pawns = board.pieces(make_piece(~side, PieceType::Pawn));
    Bitboard opp_pawn_attacks = (~side == Color::White)
        ? (((opp_pawns & not_file_a) << 7) | ((opp_pawns & not_file_h) << 9))
        : (((opp_pawns & not_file_a) >> 9) | ((opp_pawns & not_file_h) >> 7));

    Bitboard safe_mask = ~(my_pieces | opp_pawn_attacks);

    const auto& p = g_eval_params;
    auto add_mob = [&](PieceType pt, const auto& mg_tab, const auto& eg_tab, auto attack_fn) {
        Bitboard pieces = board.pieces(make_piece(side, pt));
        while (pieces) {
            Square psq = pop_lsb(pieces);
            const size_t count = std::min<size_t>(static_cast<size_t>(popcount(attack_fn(psq, occ) & safe_mask)), mg_tab.size() - 1);
            score.mg += mg_tab[count];
            score.eg += eg_tab[count];
        }
    };

    add_mob(PieceType::Knight, p.knight_mob_tab_mg, p.knight_mob_tab_eg, [](Square s, Bitboard) { return AttackMasks::knight_attacks(s); });
    add_mob(PieceType::Bishop, p.bishop_mob_tab_mg, p.bishop_mob_tab_eg, [](Square s, Bitboard o) { return AttackMasks::bishop_attacks(s, o); });
    add_mob(PieceType::Rook,   p.rook_mob_tab_mg,   p.rook_mob_tab_eg,   [](Square s, Bitboard o) { return AttackMasks::rook_attacks(s, o); });
    add_mob(PieceType::Queen,  p.queen_mob_tab_mg,  p.queen_mob_tab_eg,  [](Square s, Bitboard o) { return AttackMasks::queen_attacks(s, o); });

    return score;
}

ScorePair EvalFeatures::evaluate_material_imbalances(const Board& board, Color side) {
    ScorePair score;
    Color opp = ~side;

    Bitboard my_pawns   = board.pieces(make_piece(side, PieceType::Pawn));
    Bitboard opp_pawns  = board.pieces(make_piece(opp, PieceType::Pawn));
    int total_pawns = popcount(my_pawns | opp_pawns);

    int my_knights  = popcount(board.pieces(make_piece(side, PieceType::Knight)));
    int my_bishops  = popcount(board.pieces(make_piece(side, PieceType::Bishop)));
    int my_rooks    = popcount(board.pieces(make_piece(side, PieceType::Rook)));
    int opp_rooks   = popcount(board.pieces(make_piece(opp, PieceType::Rook)));
    int my_queens   = popcount(board.pieces(make_piece(side, PieceType::Queen)));
    int opp_queens  = popcount(board.pieces(make_piece(opp, PieceType::Queen)));

    // 1. Kaufman Dynamic Bishop Pair Scaling with Pawn Count
    if (my_bishops >= 2) {
        if (total_pawns < 12) {
            int open_delta = 12 - total_pawns;
            score.mg += open_delta * 2;
            score.eg += open_delta * 4;
        } else if (total_pawns > 12) {
            int closed_delta = total_pawns - 12;
            score.mg -= closed_delta * 2;
            score.eg -= closed_delta * 3;
        }
    }

    // 2. Minor Piece Synergy vs Pawn Count
    if (total_pawns > 10) {
        int closed_weight = total_pawns - 10;
        score.mg += my_knights * closed_weight * 2;
        score.eg += my_knights * closed_weight;
        score.mg -= my_bishops * closed_weight;
        score.eg -= my_bishops * closed_weight * 2;
    } else if (total_pawns < 8) {
        int open_weight = 8 - total_pawns;
        score.mg += my_bishops * open_weight * 2;
        score.eg += my_bishops * open_weight * 3;
        score.mg -= my_knights * open_weight * 2;
        score.eg -= my_knights * open_weight * 3;
    }

    // 3. Exchange Imbalances: Queen vs 2 Rooks
    if (my_queens >= 1 && opp_queens == 0 && opp_rooks >= 2 && my_rooks < opp_rooks) {
        score.mg += 15;
        score.eg -= 25;
    }
    // Return this side's contribution only. Evaluator subtracts Black once.

    // 4. Endgame Rook Pair Synergy
    if (my_rooks >= 2) {
        score.eg += 12;
    }

    return score;
}

} // namespace heavensgate
