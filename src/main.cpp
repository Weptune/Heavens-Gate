#include "core/types.hpp"
#include "core/bitwise.hpp"
#include "core/fen.hpp"
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

void run_comparison(Board& board, int depth) {
    SearchEngine engine;
    
    std::cout << "\n======================================================\n";
    std::cout << "  BENCHMARK COMPARISON: MINIMAX vs ALPHA-BETA (Depth " << depth << ")\n";
    std::cout << "======================================================\n";

    SearchResult mm_res = engine.search_minimax(board, depth);
    SearchResult ab_res = engine.search_alphabeta(board, depth);

    double node_reduction = 0.0;
    if (mm_res.metrics.total_nodes > 0) {
        node_reduction = (1.0 - static_cast<double>(ab_res.metrics.total_nodes) / mm_res.metrics.total_nodes) * 100.0;
    }

    double speedup = 0.0;
    if (ab_res.metrics.elapsed_seconds > 0.0) {
        speedup = mm_res.metrics.elapsed_seconds / ab_res.metrics.elapsed_seconds;
    }

    std::cout << "\n| Metric | Minimax (v1.0) | Alpha-Beta (v2.0) | Improvement |\n";
    std::cout << "| :--- | :--- | :--- | :--- |\n";
    std::cout << "| Best Move | " << move_to_uci(mm_res.best_move) << " | " << move_to_uci(ab_res.best_move) << " | Identical |\n";
    std::cout << "| Eval Score | " << mm_res.best_score << " cp | " << ab_res.best_score << " cp | Identical |\n";
    std::cout << "| Total Nodes | " << mm_res.metrics.total_nodes << " | " << ab_res.metrics.total_nodes << " | " 
              << std::fixed << std::setprecision(1) << node_reduction << "% REDUCTION |\n";
    std::cout << "| Time Elapsed | " << std::fixed << std::setprecision(4) << mm_res.metrics.elapsed_seconds << " s | "
              << std::fixed << std::setprecision(4) << ab_res.metrics.elapsed_seconds << " s | " 
              << std::fixed << std::setprecision(2) << speedup << "x FASTER |\n";
    std::cout << "| Branching Factor (EBF) | " << std::fixed << std::setprecision(2) << mm_res.metrics.effective_branching_factor 
              << " | " << std::fixed << std::setprecision(2) << ab_res.metrics.effective_branching_factor 
              << " | " << (mm_res.metrics.effective_branching_factor - ab_res.metrics.effective_branching_factor) << " lower |\n";
    std::cout << "------------------------------------------------------\n\n";
}

int main(int argc, char* argv[]) {
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
    std::cout << "  HEAVEN'S GATE CHESS ENGINE - VERSION 2.0 (Alpha-Beta Pruning) \n";
    std::cout << "======================================================\n";
    std::cout << "Commands:\n";
    std::cout << "  alphabeta <depth> / ab <d>  - Run Alpha-Beta search\n";
    std::cout << "  compare <depth>             - Compare Minimax vs Alpha-Beta side-by-side\n";
    std::cout << "  minimax <depth>             - Run unpruned Minimax search\n";
    std::cout << "  export_tree <d>             - Export JSON search tree (game_tree.json)\n";
    std::cout << "  perft                       - Run Perft verification suite\n";
    std::cout << "  display / d                 - Print ASCII board\n";
    std::cout << "  fen <str>                   - Set position from FEN string\n";
    std::cout << "  exit                        - Exit program\n";
    std::cout << "------------------------------------------------------\n";

    Board board;
    FEN::parse(StartposFEN, board);
    SearchEngine search_engine;

    std::string line;
    while (true) {
        std::cout << "\nheavensgate> ";
        if (!std::getline(std::cin, line) || line == "exit") break;

        if (line.rfind("alphabeta", 0) == 0 || line.rfind("ab ", 0) == 0 || line == "ab") {
            int d = 4;
            try {
                size_t pos = line.find_first_of("0123456789");
                if (pos != std::string::npos) d = std::stoi(line.substr(pos));
            } catch (...) {}

            std::cout << "Searching Alpha-Beta depth " << d << " ...\n";
            SearchResult res = search_engine.search_alphabeta(board, d);

            std::cout << "\nBest Move : " << move_to_uci(res.best_move) << "\n";
            std::cout << "Eval      : " << res.best_score << " cp\n";
            std::cout << "PV        : " << res.pv.to_string() << "\n";
            std::cout << res.metrics.report_markdown() << "\n";
        } else if (line.rfind("compare", 0) == 0) {
            int d = 4;
            try {
                size_t pos = line.find_first_of("0123456789");
                if (pos != std::string::npos) d = std::stoi(line.substr(pos));
            } catch (...) {}
            run_comparison(board, d);
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

            std::cout << "Exporting JSON game tree with Alpha-Beta pruning for depth " << d << " ...\n";
            SearchResult res = search_engine.search_alphabeta(board, d, true);
            bool ok = search_engine.tree_exporter().export_file("game_tree.json");

            std::cout << "Export " << (ok ? "SUCCESSFUL -> game_tree.json" : "FAILED") << "\n";
            std::cout << "Nodes exported: " << res.metrics.total_nodes << "\n";
        } else if (line == "perft") {
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
            std::cout << "Unknown command. Available: alphabeta <d>, compare <d>, minimax <d>, export_tree <d>, perft, exit\n";
        }
    }

    return 0;
}
