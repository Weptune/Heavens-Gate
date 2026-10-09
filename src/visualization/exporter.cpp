#include "exporter.hpp"
#include "../core/fen.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <charconv>

namespace heavensgate {

GameTreeExporter::GameTreeExporter(bool allocate_trace) {
    root_node_ = TreeNodeJSON{};
    if (allocate_trace) trace_ = std::make_unique<std::array<SearchTreeNode, TraceCapacity>>();
}

static void write_fen_buffer(const Board& board, std::array<char, 128>& buffer) noexcept;

void GameTreeExporter::reset(const Board& root_board) {
    root_node_ = TreeNodeJSON{};
    root_node_.depth = 0;
    root_node_.ply = 0;
    // The owning engine preallocates this buffer during construction, not go.
    if (!trace_) { trace_truncated_ = true; return; }
    trace_size_ = 1;
    trace_truncated_ = false;
    (*trace_)[0] = SearchTreeNode{};
    write_fen_buffer(root_board, (*trace_)[0].fen);
}

SearchTreeNode* GameTreeExporter::add_child(SearchTreeNode* parent, const Board& board, Move move, int depth, int ply) noexcept {
    if (!parent) return nullptr;
    if (trace_size_ >= TraceCapacity) { trace_truncated_ = true; return nullptr; }
    const size_t index = trace_size_++;
    auto& child = (*trace_)[index];
    child = SearchTreeNode{};
    child.move = move; child.depth = depth; child.ply = ply;
    if (parent->last_child) (*trace_)[parent->last_child].next_sibling = index;
    else parent->first_child = index;
    parent->last_child = index;
    write_fen_buffer(board, child.fen);
    return &child;
}

static void write_fen_buffer(const Board& board, std::array<char, 128>& buffer) noexcept {
    char* out = buffer.data();
    for (int rank = 7; rank >= 0; --rank) {
        int empty = 0;
        for (int file = 0; file < 8; ++file) {
            const auto piece = board.piece_at(static_cast<Square>(rank * 8 + file));
            if (piece == Piece::None) { ++empty; continue; }
            if (empty) { *out++ = static_cast<char>('0' + empty); empty = 0; }
            *out++ = piece_to_char(piece);
        }
        if (empty) *out++ = static_cast<char>('0' + empty);
        if (rank) *out++ = '/';
    }
    *out++ = ' '; *out++ = board.side_to_move() == Color::White ? 'w' : 'b'; *out++ = ' ';
    const auto rights = board.castling_rights();
    if (rights == CastlingNone) *out++ = '-';
    else {
        if (rights & WhiteOO) *out++ = 'K';
        if (rights & WhiteOOO) *out++ = 'Q';
        if (rights & BlackOO) *out++ = 'k';
        if (rights & BlackOOO) *out++ = 'q';
    }
    *out++ = ' ';
    if (board.ep_square() == Square::None) *out++ = '-';
    else { *out++ = 'a' + static_cast<int>(file_of(board.ep_square())); *out++ = '1' + static_cast<int>(rank_of(board.ep_square())); }
    *out++ = ' ';
    out = std::to_chars(out, buffer.data() + buffer.size() - 1, board.halfmove_clock()).ptr;
    *out++ = ' ';
    out = std::to_chars(out, buffer.data() + buffer.size() - 1, board.fullmove_number()).ptr;
    *out = '\0';
}

void GameTreeExporter::write_trace(std::ostream& out, size_t index) const {
    const auto& node = (*trace_)[index];
    out << "{\"move\":\"" << (node.move ? move_to_uci(node.move) : "") << "\",\"fen\":\"" << node.fen.data()
        << "\",\"eval\":" << node.eval << ",\"depth\":" << node.depth << ",\"ply\":" << node.ply
        << ",\"is_pruned\":" << (node.is_pruned ? "true" : "false")
        << ",\"is_terminal\":" << (node.is_terminal ? "true" : "false");
    if (index == 0) out << ",\"trace_truncated\":" << (trace_truncated_ ? "true" : "false");
    out << ",\"children\":[";
    for (size_t child = node.first_child; child; child = (*trace_)[child].next_sibling) {
        if (child != node.first_child) out << ',';
        write_trace(out, child);
    }
    out << "]}";
}

static void save_node_json(const TreeNodeJSON& node, std::ostream& out, int indent) {
    std::string ind(indent * 2, ' ');
    out << ind << "{\n";
    out << ind << "  \"move\": \"" << node.move_uci << "\",\n";
    out << ind << "  \"fen\": \"" << node.fen << "\",\n";
    out << ind << "  \"eval\": " << node.eval << ",\n";
    out << ind << "  \"depth\": " << node.depth << ",\n";
    out << ind << "  \"ply\": " << node.ply << ",\n";
    out << ind << "  \"is_pruned\": " << (node.is_pruned ? "true" : "false") << ",\n";
    out << ind << "  \"children\": [\n";

    for (size_t i = 0; i < node.children.size(); ++i) {
        save_node_json(node.children[i], out, indent + 2);
        if (i + 1 < node.children.size()) out << ",";
        out << "\n";
    }

    out << ind << "  ]\n";
    out << ind << "}";
}

std::string GameTreeExporter::to_json_string() const {
    std::ostringstream ss;
    if (trace_) write_trace(ss, 0); else save_node_json(root_node_, ss, 0);
    return ss.str();
}

void GameTreeExporter::export_to_file(const std::string& filename) const {
    export_file(filename);
}

bool GameTreeExporter::export_file(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    if (trace_) write_trace(file, 0); else save_node_json(root_node_, file, 0);
    file << "\n";
    return true;
}

} // namespace heavensgate
