#include "spectral_graph.hpp"
#include "../board/board.hpp"
#include "../movegen/attack_masks.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <iostream>

namespace heavensgate {

float SpectralGraph::compute_edge_weight(Piece p1, Square sq1, Piece p2, Square sq2) {
    if (p1 == Piece::None || p2 == Piece::None || sq1 == sq2) return 0.0f;

    Color c1 = color_of(p1);
    Color c2 = color_of(p2);
    PieceType pt1 = piece_type_of(p1);
    PieceType pt2 = piece_type_of(p2);

    int r1 = static_cast<int>(rank_of(sq1)), f1 = static_cast<int>(file_of(sq1));
    int r2 = static_cast<int>(rank_of(sq2)), f2 = static_cast<int>(file_of(sq2));
    int dist = std::max(std::abs(r1 - r2), std::abs(f1 - f2));

    float weight = 0.0f;

    // Direct interaction weight
    if (c1 == c2) {
        // Defense / Support
        weight = 1.0f / static_cast<float>(dist);
    } else {
        // Attack / Pressure
        float target_val = 1.0f;
        switch (pt2) {
            case PieceType::Pawn:   target_val = 1.0f; break;
            case PieceType::Knight: target_val = 3.0f; break;
            case PieceType::Bishop: target_val = 3.2f; break;
            case PieceType::Rook:   target_val = 5.0f; break;
            case PieceType::Queen:  target_val = 9.0f; break;
            case PieceType::King:   target_val = 12.0f; break;
            default: break;
        }
        weight = (2.0f * target_val) / static_cast<float>(dist);
    }

    // Battery bonus for aligned sliding pieces (Rooks/Queens/Bishops)
    if (c1 == c2 && (pt1 == PieceType::Rook || pt1 == PieceType::Queen || pt1 == PieceType::Bishop)) {
        if (f1 == f2 || r1 == r2 || std::abs(r1 - r2) == std::abs(f1 - f2)) {
            weight += 1.5f;
        }
    }

    return weight;
}

// =============================================================================
// Helper: Compute Fiedler value (λ₂) for a subgraph of same-color pieces
// =============================================================================
static float compute_side_fiedler(
    const std::vector<std::pair<Piece, Square>>& side_nodes
) {
    int N = static_cast<int>(side_nodes.size());
    if (N < 2) return 0.0f;

    // Build adjacency and degree for this side's subgraph
    std::vector<float> A(N * N, 0.0f);
    std::vector<float> deg(N, 0.0f);

    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            float w = SpectralGraph::compute_edge_weight(
                side_nodes[i].first, side_nodes[i].second,
                side_nodes[j].first, side_nodes[j].second
            );
            A[i * N + j] = w;
            A[j * N + i] = w;
            deg[i] += w;
            deg[j] += w;
        }
    }

    // Build Laplacian L = D - A
    std::vector<float> L(N * N, 0.0f);
    for (int i = 0; i < N; i++) {
        L[i * N + i] = deg[i];
        for (int j = 0; j < N; j++) {
            if (i != j) L[i * N + j] = -A[i * N + j];
        }
    }

    // Power Iteration for max eigenvalue λ_N
    std::vector<float> v(N, 1.0f / std::sqrt(static_cast<float>(N)));
    std::vector<float> v_next(N, 0.0f);
    float max_lambda = 0.0f;

    for (int iter = 0; iter < 12; iter++) {
        float norm_sq = 0.0f;
        for (int i = 0; i < N; i++) {
            float sum = 0.0f;
            for (int j = 0; j < N; j++) sum += L[i * N + j] * v[j];
            v_next[i] = sum;
            norm_sq += sum * sum;
        }
        float norm = std::sqrt(norm_sq);
        max_lambda = norm;
        if (norm > 1e-6f) {
            for (int i = 0; i < N; i++) v[i] = v_next[i] / norm;
        }
    }

    if (max_lambda < 1e-6f) return 0.0f;

    // Inverse Power Iteration for Fiedler value λ₂
    // Iterate on M = (max_lambda * I - L), deflating the constant eigenvector
    std::vector<float> u(N);
    for (int i = 0; i < N; i++) u[i] = (i % 2 == 0) ? 1.0f : -1.0f;

    float fiedler = 0.0f;
    for (int iter = 0; iter < 15; iter++) {
        // Project orthogonal to constant eigenvector
        float mean = 0.0f;
        for (int i = 0; i < N; i++) mean += u[i];
        mean /= static_cast<float>(N);
        for (int i = 0; i < N; i++) u[i] -= mean;

        // Multiply by M = (max_lambda * I - L)
        float norm_sq = 0.0f;
        for (int i = 0; i < N; i++) {
            float sum = max_lambda * u[i];
            for (int j = 0; j < N; j++) sum -= L[i * N + j] * u[j];
            v_next[i] = sum;
            norm_sq += sum * sum;
        }
        float norm = std::sqrt(norm_sq);
        fiedler = max_lambda - norm;
        if (norm > 1e-6f) {
            for (int i = 0; i < N; i++) u[i] = v_next[i] / norm;
        }
    }

    return std::max(0.0f, fiedler);
}

SpectralFeatures SpectralGraph::compute_spectrum(const Board& board) {
    SpectralFeatures feat{};

    // Collect active pieces
    struct Node {
        Piece piece;
        Square sq;
    };
    std::vector<Node> nodes;
    nodes.reserve(MAX_NODES);

    for (int s = 0; s < 64; s++) {
        Square sq = static_cast<Square>(s);
        Piece p = board.piece_at(sq);
        if (p != Piece::None) {
            nodes.push_back({p, sq});
        }
    }

    int N = static_cast<int>(nodes.size());
    if (N < 2) return feat;

    // =========================================================================
    // Global Laplacian for spectral_gap and laplacian_trace
    // =========================================================================
    std::vector<float> A(N * N, 0.0f);
    std::vector<float> deg(N, 0.0f);

    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            float w = compute_edge_weight(nodes[i].piece, nodes[i].sq, nodes[j].piece, nodes[j].sq);
            A[i * N + j] = w;
            A[j * N + i] = w;
            deg[i] += w;
            deg[j] += w;
        }
    }

    // Compute Laplacian Matrix L = D - A
    std::vector<float> L(N * N, 0.0f);
    float trace = 0.0f;

    for (int i = 0; i < N; i++) {
        L[i * N + i] = deg[i];
        trace += deg[i];
        for (int j = 0; j < N; j++) {
            if (i != j) {
                L[i * N + j] = -A[i * N + j];
            }
        }
    }

    feat.laplacian_trace = trace;

    // Power Iteration for Max Eigenvalue λ_N
    std::vector<float> v(N, 1.0f / std::sqrt(static_cast<float>(N)));
    std::vector<float> v_next(N, 0.0f);
    float max_lambda = 0.0f;

    for (int iter = 0; iter < 15; iter++) {
        float norm_sq = 0.0f;
        for (int i = 0; i < N; i++) {
            float sum = 0.0f;
            for (int j = 0; j < N; j++) {
                sum += L[i * N + j] * v[j];
            }
            v_next[i] = sum;
            norm_sq += sum * sum;
        }

        float norm = std::sqrt(norm_sq);
        max_lambda = norm;
        if (norm > 1e-6f) {
            for (int i = 0; i < N; i++) v[i] = v_next[i] / norm;
        }
    }

    // Global Fiedler value for spectral_gap computation
    std::vector<float> u(N);
    for (int i = 0; i < N; i++) u[i] = (i % 2 == 0) ? 1.0f : -1.0f;

    float global_fiedler = 0.0f;
    for (int iter = 0; iter < 20; iter++) {
        float mean = 0.0f;
        for (int i = 0; i < N; i++) mean += u[i];
        mean /= static_cast<float>(N);
        for (int i = 0; i < N; i++) u[i] -= mean;

        float norm_sq = 0.0f;
        for (int i = 0; i < N; i++) {
            float sum = max_lambda * u[i];
            for (int j = 0; j < N; j++) {
                sum -= L[i * N + j] * u[j];
            }
            v_next[i] = sum;
            norm_sq += sum * sum;
        }

        float norm = std::sqrt(norm_sq);
        global_fiedler = max_lambda - norm;
        if (norm > 1e-6f) {
            for (int i = 0; i < N; i++) u[i] = v_next[i] / norm;
        }
    }

    feat.fiedler_val = std::max(0.0f, global_fiedler);
    feat.spectral_gap = std::max(0.0f, max_lambda - feat.fiedler_val);

    // =========================================================================
    // Per-Side Fiedler Values (separate subgraph Laplacians)
    // =========================================================================
    Color us = board.side_to_move();
    Color them = ~us;

    std::vector<std::pair<Piece, Square>> us_nodes, them_nodes;
    us_nodes.reserve(16);
    them_nodes.reserve(16);

    for (int i = 0; i < N; i++) {
        Color c = color_of(nodes[i].piece);
        if (c == us) {
            us_nodes.push_back({nodes[i].piece, nodes[i].sq});
        } else {
            them_nodes.push_back({nodes[i].piece, nodes[i].sq});
        }
    }

    feat.fiedler_us = compute_side_fiedler(us_nodes);
    feat.fiedler_them = compute_side_fiedler(them_nodes);

    // =========================================================================
    // Per-Side Features: Cohesion, King Pressure, Battery, Pawn Structure,
    //                    Mobility, Center Control, King Shield, Game Phase
    // =========================================================================
    Piece us_king = (us == Color::White) ? Piece::WhiteKing : Piece::BlackKing;
    Piece them_king = (them == Color::White) ? Piece::WhiteKing : Piece::BlackKing;
    Square us_king_sq = board.pieces(us_king) ? lsb(board.pieces(us_king)) : Square::None;
    Square them_king_sq = board.pieces(them_king) ? lsb(board.pieces(them_king)) : Square::None;

    float us_deg_sum = 0.0f, them_deg_sum = 0.0f;
    float us_king_press = 0.0f, them_king_press = 0.0f;
    float us_battery = 0.0f, them_battery = 0.0f;
    float us_pawn_coh = 0.0f, them_pawn_coh = 0.0f;
    float us_mobility = 0.0f, them_mobility = 0.0f;
    float us_center = 0.0f, them_center = 0.0f;
    float us_king_shield = 0.0f, them_king_shield = 0.0f;
    float phase_material = 0.0f;

    // Center squares: e4=28, d4=27, e5=36, d5=35
    static constexpr int center_squares[4] = {27, 28, 35, 36};

    for (int i = 0; i < N; i++) {
        Color c = color_of(nodes[i].piece);
        PieceType pt = piece_type_of(nodes[i].piece);
        Square sq = nodes[i].sq;

        if (c == us) us_deg_sum += deg[i];
        else them_deg_sum += deg[i];

        // King Shield Laplacian energy: friendly pawns/pieces + open file penalties
        if (c == us && us_king_sq != Square::None) {
            int kdist = std::max(std::abs(static_cast<int>(rank_of(sq)) - static_cast<int>(rank_of(us_king_sq))),
                                 std::abs(static_cast<int>(file_of(sq)) - static_cast<int>(file_of(us_king_sq))));
            if (kdist <= 2) {
                float shield_val = (pt == PieceType::Pawn) ? (kdist == 1 ? 3.0f : 1.5f) : 0.8f;
                us_king_shield += shield_val;
            }
        } else if (c == them && them_king_sq != Square::None) {
            int kdist = std::max(std::abs(static_cast<int>(rank_of(sq)) - static_cast<int>(rank_of(them_king_sq))),
                                 std::abs(static_cast<int>(file_of(sq)) - static_cast<int>(file_of(them_king_sq))));
            if (kdist <= 2) {
                float shield_val = (pt == PieceType::Pawn) ? (kdist == 1 ? 3.0f : 1.5f) : 0.8f;
                them_king_shield += shield_val;
            }
        }

        // Mobility: count edges (attack/defense interactions) per side
        float piece_edges = 0.0f;
        for (int j = 0; j < N; j++) {
            if (i != j) piece_edges += (A[i * N + j] > 0.0f) ? 1.0f : 0.0f;
        }
        if (c == us) us_mobility += piece_edges;
        else them_mobility += piece_edges;

        // Center control: Chebyshev distance to center squares
        int sq_rank = static_cast<int>(rank_of(sq));
        int sq_file = static_cast<int>(file_of(sq));
        for (int cs = 0; cs < 4; cs++) {
            int cr = center_squares[cs] / 8;
            int cf = center_squares[cs] % 8;
            int cdist = std::max(std::abs(sq_rank - cr), std::abs(sq_file - cf));
            if (cdist <= 1) {
                if (c == us) us_center += 1.0f;
                else them_center += 1.0f;
            }
        }

        // Game phase material (exclude kings and pawns)
        if (pt != PieceType::King && pt != PieceType::Pawn) {
            float pval = 0.0f;
            switch (pt) {
                case PieceType::Knight: pval = 320.0f; break;
                case PieceType::Bishop: pval = 330.0f; break;
                case PieceType::Rook:   pval = 500.0f; break;
                case PieceType::Queen:  pval = 900.0f; break;
                default: break;
            }
            phase_material += pval;
        }

        // King pressure energy targeting opponent King
        if (c == us && them_king_sq != Square::None) {
            int dist = std::max(std::abs(static_cast<int>(rank_of(sq)) - static_cast<int>(rank_of(them_king_sq))),
                                std::abs(static_cast<int>(file_of(sq)) - static_cast<int>(file_of(them_king_sq))));
            if (dist <= 3) us_king_press += 3.0f / static_cast<float>(dist);
        } else if (c == them && us_king_sq != Square::None) {
            int dist = std::max(std::abs(static_cast<int>(rank_of(sq)) - static_cast<int>(rank_of(us_king_sq))),
                                std::abs(static_cast<int>(file_of(sq)) - static_cast<int>(file_of(us_king_sq))));
            if (dist <= 3) them_king_press += 3.0f / static_cast<float>(dist);
        }

        // Battery ray alignment energy
        if (pt == PieceType::Rook || pt == PieceType::Queen || pt == PieceType::Bishop) {
            for (int j = i + 1; j < N; j++) {
                if (color_of(nodes[j].piece) == c) {
                    PieceType pt2 = piece_type_of(nodes[j].piece);
                    if (pt2 == PieceType::Rook || pt2 == PieceType::Queen || pt2 == PieceType::Bishop) {
                        Square sq2 = nodes[j].sq;
                        if (file_of(sq) == file_of(sq2) || rank_of(sq) == rank_of(sq2) ||
                            std::abs(static_cast<int>(rank_of(sq)) - static_cast<int>(rank_of(sq2))) ==
                            std::abs(static_cast<int>(file_of(sq)) - static_cast<int>(file_of(sq2)))) {
                            if (c == us) us_battery += 2.0f;
                            else them_battery += 2.0f;
                        }
                    }
                }
            }
        }

        // Pawn chain cohesion
        if (pt == PieceType::Pawn) {
            for (int j = i + 1; j < N; j++) {
                if (piece_type_of(nodes[j].piece) == PieceType::Pawn && color_of(nodes[j].piece) == c) {
                    Square sq2 = nodes[j].sq;
                    int f_diff = std::abs(static_cast<int>(file_of(sq)) - static_cast<int>(file_of(sq2)));
                    int r_diff = std::abs(static_cast<int>(rank_of(sq)) - static_cast<int>(rank_of(sq2)));
                    if (f_diff == 1 && r_diff == 1) {
                        if (c == us) us_pawn_coh += 2.5f;
                        else them_pawn_coh += 2.5f;
                    }
                }
            }
        }
    }

    feat.cohesion_us = us_deg_sum;
    feat.cohesion_them = them_deg_sum;
    feat.king_pressure_us = us_king_press;
    feat.king_pressure_them = them_king_press;
    feat.battery_energy_us = us_battery;
    feat.battery_energy_them = them_battery;
    feat.pawn_cohesion_us = us_pawn_coh;
    feat.pawn_cohesion_them = them_pawn_coh;
    feat.mobility_us = us_mobility;
    feat.mobility_them = them_mobility;
    feat.center_control_us = us_center;
    feat.center_control_them = them_center;
    feat.king_shield_us = us_king_shield;
    feat.king_shield_them = them_king_shield;

    // Game phase: 1.0 = all pieces present, 0.0 = endgame (pawns + kings only)
    feat.game_phase = std::min(1.0f, phase_material / 8200.0f);

    return feat;
}

} // namespace heavensgate
