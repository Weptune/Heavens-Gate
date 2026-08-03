#pragma once

#include "types.hpp"
#include <string>
#include <string_view>

namespace heavensgate {

class Board; // Forward declaration

class FEN {
public:
    static constexpr std::string_view StartPOS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    static constexpr std::string_view StartposFEN = StartPOS;

    static bool parse(std::string_view fen, Board& board);
    static std::string to_string(const Board& board);
};

inline constexpr std::string_view StartposFEN = FEN::StartPOS;

} // namespace heavensgate
