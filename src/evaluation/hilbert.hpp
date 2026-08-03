#pragma once

#include <array>
#include <cstdint>

namespace heavensgate {

// Hilbert curve mapping for an 8x8 board.
// hilbert_order[chain_pos] = board_square_index
// where board_square_index = rank * 8 + file
//
// This ordering ensures that geometrically adjacent squares on the 8x8 board
// remain close in the 1D chain, maximizing the locality-preserving property
// critical for efficient MPS correlation capture.

class HilbertCurve {
public:
    // Convert (x, y) in an NxN grid (N must be power of 2) to Hilbert distance
    static int xy_to_d(int n, int x, int y) {
        int d = 0;
        for (int s = n / 2; s > 0; s /= 2) {
            int rx = (x & s) > 0 ? 1 : 0;
            int ry = (y & s) > 0 ? 1 : 0;
            d += s * s * ((3 * rx) ^ ry);
            // Rotate quadrant
            if (ry == 0) {
                if (rx == 1) {
                    x = s - 1 - x;
                    y = s - 1 - y;
                }
                int tmp = x;
                x = y;
                y = tmp;
            }
        }
        return d;
    }

    // Convert Hilbert distance to (x, y) in an NxN grid
    static void d_to_xy(int n, int d, int& x, int& y) {
        x = 0;
        y = 0;
        for (int s = 1; s < n; s *= 2) {
            int rx = 1 & (d / 2);
            int ry = 1 & (d ^ rx);
            // Rotate
            if (ry == 0) {
                if (rx == 1) {
                    x = s - 1 - x;
                    y = s - 1 - y;
                }
                int tmp = x;
                x = y;
                y = tmp;
            }
            x += s * rx;
            y += s * ry;
            d /= 4;
        }
    }

    // Get the precomputed Hilbert ordering for the 8x8 chess board.
    // Returns hilbert_order[chain_position] = square_index (rank*8 + file)
    static const std::array<int, 64>& order() {
        static std::array<int, 64> table = compute_order();
        return table;
    }

    // Get the inverse mapping: square_index -> chain_position
    static const std::array<int, 64>& inverse_order() {
        static std::array<int, 64> table = compute_inverse_order();
        return table;
    }

private:
    static std::array<int, 64> compute_order() {
        std::array<int, 64> result{};
        for (int d = 0; d < 64; d++) {
            int file, rank;
            d_to_xy(8, d, file, rank);
            result[d] = rank * 8 + file; // board square index
        }
        return result;
    }

    static std::array<int, 64> compute_inverse_order() {
        std::array<int, 64> result{};
        const auto& fwd = order();
        for (int d = 0; d < 64; d++) {
            result[fwd[d]] = d;
        }
        return result;
    }
};

} // namespace heavensgate
