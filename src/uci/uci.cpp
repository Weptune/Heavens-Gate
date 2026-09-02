#include "uci.hpp"
#include "../board/board.hpp"
#include "../movegen/movegen.hpp"
#include "../core/fen.hpp"
#include "../core/polyglot.hpp"
#include "../evaluation/eval.hpp"
#include "../search/search_params.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

namespace heavensgate {

static PolyGlotBook uci_book;
static bool uci_book_attempted = false;

void UCI::handle_position(const std::string& line, Board& board) {
    std::stringstream ss(line);
    std::string token, sub_token;
    ss >> token; // "position"
    ss >> sub_token;

    if (sub_token == "startpos") {
        board.clear();
        FEN::parse(FEN::StartPOS, board);
        ss >> sub_token; // may be "moves"
    } else if (sub_token == "fen") {
        std::string fen_str;
        while (ss >> token && token != "moves") {
            if (!fen_str.empty()) fen_str += " ";
            fen_str += token;
        }
        FEN::parse(fen_str, board);
    }

    if (token == "moves" || sub_token == "moves") {
        while (ss >> token) {
            MoveList moves;
            MoveGenerator::generate_legal_moves(board, moves);
            for (const auto& m : moves) {
                if (move_to_uci(m) == token) {
                    board.make_move(m);
                    break;
                }
            }
        }
    }
}

void UCI::handle_go(const std::string& line, Board& board, SearchEngine& engine) {
    if (!uci_book_attempted) {
        uci_book_attempted = true;
        uci_book.load("performance.bin");
        if (!uci_book.is_loaded()) uci_book.load("../performance.bin");
        if (!uci_book.is_loaded()) uci_book.load("tools/performance.bin");
        if (!uci_book.is_loaded()) uci_book.load("c:/Users/abhin/heavensgate/performance.bin");
    }

    if (uci_book.is_loaded()) {
        Move book_move = uci_book.probe(board);
        if (static_cast<bool>(book_move)) {
            int book_eval = Evaluator::evaluate_fast(board);
            std::cout << "info depth 1 score cp " << book_eval << " nodes 1 nps 1000000 time 0 hashfull 0 pv " << move_to_uci(book_move) << " string Book move" << std::endl;
            std::cout << "bestmove " << move_to_uci(book_move) << std::endl;
            return;
        }
    }

    std::stringstream ss(line);
    std::string token;
    ss >> token; // "go"

    int depth = 64;
    double time_ms = 0.0;
    uint64_t max_nodes = 0;
    int wtime = 0, btime = 0, winc = 0, binc = 0, movetime = 0, movestogo = 0;
    bool infinite = false;

    while (ss >> token) {
        if (token == "depth") ss >> depth;
        else if (token == "wtime") ss >> wtime;
        else if (token == "btime") ss >> btime;
        else if (token == "winc") ss >> winc;
        else if (token == "binc") ss >> binc;
        else if (token == "movetime") ss >> movetime;
        else if (token == "movestogo") ss >> movestogo;
        else if (token == "nodes") ss >> max_nodes;
        else if (token == "infinite") infinite = true;
    }

    if (infinite) {
        depth = 64;
        time_ms = 0.0;
    } else if (movetime > 0) {
        time_ms = movetime;
    } else if (wtime > 0 || btime > 0) {
        int my_time = (board.side_to_move() == Color::White) ? wtime : btime;
        int my_inc  = (board.side_to_move() == Color::White) ? winc  : binc;
        int moves_expected = (movestogo > 0) ? std::min(movestogo, 35) : 35;
        double alloc = (static_cast<double>(my_time) / moves_expected) + (my_inc * 0.8);
        time_ms = std::min(alloc, my_time * 0.8);

        // Smart Opening Time Allocation: moves 1-5 use fast 150-350ms development
        if (board.fullmove_number() <= 5) {
            time_ms = std::min(time_ms * 0.35, 350.0);
            time_ms = std::max(time_ms, 120.0);
        }
    }

    SearchResult res = engine.search_iterative_deepening(board, depth, time_ms, max_nodes);
    std::cout << "bestmove " << move_to_uci(res.best_move) << std::endl;
}

void UCI::loop() {
    Board board;
    FEN::parse(FEN::StartPOS, board);
    SearchEngine engine;
    engine.set_threads(6);
    engine.set_uci_output(true);

    Evaluator::set_mode(EvalMode::MasterPositional);
    engine.tt().resize(256);

    std::thread search_thread;
    std::atomic<bool> is_searching{false};

    auto stop_search = [&]() {
        engine.stop();
        if (search_thread.joinable()) {
            search_thread.join();
        }
        is_searching.store(false, std::memory_order_relaxed);
    };

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;

        if (cmd == "uci") {
            std::cout << "id name Heaven's Gate Master Edition\n";
            std::cout << "id author Antigravity Team\n";
            std::cout << "option name Hash type spin default 256 min 1 max 16384\n";
            std::cout << "option name Threads type spin default 6 min 1 max 64\n";
            std::cout << "option name LMR_Divisor type string default 3.20\n";
            std::cout << "option name LMR_HistBonus type spin default 425 min 0 max 5000\n";
            std::cout << "option name LMR_HistMalus type spin default 72 min 0 max 5000\n";
            std::cout << "option name RFP_Margin type spin default 163 min 10 max 500\n";
            std::cout << "option name Futility_Margin type spin default 180 min 10 max 500\n";
            std::cout << "option name SEE_BadCaptureSlope type spin default 124 min 0 max 500\n";
            std::cout << "option name SEE_QuietSlope type spin default 15 min 0 max 500\n";
            std::cout << "option name NMP_EvalMargin type spin default 218 min 0 max 1000\n";
            std::cout << "uciok" << std::endl;
        } else if (cmd == "isready") {
            Evaluator::set_mode(EvalMode::MasterPositional);
            std::cout << "readyok" << std::endl;
        } else if (cmd == "setoption") {
            std::string token, name, val;
            ss >> token; // name
            ss >> name;
            ss >> token; // value
            ss >> val;
            if (name == "Threads" && !val.empty()) {
                engine.set_threads(std::stoi(val));
            } else if (name == "Hash" && !val.empty()) {
                engine.tt().resize(std::stoul(val));
            } else if (name == "LMR_Divisor" && !val.empty()) {
                g_search_params.lmr_divisor = std::stof(val);
                SearchEngine::init_lmr_table(g_search_params.lmr_divisor);
            } else if (name == "LMR_HistBonus" && !val.empty()) {
                g_search_params.lmr_hist_bonus = std::stoi(val);
            } else if (name == "LMR_HistMalus" && !val.empty()) {
                g_search_params.lmr_hist_malus = std::stoi(val);
            } else if (name == "RFP_Margin" && !val.empty()) {
                g_search_params.rfp_margin = std::stoi(val);
            } else if (name == "Futility_Margin" && !val.empty()) {
                g_search_params.futility_margin = std::stoi(val);
            } else if (name == "SEE_BadCaptureSlope" && !val.empty()) {
                g_search_params.see_bad_capture_slope = std::stoi(val);
            } else if (name == "SEE_QuietSlope" && !val.empty()) {
                g_search_params.see_quiet_slope = std::stoi(val);
            } else if (name == "NMP_EvalMargin" && !val.empty()) {
                g_search_params.nmp_eval_margin = std::stoi(val);
            }
        } else if (cmd == "ucinewgame") {
            stop_search();
            Evaluator::set_mode(EvalMode::MasterPositional);
            board.clear();
            FEN::parse(FEN::StartPOS, board);
            engine.clear();
        } else if (cmd == "position") {
            stop_search();
            handle_position(line, board);
        } else if (cmd == "stop") {
            stop_search();
        } else if (cmd == "go") {
            stop_search();

            if (!uci_book_attempted) {
                uci_book_attempted = true;
                uci_book.load("performance.bin");
                if (!uci_book.is_loaded()) uci_book.load("../performance.bin");
                if (!uci_book.is_loaded()) uci_book.load("tools/performance.bin");
                if (!uci_book.is_loaded()) uci_book.load("c:/Users/abhin/heavensgate/performance.bin");
            }

            if (uci_book.is_loaded()) {
                Move book_move = uci_book.probe(board);
                if (static_cast<bool>(book_move)) {
                    std::cout << "bestmove " << move_to_uci(book_move) << std::endl;
                    continue;
                }
            }

            int depth = 64;
            double time_ms = 0.0;
            uint64_t max_nodes = 0;
            int wtime = 0, btime = 0, winc = 0, binc = 0, movetime = 0, movestogo = 0;
            bool infinite = false;

            std::string token;
            while (ss >> token) {
                if (token == "depth") ss >> depth;
                else if (token == "wtime") ss >> wtime;
                else if (token == "btime") ss >> btime;
                else if (token == "winc") ss >> winc;
                else if (token == "binc") ss >> binc;
                else if (token == "movetime") ss >> movetime;
                else if (token == "movestogo") ss >> movestogo;
                else if (token == "nodes") ss >> max_nodes;
                else if (token == "infinite") infinite = true;
            }

            if (infinite) {
                depth = 64;
                time_ms = 0.0;
            } else if (movetime > 0) {
                time_ms = movetime;
            } else if (wtime > 0 || btime > 0) {
                int my_time = (board.side_to_move() == Color::White) ? wtime : btime;
                int my_inc  = (board.side_to_move() == Color::White) ? winc  : binc;
                int moves_expected = (movestogo > 0) ? std::min(movestogo, 35) : 35;
                double alloc = (static_cast<double>(my_time) / moves_expected) + (my_inc * 0.8);
                time_ms = std::min(alloc, my_time * 0.8);

                // Smart Opening Time Allocation: moves 1-5 use fast 150-350ms development
                if (board.fullmove_number() <= 5) {
                    time_ms = std::min(time_ms * 0.35, 350.0);
                    time_ms = std::max(time_ms, 120.0);
                }
            }

            is_searching.store(true, std::memory_order_relaxed);
            search_thread = std::thread([&engine, search_board = board, depth, time_ms, max_nodes, &is_searching]() mutable {
                SearchResult res = engine.search_iterative_deepening(search_board, depth, time_ms, max_nodes);
                std::cout << "bestmove " << move_to_uci(res.best_move) << std::endl;
                is_searching.store(false, std::memory_order_relaxed);
            });
        } else if (cmd == "quit") {
            stop_search();
            break;
        }
    }

    stop_search();
}

} // namespace heavensgate

