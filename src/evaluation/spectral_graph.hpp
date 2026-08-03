#pragma once

#include "../core/types.hpp"
#include <array>
#include <vector>
#include <cstdint>

namespace heavensgate {

class Board; // Forward declaration

// =============================================================================
// SpectralGraph — Dynamic Piece Attack Graph & Graph Laplacian Eigensolver
// =============================================================================
//
// Represents the board position as a dynamic weighted graph G = (V, E):
//   - Nodes V: Active pieces on the board (up to N=32 nodes)
//   - Edges E: Piece attack/defense interactions, ray protection, central control
//   - Laplacian L = D - A
//
// Eigensolver extracts scale-invariant topological features:
//   1. λ₂ (Fiedler Value): Algebraic Connectivity / Piece Coordination
//   2. λ_N - λ₂ (Spectral Gap): Cheeger Constant Bound / Control Bottleneck
//   3. Tr(L): Total Board Pressure / Activity Energy
//
// =============================================================================

struct SpectralFeatures {
    float fiedler_val;         // Global λ₂: Algebraic Connectivity (Piece Coordination)
    float fiedler_us;          // λ₂ for side to move pieces
    float fiedler_them;        // λ₂ for opponent pieces
    float spectral_gap;        // λ_N - λ₂: Control Bottleneck (Cheeger Constant)
    float laplacian_trace;     // Tr(L): Total Activity & Pressure Energy
    float cohesion_us;         // Subgraph connectivity for side to move
    float cohesion_them;       // Subgraph connectivity for opponent
    float king_pressure_us;    // Attack Laplacian energy targeting opponent King
    float king_pressure_them;  // Attack Laplacian energy targeting us King
    float battery_energy_us;   // Ray alignment energy (Rook/Queen/Bishop batteries)
    float battery_energy_them; // Ray alignment energy for opponent
    float pawn_cohesion_us;    // Pawn chain Laplacian cohesion
    float pawn_cohesion_them;  // Pawn chain Laplacian cohesion for opponent
};

class SpectralGraph {
public:
    static constexpr int MAX_NODES = 32;

    // Constructs dynamic piece attack graph and computes Laplacian spectrum
    static SpectralFeatures compute_spectrum(const Board& board);

private:
    // Helper: Compute attack/defense weight between two pieces
    static float compute_edge_weight(Piece p1, Square sq1, Piece p2, Square sq2);
};

} // namespace heavensgate
