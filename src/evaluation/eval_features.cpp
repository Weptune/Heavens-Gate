#include "eval_features.hpp"
#include "../core/bitwise.hpp"
#include "../movegen/attack_masks.hpp"
#include <cmath>

namespace heavensgate {

std::array<Bitboard, 64> EvalFeatures::PassedPawnMask[2]{};
std::array<Bitboard, 8>  EvalFeatures::IsolatedPawnMask{};
std::array<int, 32>      EvalFeatures::KingDangerTable{};
Bitboard EvalFeatures::LightSquaresBB = 0ULL;
Bitboard EvalFeatures::DarkSquaresBB = 0ULL;

void EvalFeatures::init() {
    // 0. Light/Dark Square Bitboard Masks
    LightSquaresBB = 0x55AA55AA55AA55AAULL;
    DarkSquaresBB  = ~LightSquaresBB;

    // 1. Isolated Pawn Masks (adjacent files)
    for (int f = 0; f < 8; ++f) {
        File file_enum = static_cast<File>(f);
        Bitboard mask = EmptyBB;
        if (f > 0) mask |= file_bb(static_cast<File>(f - 1));
        if (f < 7) mask |= file_bb(static_cast<File>(f + 1));
        IsolatedPawnMask[static_cast<size_t>(file_enum)] = mask;
    }

    // 2. Passed Pawn Masks (file + adjacent files ahead of pawn)
    for (int sq = 0; sq < 64; ++sq) {
        Square square_enum = static_cast<Square>(sq);
        File f = file_of(square_enum);
        Rank r = rank_of(square_enum);

        Bitboard white_mask = EmptyBB;
        Bitboard black_mask = EmptyBB;

        for (int r_ahead = static_cast<int>(r) + 1; r_ahead < 8; ++r_ahead) {
            white_mask |= square_bb(make_square(f, static_cast<Rank>(r_ahead)));
            if (f > File::FileA) white_mask |= square_bb(make_square(static_cast<File>(static_cast<int>(f) - 1), static_cast<Rank>(r_ahead)));
            if (f < File::FileH) white_mask |= square_bb(make_square(static_cast<File>(static_cast<int>(f) + 1), static_cast<Rank>(r_ahead)));
        }

        for (int r_behind = static_cast<int>(r) - 1; r_behind >= 0; --r_behind) {
            black_mask |= square_bb(make_square(f, static_cast<Rank>(r_behind)));
            if (f > File::FileA) black_mask |= square_bb(make_square(static_cast<File>(static_cast<int>(f) - 1), static_cast<Rank>(r_behind)));
            if (f < File::FileH) black_mask |= square_bb(make_square(static_cast<File>(static_cast<int>(f) + 1), static_cast<Rank>(r_behind)));
        }

        PassedPawnMask[static_cast<size_t>(Color::White)][static_cast<size_t>(sq)] = white_mask;
        PassedPawnMask[static_cast<size_t>(Color::Black)][static_cast<size_t>(sq)] = black_mask;
    }

    // 3. Calibrated King Danger Quadratic Curve
    for (int d = 0; d < 32; ++d) {
        KingDangerTable[static_cast<size_t>(d)] = (d * d) / 8;
    }
}

int EvalFeatures::evaluate_pawn_structure(const Board& board, Color us) {
    int score = 0;
    Bitboard our_pawns = board.pieces(make_piece(us, PieceType::Pawn));

    Bitboard temp = our_pawns;
    while (temp) {
        Square sq = pop_lsb(temp);
        File f = file_of(sq);

        // Isolated Pawn Penalty
        if ((our_pawns & IsolatedPawnMask[static_cast<size_t>(f)]) == EmptyBB) {
            score -= 10;
        }

        // Doubled Pawn Penalty
        Bitboard file_pawns = our_pawns & file_bb(f);
        if (popcount(file_pawns) > 1) {
            score -= 8;
        }
    }

    return score;
}

int EvalFeatures::evaluate_passed_pawns(const Board& board, Color us) {
    int score = 0;
    Color them = ~us;
    Bitboard our_pawns   = board.pieces(make_piece(us, PieceType::Pawn));
    Bitboard enemy_pawns = board.pieces(make_piece(them, PieceType::Pawn));

    constexpr int PassedPawnBonus[8] = { 0, 5, 10, 20, 40, 70, 110, 0 };

    Bitboard temp = our_pawns;
    while (temp) {
        Square sq = pop_lsb(temp);
        Bitboard mask = PassedPawnMask[static_cast<size_t>(us)][static_cast<size_t>(sq)];

        if ((enemy_pawns & mask) == EmptyBB) {
            Rank r = rank_of(sq);
            int rel_rank = (us == Color::White) ? static_cast<int>(r) : (7 - static_cast<int>(r));
            score += PassedPawnBonus[rel_rank];
        }
    }

    return score;
}

int EvalFeatures::evaluate_bad_bishops(const Board& board, Color us) {
    int penalty = 0;
    Bitboard our_pawns   = board.pieces(make_piece(us, PieceType::Pawn));
    Bitboard our_bishops = board.pieces(make_piece(us, PieceType::Bishop));

    Bitboard temp = our_bishops;
    while (temp) {
        Square sq = pop_lsb(temp);
        bool is_light = (static_cast<int>(file_of(sq)) + static_cast<int>(rank_of(sq))) % 2 != 0;
        Bitboard same_color_squares = is_light ? LightSquaresBB : DarkSquaresBB;

        int blocked_pawns = popcount(our_pawns & same_color_squares);
        penalty += blocked_pawns * 6; // -6 cp per friendly pawn on same color square
    }

    return -penalty;
}

int EvalFeatures::evaluate_pin_threats(const Board& board, Color us) {
    int score = 0;
    Color them = ~us;

    Square e_king = board.king_square(them);
    if (e_king == Square::None) return 0;

    Bitboard occ = board.occupied();
    Bitboard them_pieces = board.pieces(them);

    // X-ray attacks along King rays from slider pieces
    Bitboard slider_atks = AttackMasks::bishop_attacks(e_king, occ) | AttackMasks::rook_attacks(e_king, occ);
    Bitboard pinned = slider_atks & them_pieces;

    score += popcount(pinned) * 20; // +20 cp per enemy piece in pin ray
    return score;
}

int EvalFeatures::evaluate_king_safety(const Board& board, Color us) {
    Square ksq = board.king_square(us);
    if (ksq == Square::None) return 0;

    File kf = file_of(ksq);
    Rank kr = rank_of(ksq);

    bool is_castled = (kf <= File::FileC || kf >= File::FileF) && 
                      ((us == Color::White && kr == Rank::Rank1) || (us == Color::Black && kr == Rank::Rank8));

    if (!is_castled) return 0;

    Color them = ~us;
    int danger_units = 0;

    Bitboard king_zone = AttackMasks::king_attacks(ksq) | square_bb(ksq);

    Bitboard shield_pawns = board.pieces(make_piece(us, PieceType::Pawn)) & king_zone;
    int shield_count = popcount(shield_pawns);
    int shield_score = shield_count * 10;

    Bitboard occ = board.occupied();

    Bitboard enemy_knights = board.pieces(make_piece(them, PieceType::Knight));
    Bitboard temp_k = enemy_knights;
    while (temp_k) {
        Square sq = pop_lsb(temp_k);
        if (AttackMasks::knight_attacks(sq) & king_zone) danger_units += 2;
    }

    Bitboard enemy_bishops = board.pieces(make_piece(them, PieceType::Bishop));
    Bitboard temp_b = enemy_bishops;
    while (temp_b) {
        Square sq = pop_lsb(temp_b);
        if (AttackMasks::bishop_attacks(sq, occ) & king_zone) danger_units += 2;
    }

    Bitboard enemy_rooks = board.pieces(make_piece(them, PieceType::Rook));
    Bitboard temp_r = enemy_rooks;
    while (temp_r) {
        Square sq = pop_lsb(temp_r);
        if (AttackMasks::rook_attacks(sq, occ) & king_zone) danger_units += 3;
    }

    Bitboard enemy_queens = board.pieces(make_piece(them, PieceType::Queen));
    Bitboard temp_q = enemy_queens;
    while (temp_q) {
        Square sq = pop_lsb(temp_q);
        if (AttackMasks::queen_attacks(sq, occ) & king_zone) danger_units += 4;
    }

    size_t danger_idx = static_cast<size_t>(std::min(danger_units, 31));
    int danger_penalty = KingDangerTable[danger_idx];

    return shield_score - danger_penalty;
}

int EvalFeatures::evaluate_piece_activity(const Board& board, Color us) {
    int score = 0;
    Color them = ~us;

    Bitboard our_pawns   = board.pieces(make_piece(us, PieceType::Pawn));
    Bitboard enemy_pawns = board.pieces(make_piece(them, PieceType::Pawn));

    Square q_sq = (us == Color::White) ? Square::d1 : Square::d8;
    if (board.piece_at(q_sq) != make_piece(us, PieceType::Queen)) {
        Square b1_sq = (us == Color::White) ? Square::b1 : Square::b8;
        Square c1_sq = (us == Color::White) ? Square::c1 : Square::c8;
        Square f1_sq = (us == Color::White) ? Square::f1 : Square::f8;
        Square g1_sq = (us == Color::White) ? Square::g1 : Square::g8;

        int home_minors = 0;
        if (board.piece_at(b1_sq) != Piece::None) home_minors++;
        if (board.piece_at(c1_sq) != Piece::None) home_minors++;
        if (board.piece_at(f1_sq) != Piece::None) home_minors++;
        if (board.piece_at(g1_sq) != Piece::None) home_minors++;

        if (home_minors >= 3) {
            score -= 25;
        }
    }

    // 1. Bishop Pair Bonus
    Bitboard bishops = board.pieces(make_piece(us, PieceType::Bishop));
    if (popcount(bishops) >= 2) {
        score += 25;
    }

    // 2. Rooks on Open & Semi-Open Files
    Bitboard rooks = board.pieces(make_piece(us, PieceType::Rook));
    Bitboard temp_r = rooks;
    while (temp_r) {
        Square sq = pop_lsb(temp_r);
        File f = file_of(sq);
        Rank r = rank_of(sq);

        Bitboard file_bb_mask = file_bb(f);

        bool no_our_pawns   = (our_pawns & file_bb_mask) == EmptyBB;
        bool no_enemy_pawns = (enemy_pawns & file_bb_mask) == EmptyBB;

        if (no_our_pawns && no_enemy_pawns) {
            score += 20; // Open file
        } else if (no_our_pawns) {
            score += 10; // Semi-open file
        }

        // Rook on 7th rank bonus
        Rank seventh = (us == Color::White) ? Rank::Rank7 : Rank::Rank2;
        if (r == seventh) {
            score += 20;
        }
    }

    // 3. Knight Outposts
    Bitboard knights = board.pieces(make_piece(us, PieceType::Knight));
    Bitboard temp_n = knights;
    while (temp_n) {
        Square sq = pop_lsb(temp_n);
        Rank r = rank_of(sq);
        int rel_rank = (us == Color::White) ? static_cast<int>(r) : (7 - static_cast<int>(r));

        if (rel_rank >= 3 && rel_rank <= 5) {
            Bitboard pawn_atks = AttackMasks::pawn_attacks(them, sq);
            if (pawn_atks & our_pawns) {
                score += 15; // Knight Outpost
            }
        }
    }

    return score;
}

int EvalFeatures::evaluate_mobility(const Board& board, Color us) {
    int score = 0;
    Bitboard us_pieces = board.pieces(us);
    Bitboard occ = board.occupied();

    auto add_mob = [&](PieceType pt, int weight, auto atk_fn) {
        Bitboard pieces = board.pieces(make_piece(us, pt));
        while (pieces) {
            Square sq = pop_lsb(pieces);
            Bitboard atks = atk_fn(sq, occ) & ~us_pieces;
            score += popcount(atks) * weight;
        }
    };

    add_mob(PieceType::Knight, 1, [](Square s, Bitboard) { return AttackMasks::knight_attacks(s); });
    add_mob(PieceType::Bishop, 1, [](Square s, Bitboard o) { return AttackMasks::bishop_attacks(s, o); });
    add_mob(PieceType::Rook,   1, [](Square s, Bitboard o) { return AttackMasks::rook_attacks(s, o); });
    add_mob(PieceType::Queen,  1, [](Square s, Bitboard o) { return AttackMasks::queen_attacks(s, o); });

    return score / 2;
}

} // namespace heavensgate
