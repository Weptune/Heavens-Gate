#pragma once

#include "../board/board.hpp"
#include "../search/search.hpp"
#include <string>

namespace heavensgate {

class UCI {
public:
    static void loop();
    static void handle_position(const std::string& line, Board& board);
    static void handle_go(const std::string& line, Board& board, SearchEngine& engine);
};

} // namespace heavensgate
