#include "../src/evaluation/tropical_eval.hpp"
#include "../src/evaluation/spectral_graph.hpp"
#include "../src/evaluation/eval.hpp"
#include "../src/core/fen.hpp"
#include "../src/core/zobrist.hpp"
#include "../src/board/board.hpp"
#include "../src/movegen/movegen.hpp"
#include "../src/search/search.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <random>
#include <cmath>
#include <numeric>
#include <omp.h>
#include <mutex>

using namespace heavensgate;

// =============================================================================
// 51-Position Opening Book — Covers all major chess opening families
// =============================================================================
const std::vector<std::string> OpeningsBook = {
    std::string(StartposFEN),
    "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
    "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",
    "rnbqkb1r/1p2pppp/p2p1n2/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 6",
    "rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    "rnbqk1nr/ppp2ppp/4p3/3p4/1b1PP3/2N5/PPP2PPP/R1BQKBNR w KQkq - 2 4",
    "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    "rn1qkbnr/pp2pppp/2p5/5b2/3PN3/8/PPP2PPP/R1BQKBNR w KQkq - 1 5",
    "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2",
    "rnbqkbnr/ppp1pppp/3p4/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    "rnbqkb1r/pppppppp/5n2/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 1 2",
    "rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2",
    "rnbqk2r/ppp1bppp/4pn2/3p4/2PP4/2N2N2/PP2PPPP/R1BQKB1R w KQkq - 4 5",
    "rnbqkbnr/ppp1pppp/8/8/2pP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3",
    "rnbqkbnr/pp2pppp/2p5/3p4/2PP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3",
    "rnbqkb1r/pppppp1p/5np1/8/2PP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 3",
    "rnbq1rk1/ppp1ppbp/3p1np1/8/2PPP3/2N2N2/PP2BPPP/R1BQK2R b KQ - 5 6",
    "rnbqk2r/pppp1ppp/4pn2/8/1bPP4/2N5/PP2PPPP/R1BQKBNR w KQkq - 2 4",
    "rnbqkb1r/p1pp1ppp/1p2pn2/8/2PP4/5N2/PP2PPPP/RNBQKB1R w KQkq - 0 4",
    "rnbqkb1r/ppp1pp1p/5np1/3p4/2PP4/2N5/PP2PPPP/R1BQKBNR w KQkq d6 0 4",
    "rnbqkb1r/pp1p1ppp/4pn2/2pP4/2P5/8/PP2PPPP/RNBQKBNR w KQkq - 0 4",
    "rnbqkbnr/ppppp1pp/8/5p2/3P4/8/PPP1PPPP/RNBQKBNR w KQkq f6 0 2",
    "rnbqkbnr/pppppppp/8/8/2P5/8/PP1PPPPP/RNBQKBNR b KQkq c3 0 1",
    "rnbqkbnr/pp1ppppp/8/2p5/2P5/8/PP1PPPPP/RNBQKBNR w KQkq c6 0 2",
    "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1",
    "rnbqkbnr/pppp1ppp/8/4p3/4PP2/8/PPPP2PP/RNBQKBNR b KQkq f3 0 2",
    "r1bqkbnr/pppp1ppp/2n5/4p3/3PP3/5N2/PPP2PPP/RNBQKB1R b KQkq d3 0 3",
    "r1bqkb1r/pppp1ppp/2n2n2/4p3/4P3/2N2N2/PPPP1PPP/R1BQKB1R w KQkq - 4 4",
    "rnbqkb1r/pppp1ppp/5n2/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
    "rnbqkbnr/ppp2ppp/3p4/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 0 3",
    "rnbqkbnr/pppp1ppp/8/4p3/4P3/2N5/PPPP1PPP/R1BQKBNR b KQkq - 1 2",
    "rnbqkbnr/pppppppp/8/8/5P2/8/PPPPP1PP/RNBQKBNR b KQkq f3 0 1",
    "rnbqkbnr/ppp1pppp/3p4/8/3P1B2/5N2/PPP1PPPP/RN1QKB1R b KQkq - 3 3",
    "rnbqkb1r/pppp1ppp/4pn2/8/2PP4/6P1/PP2PP1P/RNBQKBNR b KQkq - 0 3",
    "rnbqkb1r/pppppppp/5n2/6B1/3P4/8/PPP1PPPP/RN1QKBNR b KQkq - 2 2",
    "rnbqkbnr/pppppp1p/6p1/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/2N5/PPPP1PPP/R1BQKBNR w KQkq - 2 3",
    "rnbqkbnr/pppp1ppp/8/4p3/2B1P3/8/PPPP1PPP/RNBQK1NR b KQkq - 1 2",
    "rnbqkbnr/pp1ppppp/8/8/2pP4/5N2/PPP1PPPP/RNBQKB1R b KQkq d3 0 2",
    "rnbqkbnr/pppp1ppp/8/4p3/3P4/8/PPP1PPPP/RNBQKBNR w KQkq e6 0 2",
    "rnbqkbnr/pp1ppppp/8/8/3pP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 2",
    "rnbqkb1r/pppppp1p/5np1/8/3P4/5N2/PPP1PPPP/RNBQKB1R w KQkq - 2 3",
    "rnbqkbnr/pppp1ppp/8/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2",
    "rnbqkbnr/pp1ppppp/8/2p5/3P4/8/PPP1PPPP/RNBQKBNR w KQkq c6 0 2",
    "rnbqkbnr/p1pppppp/1p6/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
    "r1bqkbnr/pp1ppppp/2n5/2p5/4P3/2N5/PPPP1PPP/R1BQKBNR w KQkq - 2 3",
    "rnbqkbnr/pppp1ppp/8/4p3/3PP3/8/PPP2PPP/RNBQKBNR b KQkq d3 0 2",
    "r1bqkbnr/pp1ppppp/2n5/2p5/2P5/2N5/PP1PPPPP/R1BQKBNR w KQkq - 2 3",
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/2N5/PPPP1PPP/R1BQKBNR b KQkq - 1 2"
};

struct Sample {
    Board board;
    float target;
};

struct CachedSample {
    std::array<float, TropicalEvaluator::NUM_FEATURES> features;
    float target;
    size_t bucket;
};

struct AdamSectorState {
    std::array<float, TropicalEvaluator::NUM_FEATURES> m_w{};
    std::array<float, TropicalEvaluator::NUM_FEATURES> v_w{};
    float m_b = 0.0f;
    float v_b = 0.0f;
};

int main(int argc, char* argv[]) {
    std::cout << "======================================================\n";
    std::cout << "  Heaven's Gate — Candidate v7 Deep Model Trainer     \n";
    std::cout << "  High-Precision Adam SGD on Corrected PST Geometry   \n";
    std::cout << "======================================================\n\n";

    int num_games = 1500;
    int depth = 5;
    int epochs = 120;
    float lr_max = 0.001f;
    std::string output_model = "heavensgate_v7_candidate.trm";

    if (argc > 1) num_games = std::stoi(argv[1]);
    if (argc > 2) depth = std::stoi(argv[2]);
    if (argc > 3) epochs = std::stoi(argv[3]);
    if (argc > 4) lr_max = std::stof(argv[4]);
    if (argc > 5) output_model = argv[5];

    Zobrist::init();
    MoveGenerator::init();
    Evaluator::init();
    Evaluator::set_mode(EvalMode::SpectralTropical);

    std::vector<Sample> dataset;
    std::cout << "[CandidateTrainer] Simulating " << num_games << " Self-Play Games @ Depth " << depth << "...\n" << std::flush;
    int white_wins = 0, black_wins = 0, draws = 0;
    auto t_start = std::chrono::steady_clock::now();

    int num_threads = omp_get_max_threads();
    omp_set_num_threads(num_threads);
    std::cout << "[CandidateTrainer] Parallelizing across " << num_threads << " CPU threads (100% Full Hardware Mode)...\n\n" << std::flush;

    std::mutex dataset_mutex;
    int completed_games = 0;

    #pragma omp parallel
    {
        SearchEngine search_engine;
        search_engine.tt().resize(64);
        Evaluator::set_mode(EvalMode::SpectralTropical);

        #pragma omp for schedule(dynamic) reduction(+:white_wins,black_wins,draws)
        for (int g = 0; g < num_games; g++) {
            std::string fen = OpeningsBook[g % OpeningsBook.size()];
            Board board;
            if (!FEN::parse(fen, board)) board.reset();

            std::vector<std::pair<Board, float>> history;
            int moves_count = 0;
            int result_score = 0;

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

                SearchResult res = search_engine.search_iterative_deepening(board, depth, 0.0);
                Move chosen_move = res.best_move;
                if (!static_cast<bool>(chosen_move)) chosen_move = legal_moves[0];

                history.push_back({board, static_cast<float>(res.best_score)});
                board.make_move(chosen_move);
                moves_count++;

                if (res.best_score > 2500) {
                    result_score = (board.side_to_move() == Color::White) ? 1 : -1;
                    break;
                } else if (res.best_score < -2500) {
                    result_score = (board.side_to_move() == Color::White) ? -1 : 1;
                    break;
                }
            }

            if (result_score == 1) white_wins++;
            else if (result_score == -1) black_wins++;
            else draws++;

            std::vector<Sample> local_samples;
            for (size_t i = 0; i < history.size(); i++) {
                float game_val = static_cast<float>(result_score) * 1000.0f;
                float search_val = history[i].second;
                float target = 0.75f * search_val + 0.25f * game_val;
                local_samples.push_back({history[i].first, target});
            }

            {
                std::lock_guard<std::mutex> lock(dataset_mutex);
                dataset.insert(dataset.end(), local_samples.begin(), local_samples.end());
                completed_games++;

                if (completed_games % 5 == 0 || completed_games == num_games) {
                    auto t_now = std::chrono::steady_clock::now();
                    float elapsed = std::chrono::duration<float>(t_now - t_start).count();
                    std::cout << "  Game " << std::setw(4) << completed_games << "/" << num_games
                              << " | Dataset: " << std::setw(7) << dataset.size()
                              << " | Time: " << std::fixed << std::setprecision(1) << elapsed << "s\n" << std::flush;
                }
            }
        }
    }

    std::cout << "\n[CandidateTrainer] Dataset Generation Complete: " << dataset.size() << " position samples\n";
    std::cout << "  Outcomes: " << white_wins << " White Wins, " << black_wins << " Black Wins, " << draws << " Draws\n\n";

    TropicalEvaluator model;
    model.initialize_weights(42);

    std::vector<AdamSectorState> adam_state(TropicalEvaluator::TOTAL_SECTORS);
    uint64_t timestep = 0;

    std::cout << "[CandidateTrainer] Pre-computing 22D Feature Vectors for " << dataset.size() << " positions...\n";
    std::vector<CachedSample> cached_dataset(dataset.size());

    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < dataset.size(); i++) {
        const auto& sample = dataset[i];
        Color us = sample.board.side_to_move();
        Color them = ~us;
        Piece opp_king = (them == Color::White) ? Piece::WhiteKing : Piece::BlackKing;
        Square opp_king_sq = sample.board.pieces(opp_king) ? lsb(sample.board.pieces(opp_king)) : Square::None;
        size_t bucket = TropicalEvaluator::get_king_bucket(opp_king_sq, us);

        cached_dataset[i] = {
            model.extract_features(sample.board),
            sample.target,
            bucket
        };
    }

    std::cout << "[CandidateTrainer] Feature Caching Complete! Launching " << epochs << "-Epoch Adam SGD...\n\n";

    std::vector<size_t> indices(cached_dataset.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::mt19937 shuffle_rng(1337);

    float beta1 = 0.9f;
    float beta2 = 0.999f;
    float eps = 1e-8f;
    float lr_min = 0.00005f;
    float final_rmse = 0.0f;

    for (int epoch = 1; epoch <= epochs; epoch++) {
        std::shuffle(indices.begin(), indices.end(), shuffle_rng);

        float current_lr = lr_min + 0.5f * (lr_max - lr_min) * (1.0f + std::cos(static_cast<float>(epoch - 1) / static_cast<float>(std::max(1, epochs - 1)) * 3.1415926535f));
        float total_loss = 0.0f;
        size_t count = 0;

        constexpr size_t BATCH_SIZE = 64;

        struct GradAccum {
            std::array<float, TropicalEvaluator::NUM_FEATURES> grad_w{};
            float grad_b = 0.0f;
        };

        for (size_t b_idx = 0; b_idx < cached_dataset.size(); b_idx += BATCH_SIZE) {
            size_t batch_end = std::min(cached_dataset.size(), b_idx + BATCH_SIZE);
            float batch_len_f = static_cast<float>(batch_end - b_idx);

            std::vector<GradAccum> batch_grads(TropicalEvaluator::TOTAL_SECTORS);

            for (size_t k = b_idx; k < batch_end; k++) {
                size_t idx = indices[k];
                const auto& sample = cached_dataset[idx];

                auto eval_res = model.evaluate_detailed_from_features(sample.features, sample.bucket);
                float error = std::max(-1000.0f, std::min(1000.0f, static_cast<float>(eval_res.score) - sample.target));
                total_loss += error * error;
                count++;

                size_t base_sec_idx = sample.bucket * TropicalEvaluator::NUM_SECTORS_PER_BUCKET;
                for (size_t j = 0; j < TropicalEvaluator::NUM_SECTORS_PER_BUCKET; j++) {
                    size_t sec_idx = base_sec_idx + j;
                    float prob = eval_res.softmax_probs[j];
                    if (prob < 1e-4f) continue;

                    float sector_error = error * prob;
                    for (size_t i = 0; i < TropicalEvaluator::NUM_FEATURES; i++) {
                        batch_grads[sec_idx].grad_w[i] += (sector_error * sample.features[i]) / (25.0f * batch_len_f);
                    }
                    batch_grads[sec_idx].grad_b += (sector_error) / (5.0f * batch_len_f);
                }
            }

            timestep++;
            float denom1 = std::max(1e-6f, static_cast<float>(1.0f - std::pow(beta1, timestep)));
            float denom2 = std::max(1e-6f, static_cast<float>(1.0f - std::pow(beta2, timestep)));

            for (size_t sec_idx = 0; sec_idx < TropicalEvaluator::TOTAL_SECTORS; sec_idx++) {
                auto& sec = model.sectors()[sec_idx];
                auto& adam = adam_state[sec_idx];
                const auto& g_acc = batch_grads[sec_idx];

                for (size_t i = 0; i < TropicalEvaluator::NUM_FEATURES; i++) {
                    float grad = std::max(-50.0f, std::min(50.0f, g_acc.grad_w[i])) + 0.0001f * sec.w[i];
                    if (std::abs(grad) < 1e-7f) continue;

                    adam.m_w[i] = beta1 * adam.m_w[i] + (1.0f - beta1) * grad;
                    adam.v_w[i] = beta2 * adam.v_w[i] + (1.0f - beta2) * (grad * grad);

                    float m_hat = adam.m_w[i] / denom1;
                    float v_hat = adam.v_w[i] / denom2;

                    sec.w[i] -= (current_lr * m_hat) / (std::sqrt(v_hat) + eps);
                    float min_w = (i == 0) ? 0.75f : ((i == 4) ? 0.20f : ((i == 1 || i == 5 || i == 10) ? 0.10f : 0.0f));
                    float max_w = (i == 0) ? 1.25f : 5.0f;
                    sec.w[i] = std::max(min_w, std::min(max_w, sec.w[i]));
                }

                float grad_b = std::max(-50.0f, std::min(50.0f, g_acc.grad_b));
                if (std::abs(grad_b) > 1e-7f) {
                    adam.m_b = beta1 * adam.m_b + (1.0f - beta1) * grad_b;
                    adam.v_b = beta2 * adam.v_b + (1.0f - beta2) * (grad_b * grad_b);

                    float m_hat_b = adam.m_b / denom1;
                    float v_hat_b = adam.v_b / denom2;
                    sec.b -= (current_lr * m_hat_b) / (std::sqrt(v_hat_b) + eps);
                    sec.b = std::max(-500.0f, std::min(500.0f, sec.b));
                }
            }
        }

        float rmse = std::sqrt(total_loss / std::max(1ULL, count));
        final_rmse = rmse;

        if (epoch % 10 == 0 || epoch == 1 || epoch == epochs) {
            std::cout << "  [Epoch " << std::setw(3) << epoch << "/" << epochs << "] RMSE: " 
                      << std::fixed << std::setprecision(2) << rmse << " cp | LR: " 
                      << std::setprecision(6) << current_lr << "\n" << std::flush;
        }
    }

    if (model.save_weights(output_model)) {
        std::cout << "\n[SUCCESS] Candidate model saved to " << output_model << " (RMSE: " << final_rmse << " cp)\n";
    }

    return 0;
}
