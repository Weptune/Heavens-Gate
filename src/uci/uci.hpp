#pragma once

#include "../board/board.hpp"
#include "../search/search.hpp"
#include <string>

namespace heavensgate {

class UCI {
public:
    // first_line: a command already read by the caller (e.g. "uci" from the menu).
    static void loop(const std::string& first_line = "");
    static void handle_position(const std::string& line, Board& board);
    static void handle_go(const std::string& line, Board& board, SearchEngine& engine);
};

} // namespace heavensgate
