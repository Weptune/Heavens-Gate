#pragma once

#include "../core/types.hpp"
#include "../board/board.hpp"

namespace heavensgate {

struct ScorePair {
    int mg{0};
    int eg{0};

    constexpr ScorePair() = default;
    constexpr ScorePair(int m, int e) : mg(m), eg(e) {}

    constexpr ScorePair operator+(const ScorePair& o) const noexcept {
        return ScorePair(mg + o.mg, eg + o.eg);
    }
    constexpr ScorePair operator-(const ScorePair& o) const noexcept {
        return ScorePair(mg - o.mg, eg - o.eg);
    }
    constexpr ScorePair& operator+=(const ScorePair& o) noexcept {
        mg += o.mg;
        eg += o.eg;
        return *this;
    }
    constexpr ScorePair& operator-=(const ScorePair& o) noexcept {
        mg -= o.mg;
        eg -= o.eg;
        return *this;
    }
};

class EvalFeatures {
public:
    static std::array<Bitboard, 64> PassedPawnMask[2];
    static std::array<Bitboard, 8>  IsolatedPawnMask;
    static std::array<int, 32>      KingDangerTable;

    static void init();

    static ScorePair evaluate_pawn_structure(const Board& board, Color side);
    static ScorePair evaluate_passed_pawns(const Board& board, Color side);
    static ScorePair evaluate_king_safety(const Board& board, Color side);
    static ScorePair evaluate_piece_activity(const Board& board, Color side);
    static ScorePair evaluate_mobility(const Board& board, Color side);
};

} // namespace heavensgate
