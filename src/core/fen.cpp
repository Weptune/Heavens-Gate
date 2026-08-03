#include "fen.hpp"
#include <sstream>
#include <cctype>
#include <vector>

namespace heavensgate {

bool FEN::parse(std::string_view fen_str, Board& board) {
    board.clear();
    std::string fen(fen_str);
    std::stringstream ss(fen);
    
    std::string piece_placement, turn, castling, ep_sq, halfmove, fullmove;
    ss >> piece_placement >> turn >> castling >> ep_sq >> halfmove >> fullmove;

    if (piece_placement.empty()) return false;

    // 1. Piece placement
    int r = 7;
    int f = 0;
    for (char c : piece_placement) {
        if (c == '/') {
            r--;
            f = 0;
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            f += (c - '0');
        } else {
            Piece p = char_to_piece(c);
            if (p != Piece::None && f >= 0 && f < 8 && r >= 0 && r < 8) {
                Square sq = make_square(static_cast<File>(f), static_cast<Rank>(r));
                board.set_piece(sq, p);
                f++;
            } else {
                return false;
            }
        }
    }

    // 2. Active color
    if (turn == "w") {
        board.set_side_to_move(Color::White);
    } else if (turn == "b") {
        board.set_side_to_move(Color::Black);
    } else {
        board.set_side_to_move(Color::White);
    }

    // 3. Castling availability
    CastlingRights cr = CastlingNone;
    if (castling.find('K') != std::string::npos) cr |= WhiteOO;
    if (castling.find('Q') != std::string::npos) cr |= WhiteOOO;
    if (castling.find('k') != std::string::npos) cr |= BlackOO;
    if (castling.find('q') != std::string::npos) cr |= BlackOOO;
    board.set_castling_rights(cr);

    // 4. En passant target square
    if (!ep_sq.empty() && ep_sq != "-") {
        if (ep_sq.length() >= 2) {
            File ep_f = static_cast<File>(ep_sq[0] - 'a');
            Rank ep_r = static_cast<Rank>(ep_sq[1] - '1');
            board.set_en_passant_sq(make_square(ep_f, ep_r));
        }
    } else {
        board.set_en_passant_sq(Square::None);
    }

    // 5. Halfmove clock
    if (!halfmove.empty()) {
        try {
            board.set_halfmove_clock(static_cast<uint16_t>(std::stoi(halfmove)));
        } catch (...) {
            board.set_halfmove_clock(0);
        }
    }

    // 6. Fullmove number
    if (!fullmove.empty()) {
        try {
            board.set_fullmove_number(static_cast<uint16_t>(std::stoi(fullmove)));
        } catch (...) {
            board.set_fullmove_number(1);
        }
    }

    // Recalculate full Zobrist hash after parsing FEN
    board.recalculate_zobrist_key();

    return true;
}

std::string FEN::to_string(const Board& board) {
    std::string fen;

    // 1. Piece placement
    for (int r = 7; r >= 0; --r) {
        int empty_cnt = 0;
        for (int f = 0; f < 8; ++f) {
            Square sq = make_square(static_cast<File>(f), static_cast<Rank>(r));
            Piece p = board.piece_at(sq);
            if (p == Piece::None) {
                empty_cnt++;
            } else {
                if (empty_cnt > 0) {
                    fen += std::to_string(empty_cnt);
                    empty_cnt = 0;
                }
                fen += piece_to_char(p);
            }
        }
        if (empty_cnt > 0) {
            fen += std::to_string(empty_cnt);
        }
        if (r > 0) fen += '/';
    }

    // 2. Active color
    fen += ' ';
    fen += (board.side_to_move() == Color::White ? 'w' : 'b');

    // 3. Castling rights
    fen += ' ';
    CastlingRights cr = board.castling_rights();
    if (cr == CastlingNone) {
        fen += '-';
    } else {
        if (cr & WhiteOO)  fen += 'K';
        if (cr & WhiteOOO) fen += 'Q';
        if (cr & BlackOO)  fen += 'k';
        if (cr & BlackOOO) fen += 'q';
    }

    // 4. En passant square
    fen += ' ';
    fen += square_to_string(board.en_passant_sq());

    // 5. Clocks
    fen += ' ';
    fen += std::to_string(board.halfmove_clock());
    fen += ' ';
    fen += std::to_string(board.fullmove_number());

    return fen;
}

} // namespace heavensgate
