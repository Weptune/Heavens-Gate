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

int Evaluator::calculate_game_phase(const Board& board) noexcept {
    constexpr int KnightPhase = 1;
    constexpr int BishopPhase = 1;
    constexpr int RookPhase   = 2;
    constexpr int QueenPhase  = 4;

    int current_phase = 0;

    current_phase += (popcount(board.pieces(make_piece(Color::White, PieceType::Knight))) +
                      popcount(board.pieces(make_piece(Color::Black, PieceType::Knight)))) * KnightPhase;

    current_phase += (popcount(board.pieces(make_piece(Color::White, PieceType::Bishop))) +
                      popcount(board.pieces(make_piece(Color::Black, PieceType::Bishop)))) * BishopPhase;

    current_phase += (popcount(board.pieces(make_piece(Color::White, PieceType::Rook))) +
                      popcount(board.pieces(make_piece(Color::Black, PieceType::Rook)))) * RookPhase;

    current_phase += (popcount(board.pieces(make_piece(Color::White, PieceType::Queen))) +
                      popcount(board.pieces(make_piece(Color::Black, PieceType::Queen)))) * QueenPhase;

    return std::min(24, current_phase);
}

int Evaluator::evaluate(const Board& board) {
    Color us = board.side_to_move();
    Color them = ~us;

    int mg_score = 0;
    int eg_score = 0;

    // Evaluate both colors for material + PST
    auto eval_color = [&](Color c) {
        int color_mg = 0;
        int color_eg = 0;

        for (int pt_i = 0; pt_i < 6; ++pt_i) {
            PieceType pt = static_cast<PieceType>(pt_i);
            Piece p = make_piece(c, pt);
            Bitboard bb = board.pieces(p);

            int piece_val = 0;
            switch (pt) {
                case PieceType::Pawn:   piece_val = PawnValue; break;
                case PieceType::Knight: piece_val = KnightValue; break;
                case PieceType::Bishop: piece_val = BishopValue; break;
                case PieceType::Rook:   piece_val = RookValue; break;
                case PieceType::Queen:  piece_val = QueenValue; break;
                default: break;
            }

            while (bb) {
                Square sq = pop_lsb(bb);
                color_mg += piece_val + PieceSquareTables::get_pst_value(pt, c, sq, false);
                color_eg += piece_val + PieceSquareTables::get_pst_value(pt, c, sq, true);
            }
        }
        return std::make_pair(color_mg, color_eg);
    };

    auto us_scores = eval_color(us);
    auto them_scores = eval_color(them);

    mg_score = us_scores.first - them_scores.first;
    eg_score = us_scores.second - them_scores.second;

    int phase = calculate_game_phase(board);

    // Tapered evaluation interpolation
    int final_eval = (mg_score * phase + eg_score * (24 - phase)) / 24;

    return final_eval;
}

} // namespace heavensgate
