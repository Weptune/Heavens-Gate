#include "exporter.hpp"
#include <fstream>
#include <iostream>

namespace heavensgate {

GameTreeExporter::GameTreeExporter() {
    root_node_ = TreeNodeJSON{};
}

void GameTreeExporter::reset(const Board& root_board) {
    root_node_ = TreeNodeJSON{};
    root_node_.depth = 0;
    root_node_.ply = 0;
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
    save_node_json(root_node_, ss, 0);
    return ss.str();
}

void GameTreeExporter::export_to_file(const std::string& filename) const {
    export_file(filename);
}

bool GameTreeExporter::export_file(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    save_node_json(root_node_, file, 0);
    file << "\n";
    return true;
}

} // namespace heavensgate
