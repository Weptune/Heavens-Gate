#include "tensor_train.hpp"
#include "../core/fen.hpp"
#include <cmath>
#include <algorithm>
#include <random>
#include <iostream>
#include <cstring>

namespace heavensgate {

// =============================================================================
// Construction
// =============================================================================

TensorTrainer::TensorTrainer(TensorMPS& model)
    : TensorTrainer(model, Config()) {}

TensorTrainer::TensorTrainer(TensorMPS& model, Config config)
    : model_(model), config_(config) {
    int D = model_.bond_dim();
    int stm_size = TensorMPS::LOCAL_DIM_STM * D;
    int bulk_size = TensorMPS::NUM_BULK * TensorMPS::LOCAL_DIM_SQ * D * D;
    int right_size = TensorMPS::LOCAL_DIM_SQ * D;

    m_stm_.resize(stm_size, 0.0f);
    v_stm_.resize(stm_size, 0.0f);

    m_bulk_.resize(bulk_size, 0.0f);
    v_bulk_.resize(bulk_size, 0.0f);

    m_right_.resize(right_size, 0.0f);
    v_right_.resize(right_size, 0.0f);

    adam_t_ = 0;
}

// =============================================================================
// Helper: Create TrainingSample from Board
// =============================================================================

TrainingSample TensorTrainer::create_sample(const Board& board, float target_eval_cp) {
    TrainingSample sample;
    sample.side_to_move = (board.side_to_move() == Color::White) ? 0 : 1;
    sample.target_eval = target_eval_cp;

    const auto& hilbert = HilbertCurve::order();
    for (int site = 0; site < 64; site++) {
        Square sq = static_cast<Square>(hilbert[site]);
        Piece p = board.piece_at(sq);
        sample.board_sq[site] = static_cast<uint8_t>(TensorMPS::piece_to_local_index(p));
    }

    return sample;
}

// =============================================================================
// Forward-Backward Pass (Single Sample)
// =============================================================================

float TensorTrainer::train_step(const TrainingSample& sample,
                               std::vector<float>& grad_stm,
                               std::vector<float>& grad_bulk,
                               std::vector<float>& grad_right) {
    int D = model_.bond_dim();
    float scale = model_.scale();

    // Environment caches
    // L[site] is the left environment vector at site index site (0..63)
    // L[0] = T_stm[side_to_move]
    // L[k+1] = L[k] x T_bulk[k][piece_k]
    std::vector<float> L(64 * D);
    std::vector<float> R(64 * D);

    // 1. Forward Pass — Left Environments
    const float* stm_vec = model_.stm_data() + sample.side_to_move * D;
    std::memcpy(&L[0], stm_vec, D * sizeof(float));

    for (int site = 0; site < TensorMPS::NUM_BULK; site++) {
        int piece_idx = sample.board_sq[site];
        const float* mat = model_.bulk_data() + (site * TensorMPS::LOCAL_DIM_SQ + piece_idx) * D * D;

        float* l_curr = &L[site * D];
        float* l_next = &L[(site + 1) * D];

        for (int j = 0; j < D; j++) {
            float sum = 0.0f;
            for (int k = 0; k < D; k++) {
                sum += l_curr[k] * mat[k * D + j];
            }
            l_next[j] = sum;
        }
    }

    // 2. Backward Pass — Right Environments
    int last_piece = sample.board_sq[63];
    const float* right_vec = model_.right_data() + last_piece * D;
    std::memcpy(&R[63 * D], right_vec, D * sizeof(float));

    for (int site = TensorMPS::NUM_BULK - 1; site >= 0; site--) {
        int piece_idx = sample.board_sq[site];
        const float* mat = model_.bulk_data() + (site * TensorMPS::LOCAL_DIM_SQ + piece_idx) * D * D;

        float* r_next = &R[(site + 1) * D];
        float* r_curr = &R[site * D];

        for (int i = 0; i < D; i++) {
            float sum = 0.0f;
            for (int j = 0; j < D; j++) {
                sum += mat[i * D + j] * r_next[j];
            }
            r_curr[i] = sum;
        }
    }

    // 3. Compute scalar output
    float raw_pred = 0.0f;
    float* l_final = &L[63 * D];
    for (int j = 0; j < D; j++) {
        raw_pred += l_final[j] * right_vec[j];
    }

    float pred_cp = raw_pred * scale;
    pred_cp = std::max(-30000.0f, std::min(30000.0f, pred_cp));

    float diff = pred_cp - sample.target_eval;
    float delta = diff * scale; // dLoss/d(raw_pred)

    // Clip delta for numerical stability
    if (config_.grad_clip > 0.0f) {
        delta = std::max(-config_.grad_clip * 1000.0f, std::min(config_.grad_clip * 1000.0f, delta));
    }

    // 4. Gradient Accumulation
    // Side-to-move boundary gradient
    float* g_stm = grad_stm.data() + sample.side_to_move * D;
    float* r_first = &R[0];
    for (int j = 0; j < D; j++) {
        g_stm[j] += delta * r_first[j];
    }

    // Bulk site gradients
    for (int site = 0; site < TensorMPS::NUM_BULK; site++) {
        int piece_idx = sample.board_sq[site];
        float* g_mat = grad_bulk.data() + (site * TensorMPS::LOCAL_DIM_SQ + piece_idx) * D * D;
        float* l_env = &L[site * D];
        float* r_env = &R[(site + 1) * D];

        for (int i = 0; i < D; i++) {
            for (int j = 0; j < D; j++) {
                g_mat[i * D + j] += delta * l_env[i] * r_env[j];
            }
        }
    }

    // Right boundary gradient
    float* g_right = grad_right.data() + last_piece * D;
    for (int j = 0; j < D; j++) {
        g_right[j] += delta * l_final[j];
    }

    return 0.5f * diff * diff; // Squared error loss
}

// =============================================================================
// Adam Weight Update
// =============================================================================

void TensorTrainer::update_weights_adam(const std::vector<float>& grad_stm,
                                       const std::vector<float>& grad_bulk,
                                       const std::vector<float>& grad_right,
                                       int batch_count) {
    adam_t_++;

    float lr = config_.learning_rate;
    float b1 = 0.9f;
    float b2 = 0.999f;
    float eps = 1e-8f;
    float decay = config_.weight_decay;

    float correction1 = 1.0f - std::pow(b1, adam_t_);
    float correction2 = 1.0f - std::pow(b2, adam_t_);

    auto step_array = [&](float* param, const float* grad, float* m, float* v, size_t size) {
        float inv_n = 1.0f / static_cast<float>(batch_count);
        for (size_t i = 0; i < size; i++) {
            float g = grad[i] * inv_n;
            m[i] = b1 * m[i] + (1.0f - b1) * g;
            v[i] = b2 * v[i] + (1.0f - b2) * g * g;

            float m_hat = m[i] / correction1;
            float v_hat = v[i] / correction2;

            param[i] -= lr * (m_hat / (std::sqrt(v_hat) + eps) + decay * param[i]);
        }
    };

    step_array(model_.stm_data(), grad_stm.data(), m_stm_.data(), v_stm_.data(), grad_stm.size());
    step_array(model_.bulk_data(), grad_bulk.data(), m_bulk_.data(), v_bulk_.data(), grad_bulk.size());
    step_array(model_.right_data(), grad_right.data(), m_right_.data(), v_right_.data(), grad_right.size());
}

// =============================================================================
// Dataset Training Loop
// =============================================================================

void TensorTrainer::train(const std::vector<TrainingSample>& dataset, float validation_split) {
    if (dataset.empty()) return;

    // Shuffle indices
    std::vector<size_t> indices(dataset.size());
    for (size_t i = 0; i < dataset.size(); i++) indices[i] = i;

    std::mt19937 rng(config_.seed);
    std::shuffle(indices.begin(), indices.end(), rng);

    size_t val_size = static_cast<size_t>(dataset.size() * validation_split);
    size_t train_size = dataset.size() - val_size;

    std::cout << "[TensorTrainer] Starting training on " << train_size
              << " train samples, " << val_size << " validation samples." << std::endl;
    std::cout << "[TensorTrainer] D=" << model_.bond_dim()
              << ", lr=" << config_.learning_rate
              << ", batch_size=" << config_.batch_size << std::endl;

    int D = model_.bond_dim();
    std::vector<float> grad_stm(TensorMPS::LOCAL_DIM_STM * D, 0.0f);
    std::vector<float> grad_bulk(TensorMPS::NUM_BULK * TensorMPS::LOCAL_DIM_SQ * D * D, 0.0f);
    std::vector<float> grad_right(TensorMPS::LOCAL_DIM_SQ * D, 0.0f);

    for (int epoch = 1; epoch <= config_.epochs; epoch++) {
        std::shuffle(indices.begin(), indices.begin() + train_size, rng);

        double train_loss = 0.0;
        int batch_count = 0;

        std::fill(grad_stm.begin(), grad_stm.end(), 0.0f);
        std::fill(grad_bulk.begin(), grad_bulk.end(), 0.0f);
        std::fill(grad_right.begin(), grad_right.end(), 0.0f);

        for (size_t i = 0; i < train_size; i++) {
            const auto& sample = dataset[indices[i]];
            train_loss += train_step(sample, grad_stm, grad_bulk, grad_right);
            batch_count++;

            if (batch_count >= config_.batch_size || i == train_size - 1) {
                update_weights_adam(grad_stm, grad_bulk, grad_right, batch_count);
                std::fill(grad_stm.begin(), grad_stm.end(), 0.0f);
                std::fill(grad_bulk.begin(), grad_bulk.end(), 0.0f);
                std::fill(grad_right.begin(), grad_right.end(), 0.0f);
                batch_count = 0;
            }
        }

        train_loss /= static_cast<double>(train_size);
        double rmse_train = std::sqrt(2.0 * train_loss);

        // Validation pass
        double val_loss = 0.0;
        if (val_size > 0) {
            std::vector<float> dummy_stm(grad_stm.size());
            std::vector<float> dummy_bulk(grad_bulk.size());
            std::vector<float> dummy_right(grad_right.size());

            for (size_t i = train_size; i < dataset.size(); i++) {
                val_loss += train_step(dataset[indices[i]], dummy_stm, dummy_bulk, dummy_right);
            }
            val_loss /= static_cast<double>(val_size);
        }
        double rmse_val = std::sqrt(2.0 * val_loss);

        std::cout << "[Epoch " << epoch << "/" << config_.epochs << "] "
                  << "Train RMSE: " << rmse_train << " cp | "
                  << "Val RMSE: " << rmse_val << " cp" << std::endl;
    }
}

} // namespace heavensgate
