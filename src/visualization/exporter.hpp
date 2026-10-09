#pragma once

#include "../core/types.hpp"
#include "../board/board.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <array>

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

struct SearchTreeNode {
    Move move;
    std::array<char, 128> fen{};
    int eval = 0, depth = 0, ply = 0;
    bool is_pruned = false, is_terminal = false;
    size_t first_child = 0, last_child = 0, next_sibling = 0;
};

class GameTreeExporter {
public:
    explicit GameTreeExporter(bool allocate_trace = true);

    void reset(const Board& root_board);
    TreeNodeJSON& root() { return root_node_; }
    SearchTreeNode* trace_root() noexcept { return trace_ ? &(*trace_)[0] : nullptr; }
    SearchTreeNode* add_child(SearchTreeNode* parent, const Board& board, Move move, int depth, int ply) noexcept;
    bool trace_truncated() const noexcept { return trace_truncated_; }

    std::string to_json_string() const;
    void export_to_file(const std::string& filename) const;
    bool export_file(const std::string& filepath) const;

private:
    TreeNodeJSON root_node_;
    static constexpr size_t TraceCapacity = 8192;
    std::unique_ptr<std::array<SearchTreeNode, TraceCapacity>> trace_;
    size_t trace_size_ = 0;
    bool trace_truncated_ = false;
    void write_trace(std::ostream& stream, size_t index) const;
};

using TreeExporter = GameTreeExporter;

} // namespace heavensgate
