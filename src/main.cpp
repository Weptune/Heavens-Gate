#include "core/types.hpp"
#include "core/bitwise.hpp"
#include "core/fen.hpp"
#include "core/zobrist.hpp"
#include "board/board.hpp"
#include "movegen/movegen.hpp"
#include "movegen/perft.hpp"
#include "evaluation/eval.hpp"
#include "search/search.hpp"
#include "benchmark/metrics.hpp"
#include "uci/uci.hpp"
#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>
#include <cmath>
#include <fstream>

using namespace heavensgate;

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

const std::vector<std::string> TournamentOpenings = {
    std::string(StartposFEN),                                             // 1. Initial Position
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",     // 2. Sicilian Defense
    "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3", // 3. Ruy Lopez
    "rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",     // 4. French Defense
    "rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2",     // 5. Queen's Gambit
    "rnbq1rk1/ppp1ppbp/3p1np1/8/2PPP3/2N2N2/PP2BPPP/R1BQK2R b KQ - 1 6",  // 6. King's Indian Defense
    "rnbqkbnr/pp2pppp/2p5/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",     // 7. Caro-Kann Defense
    "r1bqkb1r/pppp1ppp/2n2n2/4p3/4P3/2N2N2/PPPP1PPP/R1BQKB1R w KQkq - 4 4",// 8. Four Knights Game
    "r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4", // 9. Italian Game
    "rnbqkb1r/pppp1ppp/5n2/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3"   // 10. Petrov Defense
};

void run_automated_tournament(int num_games, int depth) {
    std::cout << "\n======================================================\n";
    std::cout << "  HEAVEN'S GATE UNLIMITED ENDGAME TOURNAMENT (" << num_games << " Games @ Depth " << depth << ")\n";
    std::cout << "  Engine A: Master Edition (Advanced Positional Eval + PVS)\n";
    std::cout << "  Engine B: Baseline Engine (Raw Material + Basic PST)\n";
    std::cout << "======================================================\n\n";

    int a_wins = 0;
    int b_wins = 0;
    int draws  = 0;

    SearchEngine master_engine;
    SearchEngine baseline_engine;

    std::ofstream pgn_file("tournament_results.pgn");

    for (int g = 1; g <= num_games; ++g) {
        Board board;
        std::string opening_fen = TournamentOpenings[(g - 1) % TournamentOpenings.size()];
        FEN::parse(opening_fen, board);

        bool a_is_white = (g % 2 != 0);
        int game_moves = 0;

        pgn_file << "[Event \"Heaven's Gate Unlimited Endgame Tournament\"]\n";
        pgn_file << "[Site \"Localhost\"]\n";
        pgn_file << "[Date \"2026.08.03\"]\n";
        pgn_file << "[Round \"" << g << "\"]\n";
        pgn_file << "[White \"" << (a_is_white ? "Master Edition" : "Baseline Engine") << "\"]\n";
        pgn_file << "[Black \"" << (a_is_white ? "Baseline Engine" : "Master Edition") << "\"]\n";
        pgn_file << "[FEN \"" << opening_fen << "\"]\n";

        std::cout << "\n--- Game " << std::setw(2) << g << "/" << num_games << ": "
                  << (a_is_white ? "Master (White) vs Baseline (Black)" : "Baseline (White) vs Master (Black)")
                  << " ---\n";

        std::string result_str = "*";

        // Unlimited game move loop (safety cap 400 moves)
        while (game_moves < 400) {
            MoveList legal_moves;
            MoveGenerator::generate_legal_moves(board, legal_moves);

            if (legal_moves.empty()) {
                if (MoveGenerator::in_check(board, board.side_to_move())) {
                    if (board.side_to_move() == Color::White) {
                        std::cout << "\n[RESULT] Black wins by Checkmate!\n";
                        result_str = "0-1";
                        if (a_is_white) b_wins++; else a_wins++;
                    } else {
                        std::cout << "\n[RESULT] White wins by Checkmate!\n";
                        result_str = "1-0";
                        if (a_is_white) a_wins++; else b_wins++;
                    }
                } else {
                    std::cout << "\n[RESULT] Draw by Stalemate!\n";
                    result_str = "1/2-1/2";
                    draws++;
                }
                break;
            }

            if (board.is_insufficient_material()) {
                std::cout << "\n[RESULT] Draw by Insufficient Material!\n";
                result_str = "1/2-1/2";
                draws++;
                break;
            }

            if (board.halfmove_clock() >= 100) {
                std::cout << "\n[RESULT] Draw by 50-Move Rule!\n";
                result_str = "1/2-1/2";
                draws++;
                break;
            }

            bool current_is_master = (board.side_to_move() == Color::White && a_is_white) ||
                                     (board.side_to_move() == Color::Black && !a_is_white);

            SearchResult res;
            if (current_is_master) {
                Evaluator::set_mode(EvalMode::MasterPositional);
                res = master_engine.search_alphabeta(board, depth, true, true);
            } else {
                Evaluator::set_mode(EvalMode::MaterialOnly);
                res = baseline_engine.search_alphabeta(board, depth, false, false);
            }

            if (!res.best_move) {
                std::cout << "\n[RESULT] Draw by No Valid Move!\n";
                result_str = "1/2-1/2";
                draws++;
                break;
            }

            std::string uci_move = move_to_uci(res.best_move);
            if (board.side_to_move() == Color::White) {
                std::cout << (game_moves / 2 + 1) << ". " << uci_move << " (" << res.best_score << "cp) ";
                pgn_file << (game_moves / 2 + 1) << ". " << uci_move << " ";
            } else {
                std::cout << uci_move << " (" << res.best_score << "cp)\n";
                pgn_file << uci_move << " ";
            }

            board.make_move(res.best_move);
            game_moves++;
        }

        if (game_moves >= 400 && result_str == "*") {
            std::cout << "\n[RESULT] Draw by 400-Move Safety Limit!\n";
            result_str = "1/2-1/2";
            draws++;
        }

        pgn_file << result_str << "\n\n";

        std::cout << "Score after Game " << g << ": Master " 
                  << a_wins << " - " << b_wins << " Baseline (" << draws << " draws)\n";
    }

    pgn_file.close();
    Evaluator::set_mode(EvalMode::MasterPositional);

    double total_score = a_wins + 0.5 * draws;
    double score_pct = (total_score / num_games) * 100.0;

    double elo_diff = 0.0;
    if (score_pct > 0.0 && score_pct < 100.0) {
        elo_diff = -400.0 * std::log10(1.0 / (total_score / static_cast<double>(num_games)) - 1.0);
    } else if (score_pct >= 100.0) {
        elo_diff = 800.0;
    }

    std::cout << "\n------------------------------------------------------\n";
    std::cout << "TOURNAMENT FINAL RESULTS (Saved to tournament_results.pgn):\n";
    std::cout << "  Master Wins   : " << a_wins << "\n";
    std::cout << "  Baseline Wins : " << b_wins << "\n";
    std::cout << "  Draws         : " << draws  << "\n";
    std::cout << "  Master Score  : " << std::fixed << std::setprecision(1) << score_pct << "%\n";
    std::cout << "  Calculated Delta Elo : +" << std::setprecision(0) << elo_diff << " Elo Advantage\n";
    std::cout << "------------------------------------------------------\n\n";
}

void run_three_way_comparison(Board& board, int depth) {
    SearchEngine engine;
    
    std::cout << "\n======================================================\n";
    std::cout << "  BENCHMARK: MINIMAX vs RAW ALPHA-BETA vs MASTER SEARCH (Depth " << depth << ")\n";
    std::cout << "======================================================\n";

    SearchResult mm_res = engine.search_minimax(board, depth);
    SearchResult ab_raw_res = engine.search_alphabeta(board, depth, false, false);
    SearchResult p_res = engine.search_alphabeta(board, depth, true, true);

    double node_reduction = 0.0;
    if (ab_raw_res.metrics.total_nodes > 0) {
        node_reduction = (1.0 - static_cast<double>(p_res.metrics.total_nodes) / ab_raw_res.metrics.total_nodes) * 100.0;
    }

    double overall_reduction = 0.0;
    if (mm_res.metrics.total_nodes > 0) {
        overall_reduction = (1.0 - static_cast<double>(p_res.metrics.total_nodes) / mm_res.metrics.total_nodes) * 100.0;
    }

    std::cout << "\n| Metric | Minimax (v1.0) | Raw Alpha-Beta (v2.0) | Heaven's Gate Master | Improvement |\n";
    std::cout << "| :--- | :--- | :--- | :--- | :--- |\n";
    std::cout << "| Best Move | " << move_to_uci(mm_res.best_move) << " | " << move_to_uci(ab_raw_res.best_move) << " | " << move_to_uci(p_res.best_move) << " | Master Precision |\n";
    std::cout << "| Eval Score | " << mm_res.best_score << " cp | " << ab_raw_res.best_score << " cp | " << p_res.best_score << " cp | Positional Intelligence |\n";
    std::cout << "| Total Nodes | " << mm_res.metrics.total_nodes << " | " << ab_raw_res.metrics.total_nodes << " | " << p_res.metrics.total_nodes << " | " 
              << std::fixed << std::setprecision(1) << node_reduction << "% vs v2.0 (" << overall_reduction << "% vs v1.0) |\n";
    std::cout << "| Q-Nodes | 0 | 0 | " << p_res.q_nodes << " nodes | Horizon Effect Free |\n";
    std::cout << "| TT Hits | 0 | 0 | " << p_res.tt_hits << " hits | Subtree Pruning |\n";
    std::cout << "------------------------------------------------------\n\n";
}

int main(int argc, char* argv[]) {
    Zobrist::init();
    MoveGenerator::init();
    Evaluator::init();

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "perft" || arg == "perft_suite") {
            int max_d = (argc > 2) ? std::stoi(argv[2]) : 4;
            bool success = Perft::run_verification_suite(max_d);
            return success ? 0 : 1;
        } else if (arg == "uci") {
            UCI::loop();
            return 0;
        } else if (arg == "tournament") {
            int games = (argc > 2) ? std::stoi(argv[2]) : 4;
            int depth = (argc > 3) ? std::stoi(argv[3]) : 4;
            run_automated_tournament(games, depth);
            return 0;
        }
    }

    std::cout << "======================================================\n";
    std::cout << "  HEAVEN'S GATE CHESS ENGINE - MASTER EDITION\n";
    std::cout << "======================================================\n";
    std::cout << "Commands:\n";
    std::cout << "  uci                         - Switch to standard UCI Protocol mode\n";
    std::cout << "  tournament [games] [depth]  - Run unlimited endgame tournament & save PGN\n";
    std::cout << "  id <depth> [time_ms]        - Run Iterative Deepening + PVS + Eval\n";
    std::cout << "  alphabeta <depth> / ab <d>  - Run Move-Ordered PVS search\n";
    std::cout << "  compare <depth>             - Compare Minimax vs Raw Alpha-Beta vs Master Search\n";
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

        if (line == "uci") {
            std::cout << "id name Heaven's Gate Master Edition\n";
            std::cout << "id author DeepMind Antigravity\n";
            std::cout << "uciok" << std::endl;
            UCI::loop();
            break;
        } else if (line.rfind("tournament", 0) == 0) {
            int games = 4;
            int depth = 4;
            try {
                std::stringstream ss(line);
                std::string cmd;
                ss >> cmd;
                if (ss >> games) {
                    ss >> depth;
                }
            } catch (...) {}
            run_automated_tournament(games, depth);
        } else if (line.rfind("id", 0) == 0 || line.rfind("go", 0) == 0) {
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

            std::cout << "Running Master Iterative Deepening + PVS + Aspiration + NMP + LMR (Max Depth " << d;
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

            std::cout << "Searching Master PVS Search depth " << d << " ...\n";
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
            std::cout << "Unknown command. Available: uci, tournament [games] [d], id <d> [time], ab <d>, compare <d>, minimax <d>, perft, exit\n";
        }
    }

    return 0;
}
