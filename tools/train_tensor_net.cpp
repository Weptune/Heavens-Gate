#include "../src/evaluation/tensor_eval.hpp"
#include "../src/evaluation/tensor_train.hpp"
#include "../src/evaluation/tensor_quant.hpp"
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

const std::vector<std::string> SelfPlayOpenings = {
    std::string(StartposFEN),
    // Sicilian Defense variations
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",
    "rnbqkb1r/pp2pppp/3p1n2/8/3NP3/8/PPP2PPP/RNBQKB1R w KQkq - 0 5",
    "r1bqkb1r/pp1ppp1p/2n2np1/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 6",
    "rnbqkb1r/1p2pppp/p2p1n2/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 6",
    "r1bqk2r/pp2bppp/2n1pn2/3p4/3PP3/2N2N2/PP2BPPP/R1BQ1RK1 w kq - 0 8",
    // Ruy Lopez & Italian Game
    "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
    "r1bqkb1r/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
    "r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
    "r1bqk1nr/pppp1ppp/2n5/2b1p3/1PB1P3/5N2/P1PP1PPP/RNBQK2R b KQkq b3 0 4",
    // French Defense & Caro-Kann
    "rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    "rnbqk1nr/pppp1ppp/4p3/8/3PP3/2b5/PPP2PPP/R1BQKBNR w KQkq - 0 4",
    "rnbqkbnr/pp2pppp/2p5/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    "rnbqkbnr/pp2pppp/2p5/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3",
    // Queen's Gambit & Indian Defenses
    "rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2",
    "rnbqkbnr/ppp1pppp/8/8/2pP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3",
    "rnbq1rk1/ppp1ppbp/3p1np1/8/2PPP3/2N2N2/PP2BPPP/R1BQK2R b KQ - 1 6",
    "rnbq1rk1/ppp1ppbp/3p1np1/8/2PPP3/2N2P2/PP2G1PP/R1BQKBNR w KQ - 0 6",
    "rnbqkb1r/pppp1ppp/5n2/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
    "rnbqkbnr/ppp2ppp/4p3/3p4/2PP4/8/PP2PPPP/RNBQKBNR w KQkq d6 0 3",
    "rnbqk2r/ppp1bppp/4pn2/3p4/2PP4/2N2N2/PP2PPPP/R1BQKB1R w KQkq - 2 5",
    // English Opening & Reti
    "rnbqkbnr/pppppppp/8/8/2P5/8/PP1PPPPP/RNBQKBNR b KQkq c3 0 1",
    "rnbqkbnr/pppp1ppp/4p3/8/2P5/5N2/PP1PPPPPPP/RNBQKB1R b KQkq - 1 2",
    "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1",
    // Scandinavian & Pirc
    "rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2",
    "rnbqkb1r/ppp1pppp/3p1n2/8/3PP3/8/PPP2PPP/RNBQKBNR w KQkq - 1 3",
    // Alekhine & Benoni
    "rnbqkb1r/pppppppp/5n2/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 1 2",
    "rnbqkb1r/pp1ppppp/5n2/2p5/2PP4/8/PP2PPPP/RNBQKBNR w KQkq c6 0 3",
    // Dutch Defense & King's Gambit
    "rnbqkbnr/ppppp1pp/8/5p2/4P3/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 2",
    "rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq f3 0 2",
    // Tactical Benchmark Middlegames & Endgames
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
    "2r2rk1/pp1b1ppp/1q2p3/3pP3/1b1P4/1P1Q1N2/P3NPPP/2R2RK1 w - - 1 16",
    "r1b1qrk1/1pp1b1pp/p1n1p3/3p1p2/3P1P2/2N1PN2/PPP1B1PP/R2Q1RK1 w - - 0 11",
    "r1bqk2r/1pp1bppp/p1n1pn2/3p4/3PP3/2N2N2/PPP1BPPP/R1BQ1RK1 w kq - 0 8",
    "r1bq1rk1/pp2bppp/2n1pn2/3p4/2PP4/2N2N2/PP2BPPP/R1BQ1RK1 w - - 0 9",
    "r2q1rk1/pp1nbppp/2p1pn2/3p4/2PP4/2N1PN2/PP2BPPP/R2Q1RK1 w - - 0 10",
    "r1b2rk1/pp1qbppp/2n1pn2/3p4/2PP4/2N2N2/PP2BPPP/R1BQ1RK1 w - - 0 10",
    "r2q1rk1/1pp1bppp/p1n1pn2/3p4/2PP4/2N2N2/PP2BPPP/R1BQ1RK1 w - - 0 10",
    "r1bqk2r/pp2bppp/2n1p3/3pP3/3P4/2N2N2/PP2BPPP/R1BQ1RK1 w kq - 0 10",
    "r1b1k2r/pp2bppp/2n1pn2/3p4/3PP3/2N2N2/PP2BPPP/R1BQ1RK1 w kq - 0 10",
    "r1bq1rk1/pp2bppp/2n1pn2/3p4/3PP3/2N2N2/PP2BPPP/R1BQ1RK1 w - - 0 10",
    "rnbqk2r/ppp1bppp/4pn2/3p4/3PP3/2N2N2/PPP2PPP/R1BQKB1R w KQkq - 0 5",
    "r1bqk2r/ppp1bppp/2n1pn2/3p4/3PP3/2N2N2/PPP1BPPP/R1BQ1RK1 w kq - 0 7",
    "rnbq1rk1/ppp1bppp/4pn2/3p4/3PP3/2N2N2/PPP1BPPP/R1BQ1RK1 w - - 0 7",
    "r1bq1rk1/ppp1bppp/2n1p3/3pP3/3P4/2N2N2/PPP1BPPP/R1BQ1RK1 w - - 0 9",
    "r1bq1rk1/pp2bppp/2n1pn2/3p4/2PP4/2N2N2/PP2BPPP/R1BQK2R w KQ - 0 9",
    "r1bqk2r/pp2bppp/2n1pn2/3p4/2PP4/2N2N2/PP2BPPP/R1BQ1RK1 w kq - 0 9"
};

struct GamePos {
    Board board;
    int search_eval;
};

int main(int argc, char* argv[]) {
    std::cout << "======================================================\n";
    std::cout << "  Heaven's Gate — Self-Play MPS Dataset Bootstrapper \n";
    std::cout << "  Simulating Games, Extracting Features & Training    \n";
    std::cout << "======================================================\n\n";

    int num_games = 100;
    int search_depth = 3;
    int epochs = 25;
    int bond_dim = 16;
    std::string output_path = "heavensgate.tnw";

    if (argc > 1) num_games = std::stoi(argv[1]);
    if (argc > 2) search_depth = std::stoi(argv[2]);
    if (argc > 3) epochs = std::stoi(argv[3]);
    if (argc > 4) bond_dim = std::stoi(argv[4]);
    if (argc > 5) output_path = argv[5];

    std::cout << "[Bootstrapper] Initializing Engine Subsystems...\n";
    Zobrist::init();
    MoveGenerator::init();
    Evaluator::init();
    Evaluator::set_mode(EvalMode::MasterPositional);

    SearchEngine search_engine;
    std::vector<TrainingSample> dataset;

    int white_wins = 0;
    int black_wins = 0;
    int draws = 0;
    int total_moves = 0;

    auto sim_start = std::chrono::high_resolution_clock::now();

    std::cout << "[Bootstrapper] Simulating " << num_games << " Self-Play Games @ Depth " << search_depth << "...\n\n";

    for (int g = 0; g < num_games; g++) {
        std::string fen = SelfPlayOpenings[g % SelfPlayOpenings.size()];
        Board board;
        if (!FEN::parse(fen, board)) board.reset();

        std::vector<GamePos> game_history;

        int moves_count = 0;
        int game_result_score = 0; // +1 = White win, -1 = Black win, 0 = Draw

        std::mt19937 rng(42 + g);

        while (moves_count < 100) {
            MoveList legal_moves;
            MoveGenerator::generate_legal_moves(board, legal_moves);

            if (legal_moves.empty()) {
                if (MoveGenerator::in_check(board, board.side_to_move())) {
                    game_result_score = (board.side_to_move() == Color::White) ? -1 : 1;
                } else {
                    game_result_score = 0;
                }
                break;
            }

            SearchResult res = search_engine.search_alphabeta(board, search_depth, true, true);
            Move chosen_move = res.best_move;

            if (!static_cast<bool>(chosen_move)) {
                chosen_move = legal_moves[0];
            }

            // In early opening (first 4 moves), add slight move sampling among legal moves for diversity
            if (moves_count < 4 && legal_moves.size() > 1) {
                std::uniform_int_distribution<size_t> dist(0, std::min<size_t>(legal_moves.size() - 1, 3));
                chosen_move = legal_moves[dist(rng)];
            }

            game_history.push_back({board, res.best_score});

            board.make_move(chosen_move);
            moves_count++;

            // Adjudicate decisive evaluation advantage (> +450 cp or < -450 cp)
            if (res.best_score > 450) {
                game_result_score = (board.side_to_move() == Color::White) ? 1 : -1; // Previous mover won
                break;
            } else if (res.best_score < -450) {
                game_result_score = (board.side_to_move() == Color::White) ? -1 : 1;
                break;
            }

            if (board.halfmove_clock() >= 100 || board.is_repetition()) {
                game_result_score = 0;
                break;
            }
        }

        // If game reached max move limit without mate, adjudicate based on final score
        if (moves_count >= 100 && game_result_score == 0 && !game_history.empty()) {
            int final_eval = game_history.back().search_eval;
            if (final_eval > 200) game_result_score = 1;
            else if (final_eval < -200) game_result_score = -1;
        }

        total_moves += moves_count;

        if (game_result_score > 0) white_wins++;
        else if (game_result_score < 0) black_wins++;
        else draws++;

        // Label positions with blended TD target
        for (const auto& pos : game_history) {
            float outcome_cp = static_cast<float>(game_result_score * 800);
            float target_eval = 0.80f * static_cast<float>(pos.search_eval) + 0.20f * outcome_cp;

            dataset.push_back(TensorTrainer::create_sample(pos.board, target_eval));
        }

        std::cout << "  Game " << std::setw(2) << (g + 1) << "/" << num_games
                  << " | Opening #" << (g % SelfPlayOpenings.size() + 1)
                  << " | Moves: " << std::setw(2) << moves_count
                  << " | Outcome: " << (game_result_score > 0 ? "White Win" : (game_result_score < 0 ? "Black Win" : "Draw"))
                  << " | Cumulative Dataset: " << dataset.size() << " samples\n";
    }

    auto sim_end = std::chrono::high_resolution_clock::now();
    double sim_time = std::chrono::duration<double>(sim_end - sim_start).count();

    std::cout << "\n======================================================\n";
    std::cout << "SELF-PLAY DATA GENERATION COMPLETE:\n";
    std::cout << "  Total Games Played : " << num_games << "\n";
    std::cout << "  White Wins         : " << white_wins << "\n";
    std::cout << "  Black Wins         : " << black_wins << "\n";
    std::cout << "  Draws              : " << draws << "\n";
    std::cout << "  Total Moves        : " << total_moves << "\n";
    std::cout << "  Total Dataset Size : " << dataset.size() << " position samples\n";
    std::cout << "  Simulation Time    : " << std::fixed << std::setprecision(2) << sim_time << " seconds\n";
    std::cout << "======================================================\n\n";

    // Train Tensor MPS
    std::cout << "[Bootstrapper] Initializing TensorMPS (Bond D=" << bond_dim << ")...\n";
    TensorMPS model(bond_dim);
    model.initialize_random(42);

    TensorTrainer::Config config;
    config.bond_dim = bond_dim;
    config.epochs = epochs;
    config.batch_size = 32;
    config.learning_rate = 0.002f;
    config.weight_decay = 1e-4f;

    TensorTrainer trainer(model, config);

    auto train_start = std::chrono::high_resolution_clock::now();
    trainer.train(dataset, 0.15f);
    auto train_end = std::chrono::high_resolution_clock::now();

    double train_time = std::chrono::duration<double>(train_end - train_start).count();
    std::cout << "\n[Bootstrapper] Model training completed in " << train_time << " seconds.\n";

    if (model.save_weights(output_path)) {
        std::cout << "[SUCCESS] Saved trained Tensor Network weights to " << output_path << "\n";
    }

    // Quantize to int16 Q14
    TensorMPSQuantized q_model(bond_dim);
    q_model.quantize_from(model);
    if (q_model.save_quantized("heavensgate.qtnw")) {
        std::cout << "[SUCCESS] Saved Int16 Q14 quantized weights to heavensgate.qtnw\n";
    }

    return 0;
}
