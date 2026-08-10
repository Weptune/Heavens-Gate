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
    std::cout << "======================================================\n\n" << std::flush;

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
    int white_wins = 0, black_wins = 0, draws = 0;
    auto t_start = std::chrono::steady_clock::now();

    int num_threads = std::min(10, omp_get_max_threads());
    omp_set_num_threads(num_threads);
    std::cout << "[SpectralTropical] Parallelizing dataset generation across " << num_threads << " CPU threads (10-Core High-Performance Mode)...\n\n";

    std::mutex dataset_mutex;
    int completed_games = 0;

    #pragma omp parallel
    {
        SearchEngine search_engine;
        search_engine.tt().resize(64); // 64 MB TT cache per thread (512 MB total RAM across 8 threads)
        Evaluator::set_mode(EvalMode::SpectralTropical);

        #pragma omp for schedule(dynamic) reduction(+:white_wins,black_wins,draws)
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
                // Use engine best move from depth-5 search (51-position opening book handles opening diversity)
                // Remove uniform random blunders so games are always sound 60-120 move battles
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

                if (board.halfmove_clock() >= 100 || board.is_repetition()) {
                    result_score = 0;
                    break;
                }
            }

            if (result_score > 0) white_wins++;
            else if (result_score < 0) black_wins++;
            else draws++;

            std::vector<Sample> local_samples;
            for (size_t idx = 5; idx < history.size(); idx++) {
                const auto& item = history[idx];
                float side_outcome = (item.first.side_to_move() == Color::White) ? static_cast<float>(result_score) : -static_cast<float>(result_score);
                float outcome_cp = side_outcome * 400.0f;

                // Ground Truth Evaluation: Blend positional engine ground truth with game outcome
                Evaluator::set_mode(EvalMode::MasterPositional);
                float master_score = static_cast<float>(Evaluator::evaluate(item.first));
                Evaluator::set_mode(EvalMode::SpectralTropical);

                // 85% Ground Truth Positional Anchor + 15% Game Outcome (Original Phase 2 Proven Ratio)
                float target = 0.85f * master_score + 0.15f * outcome_cp;
                local_samples.push_back({item.first, target});
            }

            {
                std::lock_guard<std::mutex> lock(dataset_mutex);
                dataset.insert(dataset.end(), local_samples.begin(), local_samples.end());
                completed_games++;
                if (completed_games % 2 == 0 || completed_games <= 10 || completed_games == num_games) {
                    auto t_now = std::chrono::steady_clock::now();
                    float elapsed = std::chrono::duration<float>(t_now - t_start).count();
                    std::cout << "  Game " << std::setw(3) << completed_games << "/" << num_games
                              << " | Dataset: " << std::setw(6) << dataset.size()
                              << " | Time: " << std::fixed << std::setprecision(1) << elapsed << "s\n";
                    std::cout << std::flush;
                }
            }
        }
    }

    std::cout << "\n[SpectralTropical] Parallel Dataset Generation Complete: " << dataset.size() << " new position samples\n";
    std::cout << "  Outcomes: " << white_wins << " White Wins, " << black_wins << " Black Wins, " << draws << " Draws\n\n";

    // =========================================================================
    // 150,000 Position Rolling Dataset Buffer (Multi-Round Memory)
    // =========================================================================
    std::vector<Sample> dataset_buffer;
    std::ifstream buf_in("dataset_buffer.bin", std::ios::binary);
    if (buf_in.is_open()) {
        uint32_t buf_size = 0;
        buf_in.read(reinterpret_cast<char*>(&buf_size), sizeof(buf_size));
        for (uint32_t i = 0; i < buf_size; i++) {
            uint16_t fen_len = 0;
            buf_in.read(reinterpret_cast<char*>(&fen_len), sizeof(fen_len));
            std::string fen(fen_len, '\0');
            buf_in.read(&fen[0], fen_len);
            float target = 0.0f;
            buf_in.read(reinterpret_cast<char*>(&target), sizeof(target));
            Board b;
            if (FEN::parse(fen, b)) {
                dataset_buffer.push_back({b, target});
            }
        }
        std::cout << "[SpectralTropical] Loaded " << dataset_buffer.size() << " previous samples from Rolling Dataset Buffer\n";
    }

    // Append new round samples to rolling buffer
    dataset_buffer.insert(dataset_buffer.end(), dataset.begin(), dataset.end());

    // Trim to most recent 500,000 samples
    constexpr size_t MAX_BUFFER_SIZE = 500000;
    if (dataset_buffer.size() > MAX_BUFFER_SIZE) {
        size_t excess = dataset_buffer.size() - MAX_BUFFER_SIZE;
        dataset_buffer.erase(dataset_buffer.begin(), dataset_buffer.begin() + excess);
    }
    std::cout << "[SpectralTropical] Active Training Dataset Size: " << dataset_buffer.size() << " positions (Multi-Round Buffer)\n\n";

    // Save updated buffer to disk
    std::ofstream buf_out("dataset_buffer.bin", std::ios::binary);
    if (buf_out.is_open()) {
        uint32_t buf_size = static_cast<uint32_t>(dataset_buffer.size());
        buf_out.write(reinterpret_cast<const char*>(&buf_size), sizeof(buf_size));
        for (const auto& s : dataset_buffer) {
            std::string fen = FEN::to_string(s.board);
            uint16_t fen_len = static_cast<uint16_t>(fen.size());
            buf_out.write(reinterpret_cast<const char*>(&fen_len), sizeof(fen_len));
            buf_out.write(fen.data(), fen_len);
            buf_out.write(reinterpret_cast<const char*>(&s.target), sizeof(s.target));
        }
    }

    // Use full rolling dataset_buffer for training
    std::vector<Sample>& active_dataset = dataset_buffer;

    // =========================================================================
    // Adam Optimizer on the Tropical (max, +) Minimax Surface
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
    std::vector<SectorAdam> adam_state(TropicalEvaluator::TOTAL_SECTORS);
    int timestep = 0;

    // Load persistent Adam momentum and variance state (with strict binary size validation)
    std::ifstream adam_in("heavensgate_adam.dat", std::ios::binary | std::ios::ate);
    constexpr size_t EXPECTED_ADAM_SIZE = sizeof(int) + TropicalEvaluator::TOTAL_SECTORS * (2 * TropicalEvaluator::NUM_FEATURES * sizeof(float) + 2 * sizeof(float));
    if (adam_in.is_open()) {
        size_t file_size = static_cast<size_t>(adam_in.tellg());
        adam_in.seekg(0, std::ios::beg);
        if (file_size == EXPECTED_ADAM_SIZE) {
            adam_in.read(reinterpret_cast<char*>(&timestep), sizeof(timestep));
            for (auto& sec : adam_state) {
                adam_in.read(reinterpret_cast<char*>(sec.m_w.data()), TropicalEvaluator::NUM_FEATURES * sizeof(float));
                adam_in.read(reinterpret_cast<char*>(sec.v_w.data()), TropicalEvaluator::NUM_FEATURES * sizeof(float));
                adam_in.read(reinterpret_cast<char*>(&sec.m_b), sizeof(sec.m_b));
                adam_in.read(reinterpret_cast<char*>(&sec.v_b), sizeof(sec.v_b));
            }
            std::cout << "[SpectralTropical] Loaded persistent Adam state (Timestep: " << timestep << ")\n";
        } else {
            std::cout << "[SpectralTropical] Adam state binary size mismatch (" << file_size << " != " << EXPECTED_ADAM_SIZE << " bytes). Resetting Adam state.\n";
            adam_in.close();
            std::remove("heavensgate_adam.dat");
        }
    }

    float beta1 = 0.9f;
    float beta2 = 0.999f;
    float eps = 1e-8f;
    float weight_decay = 0.0f; // Disabled per-sample L2 decay to preserve positional weights across millions of steps

    std::cout << "[SpectralTropical] Training via Adam Optimizer (Epochs=" << epochs
              << ", LR=" << lr << ", Decay=" << lr_decay << ")...\n\n";

    std::cout << "[SpectralTropical] Pre-computing " << TropicalEvaluator::NUM_FEATURES << "D Feature Vectors for " << active_dataset.size() << " positions...\n";
    struct CachedSample {
        std::array<float, TropicalEvaluator::NUM_FEATURES> features;
        size_t bucket;
        float target;
    };
    std::vector<CachedSample> cached_dataset(active_dataset.size());

    #pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < active_dataset.size(); i++) {
        const auto& s = active_dataset[i];
        cached_dataset[i].features = model.extract_features(s.board);
        Square ksq = s.board.king_square(s.board.side_to_move() == Color::White ? Color::Black : Color::White);
        cached_dataset[i].bucket = model.get_king_bucket(ksq);
        cached_dataset[i].target = s.target;
    }
    std::cout << "[SpectralTropical] Feature Caching Complete! Launching 80-Epoch Adam SGD...\n\n";

    std::mt19937 shuffle_rng(123);
    std::vector<size_t> indices(cached_dataset.size());
    std::iota(indices.begin(), indices.end(), 0);

    // 0. Compute Epoch 0 Baseline RMSE on current dataset using loaded model
    float init_loss = 0.0f;
    for (size_t i = 0; i < cached_dataset.size(); i++) {
        auto eval_res = model.evaluate_detailed_from_features(cached_dataset[i].features, cached_dataset[i].bucket);
        float err = std::max(-1000.0f, std::min(1000.0f, static_cast<float>(eval_res.score) - cached_dataset[i].target));
        init_loss += err * err;
    }
    float initial_rmse = std::sqrt(init_loss / cached_dataset.size());

    float final_rmse = initial_rmse;
    float best_rmse = initial_rmse;
    int best_epoch = 0;
    TropicalEvaluator best_model = model;

    std::cout << "[SpectralTropical] Loaded Checkpoint Baseline RMSE (Epoch 0): " 
              << std::fixed << std::setprecision(2) << initial_rmse << " cp\n\n";

    for (int epoch = 1; epoch <= epochs; epoch++) {
        std::shuffle(indices.begin(), indices.end(), shuffle_rng);

        float total_loss = 0.0f;
        size_t count = 0;

        for (size_t idx : indices) {
            const auto& sample = cached_dataset[idx];
            timestep++;

            float denom1 = std::max(1e-6f, static_cast<float>(1.0f - std::pow(beta1, timestep)));
            float denom2 = std::max(1e-6f, static_cast<float>(1.0f - std::pow(beta2, timestep)));

            // 1. Fast evaluation from pre-computed feature vector
            auto eval_res = model.evaluate_detailed_from_features(sample.features, sample.bucket);

            // 2. Compute prediction error
            float error = std::max(-1000.0f, std::min(1000.0f, static_cast<float>(eval_res.score) - sample.target));
            total_loss += error * error;
            count++;

            size_t base_sec_idx = sample.bucket * TropicalEvaluator::NUM_SECTORS_PER_BUCKET;

            // 3. Softmax-Weighted Gradient Distribution across active King Bucket
            for (size_t j = 0; j < TropicalEvaluator::NUM_SECTORS_PER_BUCKET; j++) {
                size_t sec_idx = base_sec_idx + j;
                float prob = eval_res.softmax_probs[j];
                if (prob < 1e-4f) continue;

                auto& sec = model.sectors()[sec_idx];
                auto& adam = adam_state[sec_idx];

                float sector_error = error * prob;

                // Material weight w[0] (learned inside sectors, bounded [0.8, 1.2])
                {
                    float grad0 = (sector_error * sample.features[0]) / 100.0f + weight_decay * (sec.w[0] - 1.0f);
                    grad0 = std::max(-50.0f, std::min(50.0f, grad0));
                    adam.m_w[0] = beta1 * adam.m_w[0] + (1.0f - beta1) * grad0;
                    adam.v_w[0] = beta2 * adam.v_w[0] + (1.0f - beta2) * (grad0 * grad0);
                    float m_hat0 = adam.m_w[0] / denom1;
                    float v_hat0 = adam.v_w[0] / denom2;
                    sec.w[0] -= (lr * m_hat0) / (std::sqrt(v_hat0) + eps);
                    sec.w[0] = std::max(0.8f, std::min(1.2f, sec.w[0]));
                }

                // Positional weights w[1..21] (non-negative bounds [0.0, 5.0])
                for (size_t i = 1; i < TropicalEvaluator::NUM_FEATURES; i++) {
                    float grad = (sector_error * sample.features[i]) / 100.0f + weight_decay * sec.w[i];
                    grad = std::max(-50.0f, std::min(50.0f, grad));

                    adam.m_w[i] = beta1 * adam.m_w[i] + (1.0f - beta1) * grad;
                    adam.v_w[i] = beta2 * adam.v_w[i] + (1.0f - beta2) * (grad * grad);

                    float m_hat = adam.m_w[i] / denom1;
                    float v_hat = adam.v_w[i] / denom2;

                    // Enforce healthy minimum floor weights so vital chess features (mobility, passed pawns, king shield) are never eroded to zero
                    static constexpr float feature_floors[22] = {
                        0.85f, // x[ 0] Material
                        0.30f, // x[ 1] Fiedler
                        0.15f, // x[ 2] Cohesion
                        0.15f, // x[ 3] Spectral Gap
                        0.40f, // x[ 4] PST
                        0.30f, // x[ 5] King Press
                        0.20f, // x[ 6] Battery
                        0.30f, // x[ 7] Pawn Coh
                        0.15f, // x[ 8] Trace Energy
                        0.25f, // x[ 9] Mobility
                        0.30f, // x[10] Center Control
                        0.20f, // x[11] Phase
                        0.25f, // x[12] King Shield
                        0.40f, // x[13] Passed Pawns
                        0.40f, // x[14] EG Passed Pawns
                        0.20f, // x[15] Attack Ratio
                        0.20f, // x[16] BatXCenter
                        0.15f, // x[17] FiedXPWN
                        0.15f, // x[18] EG_Mobility
                        0.15f, // x[19] PassXCenter
                        0.15f, // x[20] KingXBat
                        0.15f  // x[21] ShldXPWN
                    };
                    float min_w = feature_floors[i];
                    sec.w[i] = std::max(min_w, std::min(5.0f, sec.w[i]));
                }

                // Sector bias
                float grad_b = sector_error / 10.0f;
                grad_b = std::max(-50.0f, std::min(50.0f, grad_b));

                adam.m_b = beta1 * adam.m_b + (1.0f - beta1) * grad_b;
                adam.v_b = beta2 * adam.v_b + (1.0f - beta2) * (grad_b * grad_b);

                float m_hat_b = adam.m_b / denom1;
                float v_hat_b = adam.v_b / denom2;
                sec.b -= (lr * m_hat_b) / (std::sqrt(v_hat_b) + eps);
                sec.b = std::max(-250.0f, std::min(250.0f, sec.b));
            }
        }

        float rmse = std::sqrt(total_loss / count);
        final_rmse = rmse;

        if (rmse < best_rmse) {
            best_rmse = rmse;
            best_epoch = epoch;
            best_model = model; // Save exact best snapshot weights
        }

        if (epoch <= 10 || epoch % 10 == 0 || epoch == epochs) {
            std::cout << "  [Epoch " << std::setw(3) << epoch << "/" << epochs
                      << "] RMSE: " << std::fixed << std::setprecision(2) << rmse
                      << " cp | LR: " << std::setprecision(6) << lr;
            if (epoch == best_epoch) {
                std::cout << " ⭐ [BEST CHECKPOINT: " << best_rmse << " cp]";
            }
            std::cout << "\n" << std::flush;
        }

        // Learning rate decay
        lr *= lr_decay;
    }

    std::cout << "\n[CHECKPOINT RETENTION] Selected Epoch " << best_epoch << " Best Weights with Lowest RMSE: " << best_rmse << " cp (vs Final Epoch: " << final_rmse << " cp)\n";

    std::string model_path = "heavensgate_tropical.trm";
    if (best_model.save_weights(model_path)) {
        std::cout << "[SUCCESS] Saved Best Spectral-Tropical weights (Epoch " << best_epoch << ", " << best_rmse << " cp) to " << model_path << "\n";

        // Save persistent Adam state
        std::ofstream adam_out("heavensgate_adam.dat", std::ios::binary);
        if (adam_out.is_open()) {
            adam_out.write(reinterpret_cast<const char*>(&timestep), sizeof(timestep));
            for (const auto& sec : adam_state) {
                adam_out.write(reinterpret_cast<const char*>(sec.m_w.data()), TropicalEvaluator::NUM_FEATURES * sizeof(float));
                adam_out.write(reinterpret_cast<const char*>(sec.v_w.data()), TropicalEvaluator::NUM_FEATURES * sizeof(float));
                adam_out.write(reinterpret_cast<const char*>(&sec.m_b), sizeof(sec.m_b));
                adam_out.write(reinterpret_cast<const char*>(&sec.v_b), sizeof(sec.v_b));
            }
            std::cout << "[SUCCESS] Saved persistent Adam state to heavensgate_adam.dat\n";
        }

        // Feature Weight Telemetry Summary
        static const char* feat_names[22] = {
            "Material", "Fiedler", "Cohesion", "Gap", "PST", "KingPress",
            "Battery", "PawnCoh", "Trace", "Mobility", "Center", "Phase",
            "Shield", "Passed", "EG_Passed", "Attack_Ratio",
            "BatXCenter", "FiedXPWN", "EG_Mobility", "PassXCenter", "KingXBat", "ShldXPWN"
        };
        std::cout << "\n======================================================\n";
        std::cout << "  LEARNED FEATURE WEIGHT TELEMETRY (320 SECTORS)\n";
        std::cout << "======================================================\n";

        // Collect feature stats for stdout and JSON tracking
        std::ofstream json_out("model_weight_history.log", std::ios::app);
        json_out << "FINAL_RMSE: " << final_rmse << " | WEIGHTS: ";

        for (size_t f = 0; f < TropicalEvaluator::NUM_FEATURES; f++) {
            float sum_w = 0.0f, min_w = 1e9f, max_w = -1e9f;
            for (const auto& sec : model.sectors()) {
                float w = sec.w[f];
                sum_w += w;
                min_w = std::min(min_w, w);
                max_w = std::max(max_w, w);
            }
            float avg_w = sum_w / static_cast<float>(TropicalEvaluator::TOTAL_SECTORS);
            std::cout << "  x[" << std::setw(2) << f << "] (" << std::setw(12) << feat_names[f]
                      << "): Avg=" << std::fixed << std::setprecision(4) << std::showpos << avg_w
                      << " | Range=[" << std::noshowpos << min_w << ", " << max_w << "]\n";

            json_out << feat_names[f] << "=" << avg_w << ",";
        }
        json_out << "\n";
        json_out.close();

        std::cout << "======================================================\n\n";
    }

    return 0;
}
