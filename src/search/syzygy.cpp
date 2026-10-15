#include "syzygy.hpp"
#include "../movegen/movegen.hpp"
#include "../external/fathom/tbprobe.h"
#include <iostream>

namespace heavensgate {
thread_local std::array<TBCacheEntry, SyzygyTablebase::CACHE_SIZE> SyzygyTablebase::cache_{};
thread_local uint64_t SyzygyTablebase::local_epoch_ = UINT64_MAX;

void SyzygyTablebase::init(const std::string& tb_path) {
    // Fathom (Syzygy WDL/DTZ). An empty or missing path leaves TB_LARGEST at 0,
    // in which case only the built-in draw recognizer below is active.
    tb_free();
    if (!tb_path.empty() && tb_path != "<empty>") tb_init(tb_path.c_str());
    cache_epoch_.fetch_add(1, std::memory_order_relaxed);
    set_enabled(true);
}

int SyzygyTablebase::largest() const { return static_cast<int>(TB_LARGEST); }

namespace {
struct FathomPosition { uint64_t white, black, kings, queens, rooks, bishops, knights, pawns; unsigned ep; bool turn; };
FathomPosition to_fathom(const Board& b) {
    auto both = [&](PieceType pt) { return b.pieces(make_piece(Color::White, pt)) | b.pieces(make_piece(Color::Black, pt)); };
    return {b.pieces(Color::White), b.pieces(Color::Black), both(PieceType::King), both(PieceType::Queen),
            both(PieceType::Rook), both(PieceType::Bishop), both(PieceType::Knight), both(PieceType::Pawn),
            b.ep_square() == Square::None ? 0u : static_cast<unsigned>(b.ep_square()),
            b.side_to_move() == Color::White};
}
} // namespace

Move SyzygyTablebase::probe_root(const Board& board, int& score) {
    if (!is_enabled() || TB_LARGEST == 0 || popcount(board.occupied()) > static_cast<int>(TB_LARGEST) ||
        board.castling_rights() != 0) return Move{};
    const auto f = to_fathom(board);
    const unsigned r = tb_probe_root(f.white, f.black, f.kings, f.queens, f.rooks, f.bishops, f.knights, f.pawns,
                                     static_cast<unsigned>(board.halfmove_clock()), 0, f.ep, f.turn, nullptr);
    if (r == TB_RESULT_FAILED || r == TB_RESULT_CHECKMATE || r == TB_RESULT_STALEMATE) return Move{};
    const auto from = static_cast<Square>(TB_GET_FROM(r)), to = static_cast<Square>(TB_GET_TO(r));
    const unsigned promo = TB_GET_PROMOTES(r);
    PieceType want = PieceType::None;
    if (promo == TB_PROMOTES_QUEEN) want = PieceType::Queen;
    else if (promo == TB_PROMOTES_ROOK) want = PieceType::Rook;
    else if (promo == TB_PROMOTES_BISHOP) want = PieceType::Bishop;
    else if (promo == TB_PROMOTES_KNIGHT) want = PieceType::Knight;
    MoveList legal;
    MoveGenerator::generate_legal_moves(board, legal);
    for (const Move m : legal) {
        if (m.from() != from || m.to() != to) continue;
        if (m.is_promotion() ? m.promotion_piece_type() != want : want != PieceType::None) continue;
        const unsigned wdl = TB_GET_WDL(r), dtz = TB_GET_DTZ(r);
        score = wdl == TB_WIN ? ScoreTBWin - static_cast<int>(dtz) : wdl == TB_LOSS ? -ScoreTBWin + static_cast<int>(dtz) : ScoreDraw;
        return m;
    }
    return Move{};
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

    // Syzygy WDL: exact only right after a zeroing move (Fathom requires rule50 == 0)
    // and without castling rights.
    if (TB_LARGEST > 0 && popcount(board.occupied()) <= static_cast<int>(TB_LARGEST) &&
        board.halfmove_clock() == 0 && board.castling_rights() == 0) {
        const auto f = to_fathom(board);
        const unsigned r = tb_probe_wdl(f.white, f.black, f.kings, f.queens, f.rooks, f.bishops, f.knights, f.pawns,
                                        0, 0, f.ep, f.turn);
        if (r != TB_RESULT_FAILED) {
            switch (r) {
                case TB_WIN: return wdl_to_score(WDLScore::Win, ply);
                case TB_LOSS: return wdl_to_score(WDLScore::Loss, ply);
                default: return ScoreDraw; // draw, cursed win, blessed loss
            }
        }
    }

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
