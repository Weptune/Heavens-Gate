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
        if (!uci_book.is_loaded()) uci_book.load("tools/performance.bin");
    }

    if (uci_book.is_loaded()) {
        Move book_move = uci_book.probe(board);
        if (static_cast<bool>(book_move)) {
            std::cout << "bestmove " << move_to_uci(book_move) << std::endl;
            return;
        }
    }

    std::stringstream ss(line);
    std::string token;
    ss >> token; // "go"

    int depth = 64;
    double time_ms = 0.0;
    int wtime = 0, btime = 0, winc = 0, binc = 0, movetime = 0;

    while (ss >> token) {
        if (token == "depth") ss >> depth;
        else if (token == "wtime") ss >> wtime;
        else if (token == "btime") ss >> btime;
        else if (token == "winc") ss >> winc;
        else if (token == "binc") ss >> binc;
        else if (token == "movetime") ss >> movetime;
    }

    if (movetime > 0) {
        time_ms = movetime;
    } else if (wtime > 0 || btime > 0) {
        int my_time = (board.side_to_move() == Color::White) ? wtime : btime;
        int my_inc  = (board.side_to_move() == Color::White) ? winc  : binc;
        double alloc = (my_time / 35.0) + (my_inc * 0.8);
        time_ms = std::min(alloc, my_time * 0.8);
    }

    SearchResult res = engine.search_iterative_deepening(board, depth, time_ms);

    std::cout << "bestmove " << move_to_uci(res.best_move) << std::endl;
}

void UCI::loop() {
    Board board;
    FEN::parse(FEN::StartPOS, board);
    SearchEngine engine;
    engine.set_threads(6);

    Evaluator::set_mode(EvalMode::SpectralTropical);
    engine.tt().resize(256);

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "uci") {
            std::cout << "id name Heaven's Gate Master Edition\n";
            std::cout << "id author DeepMind Antigravity\n";
            std::cout << "option name Hash type spin default 256 min 1 max 16384\n";
            std::cout << "option name Threads type spin default 6 min 1 max 64\n";
            std::cout << "option name LMR_Divisor type string default 2.20\n";
            std::cout << "option name LMR_HistBonus type spin default 500 min 0 max 5000\n";
            std::cout << "option name LMR_HistMalus type spin default 100 min 0 max 5000\n";
            std::cout << "option name RFP_Margin type spin default 120 min 10 max 500\n";
            std::cout << "option name Futility_Margin type spin default 200 min 10 max 500\n";
            std::cout << "option name SEE_BadCaptureSlope type spin default 100 min 0 max 500\n";
            std::cout << "option name SEE_QuietSlope type spin default 40 min 0 max 500\n";
            std::cout << "option name NMP_EvalMargin type spin default 200 min 0 max 1000\n";
            std::cout << "uciok" << std::endl;
        } else if (line == "isready") {
            Evaluator::set_mode(EvalMode::SpectralTropical);
            std::cout << "readyok" << std::endl;
        } else if (line.rfind("setoption", 0) == 0) {
            std::stringstream ss(line);
            std::string token, name, val;
            ss >> token >> token; // setoption name
            ss >> name;
            ss >> token; // value
            ss >> val;
            if (name == "Threads" && !val.empty()) {
                engine.set_threads(std::stoi(val));
            } else if (name == "Hash" && !val.empty()) {
                engine.tt().resize(std::stoul(val));
            } else if (name == "LMR_Divisor" && !val.empty()) {
                g_search_params.lmr_divisor = std::stof(val);
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
        } else if (line == "ucinewgame") {
            Evaluator::set_mode(EvalMode::SpectralTropical);
            board.clear();
            FEN::parse(FEN::StartPOS, board);
            engine.clear();
        } else if (line.rfind("position", 0) == 0) {
            handle_position(line, board);
        } else if (line.rfind("go", 0) == 0) {
            handle_go(line, board, engine);
        } else if (line == "quit") {
            break;
        }
    }
}

} // namespace heavensgate
