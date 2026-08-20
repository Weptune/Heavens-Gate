#include "pst.hpp"

namespace heavensgate {

// Pawns incentivized to advance and control the center (d4/e4/d5/e5)
// Array is oriented Rank 1 (sq 0..7) to Rank 8 (sq 56..63)
const std::array<int, 64> PieceSquareTables::PawnMG = {
      0,  0,  0,  0,  0,  0,  0,  0, // Rank 1 (a1..h1)
      0, 10, 15, 20, 20, 15, 10,  0, // Rank 2 (a2..h2)
      5, -5,-10,  0,  0,-10, -5,  5, // Rank 3 (a3..h3)
      0,  0,  0, 20, 20,  0,  0,  0, // Rank 4 (a4..h4)
      5,  5, 10, 25, 25, 10,  5,  5, // Rank 5 (a5..h5)
     10, 10, 20, 30, 30, 20, 10, 10, // Rank 6 (a6..h6)
     50, 50, 50, 50, 50, 50, 50, 50, // Rank 7 (a7..h7)
      0,  0,  0,  0,  0,  0,  0,  0  // Rank 8 (a8..h8)
};

const std::array<int, 64> PieceSquareTables::PawnEG = {
      0,  0,  0,  0,  0,  0,  0,  0, // Rank 1
     10, 10, 10, 10, 10, 10, 10, 10, // Rank 2
     10, 10, 10, 10, 10, 10, 10, 10, // Rank 3
     20, 20, 20, 20, 20, 20, 20, 20, // Rank 4
     30, 30, 30, 30, 30, 30, 30, 30, // Rank 5
     50, 50, 50, 50, 50, 50, 50, 50, // Rank 6
     80, 80, 80, 80, 80, 80, 80, 80, // Rank 7 (+80 cp promotion incentive!)
      0,  0,  0,  0,  0,  0,  0,  0  // Rank 8
};

// Knights heavily penalized on edges ("Knights on the rim are dim")
const std::array<int, 64> PieceSquareTables::KnightMG = {
    -50,-40,-30,-30,-30,-30,-40,-50, // Rank 1
    -40,-20,  0,  5,  5,  0,-20,-40, // Rank 2
    -30,  5, 10, 15, 15, 10,  5,-30, // Rank 3
    -30,  0, 15, 20, 20, 15,  0,-30, // Rank 4
    -30,  5, 15, 20, 20, 15,  5,-30, // Rank 5
    -30,  0, 10, 15, 15, 10,  0,-30, // Rank 6
    -40,-20,  0,  0,  0,  0,-20,-40, // Rank 7
    -50,-40,-30,-30,-30,-30,-40,-50  // Rank 8
};

const std::array<int, 64> PieceSquareTables::KnightEG = {
    -50,-40,-30,-30,-30,-30,-40,-50, // Rank 1
    -40,-20,  0,  5,  5,  0,-20,-40, // Rank 2
    -30,  0, 15, 20, 20, 15,  0,-30, // Rank 3
    -30,  5, 25, 30, 30, 25,  5,-30, // Rank 4 (Centralization bonus)
    -30,  5, 25, 30, 30, 25,  5,-30, // Rank 5 (Centralization bonus)
    -30,  0, 15, 20, 20, 15,  0,-30, // Rank 6
    -40,-20,  0,  5,  5,  0,-20,-40, // Rank 7
    -50,-40,-30,-30,-30,-30,-40,-50  // Rank 8
};

// Bishops rewarded on long diagonals & active fianchetto
const std::array<int, 64> PieceSquareTables::BishopMG = {
    -20,-10,-10,-10,-10,-10,-10,-20, // Rank 1
    -10,  5,  0,  0,  0,  0,  5,-10, // Rank 2 (b2/g2 fianchetto)
    -10, 10, 10, 10, 10, 10, 10,-10, // Rank 3
    -10,  0, 10, 10, 10, 10,  0,-10, // Rank 4
    -10,  5,  5, 10, 10,  5,  5,-10, // Rank 5
    -10,  0,  5, 10, 10,  5,  0,-10, // Rank 6
    -10,  0,  0,  0,  0,  0,  0,-10, // Rank 7
    -20,-10,-10,-10,-10,-10,-10,-20  // Rank 8
};

const std::array<int, 64> PieceSquareTables::BishopEG = {
    -20,-10,-10,-10,-10,-10,-10,-20, // Rank 1
    -10,  0,  5,  5,  5,  5,  0,-10, // Rank 2
    -10,  5, 15, 15, 15, 15,  5,-10, // Rank 3
    -10,  5, 15, 20, 20, 15,  5,-10, // Rank 4 (Central scope)
    -10,  5, 15, 20, 20, 15,  5,-10, // Rank 5 (Central scope)
    -10,  5, 15, 15, 15, 15,  5,-10, // Rank 6
    -10,  0,  5,  5,  5,  5,  0,-10, // Rank 7
    -20,-10,-10,-10,-10,-10,-10,-20  // Rank 8
};

// Rooks rewarded on 7th rank & active central files
const std::array<int, 64> PieceSquareTables::RookMG = {
      0,  0,  0,  5,  5,  0,  0,  0, // Rank 1 (d1/e1)
     -5,  0,  0,  0,  0,  0,  0, -5, // Rank 2
     -5,  0,  0,  0,  0,  0,  0, -5, // Rank 3
     -5,  0,  0,  0,  0,  0,  0, -5, // Rank 4
     -5,  0,  0,  0,  0,  0,  0, -5, // Rank 5
     -5,  0,  0,  0,  0,  0,  0, -5, // Rank 6
      5, 10, 10, 10, 10, 10, 10,  5, // Rank 7 (Rook on 7th rank!)
      0,  0,  0,  0,  0,  0,  0,  0  // Rank 8
};

const std::array<int, 64> PieceSquareTables::RookEG = {
      0,  0,  0,  0,  0,  0,  0,  0, // Rank 1
      0,  0,  0,  0,  0,  0,  0,  0, // Rank 2
      0,  0,  0,  0,  0,  0,  0,  0, // Rank 3
      0,  0,  0,  0,  0,  0,  0,  0, // Rank 4
      5,  5,  5,  5,  5,  5,  5,  5, // Rank 5
     10, 10, 10, 10, 10, 10, 10, 10, // Rank 6
     20, 20, 20, 20, 20, 20, 20, 20, // Rank 7 (Rook on 7th in EG!)
     10, 10, 10, 10, 10, 10, 10, 10  // Rank 8
};

// Queens central control & activity
const std::array<int, 64> PieceSquareTables::QueenMG = {
    -20,-10,-10, -5, -5,-10,-10,-20, // Rank 1
    -10,  0,  5,  0,  0,  0,  0,-10, // Rank 2
    -10,  5,  5,  5,  5,  5,  0,-10, // Rank 3
      0,  0,  5,  5,  5,  5,  0, -5, // Rank 4
     -5,  0,  5,  5,  5,  5,  0, -5, // Rank 5
    -10,  0,  5,  5,  5,  5,  0,-10, // Rank 6
    -10,  0,  0,  0,  0,  0,  0,-10, // Rank 7
    -20,-10,-10, -5, -5,-10,-10,-20  // Rank 8
};

const std::array<int, 64> PieceSquareTables::QueenEG = {
    -20,-10,-10, -5, -5,-10,-10,-20, // Rank 1
    -10,  0,  0,  0,  0,  0,  0,-10, // Rank 2
    -10,  0, 10, 10, 10, 10,  0,-10, // Rank 3
     -5,  0, 15, 20, 20, 15,  0, -5, // Rank 4 (Queen centralization)
     -5,  0, 15, 20, 20, 15,  0, -5, // Rank 5 (Queen centralization)
    -10,  0, 10, 10, 10, 10,  0,-10, // Rank 6
    -10,  0,  0,  0,  0,  0,  0,-10, // Rank 7
    -20,-10,-10, -5, -5,-10,-10,-20  // Rank 8
};

// King: Hidden on back rank during Midgame (g1/c1 = +30 cp), Centralized during Endgame (d4/e4/d5/e5 = +40 cp)!
const std::array<int, 64> PieceSquareTables::KingMG = {
      20, 30, 10,  0,  0, 10, 30, 20, // Rank 1 (g1 = sq 6 is +30, c1 = sq 2 is +10/30)
      20, 20,  0,  0,  0,  0, 20, 20, // Rank 2
     -10,-20,-20,-20,-20,-20,-20,-10, // Rank 3
     -20,-30,-30,-40,-40,-30,-30,-20, // Rank 4
     -30,-40,-40,-50,-50,-40,-40,-30, // Rank 5
     -30,-40,-40,-50,-50,-40,-40,-30, // Rank 6
     -30,-40,-40,-50,-50,-40,-40,-30, // Rank 7
     -30,-40,-40,-50,-50,-40,-40,-30  // Rank 8
};

const std::array<int, 64> PieceSquareTables::KingEG = {
     -50,-30,-30,-30,-30,-30,-30,-50, // Rank 1
     -30,-30,  0,  0,  0,  0,-30,-30, // Rank 2
     -30,-10, 20, 30, 30, 20,-10,-30, // Rank 3
     -30,-10, 30, 40, 40, 30,-10,-30, // Rank 4 (d4/e4 centralized = +40 cp!)
     -30,-10, 30, 40, 40, 30,-10,-30, // Rank 5 (d5/e5 centralized = +40 cp!)
     -30,-10, 20, 30, 30, 20,-10,-30, // Rank 6
     -30,-20,-10,  0,  0,-10,-20,-30, // Rank 7
     -50,-40,-30,-20,-20,-30,-40,-50  // Rank 8
};

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
