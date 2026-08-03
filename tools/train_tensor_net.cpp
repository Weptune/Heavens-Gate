#include "../src/evaluation/tensor_eval.hpp"
#include "../src/evaluation/tensor_train.hpp"
#include "../src/evaluation/eval.hpp"
#include "../src/core/fen.hpp"
#include "../src/core/zobrist.hpp"
#include "../src/board/board.hpp"
#include "../src/movegen/movegen.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>

using namespace heavensgate;

int main(int argc, char* argv[]) {
    std::cout << "====================================================\n";
    std::cout << " Heaven's Gate — Tensor Network MPS Trainer        \n";
    std::cout << " Quantum-Inspired Many-Body Evaluation Model       \n";
    std::cout << "====================================================\n\n";

    int bond_dim = 16;
    int epochs = 15;
    std::string output_path = "heavensgate.tnw";

    if (argc > 1) bond_dim = std::stoi(argv[1]);
    if (argc > 2) epochs = std::stoi(argv[2]);
    if (argc > 3) output_path = argv[3];

    // Seed dataset of benchmark & tactical positions
    std::vector<std::string> sample_fens = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1",
        "rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2",
        "rnbqkbnr/pppp1ppp/4p3/8/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2",
        "r1bqkbnr/pppp1ppp/2n1p3/8/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
        "r1bqkbnr/pppp1ppp/2n1p3/8/3PP3/5N2/PPP2PPP/RNBQKB1R b KQkq - 0 3",
        "r1bqk2r/pp2bppp/2n1pn2/3p4/3PP3/2N2N2/PP2BPPP/R1BQ1RK1 w kq - 0 8",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        "8/8/8/8/8/8/8/4K2k w - - 0 1",
        "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
        "r1bqk2r/ppp2ppp/2n5/1B1pp3/4P3/5N2/PPPP1PPP/R1BQK2R w KQkq - 0 6",
        "rnbqk2r/ppp1bppp/4pn2/3p4/2PP4/2N2N2/PP2PPPP/R1BQKB1R w KQkq - 2 5",
        "rnbqkb1r/pp2pppp/3p1n2/8/3NP3/8/PPP2PPP/RNBQKB1R w KQkq - 0 5",
        "r1bqk1r1/1p1p1p1p/p1n1p3/8/4P3/2N5/PPP2PPP/R2QKB1R w KQq - 1 10",
        "rnbq1rk1/ppp1bppp/4pn2/3p2B1/2PP4/2N2N2/PP2PPPP/R2QKB1R w KQ - 4 6",
        "2r2rk1/pp1b1ppp/1q2p3/3pP3/1b1P4/1P1Q1N2/P3NPPP/2R2RK1 w - - 1 16",
        "r1b1qrk1/1pp1b1pp/p1n1p3/3p1p2/3P1P2/2N1PN2/PPP1B1PP/R2Q1RK1 w - - 0 11",
    };

    std::cout << "[Trainer] Initializing Engine Subsystems (Zobrist, MoveGenerator, Evaluator)...\n";
    Zobrist::init();
    MoveGenerator::init();
    Evaluator::init();
    Evaluator::set_mode(EvalMode::MasterPositional);

    Board board;
    std::vector<TrainingSample> dataset;

    // Generate samples with data augmentation (all 20 base positions + FEN variations)
    for (const auto& fen : sample_fens) {
        if (!FEN::parse(fen, board)) continue;
        float target_eval = static_cast<float>(Evaluator::evaluate(board));

        dataset.push_back(TensorTrainer::create_sample(board, target_eval));

        // Create perturbed positions for broader coverage
        MoveList moves;
        MoveGenerator::generate_legal_moves(board, moves);
        for (size_t i = 0; i < std::min<size_t>(moves.size(), 8); i++) {
            Board child = board;
            child.make_move(moves[i]);
            float child_eval = static_cast<float>(Evaluator::evaluate(child));
            dataset.push_back(TensorTrainer::create_sample(child, child_eval));
        }
    }

    std::cout << "[Trainer] Generated " << dataset.size() << " training samples.\n";

    TensorMPS model(bond_dim);
    model.initialize_random(42);

    TensorTrainer::Config config;
    config.bond_dim = bond_dim;
    config.epochs = epochs;
    config.batch_size = 32;
    config.learning_rate = 0.002f;
    config.weight_decay = 1e-4f;

    TensorTrainer trainer(model, config);

    auto start_time = std::chrono::high_resolution_clock::now();

    trainer.train(dataset, 0.15f);

    auto end_time = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(end_time - start_time).count();

    std::cout << "\n[Trainer] Training completed in " << duration << " seconds.\n";

    std::cout << "[Trainer] Saving trained weights to " << output_path << "...\n";
    if (model.save_weights(output_path)) {
        std::cout << "[SUCCESS] Saved Tensor Network weights to " << output_path << "\n";
    } else {
        std::cerr << "[ERROR] Failed to save weights to " << output_path << "\n";
    }

    return 0;
}
