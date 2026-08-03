#include "exporter.hpp"
#include <iostream>

namespace heavensgate {

void TreeExporter::save_to_json(const TreeNodeJSON& node, std::ostream& out, int indent) {
    std::string ind(indent * 2, ' ');
    out << ind << "{\n";
    out << ind << "  \"move\": \"" << node.move_uci << "\",\n";
    out << ind << "  \"fen\": \"" << node.fen << "\",\n";
    out << ind << "  \"eval\": " << node.eval << ",\n";
    out << ind << "  \"depth\": " << node.depth << ",\n";
    out << ind << "  \"ply\": " << node.ply << ",\n";
    out << ind << "  \"is_terminal\": " << (node.is_terminal ? "true" : "false") << ",\n";
    out << ind << "  \"is_pruned\": " << (node.is_pruned ? "true" : "false") << ",\n";
    out << ind << "  \"children\": [\n";

    for (size_t i = 0; i < node.children.size(); ++i) {
        save_to_json(node.children[i], out, indent + 2);
        if (i + 1 < node.children.size()) out << ",";
        out << "\n";
    }

    out << ind << "  ]\n";
    out << ind << "}";
}

bool TreeExporter::export_file(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    save_to_json(root_, file, 0);
    file.close();
    return true;
}

} // namespace heavensgate
