#include "test.hpp"
#include "search_test_access.hpp"

namespace heavensgate::test {
static constexpr std::array<const char*, 2> QuietSevenPieces = {
    "7k/8/8/8/8/8/3Q4/KRNBN3 w - - 0 1",
    "krnbn3/3q4/8/8/8/8/8/7K b - - 0 1"
};

static bool expect_score(const char* label, int actual, int expected, const Board& b, const PositionSnapshot& before) {
    if (actual == expected && before.unchanged(b)) return true;
    std::cout << '\n' << label << " clock=" << b.halfmove_clock() << " color=" << static_cast<int>(b.side_to_move())
              << " expected=" << expected << " actual=" << actual << " restored=" << before.unchanged(b) << '\n';
    return false;
}

static bool rule50_boundary(bool contexts = false) {
    TranspositionTable table(1);
    table.set_clock_contexts(contexts);
    auto engine = std::make_unique<SearchEngine>(&table);
    bool ok = true;
    for (const char* fen : QuietSevenPieces) {
        Board b; b.load_fen(fen);
        for (int clock : {100, 150}) {
            b.set_halfmove_clock(clock);
            const PositionSnapshot before(b);
            for (auto window : {std::array<int, 2>{-ScoreInfinity, ScoreInfinity},
                                std::array<int, 2>{-200, -199}, std::array<int, 2>{500, 501}}) {
                for (bool warm : {false, true}) {
                    table.clear(); engine->clear();
                    if (warm) table.store(b.zobrist_key(), Move{}, 900, 12, TTBound::Exact, 3);
                    ok &= expect_score("rule50 qsearch", SearchEngineTestAccess::qsearch(*engine, b, window[0], window[1], 3), 0, b, before);
                    for (int depth : {0, 2})
                        ok &= expect_score("rule50 negamax", SearchEngineTestAccess::negamax(*engine, b, depth, 3, window[0], window[1]), 0, b, before);
                }
            }
        }
    }
    return ok;
}

static bool mate_and_checked_boundary(bool contexts = false) {
    TranspositionTable table(1);
    table.set_clock_contexts(contexts);
    auto engine = std::make_unique<SearchEngine>(&table);
    bool ok = true;
    for (const char* fen : {"7k/6Q1/5K2/8/8/8/8/8 b - - 100 1",
                           "8/8/8/8/8/5k2/6q1/7K w - - 100 1",
                           "4k3/8/8/8/8/8/4r3/4K3 w - - 100 1",
                           "4k3/4R3/8/8/8/8/8/4K3 b - - 100 1"}) {
        Board b; b.load_fen(fen);
        MoveList legal; MoveGenerator::generate_legal_moves(b, legal);
        if (!MoveGenerator::in_check(b, b.side_to_move())) return false;
        const PositionSnapshot before(b);
        for (int clock : {100, 150}) {
            b.set_halfmove_clock(clock);
            const PositionSnapshot clock_before(b);
            for (int ply : {0, 7, 64}) {
                const int expected = legal.empty() ? -ScoreMate + ply : ScoreDraw;
                for (bool warm : {false, true}) {
                    table.clear(); engine->clear();
                    if (warm) table.store(b.zobrist_key(), Move{}, 900, 12, TTBound::Exact, ply);
                    ok &= expect_score("checked boundary qsearch", SearchEngineTestAccess::qsearch(*engine, b, -ScoreInfinity, ScoreInfinity, ply), expected, b, clock_before);
                    ok &= expect_score("checked boundary negamax", SearchEngineTestAccess::negamax(*engine, b, 2, ply), expected, b, clock_before);
                }
            }
        }
        b.set_halfmove_clock(100);
        if (!before.unchanged(b)) return false;
    }
    return ok;
}

static bool crossings_and_roots(bool contexts = false) {
    TranspositionTable table(1);
    table.set_clock_contexts(contexts);
    auto engine = std::make_unique<SearchEngine>(&table);
    engine->set_book_enabled(false);
    bool ok = true;
    for (const char* fen : QuietSevenPieces) {
        Board b; b.load_fen(fen); b.set_halfmove_clock(99);
        const PositionSnapshot before(b);
        table.clear(); engine->clear();
        ok &= expect_score("quiet 99->100", SearchEngineTestAccess::negamax(*engine, b, 1, 0), 0, b, before);
        // All root moves are reversible and none mates: the draw is forced next ply.
        table.clear(); engine->clear();
        ok &= expect_score("root alpha-beta 99", engine->search_alphabeta(b, 1).best_score, 0, b, before);
        table.clear(); engine->clear();
        ok &= expect_score("root iterative 99", engine->search_iterative_deepening(b, 1, 0).best_score, 0, b, before);
    }
    for (const char* fen : {"7k/8/5KQ1/8/8/8/8/8 w - - 99 1",
                           "8/8/8/8/8/5kq1/8/7K b - - 99 1"}) {
        Board b; b.load_fen(fen); const PositionSnapshot before(b);
        table.clear(); engine->clear();
        ok &= expect_score("mate on halfmove100", engine->search_iterative_deepening(b, 1, 0).best_score, ScoreMate - 1, b, before);
    }
    for (bool black : {false, true}) {
        Board b; b.reset(); b.set_halfmove_clock(100);
        if (black) { b.set_side_to_move(Color::Black); b.recalculate_zobrist_key(); }
        const PositionSnapshot before(b);
        table.clear(); engine->clear();
        auto result = engine->search_alphabeta(b, 1);
        ok &= expect_score("claimable root with pawn resets (AB)", result.best_score, 0, b, before);
        table.clear(); engine->clear();
        result = engine->search_iterative_deepening(b, 1, 0);
        ok &= expect_score("claimable root with pawn resets (ID)", result.best_score, 0, b, before);
    }
    return ok;
}

static bool cached_clock_context(bool contexts = false) {
    TranspositionTable table(1);
    table.set_clock_contexts(contexts);
    auto engine = std::make_unique<SearchEngine>(&table);
    bool ok = true;
    for (const char* fen : QuietSevenPieces) {
        Board b; b.load_fen(fen);
        const auto position_key = b.zobrist_key();
        table.clear(); engine->clear();
        const int fresh_zero = SearchEngineTestAccess::negamax(*engine, b, 1, 0);
        if (fresh_zero <= 0) return false;
        b.set_halfmove_clock(99); const PositionSnapshot at99(b);
        if (position_key != b.zobrist_key()) return false; // Repetition identity must NOT change.
        // The clock-0 score is real, not an arbitrary seeded contradiction.
        ok &= expect_score("clock0 cached score reused at99", SearchEngineTestAccess::negamax(*engine, b, 1, 0), 0, b, at99);
        table.clear(); engine->clear();
        const int fresh_99 = SearchEngineTestAccess::negamax(*engine, b, 1, 0);
        ok &= expect_score("fresh99", fresh_99, 0, b, at99);
        b.set_halfmove_clock(0); const PositionSnapshot at0(b);
        engine->clear(); // Shared table deliberately retains the clock-99 entry.
        ok &= expect_score("clock99 draw reused at0", SearchEngineTestAccess::negamax(*engine, b, 1, 0), fresh_zero, b, at0);
        table.clear(); engine->clear();
        b.set_halfmove_clock(99);
        SearchEngineTestAccess::negamax(*engine, b, 1, 0); // Cache real full-search clock-99 draw.
        b.set_halfmove_clock(0);
        const int expected_q = Evaluator::evaluate_fast(b, -ScoreInfinity, ScoreInfinity);
        ok &= expect_score("clock99 full-search draw reused by clock0 qsearch", SearchEngineTestAccess::qsearch(*engine, b, -ScoreInfinity, ScoreInfinity, 0), expected_q, b, at0);
    }
    return ok;
}

static bool reset_and_history_contract(bool contexts = false) {
    for (auto fixture : {std::pair{"4k3/8/8/8/8/8/4P3/4K3 w - - 99 1", Move(Square::e2, Square::e4, MoveType::DoublePawnPush)},
                         std::pair{"4k3/8/8/8/4r3/8/4Q3/4K3 w - - 99 1", Move(Square::e2, Square::e4, MoveType::Capture)},
                         std::pair{"4k3/8/8/3pP3/8/8/8/4K3 w - d6 99 1", Move(Square::e5, Square::d6, MoveType::EnPassant)},
                         std::pair{"4k3/P7/8/8/8/8/8/4K3 w - - 99 1", Move(Square::a7, Square::a8, MoveType::PromoQueen)}}) {
        Board b; b.load_fen(fixture.first); const PositionSnapshot before(b);
        b.make_move(fixture.second);
        if (b.halfmove_clock() != 0) return false;
        TranspositionTable table(1); auto engine = std::make_unique<SearchEngine>(&table);
        table.set_clock_contexts(contexts);
        if (SearchEngineTestAccess::qsearch(*engine, b, -ScoreInfinity, ScoreInfinity, 1) == ScoreDraw) return false;
        b.unmake_move(fixture.second);
        if (!before.unchanged(b)) return false;
    }
    Board b; b.reset(); b.set_halfmove_clock(99); const PositionSnapshot before(b);
    b.make_null_move();
    if (b.halfmove_clock() != 99 || b.is_repetition(2)) return false;
    b.unmake_null_move();
    if (!before.unchanged(b)) return false;
    for (Move move : {Move(Square::g1, Square::f3), Move(Square::g8, Square::f6),
                      Move(Square::f3, Square::g1), Move(Square::f6, Square::g8)}) b.make_move(move);
    return b.zobrist_key() == before.key && b.halfmove_clock() == 103 && b.is_repetition(2);
}

static bool tt_metadata_contract() {
    TranspositionTable table(1);
    auto engine = std::make_unique<SearchEngine>(&table);
    bool ok = true;
    for (const char* fen : QuietSevenPieces) {
        Board b; b.load_fen(fen);
        for (int clock : {0, 1, 98, 99}) {
            b.set_halfmove_clock(clock); const PositionSnapshot before(b);
            for (TTBound bound : {TTBound::Exact, TTBound::Lower, TTBound::Upper}) {
                table.clear(); engine->clear();
                const int alpha = bound == TTBound::Upper ? 1000 : -ScoreInfinity;
                const int beta = bound == TTBound::Lower ? 800 : ScoreInfinity;
                table.store(b.zobrist_key(), Move{}, 900, 12, bound, 3, clock);
                TTEntry entry;
                if (!table.probe(b.zobrist_key(), entry) || entry.rule50_clock != clock ||
                    !entry.score_matches_rule50(clock) || entry.score_matches_rule50(100) ||
                    entry.score_matches_rule50((clock + 1) % 100) || (entry.data_word() >> 56) != static_cast<uint64_t>(clock)) return false;
                ok &= expect_score("matching-clock cutoff (negamax)", SearchEngineTestAccess::negamax(*engine, b, 2, 3, alpha, beta), 900, b, before);
                ok &= expect_score("matching-clock cutoff (qsearch)", SearchEngineTestAccess::qsearch(*engine, b, alpha, beta, 3), 900, b, before);
            }
        }
        // An incompatible deep bound must not prevent storing a shallower new-context result.
        table.clear();
        const Move hint(Square::d2, Square::e2);
        table.store(b.zobrist_key(), hint, 900, 16, TTBound::Exact, 0, 0);
        table.store(b.zobrist_key(), Move{}, 0, 1, TTBound::Upper, 0, 99);
        TTEntry entry;
        if (!table.probe(b.zobrist_key(), entry) || entry.rule50_clock != 99 || entry.score != 0 || entry.depth != 1 ||
            entry.bound != TTBound::Upper || entry.move != hint) return false;
        // All bound kinds at the wrong clock must be ignored, including eval refinement/singularity metadata.
        for (TTBound bound : {TTBound::Exact, TTBound::Lower, TTBound::Upper}) {
            b.set_halfmove_clock(99); const PositionSnapshot before(b);
            table.clear(); engine->clear();
            table.store(b.zobrist_key(), Move{}, 900, 12, bound, 0, 0);
            ok &= expect_score("mismatched-clock bound", SearchEngineTestAccess::negamax(*engine, b, 1, 0), 0, b, before);
        }
    }
    return ok;
}

static const bool boundary_registered = register_test("Rule50: 7-piece draw boundary before stand-pat/TT (both colors)", [] { return rule50_boundary(); });
static const bool mate_registered = register_test("Rule50: checkmate precedence and checked legal replies", [] { return mate_and_checked_boundary(); });
static const bool root_registered = register_test("Rule50: quiet crossing, mate on halfmove100 and root claims", [] { return crossings_and_roots(); });
static const bool cache_registered = register_test("Rule50: TT bounds cannot cross halfmove-clock contexts", [] { return cached_clock_context(); });
static const bool reset_registered = register_test("Rule50: pawn/capture/EP/promotion resets and unchanged repetition identity", [] { return reset_and_history_contract(); });
static const bool metadata_registered = register_test("Rule50: compatible bounds retained, clock metadata and context replacement", tt_metadata_contract);
static const bool contexts_registered = register_test("TTContexts: mirrored rule50/mate/reset/cache contracts with contexts enabled", [] {
    return rule50_boundary(true) && mate_and_checked_boundary(true) && crossings_and_roots(true) &&
           cached_clock_context(true) && reset_and_history_contract(true);
});
} // namespace heavensgate::test
