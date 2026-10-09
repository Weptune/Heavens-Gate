#include "test.hpp"
#include "search_test_access.hpp"

namespace heavensgate::test {

static bool stalemate_contract() {
    TranspositionTable table(1);
    auto engine = std::make_unique<SearchEngine>(&table);
    bool passed = true;
    for (const char* fen : {"k7/2Q5/2K5/8/8/8/8/8 b - - 0 1",
                           "8/8/8/8/8/2k5/2q5/K7 w - - 0 1"}) {
        Board b; b.load_fen(fen);
        MoveList legal; MoveGenerator::generate_legal_moves(b, legal);
        if (!legal.empty() || MoveGenerator::in_check(b, b.side_to_move())) return false;
        const PositionSnapshot before(b);
        for (int ply : {0, 7, 63, 64}) {
            for (const auto window : {std::array<int, 2>{-ScoreInfinity, ScoreInfinity},
                                      std::array<int, 2>{-1501, -1500},
                                      std::array<int, 2>{900, 901}}) {
                for (bool warm : {false, true}) {
                    table.clear();
                    if (warm) table.store(b.zobrist_key(), Move{}, 900, 12, TTBound::Exact, ply);
                    const int score = SearchEngineTestAccess::qsearch(*engine, b, window[0], window[1], ply);
                    if (score != ScoreDraw || !before.unchanged(b)) {
                        std::cout << "\nstalemate color=" << static_cast<int>(b.side_to_move())
                                  << " ply=" << ply << " window=" << window[0] << ',' << window[1]
                                  << " warm=" << warm << " expected=0 actual=" << score << '\n';
                        passed = false;
                    }
                }
            }
        }
    }
    return passed;
}

static bool capture_into_stalemate() {
    TranspositionTable table(1);
    auto engine = std::make_unique<SearchEngine>(&table);
    for (const char* fen : {"k7/2pQ4/5K2/8/8/8/8/8 w - - 0 1",
                           "8/8/8/8/8/5k2/2Pq4/K7 b - - 0 1"}) {
        Board b; b.load_fen(fen);
        MoveList captures; MoveGenerator::generate_capture_moves(b, captures);
        if (captures.size() != 1) return false;
        const PositionSnapshot before(b);
        const int stand_pat = Evaluator::evaluate_fast(b, -ScoreInfinity, ScoreInfinity);
        b.make_move(captures[0]);
        MoveList legal; MoveGenerator::generate_legal_moves(b, legal);
        if (!legal.empty() || MoveGenerator::in_check(b, b.side_to_move())) return false;
        b.unmake_move(captures[0]);
        table.clear();
        const int score = SearchEngineTestAccess::qsearch(*engine, b, -ScoreInfinity, ScoreInfinity, 0);
        if (score != stand_pat || !before.unchanged(b)) {
            std::cout << "\ncapture into stalemate: expected stand-pat=" << stand_pat << " actual=" << score << '\n';
            return false;
        }
    }
    return true;
}

static bool checkmate_and_quiet_contract() {
    TranspositionTable table(1);
    auto engine = std::make_unique<SearchEngine>(&table);
    for (const char* fen : {"k7/1Q6/2K5/8/8/8/8/8 b - - 0 1",
                           "8/8/8/8/8/2k5/1q6/K7 w - - 0 1"}) {
        Board b; b.load_fen(fen);
        const PositionSnapshot before(b);
        for (int ply : {0, 7, 63}) {
            table.clear();
            if (SearchEngineTestAccess::qsearch(*engine, b, -ScoreInfinity, ScoreInfinity, ply) != -ScoreMate + ply ||
                !before.unchanged(b)) return false;
        }
    }
    for (const char* fen : {StartposFEN.data(),
                           "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
                           "k7/8/8/8/8/5KQ1/8/8 w - - 0 1",
                           "8/8/5kq1/8/8/8/8/K7 b - - 0 1"}) {
        Board b; b.load_fen(fen);
        MoveList captures, legal;
        MoveGenerator::generate_capture_moves(b, captures);
        MoveGenerator::generate_legal_moves(b, legal);
        if (!captures.empty() || legal.empty()) return false;
        const PositionSnapshot before(b);
        const int stand_pat = Evaluator::evaluate_fast(b, -ScoreInfinity, ScoreInfinity);
        table.clear();
        if (SearchEngineTestAccess::qsearch(*engine, b, -ScoreInfinity, ScoreInfinity, 0) != stand_pat ||
            !before.unchanged(b)) return false;
    }
    return true;
}

static bool legal_existence_walk(Board& board, int depth, uint64_t& checked) {
    const PositionSnapshot before(board);
    MoveList legal;
    MoveGenerator::generate_legal_moves(board, legal);
    if (MoveGenerator::has_legal_move(board) != !legal.empty() || !before.unchanged(board)) return false;
    // Independent make/check/unmake oracle also covers pinned and EP moves.
    MoveList pseudo;
    MoveGenerator::generate_pseudo_legal_moves(board, pseudo);
    bool exists = false;
    const Color us = board.side_to_move();
    for (Move move : pseudo) {
        board.make_move(move);
        exists |= !MoveGenerator::in_check(board, us);
        board.unmake_move(move);
    }
    if (exists != !legal.empty() || !before.unchanged(board)) return false;
    ++checked;
    if (depth > 0) {
        for (Move move : legal) {
            board.make_move(move);
            const bool child_ok = legal_existence_walk(board, depth - 1, checked);
            board.unmake_move(move);
            if (!child_ok || !before.unchanged(board)) return false;
        }
    }
    return true;
}

static bool legal_existence_contract() {
    uint64_t checked = 0;
    for (const char* fen : {StartposFEN.data(),
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
            "4k3/8/8/r4pPK/8/8/8/8 w - f6 0 1", // illegal EP uncovers rook
            "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1",
            "k7/2Q5/2K5/8/8/8/8/8 b - - 0 1",
            "8/8/8/8/8/2k5/2q5/K7 w - - 0 1"}) {
        Board board; board.load_fen(fen);
        if (!legal_existence_walk(board, 2, checked)) return false;
    }
    std::cout << "(" << checked << " legal-existence/oracle nodes) ";
    return true;
}

static const bool stalemate_registered = register_test("Qsearch: stalemate precedes stand-pat and cached cutoffs (both colors)", stalemate_contract);
static const bool capture_registered = register_test("Qsearch: capture into stalemate is a draw, not a material gain", capture_into_stalemate);
static const bool other_registered = register_test("Qsearch: preserve checkmate distance and quiet nonterminal stand-pat", checkmate_and_quiet_contract);
static const bool existence_registered = register_test("Qsearch: legal-existence agrees with full generator and make/check oracle", legal_existence_contract);
} // namespace heavensgate::test
