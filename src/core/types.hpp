#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <array>
#include <ostream>

namespace heavensgate {

constexpr int ScoreInfinity = 30000;
constexpr int ScoreMate     = 25000;
constexpr int ScoreDraw     = 0;

// Colors
enum class Color : uint8_t {
    White = 0,
    Black = 1,
    None  = 2
};

constexpr Color operator~(Color c) noexcept {
    return c == Color::White ? Color::Black : (c == Color::Black ? Color::White : Color::None);
}

// Piece Types
enum class PieceType : uint8_t {
    Pawn   = 0,
    Knight = 1,
    Bishop = 2,
    Rook   = 3,
    Queen  = 4,
    King   = 5,
    None   = 6
};

// Pieces (0 to 11, 12 for None)
enum class Piece : uint8_t {
    WhitePawn   = 0,
    WhiteKnight = 1,
    WhiteBishop = 2,
    WhiteRook   = 3,
    WhiteQueen  = 4,
    WhiteKing   = 5,
    BlackPawn   = 6,
    BlackKnight = 7,
    BlackBishop = 8,
    BlackRook   = 9,
    BlackQueen  = 10,
    BlackKing   = 11,
    None        = 12
};

// Files (0 = A, 7 = H)
enum class File : uint8_t {
    FileA = 0, FileB, FileC, FileD, FileE, FileF, FileG, FileH, None
};

// Ranks (0 = 1st rank, 7 = 8th rank)
enum class Rank : uint8_t {
    Rank1 = 0, Rank2, Rank3, Rank4, Rank5, Rank6, Rank7, Rank8, None
};

// Squares (0 = a1, 63 = h8, 64 = None)
enum class Square : uint8_t {
    a1 = 0,  b1, c1, d1, e1, f1, g1, h1,
    a2,      b2, c2, d2, e2, f2, g2, h2,
    a3,      b3, c3, d3, e3, f3, g3, h3,
    a4,      b4, c4, d4, e4, f4, g4, h4,
    a5,      b5, c5, d5, e5, f5, g5, h5,
    a6,      b6, c6, d6, e6, f6, g6, h6,
    a7,      b7, c7, d7, e7, f7, g7, h7,
    a8,      b8, c8, d8, e8, f8, g8, h8,
    None = 64
};

// Castling Rights (Bitfield)
enum CastlingRights : uint8_t {
    CastlingNone     = 0,
    WhiteOO          = 1 << 0, // White Kingside
    WhiteOOO         = 1 << 1, // White Queenside
    BlackOO          = 1 << 2, // Black Kingside
    BlackOOO         = 1 << 3, // Black Queenside
    WhiteCastling    = WhiteOO | WhiteOOO,
    BlackCastling    = BlackOO | BlackOOO,
    AllCastling      = WhiteCastling | BlackCastling
};

constexpr CastlingRights operator|(CastlingRights a, CastlingRights b) noexcept {
    return static_cast<CastlingRights>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
constexpr CastlingRights operator&(CastlingRights a, CastlingRights b) noexcept {
    return static_cast<CastlingRights>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
constexpr CastlingRights& operator|=(CastlingRights& a, CastlingRights b) noexcept {
    a = a | b;
    return a;
}
constexpr CastlingRights& operator&=(CastlingRights& a, CastlingRights b) noexcept {
    a = a & b;
    return a;
}
constexpr CastlingRights operator~(CastlingRights a) noexcept {
    return static_cast<CastlingRights>(~static_cast<uint8_t>(a) & 0x0F);
}

// Move Types
enum class MoveType : uint8_t {
    Quiet           = 0,
    DoublePawnPush  = 1,
    KingCastle      = 2,
    QueenCastle     = 3,
    Capture         = 4,
    EnPassant       = 5,
    PromoKnight     = 8,
    PromoBishop     = 9,
    PromoRook       = 10,
    PromoQueen      = 11,
    PromoCaptureKnight = 12,
    PromoCaptureBishop = 13,
    PromoCaptureRook   = 14,
    PromoCaptureQueen  = 15
};

// Move Structure (16-bit compact layout)
class Move {
private:
    uint16_t data_{0};

public:
    constexpr Move() noexcept = default;
    constexpr Move(Square from, Square to, MoveType type = MoveType::Quiet) noexcept
        : data_(static_cast<uint16_t>(from) | 
               (static_cast<uint16_t>(to) << 6) | 
               (static_cast<uint16_t>(type) << 12)) {}

    constexpr Square from() const noexcept { return static_cast<Square>(data_ & 0x3F); }
    constexpr Square to() const noexcept { return static_cast<Square>((data_ >> 6) & 0x3F); }
    constexpr MoveType type() const noexcept { return static_cast<MoveType>((data_ >> 12) & 0x0F); }
    constexpr uint16_t raw() const noexcept { return data_; }

    constexpr bool is_quiet() const noexcept {
        return type() == MoveType::Quiet || type() == MoveType::DoublePawnPush || 
               type() == MoveType::KingCastle || type() == MoveType::QueenCastle;
    }

    constexpr bool is_capture() const noexcept {
        auto t = type();
        return t == MoveType::Capture || t == MoveType::EnPassant || 
               (t >= MoveType::PromoCaptureKnight && t <= MoveType::PromoCaptureQueen);
    }

    constexpr bool is_promotion() const noexcept {
        auto t = type();
        return (t >= MoveType::PromoKnight && t <= MoveType::PromoQueen) ||
               (t >= MoveType::PromoCaptureKnight && t <= MoveType::PromoCaptureQueen);
    }

    constexpr PieceType promotion_piece_type() const noexcept {
        switch (type()) {
            case MoveType::PromoKnight:
            case MoveType::PromoCaptureKnight: return PieceType::Knight;
            case MoveType::PromoBishop:
            case MoveType::PromoCaptureBishop: return PieceType::Bishop;
            case MoveType::PromoRook:
            case MoveType::PromoCaptureRook:   return PieceType::Rook;
            case MoveType::PromoQueen:
            case MoveType::PromoCaptureQueen:  return PieceType::Queen;
            default: return PieceType::None;
        }
    }

    constexpr bool is_castle() const noexcept {
        return type() == MoveType::KingCastle || type() == MoveType::QueenCastle;
    }

    constexpr bool operator==(const Move& other) const noexcept { return data_ == other.data_; }
    constexpr bool operator!=(const Move& other) const noexcept { return data_ != other.data_; }
    constexpr explicit operator bool() const noexcept { return data_ != 0; }
};

// Helper Functions
constexpr Square make_square(File f, Rank r) noexcept {
    return static_cast<Square>(static_cast<uint8_t>(r) * 8 + static_cast<uint8_t>(f));
}

constexpr File file_of(Square sq) noexcept {
    return static_cast<File>(static_cast<uint8_t>(sq) & 7);
}

constexpr Rank rank_of(Square sq) noexcept {
    return static_cast<Rank>(static_cast<uint8_t>(sq) >> 3);
}

constexpr Piece make_piece(Color c, PieceType pt) noexcept {
    if (c == Color::None || pt == PieceType::None) return Piece::None;
    return static_cast<Piece>(static_cast<uint8_t>(c) * 6 + static_cast<uint8_t>(pt));
}

constexpr PieceType piece_type_of(Piece p) noexcept {
    if (p == Piece::None) return PieceType::None;
    return static_cast<PieceType>(static_cast<uint8_t>(p) % 6);
}

constexpr Color color_of(Piece p) noexcept {
    if (p == Piece::None) return Color::None;
    return static_cast<Color>(static_cast<uint8_t>(p) / 6);
}

inline std::string square_to_string(Square sq) {
    if (sq == Square::None) return "-";
    std::string s;
    s += static_cast<char>('a' + static_cast<uint8_t>(file_of(sq)));
    s += static_cast<char>('1' + static_cast<uint8_t>(rank_of(sq)));
    return s;
}

inline std::string move_to_uci(Move m) {
    if (!m) return "0000";
    std::string uci = square_to_string(m.from()) + square_to_string(m.to());
    if (m.is_promotion()) {
        switch (m.promotion_piece_type()) {
            case PieceType::Knight: uci += 'n'; break;
            case PieceType::Bishop: uci += 'b'; break;
            case PieceType::Rook:   uci += 'r'; break;
            case PieceType::Queen:  uci += 'q'; break;
            default: break;
        }
    }
    return uci;
}

constexpr char piece_to_char(Piece p) noexcept {
    constexpr std::array<char, 13> PieceChars = {
        'P', 'N', 'B', 'R', 'Q', 'K',
        'p', 'n', 'b', 'r', 'q', 'k',
        '.'
    };
    return PieceChars[static_cast<size_t>(p)];
}

constexpr Piece char_to_piece(char c) noexcept {
    switch (c) {
        case 'P': return Piece::WhitePawn;
        case 'N': return Piece::WhiteKnight;
        case 'B': return Piece::WhiteBishop;
        case 'R': return Piece::WhiteRook;
        case 'Q': return Piece::WhiteQueen;
        case 'K': return Piece::WhiteKing;
        case 'p': return Piece::BlackPawn;
        case 'n': return Piece::BlackKnight;
        case 'b': return Piece::BlackBishop;
        case 'r': return Piece::BlackRook;
        case 'q': return Piece::BlackQueen;
        case 'k': return Piece::BlackKing;
        default:  return Piece::None;
    }
}

} // namespace heavensgate
