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
#include <cmath>
#include <numeric>

using namespace heavensgate;

// =============================================================================
// 51-Position Opening Book — Covers all major chess opening families
// =============================================================================
const std::vector<std::string> OpeningsBook = {
    // Standard startpos
    std::string(StartposFEN),
    // Italian Game
    "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
    // Ruy Lopez
    "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
    // Sicilian Defense
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",
    // Sicilian Najdorf
    "rnbqkb1r/1p2pppp/p2p1n2/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 6",
    // French Defense
    "rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    // French Winawer
    "rnbqk1nr/ppp2ppp/4p3/3p4/1b1PP3/2N5/PPP2PPP/R1BQKBNR w KQkq - 2 4",
    // Caro-Kann
    "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    // Caro-Kann Classical
    "rn1qkbnr/pp2pppp/2p5/5b2/3PN3/8/PPP2PPP/R1BQKBNR w KQkq - 1 5",
    // Scandinavian
    "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2",
    // Pirc Defense
    "rnbqkbnr/ppp1pppp/3p4/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    // Alekhine Defense
    "rnbqkb1r/pppppppp/5n2/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 1 2",
    // Queen's Gambit Declined
    "rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2",
    // QGD Orthodox
    "rnbqk2r/ppp1bppp/4pn2/3p4/2PP4/2N2N2/PP2PPPP/R1BQKB1R w KQkq - 4 5",
    // Queen's Gambit Accepted
    "rnbqkbnr/ppp1pppp/8/8/2pP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3",
    // Slav Defense
    "rnbqkbnr/pp2pppp/2p5/3p4/2PP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3",
    // King's Indian Defense
    "rnbqkb1r/pppppp1p/5np1/8/2PP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3",
    // KID Classical
    "rnbq1rk1/ppp1ppbp/3p1np1/8/2PPP3/2N2N2/PP2BPPP/R1BQK2R b KQ - 5 6",
    // Nimzo-Indian
    "rnbqk2r/pppp1ppp/4pn2/8/1bPP4/2N5/PP2PPPP/R1BQKBNR w KQkq - 2 4",
    // Queen's Indian
    "rnbqkb1r/p1pp1ppp/1p2pn2/8/2PP4/5N2/PP2PPPP/RNBQKB1R w KQkq - 0 4",
    // Grunfeld Defense
    "rnbqkb1r/ppp1pp1p/5np1/3p4/2PP4/2N5/PP2PPPP/R1BQKBNR w KQkq d6 0 4",
    // Benoni Defense
    "rnbqkb1r/pp1p1ppp/4pn2/2pP4/2P5/8/PP2PPPP/RNBQKBNR w KQkq - 0 4",
    // Dutch Defense
    "rnbqkbnr/ppppp1pp/8/5p2/3P4/8/PPP1PPPP/RNBQKBNR w KQkq f6 0 2",
    // English Opening
    "rnbqkbnr/pppppppp/8/8/2P5/8/PP1PPPPP/RNBQKBNR b KQkq c3 0 1",
    // English Symmetrical
    "rnbqkbnr/pp1ppppp/8/2p5/2P5/8/PP1PPPPP/RNBQKBNR w KQkq c6 0 2",
    // Reti Opening
    "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1",
    // King's Gambit
    "rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq f3 0 2",
    // Scotch Game
    "r1bqkbnr/pppp1ppp/2n5/4p3/3PP3/5N2/PPP2PPP/RNBQKB1R b KQkq d3 0 3",
    // Four Knights
    "r1bqkb1r/pppp1ppp/2n2n2/4p3/4P3/2N2N2/PPPP1PPP/R1BQKB1R w KQkq - 4 4",
    // Petroff Defense
    "rnbqkb1r/pppp1ppp/5n2/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
    // Philidor Defense
    "rnbqkbnr/ppp2ppp/3p4/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 3",
    // Vienna Game
    "rnbqkbnr/pppp1ppp/8/4p3/4P3/2N5/PPPP1PPP/R1BQKBNR b KQkq - 1 2",
    // Bird Opening
    "rnbqkbnr/pppppppp/8/8/5P2/8/PPPPP1PP/RNBQKBNR b KQkq f3 0 1",
    // London System
    "rnbqkbnr/ppp1pppp/3p4/8/3P1B2/5N2/PPP1PPPP/RN1QKB1R b KQkq - 3 3",
    // Catalan
    "rnbqkb1r/pppp1ppp/4pn2/8/2PP4/6P1/PP2PP1P/RNBQKBNR b KQkq - 0 3",
    // Trompowsky
    "rnbqkb1r/pppppppp/5n2/6B1/3P4/8/PPP1PPPP/RN1QKBNR b KQkq - 2 2",
    // Modern Defense
    "rnbqkbnr/pppppp1p/6p1/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    // Sicilian Dragon
    "rnbqkb1r/pp2pp1p/3p1np1/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 6",
    // Semi-Slav
    "rnbqkb1r/pp3ppp/2p1pn2/3p4/2PP4/2N2N2/PP2PPPP/R1BQKB1R w KQkq - 0 5",
    // Tarrasch Defense
    "rnbqkbnr/pp3ppp/4p3/2pp4/2PP4/2N5/PP2PPPP/R1BQKBNR w KQkq d6 0 4",
    // Budapest Gambit
    "rnbqkb1r/pppp1ppp/5n2/4p3/2PP4/8/PP2PPPP/RNBQKBNR w KQkq e6 0 3",
    // Benko Gambit
    "rnbqkb1r/p2ppppp/5n2/1ppP4/2P5/8/PP2PPPP/RNBQKBNR w KQkq b6 0 4",
    // Italian Giuoco Piano
    "r1bqk1nr/pppp1ppp/2n5/2b1p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
    // Evans Gambit
    "r1bqk1nr/pppp1ppp/2n5/2b1p3/1PB1P3/5N2/P1PP1PPP/RNBQK2R b KQkq b3 0 4",
    // Sicilian Scheveningen
    "rnbqkbnr/pp3ppp/3pp3/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 5",
    // Two Knights Defense
    "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
    // Owen Defense
    "rnbqkbnr/p1pppppp/1p6/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    // Closed Sicilian
    "r1bqkbnr/pp1ppppp/2n5/2p5/4P3/2N5/PPPP1PPP/R1BQKBNR w KQkq - 2 3",
    // Center Game
    "rnbqkbnr/pppp1ppp/8/4p3/3PP3/8/PPP2PPP/RNBQKBNR b KQkq d3 0 2",
    // Symmetrical English
    "r1bqkbnr/pp1ppppp/2n5/2p5/2P5/2N5/PP1PPPPP/R1BQKBNR w KQkq - 2 3",
    // Anti-Sicilian (Nc3)
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/2N5/PPPP1PPP/R1BQKBNR b KQkq - 1 2",
};

struct Sample {
    Board board;
    float target;
};

int main(int argc, char* argv[]) {
    std::cout << "======================================================\n";
    std::cout << "  Heaven's Gate — Spectral-Tropical Hybrid Trainer v2  \n";
    std::cout << "  Gradient Descent on (max, +) Minimax Surface        \n";
    std::cout << "======================================================\n\n";

    int num_games = 500;
    int depth = 5;
    int epochs = 100;
    float lr = 0.001f;
    float lr_decay = 0.99f;

    if (argc > 1) num_games = std::stoi(argv[1]);
    if (argc > 2) depth = std::stoi(argv[2]);
    if (argc > 3) epochs = std::stoi(argv[3]);
    if (argc > 4) lr = std::stof(argv[4]);

    Zobrist::init();
    MoveGenerator::init();
    Evaluator::init();
    Evaluator::set_mode(EvalMode::SpectralTropical);

    SearchEngine search_engine;
    std::vector<Sample> dataset;

    std::cout << "[SpectralTropical] Simulating " << num_games << " Self-Play Games @ Depth " << depth << "...\n";
    std::cout << "[SpectralTropical] Opening Book: " << OpeningsBook.size() << " positions\n\n";

    int white_wins = 0, black_wins = 0, draws = 0;
    auto t_start = std::chrono::steady_clock::now();

    for (int g = 0; g < num_games; g++) {
        std::string fen = OpeningsBook[g % OpeningsBook.size()];
        Board board;
        if (!FEN::parse(fen, board)) board.reset();

        std::vector<std::pair<Board, float>> history;
        int moves_count = 0;
        int result_score = 0;

        std::mt19937 rng(42 + g);

        while (moves_count < 120) {
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

            // Randomize first 6 moves for opening diversity
            if (moves_count < 6 && legal_moves.size() > 1) {
                std::uniform_int_distribution<size_t> dist(0, legal_moves.size() - 1);
                chosen_move = legal_moves[dist(rng)];
            }

            history.push_back({board, static_cast<float>(res.best_score)});
            board.make_move(chosen_move);
            moves_count++;

            // Higher termination threshold for longer, more informative games
            if (res.best_score > 900) {
                result_score = (board.side_to_move() == Color::White) ? 1 : -1;
                break;
            } else if (res.best_score < -900) {
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

        // Skip near-opening positions (first 5 moves) — they're bookish, not evaluatable
        for (size_t idx = 5; idx < history.size(); idx++) {
            const auto& item = history[idx];
            // Blend search score with game outcome (more conservative)
            float outcome_cp = result_score * 600.0f;
            float target = 0.70f * item.second + 0.30f * outcome_cp;
            dataset.push_back({item.first, target});
        }

        if ((g + 1) % 10 == 0 || g == 0) {
            auto t_now = std::chrono::steady_clock::now();
            float elapsed = std::chrono::duration<float>(t_now - t_start).count();
            std::cout << "  Game " << std::setw(3) << (g + 1) << "/" << num_games
                      << " | Moves: " << std::setw(3) << moves_count
                      << " | " << (result_score > 0 ? "W" : (result_score < 0 ? "B" : "D"))
                      << " | Dataset: " << std::setw(6) << dataset.size()
                      << " | " << std::fixed << std::setprecision(1) << elapsed << "s\n";
            std::cout << std::flush;
        }
    }

    std::cout << "\n[SpectralTropical] Dataset Generation Complete: " << dataset.size() << " position samples\n";
    std::cout << "  Outcomes: " << white_wins << " White Wins, " << black_wins << " Black Wins, " << draws << " Draws\n\n";

    // =========================================================================
    // Adam Optimizer on the Tropical (max, +) Minimax Surface
    // =========================================================================
    //
    // The tropical surface T(x) = max_j (w_j^T x + b_j) is piecewise linear.
    // We use Adam optimizer with gradient clipping to ensure smooth, stable
    // weight convergence without gradient explosion or wild oscillations.
    // =========================================================================

    TropicalEvaluator model;
    if (model.load_weights("heavensgate_tropical.trm")) {
        std::cout << "[SpectralTropical] Loaded existing checkpoint 'heavensgate_tropical.trm' (Warm-Start Training)\n";
    } else {
        std::cout << "[SpectralTropical] Initialized new weights (Seed 42)\n";
        model.initialize_weights(42);
    }

    struct SectorAdam {
        std::array<float, TropicalEvaluator::NUM_FEATURES> m_w{};
        std::array<float, TropicalEvaluator::NUM_FEATURES> v_w{};
        float m_b = 0.0f;
        float v_b = 0.0f;
    };
    std::vector<SectorAdam> adam_state(TropicalEvaluator::NUM_SECTORS);

    float beta1 = 0.9f;
    float beta2 = 0.999f;
    float eps = 1e-8f;
    float weight_decay = 0.0001f;
    int timestep = 0;

    std::cout << "[SpectralTropical] Training via Adam Optimizer (Epochs=" << epochs
              << ", LR=" << lr << ", Decay=" << lr_decay << ")...\n\n";

    std::mt19937 shuffle_rng(123);
    std::vector<size_t> indices(dataset.size());
    std::iota(indices.begin(), indices.end(), 0);

    for (int epoch = 1; epoch <= epochs; epoch++) {
        std::shuffle(indices.begin(), indices.end(), shuffle_rng);

        float total_loss = 0.0f;
        size_t count = 0;

        for (size_t idx : indices) {
            const auto& sample = dataset[idx];
            timestep++;

            // 1. Extract feature vector
            auto features = model.extract_features(sample.board);

            // 2. Forward pass: hard-max (MUST match inference exactly)
            auto [prediction, winning_j] = model.evaluate_with_sector(sample.board);

            // 3. Compute error (clamped for stability)
            float error = std::max(-1000.0f, std::min(1000.0f, static_cast<float>(prediction) - sample.target));
            total_loss += error * error;
            count++;

            auto& sec = model.sectors()[winning_j];
            auto& adam = adam_state[winning_j];

            // 4. Adam update on winning sector's positional weights
            //    Skip x[0] (Material) — it is raw pass-through only
            for (size_t i = 1; i < TropicalEvaluator::NUM_FEATURES; i++) {
                float grad = (error * features[i]) / 100.0f + weight_decay * sec.w[i];
                grad = std::max(-50.0f, std::min(50.0f, grad));

                adam.m_w[i] = beta1 * adam.m_w[i] + (1.0f - beta1) * grad;
                adam.v_w[i] = beta2 * adam.v_w[i] + (1.0f - beta2) * (grad * grad);

                float m_hat = adam.m_w[i] / (1.0f - std::pow(beta1, std::min(timestep, 1000)));
                float v_hat = adam.v_w[i] / (1.0f - std::pow(beta2, std::min(timestep, 1000)));

                sec.w[i] -= (lr * m_hat) / (std::sqrt(v_hat) + eps);
                sec.w[i] = std::max(-5.0f, std::min(5.0f, sec.w[i]));
            }

            // 5. Adam update for sector bias
            float grad_b = error / 10.0f;
            grad_b = std::max(-50.0f, std::min(50.0f, grad_b));

            adam.m_b = beta1 * adam.m_b + (1.0f - beta1) * grad_b;
            adam.v_b = beta2 * adam.v_b + (1.0f - beta2) * (grad_b * grad_b);

            float m_hat_b = adam.m_b / (1.0f - std::pow(beta1, std::min(timestep, 1000)));
            float v_hat_b = adam.v_b / (1.0f - std::pow(beta2, std::min(timestep, 1000)));

            sec.b -= (lr * m_hat_b) / (std::sqrt(v_hat_b) + eps);
            sec.b = std::max(-250.0f, std::min(250.0f, sec.b));
        }

        float rmse = std::sqrt(total_loss / count);

        if (epoch <= 10 || epoch % 10 == 0 || epoch == epochs) {
            std::cout << "  [Epoch " << std::setw(3) << epoch << "/" << epochs
                      << "] RMSE: " << std::fixed << std::setprecision(2) << rmse
                      << " cp | LR: " << std::setprecision(6) << lr << "\n";
            std::cout << std::flush;
        }

        // Learning rate decay
        lr *= lr_decay;
    }

    std::string model_path = "heavensgate_tropical.trm";
    if (model.save_weights(model_path)) {
        std::cout << "\n[SUCCESS] Saved Spectral-Tropical weights to " << model_path << "\n";
    }

    return 0;
}
