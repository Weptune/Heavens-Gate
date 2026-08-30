#include "eval_features.hpp"
#include "../core/bitwise.hpp"
#include "../movegen/attack_masks.hpp"
#include <cmath>

namespace heavensgate {

std::array<Bitboard, 64> EvalFeatures::PassedPawnMask[2]{};
std::array<Bitboard, 8>  EvalFeatures::IsolatedPawnMask{};
std::array<int, 32>      EvalFeatures::KingDangerTable{};

void EvalFeatures::init() {
    // 1. Isolated Pawn Masks (adjacent files)
    for (int f = 0; f < 8; ++f) {
        File file_enum = static_cast<File>(f);
        Bitboard mask = EmptyBB;
        if (f > 0) mask |= file_bb(static_cast<File>(f - 1));
        if (f < 7) mask |= file_bb(static_cast<File>(f + 1));
        IsolatedPawnMask[static_cast<size_t>(file_enum)] = mask;
    }

    // 2. Passed Pawn Masks (squares ahead in same file & adjacent files)
    for (int s_idx = 0; s_idx < 64; ++s_idx) {
        Square sq = static_cast<Square>(s_idx);
        File f = file_of(sq);
        Rank r = rank_of(sq);

        Bitboard w_mask = EmptyBB;
        Bitboard b_mask = EmptyBB;

        for (int rank_idx = static_cast<int>(r) + 1; rank_idx < 8; ++rank_idx) {
            Rank r_enum = static_cast<Rank>(rank_idx);
            w_mask |= square_bb(make_square(f, r_enum));
            if (static_cast<int>(f) > 0) w_mask |= square_bb(make_square(static_cast<File>(static_cast<int>(f) - 1), r_enum));
            if (static_cast<int>(f) < 7) w_mask |= square_bb(make_square(static_cast<File>(static_cast<int>(f) + 1), r_enum));
        }

        for (int rank_idx = static_cast<int>(r) - 1; rank_idx >= 0; --rank_idx) {
            Rank r_enum = static_cast<Rank>(rank_idx);
            b_mask |= square_bb(make_square(f, r_enum));
            if (static_cast<int>(f) > 0) b_mask |= square_bb(make_square(static_cast<File>(static_cast<int>(f) - 1), r_enum));
            if (static_cast<int>(f) < 7) b_mask |= square_bb(make_square(static_cast<File>(static_cast<int>(f) + 1), r_enum));
        }

        PassedPawnMask[0][static_cast<size_t>(sq)] = w_mask;
        PassedPawnMask[1][static_cast<size_t>(sq)] = b_mask;
    }

    // 3. King Danger Table (quadratic scaling for king attack count)
    for (size_t i = 0; i < 32; ++i) {
        KingDangerTable[i] = static_cast<int>(i * i * 3);
    }
}

ScorePair EvalFeatures::evaluate_pawn_structure(const Board& board, Color side) {
    ScorePair score{0, 0};

    Piece pawn_piece = make_piece(side, PieceType::Pawn);
    Bitboard my_pawns = board.pieces(pawn_piece);

    Piece opp_pawn_piece = make_piece(~side, PieceType::Pawn);
    Bitboard opp_pawns = board.pieces(opp_pawn_piece);

    // Doubled pawns (-14 cp MG, -24 cp EG per extra pawn on same file)
    for (int f = 0; f < 8; ++f) {
        File file_enum = static_cast<File>(f);
        Bitboard file_pawns = my_pawns & file_bb(file_enum);
        int count = popcount(file_pawns);
        if (count > 1) {
            score.mg -= (count - 1) * 14;
            score.eg -= (count - 1) * 24;
        }
    }

    // Isolated pawns (-16 cp MG, -26 cp EG if no friendly pawns on adjacent files)
    Bitboard pawns_copy = my_pawns;
    while (pawns_copy) {
        Square sq = pop_lsb(pawns_copy);
        File f = file_of(sq);
        Bitboard adj_mask = IsolatedPawnMask[static_cast<size_t>(f)];

        if ((my_pawns & adj_mask) == EmptyBB) {
            score.mg -= 16;
            score.eg -= 26;
        }
    }

    // Backward pawns (-12 cp MG, -22 cp EG)
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
            if (stop_sq != Square::None) {
                Bitboard opp_attacks = EmptyBB;
                Bitboard opp_pawns_copy = opp_pawns;
                while (opp_pawns_copy) {
                    Square op_sq = pop_lsb(opp_pawns_copy);
                    opp_attacks |= AttackMasks::pawn_attacks(~side, op_sq);
                }

                if (opp_attacks & square_bb(stop_sq)) {
                    score.mg -= 12;
                    score.eg -= 22;
                }
            }
        }
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

    constexpr std::array<int, 8> PassedBonusMG = { 0,  5, 12, 24, 42,  72, 115, 0 };
    constexpr std::array<int, 8> PassedBonusEG = { 0, 12, 25, 50, 92, 155, 245, 0 };

    while (my_pawns) {
        Square sq = pop_lsb(my_pawns);
        Bitboard mask = PassedPawnMask[pers_idx][static_cast<size_t>(sq)];

        if ((opp_pawns & mask) == EmptyBB) {
            Rank r = rank_of(sq);
            int rank_idx = (side == Color::White) ? static_cast<int>(r) : (7 - static_cast<int>(r));
            score.mg += PassedBonusMG[static_cast<size_t>(rank_idx)];
            score.eg += PassedBonusEG[static_cast<size_t>(rank_idx)];

            // Protected passed pawn bonus (+18 cp MG, +38 cp EG)
            Bitboard friendly_defenders = board.pieces(pawn_piece);
            Bitboard pawn_def_mask = AttackMasks::pawn_attacks(~side, sq);
            if (friendly_defenders & pawn_def_mask) {
                score.mg += 18;
                score.eg += 38;
            }
        }
    }

    return score;
}

ScorePair EvalFeatures::evaluate_king_safety(const Board& board, Color side) {
    ScorePair score{0, 0};

    Square ksq = board.king_square(side);
    if (ksq == Square::None) return score;

    // Check if Queens are off the board (Endgame Phase)
    int queens_count = popcount(board.pieces(Piece::WhiteQueen) | board.pieces(Piece::BlackQueen));
    if (queens_count == 0) {
        // Endgame King Centralization Bonus (Up to +40 cp in EG, 0 cp in MG)
        int kr_idx = static_cast<int>(rank_of(ksq));
        int kf_idx = static_cast<int>(file_of(ksq));
        int center_dist = std::max(std::abs(kr_idx - 3), std::abs(kf_idx - 3));
        score.eg += (4 - center_dist) * 10;
    } else {
        // 0. Uncastled Exposed King Penalty (-120 cp MG, -20 cp EG)
        int kr = static_cast<int>(rank_of(ksq));
        int kf = static_cast<int>(file_of(ksq));
        if ((side == Color::White && kr <= 3 && kf >= 2 && kf <= 5) ||
            (side == Color::Black && kr >= 4 && kf >= 2 && kf <= 5)) {
            score.mg -= 120;
            score.eg -= 20;
        }

        // 1. Middlegame Pawn Shield Bonus (+15 cp per shield pawn in MG, 0 in EG)
        File kf_enum = file_of(ksq);
        Rank kr_enum = rank_of(ksq);

        Bitboard shield_mask = EmptyBB;
        if (side == Color::White && kr_enum <= Rank::Rank3) {
            for (int df = -1; df <= 1; ++df) {
                int f_idx = static_cast<int>(kf_enum) + df;
                if (f_idx >= 0 && f_idx < 8) {
                    shield_mask |= square_bb(make_square(static_cast<File>(f_idx), Rank::Rank2));
                    shield_mask |= square_bb(make_square(static_cast<File>(f_idx), Rank::Rank3));
                }
            }
        } else if (side == Color::Black && kr_enum >= Rank::Rank6) {
            for (int df = -1; df <= 1; ++df) {
                int f_idx = static_cast<int>(kf_enum) + df;
                if (f_idx >= 0 && f_idx < 8) {
                    shield_mask |= square_bb(make_square(static_cast<File>(f_idx), Rank::Rank7));
                    shield_mask |= square_bb(make_square(static_cast<File>(f_idx), Rank::Rank6));
                }
            }
        }

        Bitboard my_pawns = board.pieces(make_piece(side, PieceType::Pawn));
        int shield_pawns = popcount(my_pawns & shield_mask);
        score.mg += shield_pawns * 15;

        // 2. Enemy Attackers Count & Danger Scale
        Bitboard king_zone = AttackMasks::king_attacks(ksq) | square_bb(ksq);
        Bitboard occ = board.occupied();

        int attacker_weight = 0;

        auto check_attackers = [&](PieceType pt, int weight, auto attack_fn) {
            Bitboard pieces = board.pieces(make_piece(~side, pt));
            while (pieces) {
                Square psq = pop_lsb(pieces);
                Bitboard attacks = attack_fn(psq, occ);
                if (attacks & king_zone) {
                    attacker_weight += weight;
                }
            }
        };

        check_attackers(PieceType::Knight, 3, [](Square s, Bitboard) { return AttackMasks::knight_attacks(s); });
        check_attackers(PieceType::Bishop, 3, [](Square s, Bitboard o) { return AttackMasks::bishop_attacks(s, o); });
        check_attackers(PieceType::Rook,   5, [](Square s, Bitboard o) { return AttackMasks::rook_attacks(s, o); });
        check_attackers(PieceType::Queen,  8, [](Square s, Bitboard o) { return AttackMasks::queen_attacks(s, o); });

        // Open/Semi-Open File King Danger Penalty
        File kf_val = file_of(ksq);
        Bitboard enemy_rooks_queens = board.pieces(make_piece(~side, PieceType::Rook)) | board.pieces(make_piece(~side, PieceType::Queen));
        while (enemy_rooks_queens) {
            Square sq = pop_lsb(enemy_rooks_queens);
            if (std::abs(static_cast<int>(file_of(sq)) - static_cast<int>(kf_val)) <= 1) {
                score.mg -= 45;
                score.eg -= 10;
            }
        }

        size_t danger_idx = std::min(static_cast<size_t>(attacker_weight), static_cast<size_t>(31));
        score.mg -= KingDangerTable[danger_idx];
    }

    return score;
}

ScorePair EvalFeatures::evaluate_piece_activity(const Board& board, Color side) {
    ScorePair score{0, 0};

    Bitboard knights = board.pieces(make_piece(side, PieceType::Knight));
    Bitboard bishops = board.pieces(make_piece(side, PieceType::Bishop));
    Bitboard rooks   = board.pieces(make_piece(side, PieceType::Rook));

    // Bishop Pair Bonus (+32 cp MG, +52 cp EG in open endgames)
    if (popcount(bishops) >= 2) {
        score.mg += 32;
        score.eg += 52;
    }

    // Minor Piece Development: Penalize sleeping minors on starting squares in MG (-15 cp MG, 0 EG)
    if (side == Color::White) {
        Bitboard home_minors = (knights & (square_bb(Square::b1) | square_bb(Square::g1))) |
                               (bishops & (square_bb(Square::c1) | square_bb(Square::f1)));
        score.mg -= popcount(home_minors) * 15;
    } else {
        Bitboard home_minors = (knights & (square_bb(Square::b8) | square_bb(Square::g8))) |
                               (bishops & (square_bb(Square::c8) | square_bb(Square::f8)));
        score.mg -= popcount(home_minors) * 15;
    }

    // Knights in Center (d4, e4, d5, e5) Bonus (+15 cp MG, +20 cp EG)
    Bitboard center_mask = square_bb(Square::d4) | square_bb(Square::e4) | square_bb(Square::d5) | square_bb(Square::e5);
    int center_knights = popcount(knights & center_mask);
    score.mg += center_knights * 15;
    score.eg += center_knights * 20;

    // Rooks on Open File Bonus & 7th Rank Bonus
    Bitboard all_pawns = board.pieces(Piece::WhitePawn) | board.pieces(Piece::BlackPawn);
    Rank SeventhRank = (side == Color::White) ? Rank::Rank7 : Rank::Rank2;
    while (rooks) {
        Square rsq = pop_lsb(rooks);
        File f = file_of(rsq);
        Rank r = rank_of(rsq);
        if (r == SeventhRank) {
            score.mg += 25;
            score.eg += 40; // Rook on 7th Rank!
        }
        if ((all_pawns & file_bb(f)) == EmptyBB) {
            score.mg += 20;
            score.eg += 25; // Open file
        } else if ((board.pieces(make_piece(side, PieceType::Pawn)) & file_bb(f)) == EmptyBB) {
            score.mg += 10;
            score.eg += 15; // Semi-open file
        }
    }

    return score;
}

ScorePair EvalFeatures::evaluate_mobility(const Board& board, Color side) {
    ScorePair score{0, 0};
    Bitboard occ = board.occupied();
    Bitboard my_pieces = board.pieces(side);

    auto add_mob = [&](PieceType pt, int mg_w, int eg_w, auto attack_fn) {
        Bitboard pieces = board.pieces(make_piece(side, pt));
        while (pieces) {
            Square psq = pop_lsb(pieces);
            Bitboard attacks = attack_fn(psq, occ) & ~my_pieces;
            int count = popcount(attacks);
            score.mg += count * mg_w;
            score.eg += count * eg_w;
        }
    };

    add_mob(PieceType::Knight, 3, 4, [](Square s, Bitboard) { return AttackMasks::knight_attacks(s); });
    add_mob(PieceType::Bishop, 3, 4, [](Square s, Bitboard o) { return AttackMasks::bishop_attacks(s, o); });
    add_mob(PieceType::Rook,   2, 3, [](Square s, Bitboard o) { return AttackMasks::rook_attacks(s, o); });
    add_mob(PieceType::Queen,  1, 2, [](Square s, Bitboard o) { return AttackMasks::queen_attacks(s, o); });

    return score;
}

} // namespace heavensgate
