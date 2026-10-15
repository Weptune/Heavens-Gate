#include "pst.hpp"

namespace heavensgate {

#include "pst_tables.inc"

std::array<std::array<std::array<std::array<int, 64>, 6>, 2>, 2> PieceSquareTables::combined = [] {
    std::array<std::array<std::array<std::array<int, 64>, 6>, 2>, 2> t{};
    for (int eg = 0; eg < 2; ++eg)
        for (int c = 0; c < 2; ++c)
            for (int pt = 0; pt < 6; ++pt)
                for (int sq = 0; sq < 64; ++sq)
                    t[eg][c][pt][sq] = get_pst_value(static_cast<PieceType>(pt), static_cast<Color>(c), static_cast<Square>(sq), eg == 1);
    return t;
}();

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
