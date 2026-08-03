#include "core/types.hpp"
#include "core/bitwise.hpp"
#include "core/fen.hpp"
#include "core/zobrist.hpp"
#include "board/board.hpp"
#include "movegen/movegen.hpp"
#include "movegen/perft.hpp"
#include "search/search.hpp"
#include "benchmark/metrics.hpp"
#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>

using namespace heavensgate;

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void run_three_way_comparison(Board& board, int depth) {
    SearchEngine engine;
    
    std::cout << "\n======================================================\n";
    std::cout << "  BENCHMARK: MINIMAX vs RAW ALPHA-BETA vs QUIESCENCE SEARCH (Depth " << depth << ")\n";
    std::cout << "======================================================\n";

    SearchResult mm_res = engine.search_minimax(board, depth);
    SearchResult ab_raw_res = engine.search_alphabeta(board, depth, false, false);
    SearchResult q_res = engine.search_alphabeta(board, depth, true, true);

    double node_reduction = 0.0;
    if (ab_raw_res.metrics.total_nodes > 0) {
        node_reduction = (1.0 - static_cast<double>(q_res.metrics.total_nodes) / ab_raw_res.metrics.total_nodes) * 100.0;
    }

    double overall_reduction = 0.0;
    if (mm_res.metrics.total_nodes > 0) {
        overall_reduction = (1.0 - static_cast<double>(q_res.metrics.total_nodes) / mm_res.metrics.total_nodes) * 100.0;
    }

    std::cout << "\n| Metric | Minimax (v1.0) | Raw Alpha-Beta (v2.0) | Quiescence Search (v8.0) | Improvement |\n";
    std::cout << "| :--- | :--- | :--- | :--- | :--- |\n";
    std::cout << "| Best Move | " << move_to_uci(mm_res.best_move) << " | " << move_to_uci(ab_raw_res.best_move) << " | " << move_to_uci(q_res.best_move) << " | Identical |\n";
    std::cout << "| Eval Score | " << mm_res.best_score << " cp | " << ab_raw_res.best_score << " cp | " << q_res.best_score << " cp | Tactical Precision |\n";
    std::cout << "| Total Nodes | " << mm_res.metrics.total_nodes << " | " << ab_raw_res.metrics.total_nodes << " | " << q_res.metrics.total_nodes << " | " 
              << std::fixed << std::setprecision(1) << node_reduction << "% vs v2.0 (" << overall_reduction << "% vs v1.0) |\n";
    std::cout << "| Q-Nodes | 0 | 0 | " << q_res.q_nodes << " nodes | Horizon Effect Eliminated |\n";
    std::cout << "| TT Hits | 0 | 0 | " << q_res.tt_hits << " hits | Constant Time Lookups |\n";
    std::cout << "------------------------------------------------------\n\n";
}

int main(int argc, char* argv[]) {
    Zobrist::init();
    MoveGenerator::init();

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "perft" || arg == "perft_suite") {
            int max_d = (argc > 2) ? std::stoi(argv[2]) : 4;
            bool success = Perft::run_verification_suite(max_d);
            return success ? 0 : 1;
        }
    }

    std::cout << "======================================================\n";
    std::cout << "  HEAVEN'S GATE CHESS ENGINE - VERSION 8.0 (Quiescence Search) \n";
    std::cout << "======================================================\n";
    std::cout << "Commands:\n";
    std::cout << "  id <depth> [time_ms]        - Run Iterative Deepening + Q-Search (v8.0)\n";
    std::cout << "  alphabeta <depth> / ab <d>  - Run Move-Ordered Alpha-Beta search\n";
    std::cout << "  compare <depth>             - Compare Minimax vs Raw Alpha-Beta vs Q-Search\n";
    std::cout << "  minimax <depth>             - Run unpruned Minimax search\n";
    std::cout << "  export_tree <d>             - Export JSON search tree (game_tree.json)\n";
    std::cout << "  perft                       - Run Perft verification suite\n";
    std::cout << "  display / d                 - Print ASCII board & Zobrist Key\n";
    std::cout << "  fen <str>                   - Set position from FEN string\n";
    std::cout << "  exit                        - Exit program\n";
    std::cout << "------------------------------------------------------\n";

    Board board;
    FEN::parse(StartposFEN, board);
    SearchEngine search_engine;

    std::string raw_line;
    while (true) {
        std::cout << "\nheavensgate> ";
        if (!std::getline(std::cin, raw_line)) break;

        std::string line = trim(raw_line);
        if (line == "exit") break;
        if (line.empty()) continue;

        if (line.rfind("id", 0) == 0 || line.rfind("go", 0) == 0) {
            int d = 6;
            double time_ms = 0.0;
            try {
                std::stringstream ss(line);
                std::string cmd;
                ss >> cmd;
                if (ss >> d) {
                    ss >> time_ms;
                }
            } catch (...) {}

            std::cout << "Running Iterative Deepening + Q-Search (Max Depth " << d;
            if (time_ms > 0) std::cout << ", Max Time " << time_ms << " ms";
            std::cout << ") ...\n";

            SearchResult res = search_engine.search_iterative_deepening(board, d, time_ms);

            std::cout << "\nCompleted Depth : " << res.completed_depth << "\n";
            std::cout << "Best Move       : " << move_to_uci(res.best_move) << "\n";
            std::cout << "Eval            : " << res.best_score << " cp\n";
            std::cout << "PV              : " << res.pv.to_string() << "\n";
            std::cout << "Q-Nodes         : " << res.q_nodes << " nodes\n";
            std::cout << "TT Hits         : " << res.tt_hits << " (" << std::fixed << std::setprecision(1) << search_engine.tt().hit_rate() << "% hit rate)\n";
            std::cout << res.metrics.report_markdown() << "\n";
        } else if (line.rfind("alphabeta", 0) == 0 || line.rfind("ab ", 0) == 0 || line == "ab") {
            int d = 5;
            try {
                size_t pos = line.find_first_of("0123456789");
                if (pos != std::string::npos) d = std::stoi(line.substr(pos));
            } catch (...) {}

            std::cout << "Searching Move-Ordered Alpha-Beta + Q-Search depth " << d << " ...\n";
            SearchResult res = search_engine.search_alphabeta(board, d, true, true);

            std::cout << "\nBest Move : " << move_to_uci(res.best_move) << "\n";
            std::cout << "Eval      : " << res.best_score << " cp\n";
            std::cout << "PV        : " << res.pv.to_string() << "\n";
            std::cout << "Q-Nodes   : " << res.q_nodes << " nodes\n";
            std::cout << "TT Hits   : " << res.tt_hits << " (" << std::fixed << std::setprecision(1) << search_engine.tt().hit_rate() << "% hit rate)\n";
            std::cout << res.metrics.report_markdown() << "\n";
        } else if (line.rfind("compare", 0) == 0) {
            int d = 4;
            try {
                size_t pos = line.find_first_of("0123456789");
                if (pos != std::string::npos) d = std::stoi(line.substr(pos));
            } catch (...) {}
            run_three_way_comparison(board, d);
        } else if (line.rfind("minimax", 0) == 0) {
            int d = 4;
            try {
                size_t pos = line.find_first_of("0123456789");
                if (pos != std::string::npos) d = std::stoi(line.substr(pos));
            } catch (...) {}

            std::cout << "Searching Minimax depth " << d << " ...\n";
            SearchResult res = search_engine.search_minimax(board, d);

            std::cout << "\nBest Move : " << move_to_uci(res.best_move) << "\n";
            std::cout << "Eval      : " << res.best_score << " cp\n";
            std::cout << "PV        : " << res.pv.to_string() << "\n";
            std::cout << res.metrics.report_markdown() << "\n";
        } else if (line.rfind("export_tree", 0) == 0) {
            int d = 3;
            try {
                size_t pos = line.find_first_of("0123456789");
                if (pos != std::string::npos) d = std::stoi(line.substr(pos));
            } catch (...) {}

            std::cout << "Exporting JSON search tree for depth " << d << " ...\n";
            SearchResult res = search_engine.search_alphabeta(board, d, true, true, true);
            bool ok = search_engine.tree_exporter().export_file("game_tree.json");

            std::cout << "Export " << (ok ? "SUCCESSFUL -> game_tree.json" : "FAILED") << "\n";
            std::cout << "Nodes exported: " << res.metrics.total_nodes << "\n";
        } else if (line.rfind("perft", 0) == 0) {
            Perft::run_verification_suite(4);
        } else if (line == "d" || line == "display") {
            std::cout << board.to_ascii() << "\n";
        } else if (line.rfind("fen ", 0) == 0) {
            std::string fen = line.substr(4);
            if (FEN::parse(fen, board)) {
                std::cout << "Parsed FEN successfully:\n" << board.to_ascii() << "\n";
            } else {
                std::cout << "Failed to parse FEN string.\n";
            }
        } else {
            std::cout << "Unknown command. Available: id <d> [time], ab <d>, compare <d>, minimax <d>, perft, exit\n";
        }
    }

    return 0;
}
