#pragma once

#include "../board/board.hpp"
#include <string>
#include <string_view>

namespace heavensgate {

class FEN {
public:
    static bool parse(std::string_view fen, Board& board);
    static std::string to_string(const Board& board);
};

} // namespace heavensgate
