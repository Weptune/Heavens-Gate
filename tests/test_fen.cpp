#include "test.hpp"
#include "../src/core/fen.hpp"
#include "../src/board/board.hpp"
#include <iostream>

namespace heavensgate::test {

static bool test_fen_startpos() {
    Board board;
    if (!FEN::parse(StartposFEN, board)) return false;

    if (board.side_to_move() != Color::White) return false;
    if (board.castling_rights() != AllCastling) return false;
    if (board.en_passant_sq() != Square::None) return false;
    if (board.piece_at(Square::e1) != Piece::WhiteKing) return false;
    if (board.piece_at(Square::e8) != Piece::BlackKing) return false;

    std::string fen_out = FEN::to_string(board);
    return fen_out == StartposFEN;
}

static bool test_fen_custom_position() {
    std::string custom_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    Board board;
    if (!FEN::parse(custom_fen, board)) return false;

    if (board.side_to_move() != Color::White) return false;
    if (board.castling_rights() != AllCastling) return false;
    if (board.piece_at(Square::e5) != Piece::WhiteKnight) return false;
    if (board.piece_at(Square::a6) != Piece::BlackBishop) return false;

    std::string fen_out = FEN::to_string(board);
    return fen_out == custom_fen;
}

static bool dummy_fen_init = []() {
    register_test("FEN Parser: Standard Initial Position", test_fen_startpos);
    register_test("FEN Parser: Custom Kiweteam Position", test_fen_custom_position);
    return true;
}();

} // namespace heavensgate::test
