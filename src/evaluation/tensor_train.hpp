#pragma once

#include "tensor_eval.hpp"
#include "../board/board.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <array>

namespace heavensgate {

// Training sample containing board position features and target evaluation (centipawns)
struct TrainingSample {
    int side_to_move;                 // 0 = White, 1 = Black
    std::array<uint8_t, 64> board_sq; // Piece index [0..12] for each site in Hilbert order
    float target_eval;                // Target centipawn evaluation
};

class TensorTrainer {
public:
    struct Config {
        int bond_dim = 16;
        float learning_rate = 0.001f;
        float weight_decay = 1e-5f;
        float grad_clip = 1.0f;
        int batch_size = 256;
        int epochs = 10;
        uint32_t seed = 42;
    };

    explicit TensorTrainer(TensorMPS& model);
    TensorTrainer(TensorMPS& model, Config config);

    // Convert FEN position into a TrainingSample
    static TrainingSample create_sample(const Board& board, float target_eval_cp);

    // Train model on a dataset of TrainingSamples using mini-batch SGD with Adam optimizer
    void train(const std::vector<TrainingSample>& dataset, float validation_split = 0.1f);

    // Single step forward-backward pass returning prediction and accumulating gradients
    float train_step(const TrainingSample& sample, std::vector<float>& grad_stm,
                     std::vector<float>& grad_bulk, std::vector<float>& grad_right);

private:
    TensorMPS& model_;
    Config config_;

    // Adam optimizer momentum buffers
    std::vector<float> m_stm_, v_stm_;
    std::vector<float> m_bulk_, v_bulk_;
    std::vector<float> m_right_, v_right_;
    int adam_t_ = 0;

    void update_weights_adam(const std::vector<float>& grad_stm,
                             const std::vector<float>& grad_bulk,
                             const std::vector<float>& grad_right,
                             int batch_count);
};

} // namespace heavensgate
