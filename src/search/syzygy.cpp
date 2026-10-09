#include "syzygy.hpp"
#include "../movegen/movegen.hpp"

namespace heavensgate {
thread_local std::array<TBCacheEntry, SyzygyTablebase::CACHE_SIZE> SyzygyTablebase::cache_{};
thread_local uint64_t SyzygyTablebase::local_epoch_ = UINT64_MAX;

void SyzygyTablebase::init(const std::string& /*tb_path*/) {
    // No tablebase files are read by this built-in recognizer.
    cache_epoch_.fetch_add(1, std::memory_order_relaxed);
    set_enabled(true);
}

int SyzygyTablebase::wdl_to_score(WDLScore wdl, int ply) const {
    switch (wdl) {
        case WDLScore::Win: return ScoreTBWin - ply;
        case WDLScore::Loss: return -ScoreTBWin + ply;
        case WDLScore::Draw:
        case WDLScore::CursedWin:
        case WDLScore::BlessedLoss: return ScoreDraw;
        default: return NO_SCORE;
    }
}

int SyzygyTablebase::probe_wdl(const Board& board, int ply) {
    if (!is_enabled() || popcount(board.occupied()) > MAX_TB_PIECES ||
        board.king_square(Color::White) == Square::None ||
        board.king_square(Color::Black) == Square::None) return NO_SCORE;

    // Terminal legality takes precedence over both the rule-50 clock and cache.
    MoveList legal;
    MoveGenerator::generate_legal_moves(board, legal);
    if (legal.empty())
        return MoveGenerator::in_check(board, board.side_to_move()) ? -ScoreMate + ply : ScoreDraw;
    if (board.halfmove_clock() >= 100 || board.is_repetition(3)) return ScoreDraw;

    const auto epoch = cache_epoch_.load(std::memory_order_relaxed);
    if (local_epoch_ != epoch) { cache_.fill(TBCacheEntry{}); local_epoch_ = epoch; }
    auto& entry = cache_[board.zobrist_key() & (CACHE_SIZE - 1)];
    if (entry.key == board.zobrist_key() && entry.halfmove_clock == board.halfmove_clock() &&
        entry.wdl != WDLScore::Unknown) return wdl_to_score(entry.wdl, ply);
    const auto wdl = evaluate_endgame_wdl(board);
    if (wdl != WDLScore::Unknown) entry = {board.zobrist_key(), wdl, board.halfmove_clock()};
    return wdl_to_score(wdl, ply);
}

// A legal capture of the sole mating piece by the bare king proves a draw:
// the defender can reach KK immediately and the bare king can never win.
static WDLScore bare_king_capture_draw(const Board& board, Color strong, PieceType type) {
    if (board.side_to_move() == strong || !board.can_push_history()) return WDLScore::Unknown;
    MoveList legal;
    MoveGenerator::generate_legal_moves(board, legal);
    for (const auto move : legal) {
        if (board.piece_at(move.to()) != make_piece(strong, type)) continue;
        Board child = board;
        child.make_move(move);
        if (child.is_insufficient_material()) return WDLScore::Draw;
    }
    return WDLScore::Unknown;
}

WDLScore SyzygyTablebase::solve_kqk(const Board& board, Color strong) {
    return bare_king_capture_draw(board, strong, PieceType::Queen);
}
WDLScore SyzygyTablebase::solve_krk(const Board& board, Color strong) {
    return bare_king_capture_draw(board, strong, PieceType::Rook);
}

WDLScore SyzygyTablebase::evaluate_endgame_wdl(const Board& board) {
    if (board.is_insufficient_material()) return WDLScore::Draw;
    if (popcount(board.occupied()) == 3) {
        for (Color strong : {Color::White, Color::Black}) {
            if (board.pieces(make_piece(strong, PieceType::Queen))) return solve_kqk(board, strong);
            if (board.pieces(make_piece(strong, PieceType::Rook))) return solve_krk(board, strong);
        }
    }
    // A material pattern alone is not a rule-aware proof, even for KQK/KRK:
    // rule-50 distance and en-prise pieces matter. Leave those nodes to search.
    return WDLScore::Unknown;
}
} // namespace heavensgate
