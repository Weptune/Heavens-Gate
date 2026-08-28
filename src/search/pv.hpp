#pragma once

#include "../core/types.hpp"
#include <array>
#include <string>
#include <vector>
#include <algorithm>

namespace heavensgate {

constexpr int MaxSearchDepth = 64;

struct PrincipalVariation {
    std::array<Move, MaxSearchDepth> moves{};
    int length{0};

    void clear() noexcept { length = 0; }

    std::vector<Move> to_vector() const {
        return std::vector<Move>(moves.begin(), moves.begin() + length);
    }

    std::string to_string() const {
        std::string s;
        for (int i = 0; i < length; ++i) {
            if (i > 0) s += " ";
            s += move_to_uci(moves[i]);
        }
        return s;
    }
};

// Triangular PV Table for tracking principal variation across recursive depth levels
class PVTable {
private:
    std::array<std::array<Move, MaxSearchDepth>, MaxSearchDepth> table_{};
    std::array<int, MaxSearchDepth> length_{};

public:
    void clear() noexcept {
        length_.fill(0);
    }

    void init_ply(int ply) noexcept {
        if (ply < MaxSearchDepth) {
            length_[ply] = ply;
        }
    }

    void update(int ply, Move move) noexcept {
        if (ply >= MaxSearchDepth) return;
        table_[ply][ply] = move;
        int next_len = (ply + 1 < MaxSearchDepth) ? length_[ply + 1] : (ply + 1);
        if (next_len < ply + 1) next_len = ply + 1;
        for (int next = ply + 1; next < next_len && next < MaxSearchDepth; ++next) {
            table_[ply][next] = table_[ply + 1][next];
        }
        length_[ply] = next_len;
    }

    void set_move(int ply, Move move) noexcept {
        if (ply < MaxSearchDepth) {
            table_[ply][ply] = move;
            length_[ply] = ply + 1;
        }
    }

    PrincipalVariation get_pv(int depth) const {
        PrincipalVariation pv;
        pv.length = std::min(depth, length_[0]);
        for (int i = 0; i < pv.length; ++i) {
            pv.moves[i] = table_[0][i];
        }
        return pv;
    }
};

} // namespace heavensgate
