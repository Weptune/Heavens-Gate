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
    float mobility_us;         // Attack-degree mobility for side to move
    float mobility_them;       // Attack-degree mobility for opponent
    float center_control_us;   // Pieces near center (e4/d4/e5/d5) for side to move
    float center_control_them; // Pieces near center for opponent
    float king_shield_us;      // Defending pawn/piece Laplacian energy around our King
    float king_shield_them;    // Defending pawn/piece Laplacian energy around opponent King
    float game_phase;          // Normalized game phase (1.0=full pieces, 0.0=endgame)

    // Phase 3: 4-Zone Localized Spatial Fiedler Subgraph Invariants
    float fiedler_ks_us;       // Kingside zone (files f,g,h) Fiedler λ₂ for us
    float fiedler_ks_them;     // Kingside zone Fiedler λ₂ for opponent
    float fiedler_qs_us;       // Queenside zone (files a,b,c) Fiedler λ₂ for us
    float fiedler_qs_them;     // Queenside zone Fiedler λ₂ for opponent
    float fiedler_ctr_us;      // Center zone (files d,e) Fiedler λ₂ for us
    float fiedler_ctr_them;    // Center zone Fiedler λ₂ for opponent
    float fiedler_br_us;       // Back-rank zone (ranks 1-2 us / 7-8 them) Fiedler λ₂ for us
    float fiedler_br_them;     // Back-rank zone Fiedler λ₂ for opponent

    // Phase 5: Chebyshev Spectral Graph Filter Features (2-Hop Graph Convolutions T2(L))
    float chebyshev_t2_us;       // 2-Hop indirect battery/support graph convolution for us
    float chebyshev_t2_them;     // 2-Hop indirect battery/support graph convolution for opponent
    float chebyshev_king_threat; // 2-Hop indirect graph attack paths targeting opponent King
};

class SpectralGraph {
public:
    static constexpr int MAX_NODES = 32;

    // Constructs dynamic piece attack graph and computes Laplacian spectrum
    static SpectralFeatures compute_spectrum(const Board& board);

    // Helper: Compute attack/defense weight between two pieces (public for per-side Fiedler)
    static float compute_edge_weight(Piece p1, Square sq1, Piece p2, Square sq2);
};

} // namespace heavensgate
