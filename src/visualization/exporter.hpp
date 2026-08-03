#pragma once

#include "../core/types.hpp"
#include "../board/board.hpp"
#include <string>
#include <vector>
#include <sstream>

namespace heavensgate {

struct TreeNodeJSON {
    std::string move_uci;
    std::string fen;
    int eval = 0;
    int depth = 0;
    int ply = 0;
    bool is_pruned = false;
    bool is_terminal = false;
    std::vector<TreeNodeJSON> children;
};

class GameTreeExporter {
public:
    GameTreeExporter();

    void reset(const Board& root_board);
    TreeNodeJSON& root() { return root_node_; }

    std::string to_json_string() const;
    void export_to_file(const std::string& filename) const;
    bool export_file(const std::string& filepath) const;

private:
    TreeNodeJSON root_node_;
};

using TreeExporter = GameTreeExporter;

} // namespace heavensgate
