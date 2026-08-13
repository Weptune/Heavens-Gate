#include "core/types.hpp"
#include "core/bitwise.hpp"
#include "core/fen.hpp"
#include "core/zobrist.hpp"
#include "board/board.hpp"
#include "movegen/movegen.hpp"
#include "movegen/perft.hpp"
#include "evaluation/eval.hpp"
#include "evaluation/tensor_eval.hpp"
#include "evaluation/tensor_train.hpp"
#include "search/search.hpp"
#include "benchmark/metrics.hpp"
#include "uci/uci.hpp"
#include "uci/stockfish_client.hpp"
#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>
#include <cmath>
#include <fstream>
#include <atomic>

using namespace heavensgate;

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

static std::string pv_to_string(const std::vector<Move>& pv) {
    std::string s;
    for (size_t i = 0; i < pv.size(); ++i) {
        if (i > 0) s += " ";
        s += move_to_uci(pv[i]);
    }
    return s;
}

const std::vector<std::string> TournamentOpenings = {
    std::string(StartposFEN),                                             // 1. Initial Position
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",     // 2. Sicilian Defense: Open
    "rnbqkb1r/pp2pppp/3p1n2/8/3NP3/8/PPP2PPP/RNBQKB1R w KQkq - 0 5",      // 3. Sicilian Defense: Najdorf
    "r1bqkb1r/pp1ppp1p/2n2np1/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 6",  // 4. Sicilian Defense: Dragon
    "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3", // 5. Ruy Lopez: Main Line
    "r1bqkb1r/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",  // 6. Ruy Lopez: Berlin
    "rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",     // 7. French Defense: Advance
    "rnbqk1nr/pppp1ppp/4p3/8/3PP3/2b5/PPP2PPP/R1BQKBNR w KQkq - 0 4",     // 8. French Defense: Winawer
    "rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2",     // 9. Queen's Gambit Declined
    "rnbqkbnr/ppp1pppp/8/8/2pP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3",         // 10. Queen's Gambit Accepted
    "rnbq1rk1/ppp1ppbp/3p1np1/8/2PPP3/2N2N2/PP2BPPP/R1BQK2R b KQ - 1 6",  // 11. King's Indian: Classical
    "rnbq1rk1/ppp1ppbp/3p1np1/8/2PPP3/2N2P2/PP4PP/R1BQKBNR w KQ - 0 6",   // 12. King's Indian: Samisch
    "rnbqkbnr/pp2pppp/2p5/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",     // 13. Caro-Kann: Main Line
    "rnbqkbnr/pp2pppp/2p5/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3",         // 14. Caro-Kann: Advance
    "r1bqkb1r/pppp1ppp/2n2n2/4p3/4P3/2N2N2/PPPP1PPP/R1BQKB1R w KQkq - 4 4",// 15. Four Knights Game
    "r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4", // 16. Italian Game: Giuoco Piano
    "r1bqk1nr/pppp1ppp/2n5/2b1p3/1PB1P3/5N2/P1PP1PPP/RNBQK2R b KQkq b3 0 4",// 17. Italian Game: Evans Gambit
    "rnbqkb1r/pppp1ppp/5n2/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",  // 18. Petrov Defense
    "rnbqkbnr/ppp2ppp/4p3/3p4/2PP4/8/PP2PPPP/RNBQKBNR w KQkq d6 0 3",    // 19. QGD Exchange
    "rnbqk2r/ppp1bppp/4pn2/3p4/2PP4/2N2N2/PP2PPPP/R1BQKB1R w KQkq - 2 5",  // 20. QGD Tartakower
    "rnbqkb1r/ppp1pp1p/5np1/3p4/2PP4/2N5/PP2PPPP/R1BQKBNR w KQkq - 0 4",  // 21. GrÃ¼nfeld Defense
    "rnbqk2r/pppp1ppp/4pn2/8/2PP4/2P5/P3PPPP/R1BQKBNR w KQkq - 0 4",     // 22. Nimzo-Indian Defense
    "rnbqkbnr/pppppp1p/6p1/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",      // 23. Modern Defense
    "rnbqkbnr/pppp1ppp/8/4p3/2P5/8/PP1PPPPP/RNBQKBNR b KQkq c3 0 2",     // 24. English Opening
    "rnbqkbnr/ppp1p1pp/8/3p1p2/2PP4/8/PP2PPPP/RNBQKBNR w KQkq f6 0 3",    // 25. Dutch Defense
    "rnbqkbnr/pp1ppppp/8/3P4/8/8/PPP1PPPP/RNBQKBNR b KQkq - 0 2",         // 26. Benoni Defense
    "r1bqk2r/pp2bppp/2n1pn2/2pp4/2PP4/2N1PN2/PP2BPPP/R1BQ1RK1 w kq - 4 8", // 27. Tarrasch Defense
    "rnbqkbnr/pppp1ppp/8/4p3/4P3/2N5/PPPP1PPP/R1BQKBNR b KQkq - 1 2",     // 28. Vienna Game
    "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2",     // 29. Scandinavian Defense
    "rnbqkb1r/pppppppp/5n2/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 1 2",      // 30. Alekhine Defense
    "rnbqkbnr/pp2pppp/2p5/3p4/2PP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3",     // 31. Slav Defense
    "rnbqk2r/pp2bppp/2p1pn2/3p4/2PP4/2N1PN2/PP3PPP/R1BQKB1R w KQkq - 0 6", // 32. Semi-Slav Defense
    "rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq f3 0 2",     // 33. King's Gambit
    "rnbqkbnr/ppp1pppp/3p4/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",      // 34. Pirc Defense
    "rnbqkbnr/ppp1pppp/3p4/8/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2",     // 35. Philidor Defense
    "r1bqkbnr/pppp1ppp/2n5/3Pp3/4P3/5N2/PPP2PPP/RNBQK2R b KQkq - 0 3",     // 36. Scotch Game
    "rnbqkbnr/pppp1ppp/8/8/3pP3/2P5/PP3PPP/RNBQKBNR b KQkq - 0 3",         // 37. Danish Gambit
    "rnbqkbnr/pppppppp/8/8/5P2/8/PPPPP1PP/RNBQKBNR b KQkq f3 0 1",         // 38. Bird's Opening    "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1",         // 39. Reti Opening
    "rnbqkb1r/pppp1ppp/4pn2/8/2PP4/6P1/PP2PP1P/RNBQKBNR b KQkq - 0 3",     // 40. Catalan Opening
    "rnbqkb1r/p2ppppp/5n2/1ppP4/2P5/8/PP2PPPP/RNBQKBNR w KQkq b6 0 4",     // 41. Benko Gambit
};

void run_automated_tournament(int num_games, int bank_param = 0, int inc_param = 0, int batch_id = 0, int elo_param = 3500) {
    bool is_fixed_movetime = (bank_param > 0 && bank_param <= 60 && inc_param <= 0);
    double fixed_movetime_ms = is_fixed_movetime ? (bank_param * 1000.0) : 0.0;

    bool is_time_control = (bank_param > 0 && !is_fixed_movetime);
    int main_bank_ms = is_time_control ? ((bank_param <= 3600) ? (bank_param * 1000) : bank_param) : 0;
    int inc_ms = is_time_control ? ((inc_param <= 60) ? (inc_param * 1000) : inc_param) : 0;

    std::string pgn_filename = (batch_id > 0) 
        ? ("c:/Users/abhin/heavensgate/tournament_results_batch" + std::to_string(batch_id) + ".pgn")
        : "c:/Users/abhin/heavensgate/tournament_results.pgn";

    // Truncate/create PGN file at start of tournament
    std::ofstream pgn_init(pgn_filename, std::ios::trunc);
    pgn_init.close();

    std::string opp_label = (elo_param >= 3500 || elo_param <= 0) ? "Uncapped ~3500 Elo" : (std::to_string(elo_param) + " Elo");

    std::cout << "\n======================================================\n";
    if (is_fixed_movetime) {
        std::cout << "  HEAVEN'S GATE GRANDMASTER TOURNAMENT (" << num_games << " Games @ Fixed " << static_cast<int>(fixed_movetime_ms / 1000.0) << "s / Move vs Stockfish " << opp_label << ")\n";
    } else if (is_time_control) {
        std::cout << "  HEAVEN'S GATE GRANDMASTER TOURNAMENT (" << num_games << " Games @ " << (main_bank_ms / 1000) << "s Bank + " << (inc_ms / 1000) << "s Inc vs Stockfish " << opp_label << ")\n";
    } else {
        std::cout << "  HEAVEN'S GATE GRANDMASTER TOURNAMENT (" << num_games << " Games @ Fixed Depth 12 vs Stockfish " << opp_label << ")\n";
    }
    if (batch_id > 0) std::cout << "  Batch #" << batch_id << " -> " << pgn_filename << "\n";
    std::cout << "======================================================\n\n" << std::flush;

    int total_a_wins = 0, total_b_wins = 0, total_draws = 0;
    std::vector<std::string> pgn_records;

    StockfishClient sf;
    bool sf_ok = sf.init(elo_param);
    if (!sf_ok) {
        std::cerr << "[FATAL] Could not start Stockfish! Aborting tournament.\n";
        return;
    }

    for (int g = 1; g <= num_games; ++g) {
        Board board;
        std::string opening_fen = TournamentOpenings[(g - 1) % TournamentOpenings.size()];
        FEN::parse(opening_fen, board);

        sf.reset_game();

        SearchEngine master_engine;
        master_engine.tt().resize(64);
        master_engine.tt().clear();
        master_engine.polyglot_book().load("performance.bin");

        bool a_is_white = (g % 2 != 0);
        int game_moves = 0;
        std::string opp_name = "Stockfish 16.1 (" + opp_label + ")";
        std::stringstream ss_pgn;

        ss_pgn << "[Event \"Heaven's Gate Grandmaster Tournament\"]\n";
        ss_pgn << "[Site \"Localhost\"]\n";
        ss_pgn << "[Date \"2026.08.13\"]\n";
        ss_pgn << "[Round \"" << g << "\"]\n";
        ss_pgn << "[White \"" << (a_is_white ? "Master Edition" : opp_name) << "\"]\n";
        ss_pgn << "[Black \"" << (a_is_white ? opp_name : "Master Edition") << "\"]\n";
        ss_pgn << "[FEN \"" << opening_fen << "\"]\n";

        std::string result_str = "*";
        std::string end_reason = "";
        int white_clock_ms = main_bank_ms;
        int black_clock_ms = main_bank_ms;

        while (game_moves < 400) {
            MoveList legal_moves;
            MoveGenerator::generate_legal_moves(board, legal_moves);

            if (legal_moves.empty()) {
                if (MoveGenerator::in_check(board, board.side_to_move())) {
                    result_str = (board.side_to_move() == Color::White) ? "0-1" : "1-0";
                    end_reason = "Checkmate";
                } else {
                    result_str = "1/2-1/2";
                    end_reason = "Stalemate";
                }
                break;
            }

            if (board.is_repetition() && game_moves > 4) {
                result_str = "1/2-1/2"; end_reason = "Threefold Repetition"; break;
            }
            if (board.halfmove_clock() >= 100) {
                result_str = "1/2-1/2"; end_reason = "50-Move Rule"; break;
            }
            if (board.is_insufficient_material()) {
                result_str = "1/2-1/2"; end_reason = "Insufficient Material"; break;
            }

            bool current_is_master = (board.side_to_move() == Color::White) ? a_is_white : !a_is_white;
            int active_clock_ms = (board.side_to_move() == Color::White) ? white_clock_ms : black_clock_ms;
            double time_alloc = is_fixed_movetime ? fixed_movetime_ms : (is_time_control ? std::min((active_clock_ms / 35.0) + (inc_ms * 0.8), active_clock_ms * 0.8) : 0.0);

            auto move_start = std::chrono::high_resolution_clock::now();
            SearchResult res;
            Move chosen_move;

            if (current_is_master) {
                res = master_engine.search_iterative_deepening(board, 64, (is_fixed_movetime || is_time_control) ? time_alloc : 0.0);
                chosen_move = res.best_move;
            } else {
                if (is_fixed_movetime || is_time_control) {
                    res = sf.get_search_result(board, 64, time_alloc, 0, 0, 0);
                } else {
                    res = sf.get_search_result(board, 12);
                }
                chosen_move = res.best_move;
            }

            auto move_end = std::chrono::high_resolution_clock::now();
            double elapsed_ms = std::chrono::duration<double, std::milli>(move_end - move_start).count();

            // Update clocks (informational — for passing to engines on next move)
            if (board.side_to_move() == Color::White) {
                white_clock_ms = std::max(100, static_cast<int>(white_clock_ms - elapsed_ms) + inc_ms);
            } else {
                black_clock_ms = std::max(100, static_cast<int>(black_clock_ms - elapsed_ms) + inc_ms);
            }

            MoveList valid_moves;
            MoveGenerator::generate_legal_moves(board, valid_moves);
            bool is_legal = false;
            for (const auto& lm : valid_moves) {
                if (lm.from() == chosen_move.from() && lm.to() == chosen_move.to() && lm.type() == chosen_move.type()) {
                    chosen_move = lm; // Use validated legal move struct
                    is_legal = true;
                    break;
                }
            }

            if (!is_legal || !static_cast<bool>(chosen_move)) {
                result_str = (board.side_to_move() == Color::White) ? "0-1" : "1-0";
                end_reason = (board.side_to_move() == Color::White) ? "White Engine Forfeit" : "Black Engine Forfeit";
                std::cerr << "[GAME FORFEIT] Engine returned illegal/empty move! Side: " << (board.side_to_move() == Color::White ? "White" : "Black") << "\n";
                break;
            }

            game_moves++;
            int move_num = (game_moves + 1) / 2;
            if (board.side_to_move() == Color::White) {
                ss_pgn << move_num << ". " << move_to_uci(chosen_move) << " { [%eval " << res.best_score << "] [%clk " << elapsed_ms << "ms] } ";
            } else {
                ss_pgn << move_to_uci(chosen_move) << " { [%eval " << res.best_score << "] [%clk " << elapsed_ms << "ms] }\n";
            }

            std::string mover_name = current_is_master ? "Master" : opp_name;
            std::string side_str = (board.side_to_move() == Color::White) ? "W" : "B";
            std::cout << "[G" << g << " M" << move_num << " " << side_str << "] " 
                      << mover_name << ": " << move_to_uci(chosen_move) 
                      << " | Eval: " << (res.best_score > 0 ? "+" : "") << res.best_score << " cp"
                      << " | Time: " << static_cast<int>(elapsed_ms) << "ms"
                      << " | Nodes: " << res.metrics.total_nodes << "\n" << std::flush;


            Color side_before = board.side_to_move();
            board.make_move(chosen_move);

            if (std::abs(res.best_score) > 25000) {
                result_str = (res.best_score > 0) ? ((side_before == Color::White) ? "1-0" : "0-1") : ((side_before == Color::White) ? "0-1" : "1-0");
                end_reason = "Mate Detected"; break;
            }
        }

        if (result_str == "*") { result_str = "1/2-1/2"; end_reason = "Move Limit"; }
        ss_pgn << result_str << " {" << end_reason << "}\n\n";

        bool master_won = (a_is_white && result_str == "1-0") || (!a_is_white && result_str == "0-1");
        bool opp_won    = (a_is_white && result_str == "0-1") || (!a_is_white && result_str == "1-0");
        if (master_won) total_a_wins++;
        else if (opp_won) total_b_wins++;
        else total_draws++;

        pgn_records.push_back(ss_pgn.str());

        // Append to PGN file immediately so completed games are never lost
        std::ofstream pgn_append(pgn_filename, std::ios::app);
        if (pgn_append.is_open()) {
            pgn_append << ss_pgn.str();
            pgn_append.close();
        }

        std::cout << "[TOURNAMENT] Game " << g << "/" << num_games << " (" << game_moves << " moves, " << end_reason 
                  << ") | Master " << total_a_wins << " - " << total_b_wins << " " << opp_name << " (" << total_draws << " draws)\n" << std::flush;
    }

    double total_score = total_a_wins + 0.5 * total_draws;

    double score_pct = (total_score / num_games) * 100.0;
    double elo_diff = 0.0;
    if (score_pct > 0.0 && score_pct < 100.0) {
        elo_diff = -400.0 * std::log10(1.0 / (total_score / static_cast<double>(num_games)) - 1.0);
    } else if (score_pct >= 100.0) { elo_diff = 800.0; }

    std::cout << "\n------------------------------------------------------\n";
    std::cout << "TOURNAMENT RESULTS (" << pgn_filename << "):\n";
    std::cout << "  Master Wins   : " << total_a_wins << "\n";
    std::cout << "  Opponent Wins : " << total_b_wins << "\n";
    std::cout << "  Draws         : " << total_draws  << "\n";
    std::cout << "  Master Score  : " << std::fixed << std::setprecision(1) << score_pct << "%\n";
    std::cout << "  Delta Elo     : +" << std::setprecision(0) << elo_diff << "\n";
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
        std::string cmd = argv[1];
        if (cmd == "tournament") {
            int games    = (argc > 2) ? std::stoi(argv[2]) : 100;
            int depth    = (argc > 3) ? std::stoi(argv[3]) : 8;
            int inc_ms   = (argc > 4) ? std::stoi(argv[4]) : 0;
            int batch_id = (argc > 5) ? std::stoi(argv[5]) : 0;
            int elo      = (argc > 6) ? std::stoi(argv[6]) : 3500;
            run_automated_tournament(games, depth, inc_ms, batch_id, elo);


            return 0;
        } else if (cmd == "perft" || cmd == "perft_suite") {
            int max_d = (argc > 2) ? std::stoi(argv[2]) : 4;
            bool success = Perft::run_verification_suite(max_d);
            return success ? 0 : 1;
        } else if (cmd == "uci") {
            UCI::loop();
            return 0;
        }
    }


    std::cout << "======================================================\n";
    std::cout << "  HEAVEN'S GATE CHESS ENGINE - MASTER EDITION\n";
    std::cout << "======================================================\n";
    std::cout << "Commands:\n";
    std::cout << "  uci                         - Switch to standard UCI Protocol mode\n";
    std::cout << "  tournament [games] [depth]  - Run 100-game grandmaster tournament & save PGN\n";
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
            Evaluator::set_mode(EvalMode::SpectralTropical);
            std::cout << "id name Heaven's Gate Master Edition\n";
            std::cout << "id author DeepMind Antigravity\n";
            std::cout << "uciok" << std::endl;
            UCI::loop();
            break;
        } else if (line.rfind("tournament", 0) == 0) {
            int games = 100;
            int depth = 8;
            int inc_ms = 0;
            try {
                std::stringstream ss(line);
                std::string cmd;
                ss >> cmd;
                if (ss >> games) {
                    if (ss >> depth) {
                        ss >> inc_ms;
                    }
                }
            } catch (...) {}
            run_automated_tournament(games, depth, inc_ms);
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
            std::cout << "PV              : " << pv_to_string(res.pv) << "\n";
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
            std::cout << "PV        : " << pv_to_string(res.pv) << "\n";
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
            std::cout << "PV        : " << pv_to_string(res.pv) << "\n";
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
        } else if (line.rfind("eval_mode", 0) == 0) {
            std::string mode_str = trim(line.substr(9));
            if (mode_str == "tn" || mode_str == "tensor" || mode_str == "tensornetwork") {
                Evaluator::set_mode(EvalMode::TensorNetwork);
                std::cout << "[Eval] Evaluation mode set to TensorNetwork (MPS)\n";
            } else if (mode_str == "nnue") {
                Evaluator::set_mode(EvalMode::NNUE);
                std::cout << "[Eval] Evaluation mode set to NNUE\n";
            } else if (mode_str == "hce" || mode_str == "positional") {
                Evaluator::set_mode(EvalMode::MasterPositional);
                std::cout << "[Eval] Evaluation mode set to MasterPositional (HCE)\n";
            } else if (mode_str == "material") {
                Evaluator::set_mode(EvalMode::MaterialOnly);
                std::cout << "[Eval] Evaluation mode set to MaterialOnly\n";
            } else {
                std::cout << "Usage: eval_mode <tn|nnue|hce|material>\n";
            }
        } else if (line.rfind("train_tn", 0) == 0) {
            int bond = 16;
            int epochs = 10;
            std::stringstream ss(line.substr(8));
            if (ss >> bond) ss >> epochs;

            std::cout << "Training Tensor Network MPS (Bond D=" << bond << ", Epochs=" << epochs << ") ...\n";
            TensorMPS model(bond);
            model.initialize_random(42);

            std::vector<TrainingSample> dataset;
            std::vector<std::string> sample_fens = {
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1",
                "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
                "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
                "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            };

            Board b;
            Evaluator::set_mode(EvalMode::MasterPositional);
            for (const auto& fen : sample_fens) {
                if (FEN::parse(fen, b)) {
                    float target = static_cast<float>(Evaluator::evaluate(b));
                    dataset.push_back(TensorTrainer::create_sample(b, target));
                }
            }

            TensorTrainer::Config cfg;
            cfg.bond_dim = bond;
            cfg.epochs = epochs;
            cfg.batch_size = 8;
            cfg.learning_rate = 0.003f;

            TensorTrainer trainer(model, cfg);
            trainer.train(dataset, 0.0f);

            if (model.save_weights("heavensgate.tnw")) {
                std::cout << "Saved trained Tensor Network weights to heavensgate.tnw\n";
            }
            Evaluator::set_mode(EvalMode::TensorNetwork);
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
            std::cout << "Unknown command. Available: uci, tournament [games] [d], id <d> [time], ab <d>, compare <d>, minimax <d>, eval_mode <tn|nnue|hce>, train_tn [D] [epochs], perft, exit\n";
        }
    }

    return 0;
}
