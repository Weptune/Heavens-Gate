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

// Pure Plain-Old-Data (POD) sample structure for 100% safe binary I/O
struct PODSample {
    std::array<float, TropicalEvaluator::NUM_FEATURES> features;
    float target;
    uint32_t bucket;
};

struct AdamSectorState {
    std::array<float, TropicalEvaluator::NUM_FEATURES> m_w{};
    std::array<float, TropicalEvaluator::NUM_FEATURES> v_w{};
    float m_b = 0.0f;
    float v_b = 0.0f;
};

int main(int argc, char* argv[]) {
    std::cout << "======================================================\n";
    std::cout << "  Heaven's Gate — Continuous Autonomous v7 Trainer    \n";
    std::cout << "  Depth 10 Self-Play + POD Buffer + Adam SGD          \n";
    std::cout << "======================================================\n\n";

    int num_games = 150;
    int depth = 10;
    int epochs = 80;
    float lr_max = 0.0008f;
    std::string buffer_path = "dataset_buffer_v7_pod.bin";
    std::string model_path  = "heavensgate_tropical.trm";

    if (argc > 1) num_games = std::stoi(argv[1]);
    if (argc > 2) depth = std::stoi(argv[2]);
    if (argc > 3) epochs = std::stoi(argv[3]);
    if (argc > 4) lr_max = std::stof(argv[4]);

    Zobrist::init();
    MoveGenerator::init();
    Evaluator::init();
    Evaluator::set_mode(EvalMode::SpectralTropical);

    TropicalEvaluator model;
    if (model.load_weights(model_path)) {
        std::cout << "[ContinuousTrainer] Loaded active weights from " << model_path << "\n";
    } else {
        std::cout << "[ContinuousTrainer] Initializing baseline weights (Seed 42)\n";
        model.initialize_weights(42);
    }

    std::vector<PODSample> new_samples;
    std::cout << "[ContinuousTrainer] Simulating " << num_games << " Self-Play Games @ Depth " << depth << "...\n" << std::flush;
    int white_wins = 0, black_wins = 0, draws = 0;
    auto t_start = std::chrono::steady_clock::now();

    int num_threads = omp_get_max_threads();
    omp_set_num_threads(num_threads);
    std::cout << "[ContinuousTrainer] Parallelizing across " << num_threads << " CPU threads...\n\n" << std::flush;

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

            std::vector<PODSample> local_samples;
            for (size_t i = 0; i < history.size(); i++) {
                float game_val = static_cast<float>(result_score) * 1000.0f;
                float search_val = history[i].second;
                float target = 0.75f * search_val + 0.25f * game_val;
                const Board& b = history[i].first;

                Color us = b.side_to_move();
                Color them = ~us;
                Piece opp_king = (them == Color::White) ? Piece::WhiteKing : Piece::BlackKing;
                Square opp_king_sq = b.pieces(opp_king) ? lsb(b.pieces(opp_king)) : Square::None;
                uint32_t bucket = static_cast<uint32_t>(TropicalEvaluator::get_king_bucket(opp_king_sq, us));

                local_samples.push_back({
                    model.extract_features(b),
                    target,
                    bucket
                });
            }

            {
                std::lock_guard<std::mutex> lock(dataset_mutex);
                new_samples.insert(new_samples.end(), local_samples.begin(), local_samples.end());
                completed_games++;

                if (completed_games % 5 == 0 || completed_games == num_games) {
                    auto t_now = std::chrono::steady_clock::now();
                    float elapsed = std::chrono::duration<float>(t_now - t_start).count();
                    std::cout << "  Game " << std::setw(4) << completed_games << "/" << num_games
                              << " | New Positions: " << std::setw(6) << new_samples.size()
                              << " | Time: " << std::fixed << std::setprecision(1) << elapsed << "s\n" << std::flush;
                }
            }
        }
    }

    std::cout << "\n[ContinuousTrainer] New Data Generated: " << new_samples.size() << " positions\n";
    std::cout << "  Outcomes: " << white_wins << " White Wins, " << black_wins << " Black Wins, " << draws << " Draws\n\n";

    // Load POD rolling buffer
    std::vector<PODSample> active_dataset = new_samples;
    constexpr size_t MAX_BUFFER_SIZE = 250000;

    std::ifstream buf_in(buffer_path, std::ios::binary);
    if (buf_in.is_open()) {
        uint64_t old_count = 0;
        buf_in.read(reinterpret_cast<char*>(&old_count), sizeof(old_count));
        for (uint64_t i = 0; i < old_count; i++) {
            PODSample s;
            buf_in.read(reinterpret_cast<char*>(&s), sizeof(s));
            if (active_dataset.size() < MAX_BUFFER_SIZE) {
                active_dataset.push_back(s);
            }
        }
        buf_in.close();
        std::cout << "[ContinuousTrainer] Loaded buffer: " << old_count << " historical positions (Active Buffer: " << active_dataset.size() << ")\n";
    }

    // Save updated POD rolling buffer (Safe & fast plain binary)
    std::ofstream buf_out(buffer_path, std::ios::binary);
    if (buf_out.is_open()) {
        uint64_t count = active_dataset.size();
        buf_out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (const auto& sample : active_dataset) {
            buf_out.write(reinterpret_cast<const char*>(&sample), sizeof(sample));
        }
        buf_out.close();
    }

    std::vector<AdamSectorState> adam_state(TropicalEvaluator::TOTAL_SECTORS);
    uint64_t timestep = 0;

    std::ifstream adam_in("heavensgate_adam_v7.dat", std::ios::binary);
    if (adam_in.is_open()) {
        adam_in.read(reinterpret_cast<char*>(&timestep), sizeof(timestep));
        for (auto& sec : adam_state) {
            adam_in.read(reinterpret_cast<char*>(sec.m_w.data()), TropicalEvaluator::NUM_FEATURES * sizeof(float));
            adam_in.read(reinterpret_cast<char*>(sec.v_w.data()), TropicalEvaluator::NUM_FEATURES * sizeof(float));
            adam_in.read(reinterpret_cast<char*>(&sec.m_b), sizeof(float));
            adam_in.read(reinterpret_cast<char*>(&sec.v_b), sizeof(float));
        }
        adam_in.close();
        std::cout << "[ContinuousTrainer] Restored Adam state at timestep " << timestep << "\n";
    }

    std::cout << "[ContinuousTrainer] Launching " << epochs << "-Epoch Adam SGD on " << active_dataset.size() << " positions...\n\n";

    std::vector<size_t> indices(active_dataset.size());
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

        for (size_t b_idx = 0; b_idx < active_dataset.size(); b_idx += BATCH_SIZE) {
            size_t batch_end = std::min(active_dataset.size(), b_idx + BATCH_SIZE);
            float batch_len_f = static_cast<float>(batch_end - b_idx);

            std::vector<GradAccum> batch_grads(TropicalEvaluator::TOTAL_SECTORS);

            for (size_t k = b_idx; k < batch_end; k++) {
                size_t idx = indices[k];
                const auto& sample = active_dataset[idx];

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

    // Save updated model and Adam state
    model.save_weights(model_path);
    model.save_weights("heavensgate_v7_candidate.trm");

    std::ofstream adam_out("heavensgate_adam_v7.dat", std::ios::binary);
    if (adam_out.is_open()) {
        adam_out.write(reinterpret_cast<const char*>(&timestep), sizeof(timestep));
        for (const auto& sec : adam_state) {
            adam_out.write(reinterpret_cast<const char*>(sec.m_w.data()), TropicalEvaluator::NUM_FEATURES * sizeof(float));
            adam_out.write(reinterpret_cast<const char*>(sec.v_w.data()), TropicalEvaluator::NUM_FEATURES * sizeof(float));
            adam_out.write(reinterpret_cast<const char*>(&sec.m_b), sizeof(float));
            adam_out.write(reinterpret_cast<const char*>(&sec.v_b), sizeof(float));
        }
        adam_out.close();
    }

    std::cout << "\n[SUCCESS] Model and Adam state saved cleanly! (Final RMSE: " << final_rmse << " cp)\n";
    return 0;
}
