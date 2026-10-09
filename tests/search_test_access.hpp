#pragma once
#include "../src/core/fen.hpp"
#include "../src/search/search.hpp"
#include <array>

namespace heavensgate::test {
// Leaf contracts must not be hidden by root shortcuts or endgame recognizers.
struct SearchEngineTestAccess {
    static int qsearch(SearchEngine& engine, Board& board, int alpha, int beta, int ply) {
        engine.prepare_search(0, 0);
        return engine.quiescence_search(board, alpha, beta, ply);
    }
    static int negamax(SearchEngine& engine, Board& board, int depth, int ply,
                       int alpha = -ScoreInfinity, int beta = ScoreInfinity, bool use_tt = true) {
        engine.prepare_search(0, 0);
        return engine.negamax_alphabeta(board, depth, ply, alpha, beta, true, use_tt);
    }
};

struct PositionSnapshot {
    std::string fen;
    uint64_t key;
    size_t history_ply;
    std::array<int, 9> incremental;

    explicit PositionSnapshot(const Board& b)
        : fen(FEN::to_string(b)), key(b.zobrist_key()), history_ply(b.history_ply()),
          incremental{b.mg_material(Color::White), b.mg_material(Color::Black),
                      b.eg_material(Color::White), b.eg_material(Color::Black),
                      b.mg_pst(Color::White), b.mg_pst(Color::Black),
                      b.eg_pst(Color::White), b.eg_pst(Color::Black), b.game_phase()} {}
    bool unchanged(const Board& b) const {
        const PositionSnapshot after(b);
        return fen == after.fen && key == after.key && history_ply == after.history_ply &&
               incremental == after.incremental;
    }
};
} // namespace heavensgate::test
