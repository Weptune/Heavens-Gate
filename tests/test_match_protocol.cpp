#include "test.hpp"
#include "../src/core/fen.hpp"
#include "../src/uci/match_protocol.hpp"
#include "../src/core/time_budget.hpp"
#include "../src/core/run_manifest.hpp"
#include "../src/uci/stockfish_client.hpp"
#include <filesystem>
namespace heavensgate::test {
static bool score_association() {
    UciSearchReports reports;
    reports.ingest("info depth 8 multipv 2 score cp 42 nodes 321 pv e2e4 e7e5");
    reports.ingest("info depth 9 multipv 1 score mate 6 nodes 654 pv d2d4 d7d5");
    const auto chosen = reports.for_move("e2e4");
    if (!chosen || chosen->score != 42 || chosen->kind != UciScoreKind::Cp || reports.for_move("g1f3")) return false;
    reports.ingest("info depth 10 score cp 99 lowerbound pv e2e4");
    if (reports.for_move("e2e4")->bound != UciScoreBound::Lower) return false;
    reports.ingest("info depth 2 score cp -200 pv e2e4"); // Cannot replace deeper evidence.
    if (reports.for_move("e2e4")->score != 99) return false;
    if (parse_uci_info("info score cp nope pv e2e4") || parse_uci_info("info nodes -1")) return false;
    const auto text = parse_uci_info("info string score mate 1 pv e2e4");
    UciSearchReports absent;
    absent.ingest("info score mate 2 pv d2d4");
    return text && !text->score_valid && !absent.nodes_valid && !absent.for_move("e2e4");
}
static bool history_and_controls() {
    Board board; board.reset();
    std::vector<Move> history;
    for (const auto& text : {"g1f3", "g8f6", "f3g1", "f6g8"}) {
        const auto move = legal_uci_move(board, text);
        if (!move) return false;
        history.push_back(move); board.make_move(move);
    }
    if (match_position_command(std::string(StartposFEN), history) !=
        "position fen " + std::string(StartposFEN) + " moves g1f3 g8f6 f3g1 f6g8") return false;
    if (match_go_command(TournamentTimeControl::parse(120, 0), 119876, 118000) != "go wtime 119876 btime 118000 winc 0 binc 0") return false;
    if (match_go_command(TournamentTimeControl::parse(4, -1), 0, 0) != "go depth 4") return false;
    if (match_go_command(TournamentTimeControl::parse(1, 0, true), 0, 0) != "go movetime 1000") return false;
    if (legal_uci_move(board, "e2e4q")) return false;
    board.load_fen("4k3/P7/8/8/8/8/8/4K3 w - - 0 1");
    return legal_uci_move(board, "a7a8n") && !legal_uci_move(board, "a7a8");
}
static bool truthful_termination() {
    Board board; board.reset();
    if (board_match_ending(board)) return false; // No score can adjudicate startpos.
    board.load_fen("7k/6Q1/5K2/8/8/8/8/8 b - - 100 1");
    const auto mate = board_match_ending(board);
    if (!mate || mate->result != "1-0" || mate->reason != "Checkmate") return false;
    board.load_fen("7k/5K2/6Q1/8/8/8/8/8 b - - 0 1");
    if (board_match_ending(board)->reason != "Stalemate") return false;
    board.reset();
    for (int i = 0; i < 2; ++i) for (const auto& text : {"g1f3", "g8f6", "f3g1", "f6g8"}) board.make_move(legal_uci_move(board, text));
    if (board_match_ending(board)->reason != "Threefold Repetition") return false;
    for (const double time : {0.01, 0.2, 0.9, 2.0, 3.943, 5.0, 20.0, 100.0, 1000.0, 120000.0}) {
        for (const int move : {1, 10, 80}) {
            const auto budget = bank_search_budget(time, 0, move);
            if (budget.optimum_ms <= 0 || budget.optimum_ms > budget.maximum_ms || budget.maximum_ms > time) return false;
            const double available = time - std::min(25.0, time * 0.5);
            if (budget.maximum_ms > available) return false;
        }
    }
    return true;
}
static bool timeout_policy() {
    MatchRunStatus pilot;
    if (!pilot.record(MatchFailure::None) || !pilot.clean()) return false;
    if (!pilot.record(MatchFailure::TimeForfeit) || pilot.clean() || pilot.time_forfeits != 1 ||
        pilot.master_time_forfeits != 1) return false;
    if (!pilot.record(MatchFailure::None) || !pilot.record(MatchFailure::TimeForfeit, false, false) ||
        pilot.time_forfeits != 2 || pilot.opponent_time_forfeits != 1) return false;
    if (pilot.record(MatchFailure::EngineFailure) || !pilot.engine_failure || pilot.clean()) return false;
    MatchRunStatus gate;
    return !gate.record(MatchFailure::TimeForfeit, true) && gate.time_forfeits == 1 && !gate.clean();
}
static bool opponent_failures() {
    Board board; board.reset();
    struct EnvCleanup { ~EnvCleanup() { SetEnvironmentVariableA("HG_FAKE_UCI_MODE", nullptr); } } cleanup;
    for (const std::string mode : {"associated", "mismatch", "bound", "illegal", "crash", "timeout"}) {
        SetEnvironmentVariableA("HG_FAKE_UCI_MODE", mode.c_str());
        StockfishClient client(HG_FAKE_UCI_PATH);
        if (!client.init(2400, 1, 16) || !client.reset_game()) return false;
        const auto reply = client.get_search_result(board, match_position_command(std::string(StartposFEN), {}), "go depth 4", mode == "timeout" ? 20 : 2000);
        if (mode == "associated" && (reply.status != EngineReplyStatus::Ok || !reply.info || reply.info->score != 42 || reply.search.metrics.total_nodes != 654)) return false;
        if (mode == "mismatch" && (reply.status != EngineReplyStatus::Ok || reply.info)) return false;
        if (mode == "bound" && (!reply.info || reply.info->bound != UciScoreBound::Lower)) return false;
        if (mode == "illegal" && reply.status != EngineReplyStatus::IllegalMove) return false;
        if (mode == "crash" && reply.status != EngineReplyStatus::ProcessExited) return false;
        if (mode == "timeout" && reply.status != EngineReplyStatus::Timeout) return false;
        if ((mode == "timeout" || mode == "crash") && (reply.search.best_move || reply.nodes_valid || reply.search.metrics.total_nodes)) return false;
    }
    return true;
}
static bool manifest_fingerprint() {
    const auto path = std::filesystem::temp_directory_path() / ("hg-sha-" + std::to_string(GetCurrentProcessId()) + ".txt");
    { std::ofstream file(path, std::ios::binary); file << "abc"; }
    const auto hash = file_sha256(path.string());
    std::filesystem::remove(path);
    return hash == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" &&
        json_string("a\"b\\c\n") == "\"a\\\"b\\\\c\\u000a\"";
}
static const bool a = register_test("Match scores: selected root, MultiPV, bounds, absent/malformed info", score_association);
static const bool b = register_test("Match protocol: full history, bank clocks, exact promotions", history_and_controls);
static const bool c = register_test("Match endings: board terminal only; clock-bounded opening budget", truthful_termination);
static const bool d = register_test("Opponent fixture: mismatch, bound, crash, timeout, illegal move", opponent_failures);
static const bool e = register_test("Run manifest: SHA256 known vector and JSON escaping", manifest_fingerprint);
static const bool f = register_test("Clock losses continue pilot, remain failures; protocol failures abort", timeout_policy);
} // namespace heavensgate::test
