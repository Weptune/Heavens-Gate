#include "syzygy.hpp"
#include "../movegen/movegen.hpp"
#include <cmath>
#include <algorithm>

namespace heavensgate {

void SyzygyTablebase::init(const std::string& tb_path) {
    // Default enabled for endgames with 5 or fewer pieces
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

    Square wk_sq = lsb(w_king);
    Square bk_sq = lsb(b_king);

    int num_w_queens = popcount(board.pieces(make_piece(Color::White, PieceType::Queen)));
    int num_b_queens = popcount(board.pieces(make_piece(Color::Black, PieceType::Queen)));
    int num_w_rooks  = popcount(board.pieces(make_piece(Color::White, PieceType::Rook)));
    int num_b_rooks  = popcount(board.pieces(make_piece(Color::Black, PieceType::Rook)));
    int num_w_pawns  = popcount(board.pieces(make_piece(Color::White, PieceType::Pawn)));
    int num_b_pawns  = popcount(board.pieces(make_piece(Color::Black, PieceType::Pawn)));

    int total_w = popcount(board.pieces(Color::White));
    int total_b = popcount(board.pieces(Color::Black));

    Color stm = board.side_to_move();

    // 1. KQK (King + Queen vs King)
    if (total_w == 2 && total_b == 1 && num_w_queens == 1) {
        WDLScore w = solve_kqk(board, Color::White);
        return (stm == Color::White) ? w : (w == WDLScore::Win ? WDLScore::Loss : WDLScore::Win);
    }
    if (total_b == 2 && total_w == 1 && num_b_queens == 1) {
        WDLScore w = solve_kqk(board, Color::Black);
        return (stm == Color::Black) ? w : (w == WDLScore::Win ? WDLScore::Loss : WDLScore::Win);
    }

    // 2. KRK (King + Rook vs King)
    if (total_w == 2 && total_b == 1 && num_w_rooks == 1) {
        WDLScore w = solve_krk(board, Color::White);
        return (stm == Color::White) ? w : (w == WDLScore::Win ? WDLScore::Loss : WDLScore::Win);
    }
    if (total_b == 2 && total_w == 1 && num_b_rooks == 1) {
        WDLScore w = solve_krk(board, Color::Black);
        return (stm == Color::Black) ? w : (w == WDLScore::Win ? WDLScore::Loss : WDLScore::Win);
    }

    // 3. KPK (King + Pawn vs King)
    if (total_w == 2 && total_b == 1 && num_w_pawns == 1) {
        return solve_kpk(board, Color::White);
    }
    if (total_b == 2 && total_w == 1 && num_b_pawns == 1) {
        return solve_kpk(board, Color::Black);
    }

    // 4. KK (Bare Kings = Draw)
    if (total_w == 1 && total_b == 1) {
        return WDLScore::Draw;
    }

    return WDLScore::Unknown;
}

WDLScore SyzygyTablebase::solve_kqk(const Board& board, Color strong_side) {
    return WDLScore::Win;
}

WDLScore SyzygyTablebase::solve_krk(const Board& board, Color strong_side) {
    return WDLScore::Win;
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

    // Passed pawn distance rule (Square of the pawn)
    int target_rank = (strong_side == Color::White) ? 7 : 0;
    int steps_to_promote = std::abs(target_rank - p_rank);

    Square opp_k_sq = (strong_side == Color::White) ? bk_sq : wk_sq;

    int opp_dist = std::max(std::abs(static_cast<int>(file_of(opp_k_sq)) - p_file), std::abs(static_cast<int>(rank_of(opp_k_sq)) - target_rank));

    // If opponent King is outside the square of the pawn, it's a forced Win!
    if (steps_to_promote < opp_dist) {
        return WDLScore::Win;
    }

    return WDLScore::Unknown;
}

} // namespace heavensgate
