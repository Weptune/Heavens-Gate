#pragma once

#include "../board/board.hpp"
#include "../core/fen.hpp"
#include <string>
#include <vector>
#include <fstream>

namespace heavensgate {

struct TreeNodeJSON {
    std::string move_uci;
    std::string fen;
    int eval;
    int depth;
    int ply;
    bool is_terminal{false};
    bool is_pruned{false};
    std::vector<TreeNodeJSON> children;
};

class TreeExporter {
private:
    TreeNodeJSON root_;

public:
    void reset(const Board& root_board) {
        root_ = TreeNodeJSON{};
        root_.move_uci = "root";
        root_.fen = FEN::to_string(root_board);
        root_.depth = 0;
        root_.ply = 0;
        root_.eval = 0;
        root_.is_terminal = false;
        root_.is_pruned = false;
    }

    TreeNodeJSON& root() { return root_; }

    static void save_to_json(const TreeNodeJSON& node, std::ostream& out, int indent = 0);
    bool export_file(const std::string& filepath) const;
};

} // namespace heavensgate
