#include "eval.hpp"
#include "pst.hpp"
#include <algorithm>

namespace heavensgate {

EvalMode Evaluator::current_mode_ = EvalMode::MasterPositional;

void Evaluator::init() {
    PST::init();
    EvalFeatures::init();
    current_mode_ = EvalMode::MasterPositional;
}

int Evaluator::evaluate_side(const Board& board, Color side) {
    int mg_material = 0;
    int eg_material = 0;

    int mg_pst = 0;
    int eg_pst = 0;

    auto score_piece = [&](PieceType pt, int mg_val, int eg_val) {
        Piece p = make_piece(side, pt);
        Bitboard bb = board.pieces(p);

        while (bb) {
            Square sq = pop_lsb(bb);
            mg_material += mg_val;
            eg_material += eg_val;

            mg_pst += PST::get_mg(pt, side, sq);
            eg_pst += PST::get_eg(pt, side, sq);
        }
    };

    score_piece(PieceType::Pawn,   100, 120);
    score_piece(PieceType::Knight, 320, 310);
    score_piece(PieceType::Bishop, 330, 340);
    score_piece(PieceType::Rook,   500, 530);
    score_piece(PieceType::Queen,  900, 950);
    score_piece(PieceType::King,     0,   0);

    if (current_mode_ == EvalMode::MaterialOnly) {
        return mg_material + mg_pst;
    }

    // Master Positional Features
    int pawn_struct  = EvalFeatures::evaluate_pawn_structure(board, side);
    int passed_pawns = EvalFeatures::evaluate_passed_pawns(board, side);
    int bad_bishops  = EvalFeatures::evaluate_bad_bishops(board, side);
    int pin_threats  = EvalFeatures::evaluate_pin_threats(board, side);
    int king_safety  = EvalFeatures::evaluate_king_safety(board, side);
    int activity     = EvalFeatures::evaluate_piece_activity(board, side);
    int mobility     = EvalFeatures::evaluate_mobility(board, side);

    int pos_total = pawn_struct + passed_pawns + bad_bishops + pin_threats + king_safety + activity + mobility;

    int knights = popcount(board.pieces(make_piece(side, PieceType::Knight)));
    int bishops = popcount(board.pieces(make_piece(side, PieceType::Bishop)));
    int rooks   = popcount(board.pieces(make_piece(side, PieceType::Rook)));
    int queens  = popcount(board.pieces(make_piece(side, PieceType::Queen)));
    int game_phase = knights * 1 + bishops * 1 + rooks * 2 + queens * 4;

    int mg_total = mg_material + mg_pst + pos_total;
    int eg_total = eg_material + eg_pst + pos_total;

    int mg_weight = std::min(game_phase, 24);
    int eg_weight = 24 - mg_weight;

    return (mg_total * mg_weight + eg_total * eg_weight) / 24;
}

int Evaluator::evaluate(const Board& board) {
    int white_score = evaluate_side(board, Color::White);
    int black_score = evaluate_side(board, Color::Black);

    int relative_score = white_score - black_score;
    return (board.side_to_move() == Color::White) ? relative_score : -relative_score;
}

} // namespace heavensgate
