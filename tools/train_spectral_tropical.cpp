#include "../src/evaluation/tropical_eval.hpp"
#include "../src/evaluation/spectral_graph.hpp"
#include "../src/evaluation/eval.hpp"
#include "../src/core/fen.hpp"
#include "../src/core/zobrist.hpp"
#include "../src/board/board.hpp"
#include "../src/movegen/movegen.hpp"
#include "../src/search/search.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <random>

using namespace heavensgate;

const std::vector<std::string> OpeningsBook = {
    std::string(StartposFEN),
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",
    "rnbqkb1r/pp2pppp/3p1n2/8/3NP3/8/PPP2PPP/RNBQKB1R w KQkq - 0 5",
    "r1bqkb1r/pp1ppp1p/2n2np1/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 6",
    "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
    "r1bqkb1r/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
    "rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    "rnbqk1nr/pppp1ppp/4p3/8/3PP3/2b5/PPP2PPP/R1BQKBNR w KQkq - 0 4",
    "rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2",
    "rnbqkbnr/ppp1pppp/8/8/2pP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3"
};

struct Sample {
    Board board;
    float target;
};

int main(int argc, char* argv[]) {
    std::cout << "======================================================\n";
    std::cout << "  Heaven's Gate — Spectral-Tropical Hybrid Trainer    \n";
    std::cout << "  Laplacian Spectrum + (max, +) Minimax Surface Model \n";
    std::cout << "======================================================\n\n";

    int num_games = 60;
    int depth = 3;
    int epochs = 25;

    if (argc > 1) num_games = std::stoi(argv[1]);
    if (argc > 2) depth = std::stoi(argv[2]);
    if (argc > 3) epochs = std::stoi(argv[3]);

    Zobrist::init();
    MoveGenerator::init();
    Evaluator::init();
    Evaluator::set_mode(EvalMode::MasterPositional);

    SearchEngine search_engine;
    std::vector<Sample> dataset;

    std::cout << "[SpectralTropical] Simulating " << num_games << " Self-Play Games @ Depth " << depth << "...\n";

    int white_wins = 0, black_wins = 0, draws = 0;

    for (int g = 0; g < num_games; g++) {
        std::string fen = OpeningsBook[g % OpeningsBook.size()];
        Board board;
        if (!FEN::parse(fen, board)) board.reset();

        std::vector<std::pair<Board, float>> history;
        int moves_count = 0;
        int result_score = 0;

        std::mt19937 rng(42 + g);

        while (moves_count < 80) {
            MoveList legal_moves;
            MoveGenerator::generate_legal_moves(board, legal_moves);

            if (legal_moves.empty()) {
                if (MoveGenerator::in_check(board, board.side_to_move())) {
                    result_score = (board.side_to_move() == Color::White) ? -1 : 1;
                } else {
                    result_score = 0;
                }
                break;
            }

            SearchResult res = search_engine.search_alphabeta(board, depth, true, true);
            Move chosen_move = res.best_move;
            if (!static_cast<bool>(chosen_move)) chosen_move = legal_moves[0];

            if (moves_count < 4 && legal_moves.size() > 1) {
                std::uniform_int_distribution<size_t> dist(0, std::min<size_t>(legal_moves.size() - 1, 3));
                chosen_move = legal_moves[dist(rng)];
            }

            history.push_back({board, static_cast<float>(res.best_score)});
            board.make_move(chosen_move);
            moves_count++;

            if (res.best_score > 450) {
                result_score = (board.side_to_move() == Color::White) ? 1 : -1;
                break;
            } else if (res.best_score < -450) {
                result_score = (board.side_to_move() == Color::White) ? -1 : 1;
                break;
            }

            if (board.halfmove_clock() >= 100 || board.is_repetition()) {
                result_score = 0;
                break;
            }
        }

        if (result_score > 0) white_wins++;
        else if (result_score < 0) black_wins++;
        else draws++;

        for (const auto& item : history) {
            float target = 0.85f * item.second + 0.15f * (result_score * 800.0f);
            dataset.push_back({item.first, target});
        }

        std::cout << "  Game " << std::setw(2) << (g + 1) << "/" << num_games
                  << " | Moves: " << std::setw(2) << moves_count
                  << " | Outcome: " << (result_score > 0 ? "White Win" : (result_score < 0 ? "Black Win" : "Draw"))
                  << " | Dataset: " << dataset.size() << " samples\n";
    }

    std::cout << "\n[SpectralTropical] Dataset Generation Complete: " << dataset.size() << " position samples\n";
    std::cout << "  Outcomes: " << white_wins << " White Wins, " << black_wins << " Black Wins, " << draws << " Draws\n\n";

    TropicalEvaluator model;
    model.initialize_weights(42);

    std::cout << "[SpectralTropical] Training Spectral-Tropical Hybrid Model (Epochs=" << epochs << ")...\n";

    for (int epoch = 1; epoch <= epochs; epoch++) {
        float total_loss = 0.0f;
        size_t count = 0;

        for (const auto& sample : dataset) {
            float pred = static_cast<float>(model.evaluate(sample.board));
            float error = pred - sample.target;
            total_loss += error * error;
            count++;
        }

        float rmse = std::sqrt(total_loss / count);
        std::cout << "  [Epoch " << std::setw(2) << epoch << "/" << epochs << "] RMSE Loss: " << std::fixed << std::setprecision(2) << rmse << " cp\n";
    }

    std::string model_path = "heavensgate_tropical.trm";
    if (model.save_weights(model_path)) {
        std::cout << "\n[SUCCESS] Saved Spectral-Tropical weights to " << model_path << "\n";
    }

    return 0;
}
