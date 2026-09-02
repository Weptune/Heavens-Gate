#include "eval_features.hpp"
#include "../core/bitwise.hpp"
#include "../movegen/attack_masks.hpp"
#include <cmath>
#include <algorithm>

namespace heavensgate {

std::array<Bitboard, 64> EvalFeatures::PassedPawnMask[2]{};
std::array<Bitboard, 64> EvalFeatures::OutpostMask[2]{};
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
            score.mg -= (count - 1) * 14;
            score.eg -= (count - 1) * 24;
        }
    }

    Bitboard not_file_a = ~file_bb(File::FileA);
    Bitboard not_file_h = ~file_bb(File::FileH);

    // 2. Connected & Phalanx Pawns (+10 MG, +16 EG for connected; +12 MG, +18 EG for phalanx)
    Bitboard my_pawn_attacks = (side == Color::White)
        ? (((my_pawns & not_file_a) << 7) | ((my_pawns & not_file_h) << 9))
        : (((my_pawns & not_file_a) >> 9) | ((my_pawns & not_file_h) >> 7));

    Bitboard connected_pawns = my_pawns & my_pawn_attacks;
    score.mg += popcount(connected_pawns) * 10;
    score.eg += popcount(connected_pawns) * 16;

    Bitboard phalanx_pawns = my_pawns & (((my_pawns & not_file_a) >> 1) | ((my_pawns & not_file_h) << 1));
    score.mg += (popcount(phalanx_pawns) / 2) * 12;
    score.eg += (popcount(phalanx_pawns) / 2) * 18;

    // 3. Isolated Pawns (-16 cp MG, -26 cp EG if no friendly pawns on adjacent files)
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
                score.mg -= 12;
                score.eg -= 22;
            }
        }
    }

    // 5. Pawn Levers & Central Tension Breaks (+16 cp MG, +10 cp EG)
    // A pawn push that actively challenges an opponent pawn (e.g. c4/c5, d4/d5, e4/e5, f4/f5)
    Bitboard occ = board.occupied();
    Bitboard single_pushes = (side == Color::White)
        ? ((my_pawns << 8) & ~occ)
        : ((my_pawns >> 8) & ~occ);

    Bitboard push_attacks = (side == Color::White)
        ? (((single_pushes & not_file_a) << 7) | ((single_pushes & not_file_h) << 9))
        : (((single_pushes & not_file_a) >> 9) | ((single_pushes & not_file_h) >> 7));

    Bitboard lever_targets = push_attacks & opp_pawns;
    if (lever_targets) {
        // Bonus for candidate central pawn levers on files c, d, e, f
        Bitboard central_files = file_bb(File::FileC) | file_bb(File::FileD) | file_bb(File::FileE) | file_bb(File::FileF);
        int central_levers = popcount(lever_targets & central_files);
        score.mg += central_levers * 16;
        score.eg += central_levers * 10;
    }

    // Active existing pawn tension (pawns attacking each other)
    Bitboard current_tension = my_pawn_attacks & opp_pawns;
    if (current_tension) {
        int tension_count = popcount(current_tension);
        score.mg += tension_count * 8;
        score.eg += tension_count * 5;
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

    constexpr std::array<int, 8> PassedBonusMG = { 0,  5, 14, 28, 48,  82, 135, 0 };
    constexpr std::array<int, 8> PassedBonusEG = { 0, 14, 30, 60, 105, 175, 275, 0 };

    while (my_pawns) {
        Square sq = pop_lsb(my_pawns);
        Bitboard mask = PassedPawnMask[pers_idx][static_cast<size_t>(sq)];

        if ((opp_pawns & mask) == EmptyBB) {
            Rank r = rank_of(sq);
            int rank_idx = (side == Color::White) ? static_cast<int>(r) : (7 - static_cast<int>(r));
            score.mg += PassedBonusMG[static_cast<size_t>(rank_idx)];
            score.eg += PassedBonusEG[static_cast<size_t>(rank_idx)];

            // Protected passed pawn bonus (+20 cp MG, +40 cp EG)
            Bitboard friendly_defenders = board.pieces(pawn_piece);
            Bitboard pawn_def_mask = AttackMasks::pawn_attacks(~side, sq);
            if (friendly_defenders & pawn_def_mask) {
                score.mg += 20;
                score.eg += 40;
            }
        }
    }

    return score;
}

ScorePair EvalFeatures::evaluate_king_safety(const Board& board, Color side) {
    ScorePair score{0, 0};

    Square ksq = board.king_square(side);
    if (ksq == Square::None) return score;

    int kr_idx = static_cast<int>(rank_of(ksq));
    int kf_idx = static_cast<int>(file_of(ksq));

    // Check if Queens are off the board (Endgame Phase)
    int queens_count = popcount(board.pieces(Piece::WhiteQueen) | board.pieces(Piece::BlackQueen));
    if (queens_count == 0) {
        // Symmetrical Chebyshev distance to 2x2 board center (d4, e4, d5, e5)
        int r_dist = std::min(std::abs(kr_idx - 3), std::abs(kr_idx - 4));
        int f_dist = std::min(std::abs(kf_idx - 3), std::abs(kf_idx - 4));
        int center_dist = std::max(r_dist, f_dist);
        score.eg += (3 - center_dist) * 12;

        // Dynamic King Proximity to Passed Pawns in Endgame
        Square opp_ksq = board.king_square(~side);
        if (opp_ksq != Square::None) {
            Bitboard my_pawns = board.pieces(make_piece(side, PieceType::Pawn));
            Bitboard opp_pawns = board.pieces(make_piece(~side, PieceType::Pawn));
            size_t pers_idx = (side == Color::White) ? 0 : 1;
            while (my_pawns) {
                Square psq = pop_lsb(my_pawns);
                Bitboard mask = PassedPawnMask[pers_idx][static_cast<size_t>(psq)];
                if ((opp_pawns & mask) == EmptyBB) {
                    int my_dist = std::max(std::abs(static_cast<int>(file_of(psq)) - kf_idx),
                                           std::abs(static_cast<int>(rank_of(psq)) - kr_idx));
                    int opp_dist = std::max(std::abs(static_cast<int>(file_of(psq)) - static_cast<int>(file_of(opp_ksq))),
                                            std::abs(static_cast<int>(rank_of(psq)) - static_cast<int>(rank_of(opp_ksq))));
                    int king_proximity_factor = (opp_dist * 2 - my_dist);
                    score.eg += std::clamp(king_proximity_factor * 6, -35, 60);
                }
            }
        }
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

        // 2. Pawn Storm Evaluation: Penalize advancing enemy pawns near our king
        Bitboard opp_pawns = board.pieces(make_piece(~side, PieceType::Pawn));
        for (int df = -1; df <= 1; ++df) {
            int f_idx = static_cast<int>(kf_enum) + df;
            if (f_idx >= 0 && f_idx < 8) {
                Bitboard file_opp_pawns = opp_pawns & file_bb(static_cast<File>(f_idx));
                if (file_opp_pawns) {
                    if (side == Color::White) {
                        Square psq = lsb(file_opp_pawns);
                        Rank pr = rank_of(psq);
                        if (pr == Rank::Rank5) score.mg -= 25;
                        else if (pr == Rank::Rank4) score.mg -= 55;
                        else if (pr == Rank::Rank3) score.mg -= 105;
                    } else {
                        Square psq = msb(file_opp_pawns);
                        Rank pr = rank_of(psq);
                        if (pr == Rank::Rank4) score.mg -= 25;
                        else if (pr == Rank::Rank5) score.mg -= 55;
                        else if (pr == Rank::Rank6) score.mg -= 105;
                    }
                }
            }
        }

        // 3. Open/Semi-Open File King Threat Penalty
        Bitboard all_pawns = board.pieces(Piece::WhitePawn) | board.pieces(Piece::BlackPawn);
        Bitboard enemy_majors = board.pieces(make_piece(~side, PieceType::Rook)) | board.pieces(make_piece(~side, PieceType::Queen));

        for (int df = -1; df <= 1; ++df) {
            int f_idx = static_cast<int>(kf_enum) + df;
            if (f_idx >= 0 && f_idx < 8) {
                File f = static_cast<File>(f_idx);
                Bitboard f_mask = file_bb(f);
                int majors_on_file = popcount(enemy_majors & f_mask);
                if (majors_on_file > 0) {
                    if ((all_pawns & f_mask) == EmptyBB) {
                        score.mg -= majors_on_file * 50;
                        score.eg -= majors_on_file * 15; // Fully open file aimed at king ring
                    } else if ((my_pawns & f_mask) == EmptyBB) {
                        score.mg -= majors_on_file * 32;
                        score.eg -= majors_on_file * 10; // Semi-open file
                    }
                }
            }
        }

        // 4. Enemy Attackers Count & Safe Checks Danger Scale
        Bitboard king_zone = AttackMasks::king_attacks(ksq) | square_bb(ksq);
        Bitboard occ = board.occupied();
        Bitboard my_pieces = board.pieces(side);

        Bitboard not_file_a = ~file_bb(File::FileA);
        Bitboard not_file_h = ~file_bb(File::FileH);
        Bitboard my_pawn_attacks = (side == Color::White)
            ? (((my_pawns & not_file_a) << 7) | ((my_pawns & not_file_h) << 9))
            : (((my_pawns & not_file_a) >> 9) | ((my_pawns & not_file_h) >> 7));

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

        // Safe Checks: Squares where enemy pieces can check our king without being captured by friendly defenders
        Bitboard my_knights = board.pieces(make_piece(side, PieceType::Knight));
        Bitboard my_bishops = board.pieces(make_piece(side, PieceType::Bishop));
        Bitboard my_rooks   = board.pieces(make_piece(side, PieceType::Rook));
        Bitboard my_queens  = board.pieces(make_piece(side, PieceType::Queen));
        Bitboard my_king_att = AttackMasks::king_attacks(ksq);

        Bitboard my_knight_att = EmptyBB;
        Bitboard k_copy = my_knights;
        while (k_copy) {
            my_knight_att |= AttackMasks::knight_attacks(pop_lsb(k_copy));
        }

        Bitboard my_bishop_att = EmptyBB;
        Bitboard b_copy = my_bishops;
        while (b_copy) {
            my_bishop_att |= AttackMasks::bishop_attacks(pop_lsb(b_copy), occ);
        }

        Bitboard my_rook_att = EmptyBB;
        Bitboard r_copy = my_rooks;
        while (r_copy) {
            my_rook_att |= AttackMasks::rook_attacks(pop_lsb(r_copy), occ);
        }

        Bitboard my_queen_att = EmptyBB;
        Bitboard q_copy = my_queens;
        while (q_copy) {
            my_queen_att |= AttackMasks::queen_attacks(pop_lsb(q_copy), occ);
        }

        Bitboard my_all_attacks = my_pawn_attacks | my_knight_att | my_bishop_att | my_rook_att | my_queen_att | my_king_att;
        Bitboard my_minor_plus_pawn = my_pawn_attacks | my_knight_att | my_bishop_att;
        Bitboard my_rook_plus_minor_pawn = my_minor_plus_pawn | my_rook_att;

        Bitboard rook_chk_sqs   = AttackMasks::rook_attacks(ksq, occ) & ~my_pieces;
        Bitboard bishop_chk_sqs = AttackMasks::bishop_attacks(ksq, occ) & ~my_pieces;
        Bitboard knight_chk_sqs = AttackMasks::knight_attacks(ksq) & ~my_pieces;

        Bitboard safe_queen_chk_sqs  = (rook_chk_sqs | bishop_chk_sqs) & ~my_all_attacks;
        Bitboard safe_rook_chk_sqs   = rook_chk_sqs & ~my_rook_plus_minor_pawn;
        Bitboard safe_knight_chk_sqs = knight_chk_sqs & ~my_minor_plus_pawn;
        Bitboard safe_bishop_chk_sqs = bishop_chk_sqs & ~my_minor_plus_pawn;

        // Check for safe enemy queen checks
        Bitboard opp_queens = board.pieces(make_piece(~side, PieceType::Queen));
        while (opp_queens) {
            Square qsq = pop_lsb(opp_queens);
            Bitboard q_attacks = AttackMasks::queen_attacks(qsq, occ);
            int safe_q_checks = popcount(q_attacks & safe_queen_chk_sqs);
            if (safe_q_checks > 0) {
                attacker_weight += safe_q_checks * 6;
                score.mg -= 35; // Direct safe queen check threat penalty
            }
        }

        // Check for safe enemy rook checks
        Bitboard opp_rooks = board.pieces(make_piece(~side, PieceType::Rook));
        while (opp_rooks) {
            Square rsq = pop_lsb(opp_rooks);
            Bitboard r_attacks = AttackMasks::rook_attacks(rsq, occ);
            int safe_r_checks = popcount(r_attacks & safe_rook_chk_sqs);
            if (safe_r_checks > 0) {
                attacker_weight += safe_r_checks * 4;
                score.mg -= 20; // Direct safe rook check threat penalty
            }
        }

        // Check for safe enemy knight checks
        Bitboard opp_knights = board.pieces(make_piece(~side, PieceType::Knight));
        while (opp_knights) {
            Square nsq = pop_lsb(opp_knights);
            Bitboard n_attacks = AttackMasks::knight_attacks(nsq);
            int safe_n_checks = popcount(n_attacks & safe_knight_chk_sqs);
            if (safe_n_checks > 0) {
                attacker_weight += safe_n_checks * 3;
                score.mg -= 15; // Direct safe knight check threat penalty
            }
        }

        // Check for safe enemy bishop checks
        Bitboard opp_bishops = board.pieces(make_piece(~side, PieceType::Bishop));
        while (opp_bishops) {
            Square bsq = pop_lsb(opp_bishops);
            Bitboard b_attacks = AttackMasks::bishop_attacks(bsq, occ);
            int safe_b_checks = popcount(b_attacks & safe_bishop_chk_sqs);
            if (safe_b_checks > 0) {
                attacker_weight += safe_b_checks * 2;
                score.mg -= 10; // Direct safe bishop check threat penalty
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
    Bitboard friendly_pawns = board.pieces(make_piece(side, PieceType::Pawn));
    Bitboard opp_pawns = board.pieces(make_piece(~side, PieceType::Pawn));

    // 1. Bishop Pair Bonus (+32 cp MG, +52 cp EG in open endgames)
    if (popcount(bishops) >= 2) {
        score.mg += 32;
        score.eg += 52;
    }

    // 2. Minor Piece Development: Penalize sleeping minors on starting squares in MG (-15 cp MG, 0 EG)
    if (side == Color::White) {
        Bitboard home_minors = (knights & (square_bb(Square::b1) | square_bb(Square::g1))) |
                               (bishops & (square_bb(Square::c1) | square_bb(Square::f1)));
        score.mg -= popcount(home_minors) * 15;
    } else {
        Bitboard home_minors = (knights & (square_bb(Square::b8) | square_bb(Square::g8))) |
                               (bishops & (square_bb(Square::c8) | square_bb(Square::f8)));
        score.mg -= popcount(home_minors) * 15;
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
                score.mg += 28;
                score.eg += 38;
                if (square_bb(nsq) & central_mask) {
                    score.mg += 12;
                    score.eg += 14;
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
                score.mg += 20;
                score.eg += 28;
            }
        }
    }

    // 4. Rooks on Open File Bonus & 7th Rank Bonus
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

    // 5. Trapped Pieces (Bishops, Knights, Boxed-in Rooks)
    // Trapped Bishops on the rim
    if (side == Color::White) {
        if (board.piece_at(Square::a7) == Piece::WhiteBishop && (opp_pawns & square_bb(Square::b6))) {
            score.mg -= 70; score.eg -= 110;
        }
        if (board.piece_at(Square::h7) == Piece::WhiteBishop && (opp_pawns & square_bb(Square::g6))) {
            score.mg -= 70; score.eg -= 110;
        }
        if (board.piece_at(Square::b8) == Piece::WhiteBishop && (opp_pawns & square_bb(Square::c7))) {
            score.mg -= 60; score.eg -= 90;
        }
        if (board.piece_at(Square::g8) == Piece::WhiteBishop && (opp_pawns & square_bb(Square::f7))) {
            score.mg -= 60; score.eg -= 90;
        }
    } else {
        if (board.piece_at(Square::a2) == Piece::BlackBishop && (opp_pawns & square_bb(Square::b3))) {
            score.mg -= 70; score.eg -= 110;
        }
        if (board.piece_at(Square::h2) == Piece::BlackBishop && (opp_pawns & square_bb(Square::g3))) {
            score.mg -= 70; score.eg -= 110;
        }
        if (board.piece_at(Square::b1) == Piece::BlackBishop && (opp_pawns & square_bb(Square::c2))) {
            score.mg -= 60; score.eg -= 90;
        }
        if (board.piece_at(Square::g1) == Piece::BlackBishop && (opp_pawns & square_bb(Square::f2))) {
            score.mg -= 60; score.eg -= 90;
        }
    }

    // Boxed-in Uncastled Rooks trapped by King
    Square ksq = board.king_square(side);
    if (ksq != Square::None) {
        if (side == Color::White) {
            if ((ksq == Square::f1 || ksq == Square::g1) && (rooks & (square_bb(Square::h1) | square_bb(Square::h2)))) {
                if ((all_pawns & file_bb(File::FileH)) != EmptyBB) {
                    score.mg -= 45; // Trapped Kingside Rook
                }
            }
            if ((ksq == Square::c1 || ksq == Square::d1) && (rooks & (square_bb(Square::a1) | square_bb(Square::a2)))) {
                if ((all_pawns & file_bb(File::FileA)) != EmptyBB) {
                    score.mg -= 45; // Trapped Queenside Rook
                }
            }
        } else {
            if ((ksq == Square::f8 || ksq == Square::g8) && (rooks & (square_bb(Square::h8) | square_bb(Square::h7)))) {
                if ((all_pawns & file_bb(File::FileH)) != EmptyBB) {
                    score.mg -= 45; // Trapped Kingside Rook
                }
            }
            if ((ksq == Square::c8 || ksq == Square::d8) && (rooks & (square_bb(Square::a8) | square_bb(Square::a7)))) {
                if ((all_pawns & file_bb(File::FileA)) != EmptyBB) {
                    score.mg -= 45; // Trapped Queenside Rook
                }
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
    Bitboard my_knights = board.pieces(make_piece(side, PieceType::Knight));
    Bitboard my_bishops = board.pieces(make_piece(side, PieceType::Bishop));
    Bitboard my_rooks   = board.pieces(make_piece(side, PieceType::Rook));

    Bitboard opp_rooks  = board.pieces(make_piece(opp, PieceType::Rook));
    Bitboard opp_queens = board.pieces(make_piece(opp, PieceType::Queen));
    Bitboard opp_minors = board.pieces(make_piece(opp, PieceType::Knight)) | board.pieces(make_piece(opp, PieceType::Bishop));

    // 1. Minor piece attacking enemy major pieces
    Bitboard minor_attacks = EmptyBB;
    Bitboard k_copy = my_knights;
    while (k_copy) {
        minor_attacks |= AttackMasks::knight_attacks(pop_lsb(k_copy));
    }
    Bitboard b_copy = my_bishops;
    while (b_copy) {
        minor_attacks |= AttackMasks::bishop_attacks(pop_lsb(b_copy), occ);
    }

    int minor_on_rook = popcount(minor_attacks & opp_rooks);
    score.mg += minor_on_rook * 35;
    score.eg += minor_on_rook * 45;

    int minor_on_queen = popcount(minor_attacks & opp_queens);
    score.mg += minor_on_queen * 48;
    score.eg += minor_on_queen * 62;

    // 2. Rook attacking enemy queen
    Bitboard rook_attacks = EmptyBB;
    Bitboard r_copy = my_rooks;
    while (r_copy) {
        rook_attacks |= AttackMasks::rook_attacks(pop_lsb(r_copy), occ);
    }
    int rook_on_queen = popcount(rook_attacks & opp_queens);
    score.mg += rook_on_queen * 30;
    score.eg += rook_on_queen * 35;

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
    score.mg += pawn_push_threats * 16;
    score.eg += pawn_push_threats * 22;

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

    auto add_mob = [&](PieceType pt, int mg_w, int eg_w, auto attack_fn) {
        Bitboard pieces = board.pieces(make_piece(side, pt));
        while (pieces) {
            Square psq = pop_lsb(pieces);
            Bitboard attacks = attack_fn(psq, occ) & safe_mask;
            int count = popcount(attacks);
            score.mg += count * mg_w;
            score.eg += count * eg_w;
        }
    };

    add_mob(PieceType::Knight, 4, 4, [](Square s, Bitboard) { return AttackMasks::knight_attacks(s); });
    add_mob(PieceType::Bishop, 4, 4, [](Square s, Bitboard o) { return AttackMasks::bishop_attacks(s, o); });
    add_mob(PieceType::Rook,   2, 3, [](Square s, Bitboard o) { return AttackMasks::rook_attacks(s, o); });
    add_mob(PieceType::Queen,  1, 2, [](Square s, Bitboard o) { return AttackMasks::queen_attacks(s, o); });

    return score;
}

} // namespace heavensgate
