#include "syzygy.hpp"
#include "../movegen/movegen.hpp"
#include <cmath>
#include <algorithm>

namespace heavensgate {

static inline bool is_dark_square(Square sq) noexcept {
    return (static_cast<int>(file_of(sq)) + static_cast<int>(rank_of(sq))) % 2 == 0;
}

void SyzygyTablebase::init(const std::string& tb_path) {
    // Enabled for endgames with 6 or fewer pieces
    enabled_ = true;
    for (auto& entry : cache_) {
        entry.key = 0;
        entry.wdl = WDLScore::Unknown;
    }
}

int SyzygyTablebase::wdl_to_score(WDLScore wdl, int ply) const {
    switch (wdl) {
        case WDLScore::Win:
            return 24000 - ply; // Proven Win
        case WDLScore::CursedWin:
            return 100; // Small advantage (50-move rule draw line)
        case WDLScore::Draw:
        case WDLScore::BlessedLoss:
            return 0; // Draw
        case WDLScore::Loss:
            return -24000 + ply; // Proven Loss
        default:
            return NO_SCORE;
    }
}

int SyzygyTablebase::probe_wdl(const Board& board, int ply) {
    if (!enabled_) return NO_SCORE;

    int total_pieces = popcount(board.occupied());
    if (total_pieces > max_pieces_) return NO_SCORE;

    uint64_t key = board.zobrist_key();
    size_t idx = key % CACHE_SIZE;

    if (cache_[idx].key == key && cache_[idx].wdl != WDLScore::Unknown) {
        return wdl_to_score(cache_[idx].wdl, ply);
    }

    WDLScore wdl = evaluate_endgame_wdl(board);
    if (wdl != WDLScore::Unknown) {
        cache_[idx].key = key;
        cache_[idx].wdl = wdl;
        return wdl_to_score(wdl, ply);
    }

    return NO_SCORE;
}

WDLScore SyzygyTablebase::evaluate_endgame_wdl(const Board& board) {
    Bitboard w_king = board.pieces(make_piece(Color::White, PieceType::King));
    Bitboard b_king = board.pieces(make_piece(Color::Black, PieceType::King));

    if (!w_king || !b_king) return WDLScore::Unknown;

    int num_w_queens  = popcount(board.pieces(make_piece(Color::White, PieceType::Queen)));
    int num_b_queens  = popcount(board.pieces(make_piece(Color::Black, PieceType::Queen)));
    int num_w_rooks   = popcount(board.pieces(make_piece(Color::White, PieceType::Rook)));
    int num_b_rooks   = popcount(board.pieces(make_piece(Color::Black, PieceType::Rook)));
    int num_w_bishops = popcount(board.pieces(make_piece(Color::White, PieceType::Bishop)));
    int num_b_bishops = popcount(board.pieces(make_piece(Color::Black, PieceType::Bishop)));
    int num_w_knights = popcount(board.pieces(make_piece(Color::White, PieceType::Knight)));
    int num_b_knights = popcount(board.pieces(make_piece(Color::Black, PieceType::Knight)));
    int num_w_pawns   = popcount(board.pieces(make_piece(Color::White, PieceType::Pawn)));
    int num_b_pawns   = popcount(board.pieces(make_piece(Color::Black, PieceType::Pawn)));

    int total_w = popcount(board.pieces(Color::White));
    int total_b = popcount(board.pieces(Color::Black));
    int total_pieces = total_w + total_b;

    Color stm = board.side_to_move();

    // 1. KK (Bare Kings = Draw)
    if (total_w == 1 && total_b == 1) {
        return WDLScore::Draw;
    }

    // 2. 3-Piece Endings
    if (total_pieces == 3) {
        // KQK
        if (num_w_queens == 1 && total_w == 2) return (stm == Color::White) ? WDLScore::Win : WDLScore::Loss;
        if (num_b_queens == 1 && total_b == 2) return (stm == Color::Black) ? WDLScore::Win : WDLScore::Loss;

        // KRK
        if (num_w_rooks == 1 && total_w == 2) return (stm == Color::White) ? WDLScore::Win : WDLScore::Loss;
        if (num_b_rooks == 1 && total_b == 2) return (stm == Color::Black) ? WDLScore::Win : WDLScore::Loss;

        // KPK
        if (num_w_pawns == 1 && total_w == 2) return solve_kpk(board, Color::White);
        if (num_b_pawns == 1 && total_b == 2) return solve_kpk(board, Color::Black);

        // KBK or KNK (Insufficient material = Draw)
        if ((num_w_bishops == 1 || num_w_knights == 1) && total_w == 2) return WDLScore::Draw;
        if ((num_b_bishops == 1 || num_b_knights == 1) && total_b == 2) return WDLScore::Draw;
    }

    // 3. 4-Piece Endings
    if (total_pieces == 4) {
        // KBNK (King + Bishop + Knight vs King)
        if (num_w_bishops == 1 && num_w_knights == 1 && total_w == 3 && total_b == 1) return solve_kbnk(board, Color::White);
        if (num_b_bishops == 1 && num_b_knights == 1 && total_b == 3 && total_w == 1) return solve_kbnk(board, Color::Black);

        // KBBK (King + 2 Bishops vs King)
        if (num_w_bishops == 2 && total_w == 3 && total_b == 1) return solve_kbbk(board, Color::White);
        if (num_b_bishops == 2 && total_b == 3 && total_w == 1) return solve_kbbk(board, Color::Black);

        // KNNK (King + 2 Knights vs King = Draw)
        if (num_w_knights == 2 && total_w == 3 && total_b == 1) return solve_knnk(board, Color::White);
        if (num_b_knights == 2 && total_b == 3 && total_w == 1) return solve_knnk(board, Color::Black);

        // KB vs KB (No pawns = Draw)
        if (num_w_bishops == 1 && num_b_bishops == 1 && total_w == 2 && total_b == 2) return WDLScore::Draw;

        // KN vs KN, KB vs KN (No pawns = Draw)
        if (num_w_knights == 1 && num_b_knights == 1 && total_w == 2 && total_b == 2) return WDLScore::Draw;
        if (num_w_bishops == 1 && num_b_knights == 1 && total_w == 2 && total_b == 2) return WDLScore::Draw;
        if (num_w_knights == 1 && num_b_bishops == 1 && total_w == 2 && total_b == 2) return WDLScore::Draw;

        // KRP vs K, KQP vs K, KPP vs K (Forced Win)
        if ((num_w_rooks == 1 || num_w_queens == 1 || num_w_pawns == 2) && num_w_pawns >= 1 && total_w == 3 && total_b == 1) return (stm == Color::White) ? WDLScore::Win : WDLScore::Loss;
        if ((num_b_rooks == 1 || num_b_queens == 1 || num_b_pawns == 2) && num_b_pawns >= 1 && total_b == 3 && total_w == 1) return (stm == Color::Black) ? WDLScore::Win : WDLScore::Loss;

        // KBP vs K (Wrong color bishop + rook pawn fortress)
        if (num_w_bishops == 1 && num_w_pawns == 1 && total_w == 3 && total_b == 1) return solve_wrong_color_bishop_pawn(board, Color::White);
        if (num_b_bishops == 1 && num_b_pawns == 1 && total_b == 3 && total_w == 1) return solve_wrong_color_bishop_pawn(board, Color::Black);
    }

    // 4. 5-Piece & 6-Piece Endings
    if (total_pieces <= 6) {
        // Opposite colored bishops with 1 pawn
        if (num_w_bishops == 1 && num_b_bishops == 1 && (num_w_pawns + num_b_pawns == 1)) {
            if (num_w_pawns == 1 && total_w == 3 && total_b == 2) return solve_opposite_colored_bishops_1p(board, Color::White);
            if (num_b_pawns == 1 && total_b == 3 && total_w == 2) return solve_opposite_colored_bishops_1p(board, Color::Black);
        }

        // KRP vs KR (Lucena & Philidor)
        if (num_w_rooks == 1 && num_b_rooks == 1 && (num_w_pawns + num_b_pawns == 1)) {
            if (num_w_pawns == 1 && total_w == 3 && total_b == 2) return solve_krp_kr(board, Color::White);
            if (num_b_pawns == 1 && total_b == 3 && total_w == 2) return solve_krp_kr(board, Color::Black);
        }
    }

    return WDLScore::Unknown;
}

WDLScore SyzygyTablebase::solve_kqk(const Board& board, Color strong_side) {
    return WDLScore::Win;
}

WDLScore SyzygyTablebase::solve_krk(const Board& board, Color strong_side) {
    return WDLScore::Win;
}

WDLScore SyzygyTablebase::solve_kbnk(const Board& board, Color strong_side) {
    Color stm = board.side_to_move();
    return (stm == strong_side) ? WDLScore::Win : WDLScore::Loss;
}

WDLScore SyzygyTablebase::solve_kbbk(const Board& board, Color strong_side) {
    Color stm = board.side_to_move();
    Piece b_piece = make_piece(strong_side, PieceType::Bishop);
    Bitboard b_bb = board.pieces(b_piece);
    if (popcount(b_bb) < 2) return WDLScore::Unknown;

    Square b1 = pop_lsb(b_bb);
    Square b2 = lsb(b_bb);

    // Opposite colored bishops = forced win
    if (is_dark_square(b1) != is_dark_square(b2)) {
        return (stm == strong_side) ? WDLScore::Win : WDLScore::Loss;
    }
    // Same-colored bishops (underpromotions) cannot force mate
    return WDLScore::Draw;
}

WDLScore SyzygyTablebase::solve_knnk(const Board& board, Color strong_side) {
    // 2 Knights cannot force checkmate against a lone King
    return WDLScore::Draw;
}

WDLScore SyzygyTablebase::solve_kpk(const Board& board, Color strong_side) {
    Color stm = board.side_to_move();
    Square wk_sq = lsb(board.pieces(make_piece(Color::White, PieceType::King)));
    Square bk_sq = lsb(board.pieces(make_piece(Color::Black, PieceType::King)));

    Piece p_piece = make_piece(strong_side, PieceType::Pawn);
    Bitboard p_bb = board.pieces(p_piece);
    if (!p_bb) return WDLScore::Draw;

    Square p_sq = lsb(p_bb);
    int p_file = static_cast<int>(file_of(p_sq));
    int p_rank = static_cast<int>(rank_of(p_sq));

    Square strong_k = (strong_side == Color::White) ? wk_sq : bk_sq;
    Square def_k    = (strong_side == Color::White) ? bk_sq : wk_sq;

    int strong_k_f = static_cast<int>(file_of(strong_k));
    int strong_k_r = static_cast<int>(rank_of(strong_k));
    int def_k_f    = static_cast<int>(file_of(def_k));
    int def_k_r    = static_cast<int>(rank_of(def_k));

    // Normalize orientation so White is pushing up (rank 0 -> 7)
    int norm_p_r = (strong_side == Color::White) ? p_rank : (7 - p_rank);
    int norm_sk_r = (strong_side == Color::White) ? strong_k_r : (7 - strong_k_r);
    int norm_dk_r = (strong_side == Color::White) ? def_k_r : (7 - def_k_r);

    int target_rank = (strong_side == Color::White) ? 7 : 0;
    int steps_to_promote = (strong_side == Color::White) ? (7 - p_rank) : p_rank;

    // 1. Square of the Pawn rule (passer rule)
    int promo_dist = steps_to_promote;
    if (norm_p_r == 1) promo_dist = 5; // Double step on rank 2
    int def_dist = std::max(std::abs(def_k_f - p_file), std::abs(def_k_r - target_rank));
    if (stm != strong_side) def_dist--; // Defender has the move

    if (promo_dist < def_dist) {
        return (stm == strong_side) ? WDLScore::Win : WDLScore::Loss;
    }

    // 2. Rook Pawn (a/h file) rules
    if (p_file == 0 || p_file == 7) {
        // Defender in corner or in front of pawn on same file
        if (def_k_f == p_file && norm_dk_r > norm_p_r) {
            return WDLScore::Draw;
        }
        if ((p_file == 0 && def_k_f <= 1 && norm_dk_r >= 6) ||
            (p_file == 7 && def_k_f >= 6 && norm_dk_r >= 6)) {
            return WDLScore::Draw;
        }
    }

    // 3. Key Squares Rule:
    if (norm_p_r >= 1 && norm_p_r <= 3) {
        int key_r = norm_p_r + 2;
        if (norm_sk_r >= key_r && std::abs(strong_k_f - p_file) <= 1) {
            return (stm == strong_side) ? WDLScore::Win : WDLScore::Loss;
        }
    } else if (norm_p_r >= 4) {
        int key_r1 = norm_p_r + 1;
        if (norm_sk_r >= key_r1 && std::abs(strong_k_f - p_file) <= 1) {
            return (stm == strong_side) ? WDLScore::Win : WDLScore::Loss;
        }
    }

    return WDLScore::Unknown;
}

WDLScore SyzygyTablebase::solve_wrong_color_bishop_pawn(const Board& board, Color strong_side) {
    Piece p_piece = make_piece(strong_side, PieceType::Pawn);
    Piece b_piece = make_piece(strong_side, PieceType::Bishop);
    Bitboard p_bb = board.pieces(p_piece);
    Bitboard b_bb = board.pieces(b_piece);

    if (popcount(p_bb) != 1 || popcount(b_bb) != 1) return WDLScore::Unknown;

    Square p_sq = lsb(p_bb);
    Square b_sq = lsb(b_bb);
    File f = file_of(p_sq);

    // Only applies to rook pawns (a-file or h-file)
    if (f != File::FileA && f != File::FileH) return WDLScore::Unknown;

    Square promo_sq = (strong_side == Color::White) ? (f == File::FileA ? Square::a8 : Square::h8)
                                                    : (f == File::FileA ? Square::a1 : Square::h1);

    bool bishop_is_dark = is_dark_square(b_sq);
    bool promo_is_dark  = is_dark_square(promo_sq);

    // If bishop does NOT control the color of the promotion square
    if (bishop_is_dark != promo_is_dark) {
        Square def_k = (strong_side == Color::White) ? lsb(board.pieces(make_piece(Color::Black, PieceType::King)))
                                                     : lsb(board.pieces(make_piece(Color::White, PieceType::King)));
        File def_f = file_of(def_k);
        Rank def_r = rank_of(def_k);

        // If defending king is already in/near the corner (a8/b8/a7/b7 or h8/g8/h7/g7)
        if (strong_side == Color::White) {
            if ((f == File::FileA && def_f <= File::FileB && def_r >= Rank::Rank7) ||
                (f == File::FileH && def_f >= File::FileG && def_r >= Rank::Rank7)) {
                return WDLScore::Draw;
            }
        } else {
            if ((f == File::FileA && def_f <= File::FileB && def_r <= Rank::Rank2) ||
                (f == File::FileH && def_f >= File::FileG && def_r <= Rank::Rank2)) {
                return WDLScore::Draw;
            }
        }
    }

    return WDLScore::Unknown;
}

WDLScore SyzygyTablebase::solve_opposite_colored_bishops_1p(const Board& board, Color strong_side) {
    Piece my_b_piece = make_piece(strong_side, PieceType::Bishop);
    Piece opp_b_piece = make_piece(~strong_side, PieceType::Bishop);
    Piece my_p_piece = make_piece(strong_side, PieceType::Pawn);

    Bitboard my_b = board.pieces(my_b_piece);
    Bitboard opp_b = board.pieces(opp_b_piece);
    Bitboard my_p = board.pieces(my_p_piece);

    if (popcount(my_b) != 1 || popcount(opp_b) != 1 || popcount(my_p) != 1) return WDLScore::Unknown;

    Square b1 = lsb(my_b);
    Square b2 = lsb(opp_b);

    // Opposite colored bishops
    if (is_dark_square(b1) != is_dark_square(b2)) {
        Square p_sq = lsb(my_p);
        Square opp_k = (strong_side == Color::White) ? lsb(board.pieces(make_piece(Color::Black, PieceType::King)))
                                                     : lsb(board.pieces(make_piece(Color::White, PieceType::King)));

        int p_f = static_cast<int>(file_of(p_sq));
        int opp_k_f = static_cast<int>(file_of(opp_k));
        int p_r = static_cast<int>(rank_of(p_sq));
        int opp_k_r = static_cast<int>(rank_of(opp_k));

        if (strong_side == Color::White) {
            if (opp_k_f == p_f && opp_k_r > p_r && opp_k_r >= 6) {
                return WDLScore::Draw;
            }
        } else {
            if (opp_k_f == p_f && opp_k_r < p_r && opp_k_r <= 1) {
                return WDLScore::Draw;
            }
        }
    }

    return WDLScore::Unknown;
}

WDLScore SyzygyTablebase::solve_krp_kr(const Board& board, Color strong_side) {
    Color stm = board.side_to_move();
    Piece my_r = make_piece(strong_side, PieceType::Rook);
    Piece opp_r = make_piece(~strong_side, PieceType::Rook);
    Piece my_p = make_piece(strong_side, PieceType::Pawn);

    Bitboard r1 = board.pieces(my_r);
    Bitboard r2 = board.pieces(opp_r);
    Bitboard p = board.pieces(my_p);

    if (popcount(r1) != 1 || popcount(r2) != 1 || popcount(p) != 1) return WDLScore::Unknown;

    Square p_sq = lsb(p);
    Square sk_sq = (strong_side == Color::White) ? lsb(board.pieces(make_piece(Color::White, PieceType::King)))
                                                 : lsb(board.pieces(make_piece(Color::Black, PieceType::King)));
    Square dk_sq = (strong_side == Color::White) ? lsb(board.pieces(make_piece(Color::Black, PieceType::King)))
                                                 : lsb(board.pieces(make_piece(Color::White, PieceType::King)));

    File pf = file_of(p_sq);
    Rank pr = rank_of(p_sq);
    File sk_f = file_of(sk_sq);
    Rank sk_r = rank_of(sk_sq);
    File dk_f = file_of(dk_sq);
    Rank dk_r = rank_of(dk_sq);

    // 1. Lucena Position: Pawn on 7th rank (not rook pawn), strong King on 8th rank, defending king cut off
    if (strong_side == Color::White) {
        if (pr == Rank::Rank7 && pf >= File::FileB && pf <= File::FileG) {
            if (sk_sq == make_square(pf, Rank::Rank8) && std::abs(static_cast<int>(dk_f) - static_cast<int>(pf)) >= 2) {
                return (stm == strong_side) ? WDLScore::Win : WDLScore::Loss;
            }
        }
    } else {
        if (pr == Rank::Rank2 && pf >= File::FileB && pf <= File::FileG) {
            if (sk_sq == make_square(pf, Rank::Rank1) && std::abs(static_cast<int>(dk_f) - static_cast<int>(pf)) >= 2) {
                return (stm == strong_side) ? WDLScore::Win : WDLScore::Loss;
            }
        }
    }

    // 2. Philidor Position: Defending king on promotion square, defending rook on 6th rank
    if (strong_side == Color::White) {
        if (pr <= Rank::Rank5 && dk_sq == make_square(pf, Rank::Rank8)) {
            Square opp_r_sq = lsb(r2);
            if (rank_of(opp_r_sq) == Rank::Rank6 && sk_r <= Rank::Rank5) {
                return WDLScore::Draw;
            }
        }
    } else {
        if (pr >= Rank::Rank4 && dk_sq == make_square(pf, Rank::Rank1)) {
            Square opp_r_sq = lsb(r2);
            if (rank_of(opp_r_sq) == Rank::Rank3 && sk_r >= Rank::Rank4) {
                return WDLScore::Draw;
            }
        }
    }

    return WDLScore::Unknown;
}

} // namespace heavensgate
