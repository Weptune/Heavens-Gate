#include "eval.hpp"

namespace heavensgate {

int Evaluator::evaluate_material(const Board& board) {
    Color us = board.side_to_move();
    Color them = ~us;

    int us_material = popcount(board.pieces(make_piece(us, PieceType::Pawn))) * PawnValue +
                      popcount(board.pieces(make_piece(us, PieceType::Knight))) * KnightValue +
                      popcount(board.pieces(make_piece(us, PieceType::Bishop))) * BishopValue +
                      popcount(board.pieces(make_piece(us, PieceType::Rook))) * RookValue +
                      popcount(board.pieces(make_piece(us, PieceType::Queen))) * QueenValue;

    int them_material = popcount(board.pieces(make_piece(them, PieceType::Pawn))) * PawnValue +
                        popcount(board.pieces(make_piece(them, PieceType::Knight))) * KnightValue +
                        popcount(board.pieces(make_piece(them, PieceType::Bishop))) * BishopValue +
                        popcount(board.pieces(make_piece(them, PieceType::Rook))) * RookValue +
                        popcount(board.pieces(make_piece(them, PieceType::Queen))) * QueenValue;

    return us_material - them_material;
}

int Evaluator::evaluate(const Board& board) {
    return evaluate_material(board);
}

} // namespace heavensgate
