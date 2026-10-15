#include "pst.hpp"

namespace heavensgate {

#include "pst_tables.inc"

int PieceSquareTables::get_pst_value(PieceType pt, Color c, Square sq, bool is_endgame) noexcept {
    if (pt == PieceType::None || sq == Square::None) return 0;
    Square eval_sq = (c == Color::White) ? sq : flip_sq(sq);
    size_t idx = static_cast<size_t>(eval_sq);

    switch (pt) {
        case PieceType::Pawn:   return is_endgame ? PawnEG[idx]   : PawnMG[idx];
        case PieceType::Knight: return is_endgame ? KnightEG[idx] : KnightMG[idx];
        case PieceType::Bishop: return is_endgame ? BishopEG[idx] : BishopMG[idx];
        case PieceType::Rook:   return is_endgame ? RookEG[idx]   : RookMG[idx];
        case PieceType::Queen:  return is_endgame ? QueenEG[idx]  : QueenMG[idx];
        case PieceType::King:   return is_endgame ? KingEG[idx]   : KingMG[idx];
        default: return 0;
    }
}

} // namespace heavensgate
