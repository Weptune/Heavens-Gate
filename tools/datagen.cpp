// Self-play training data for tools/tune_hce.cpp.
//
//   heavensgate_datagen <book.epd> <games> <nodes_per_move> <seed> <out.txt> [random_plies=4]
//
// Each game starts from a random book position plus a few random legal plies, then
// both sides search a fixed number of nodes per move. Quiet positions (side to move
// not in check, best move not a capture or promotion, |score| < 2000) are written as
// "FEN|result" with the result from White's point of view (1, 0.5, 0), the format the
// tuner reads. Games end on mate, stalemate, 50-move, threefold, insufficient
// material, or adjudication: |score| >= 1500 for 6 plies (win), |score| <= 10 for 20
// plies after ply 80 (draw), or 400 plies (draw).
#include "../src/board/board.hpp"
#include "../src/core/fen.hpp"
#include "../src/core/zobrist.hpp"
#include "../src/evaluation/eval.hpp"
#include "../src/movegen/movegen.hpp"
#include "../src/search/search.hpp"
#include "../src/search/syzygy.hpp"
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

using namespace heavensgate;

int main(int argc, char** argv) {
    if (argc < 6) {
        std::cerr << "usage: heavensgate_datagen <book.epd> <games> <nodes> <seed> <out.txt> [random_plies]\n";
        return 2;
    }
    Zobrist::init(); MoveGenerator::init(); Evaluator::init();
    SyzygyTablebase::instance().init("");
    Evaluator::set_mode(EvalMode::MasterPositional);

    std::vector<std::string> book;
    { std::ifstream in(argv[1]); std::string line; while (std::getline(in, line)) if (!line.empty()) book.push_back(line); }
    if (book.empty()) { std::cerr << "empty book\n"; return 2; }
    const int games = std::stoi(argv[2]);
    const uint64_t nodes = std::stoull(argv[3]);
    std::mt19937_64 rng(std::stoull(argv[4]));
    std::ofstream out(argv[5], std::ios::app);
    const int random_plies = argc > 6 ? std::stoi(argv[6]) : 4;

    TranspositionTable table(16);
    auto engine = std::make_unique<SearchEngine>(&table);
    engine->set_threads(1); engine->set_book_enabled(false); engine->set_uci_output(false);

    long positions = 0;
    for (int g = 0; g < games; ++g) {
        Board board;
        board.load_fen(book[rng() % book.size()]);
        bool ok = true;
        for (int r = 0; r < random_plies; ++r) {
            MoveList legal; MoveGenerator::generate_legal_moves(board, legal);
            if (legal.empty()) { ok = false; break; }
            board.make_move(legal[rng() % legal.size()]);
        }
        if (!ok) continue;
        table.clear(); engine->clear();

        std::vector<std::string> fens;
        double result = -1;
        int win_streak = 0, draw_streak = 0, last_sign = 0;
        for (int ply = 0; ply < 400 && result < 0; ++ply) {
            MoveList legal; MoveGenerator::generate_legal_moves(board, legal);
            const bool check = MoveGenerator::in_check(board, board.side_to_move());
            if (legal.empty()) { result = check ? (board.side_to_move() == Color::White ? 0.0 : 1.0) : 0.5; break; }
            if (board.halfmove_clock() >= 100 || board.is_repetition(3) || board.is_insufficient_material()) { result = 0.5; break; }

            const auto res = engine->search_iterative_deepening(board, 64, 0.0, nodes);
            const Move best = res.best_move ? res.best_move : legal[0];
            const int white_score = board.side_to_move() == Color::White ? res.best_score : -res.best_score;

            if (!check && !best.is_capture() && !best.is_promotion() && std::abs(res.best_score) < 2000)
                fens.push_back(FEN::to_string(board));

            const int sign = white_score >= 1500 ? 1 : white_score <= -1500 ? -1 : 0;
            win_streak = (sign != 0 && sign == last_sign) ? win_streak + 1 : (sign != 0 ? 1 : 0);
            last_sign = sign;
            if (win_streak >= 6) { result = sign > 0 ? 1.0 : 0.0; break; }
            draw_streak = std::abs(white_score) <= 10 ? draw_streak + 1 : 0;
            if (ply >= 80 && draw_streak >= 20) { result = 0.5; break; }

            if (!board.can_push_history()) { result = 0.5; break; }
            board.make_move(best);
        }
        if (result < 0) result = 0.5;
        for (const auto& fen : fens) out << fen << '|' << result << '\n';
        positions += static_cast<long>(fens.size());
        if ((g + 1) % 50 == 0) { out.flush(); std::cerr << "games " << g + 1 << " positions " << positions << "\n"; }
    }
    std::cerr << "done: " << positions << " positions\n";
    return 0;
}
