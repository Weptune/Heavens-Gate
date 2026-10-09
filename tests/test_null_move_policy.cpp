#include "test.hpp"
#include "../src/search/null_move_policy.hpp"
#include "../src/search/search_params.hpp"
#include "../src/search/search.hpp"
#include "../src/core/fen.hpp"

namespace heavensgate::test {
static bool null_eligibility() {
    if (!guarded_null_move_allowed(true, 100, 100, false) ||
        guarded_null_move_allowed(false, 200, 100, false) ||
        guarded_null_move_allowed(true, 99, 100, false) ||
        guarded_null_move_allowed(true, 200, 100, true) ||
        guarded_null_move_allowed(true, ScoreInfinity, ScoreMate, false) ||
        guarded_null_move_allowed(true, 0, -ScoreMate, false)) return false;
    SearchParams defaults;
    defaults.enable_nmp_guards = true;
    defaults.reset();
    return !defaults.enable_nmp_guards;
}
static bool candidate_restoration() {
    struct Restore { SearchParams value = g_search_params; ~Restore() { g_search_params = value; } } restore;
    auto engine = std::make_unique<SearchEngine>();
    engine->set_book_enabled(false);
    engine->set_threads(1);
    for (bool guards : {false, true}) {
        g_search_params.enable_nmp_guards = guards;
        for (const char* fen : {
            StartposFEN.data(),
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
            "8/8/8/8/4p3/4k3/8/4K3 w - - 0 1", // pawn ending: no NMP
            "8/8/4k3/3p4/3P4/4K3/8/8 b - - 0 1", // mirror/zugzwang fixture
            "4k3/8/8/8/8/8/4r3/4K3 w - - 0 1", // check/evasion
            "7k/8/5KQ1/8/8/8/8/8 w - - 0 1"}) {
            Board board; board.load_fen(fen);
            const auto original = FEN::to_string(board);
            const auto key = board.zobrist_key();
            engine->clear();
            const auto result = engine->search_iterative_deepening(board, 7, 0, 100000);
            MoveList legal; MoveGenerator::generate_legal_moves(board, legal);
            bool found = false;
            for (auto move : legal) found |= move == result.best_move;
            if (!found || result.threads_used != 1 || result.metrics.total_nodes > 100000 ||
                FEN::to_string(board) != original || board.zobrist_key() != key || board.history_ply()) return false;
        }
    }
    return true;
}
static const bool eligible_registered = register_test("Experimental NMP guards: PV, beta, previous-null and mate eligibility", null_eligibility);
static const bool board_registered = register_test("NMP candidate: legal moves, checked/pawn endings and exact board restoration", candidate_restoration);
} // namespace heavensgate::test
