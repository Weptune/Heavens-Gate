#pragma once
#include "../board/board.hpp"
#include "../core/match_clock.hpp"
#include "../movegen/movegen.hpp"
#include <algorithm>
#include <charconv>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
namespace heavensgate {
enum class UciScoreKind { Cp, Mate };
enum class UciScoreBound { Exact, Lower, Upper };
struct UciInfo {
    UciScoreKind kind = UciScoreKind::Cp;
    UciScoreBound bound = UciScoreBound::Exact;
    int score = 0, depth = 0, seldepth = 0, multipv = 1;
    uint64_t nodes = 0, time_ms = 0;
    bool score_valid = false, nodes_valid = false, time_valid = false;
    std::string root_move;
};
template <class T> inline bool parse_uci_number(std::string_view token, T& value) {
    const auto [end, error] = std::from_chars(token.data(), token.data() + token.size(), value);
    return error == std::errc{} && end == token.data() + token.size();
}
inline bool valid_uci_move(std::string_view move) noexcept {
    return (move.size() == 4 || move.size() == 5) &&
        move[0] >= 'a' && move[0] <= 'h' && move[2] >= 'a' && move[2] <= 'h' &&
        move[1] >= '1' && move[1] <= '8' && move[3] >= '1' && move[3] <= '8' &&
        (move.size() == 4 || move[4] == 'q' || move[4] == 'r' || move[4] == 'b' || move[4] == 'n');
}
inline std::optional<UciInfo> parse_uci_info(const std::string& line) {
    std::istringstream input(line);
    std::string token;
    if (!(input >> token) || token != "info") return std::nullopt;
    UciInfo info;
    while (input >> token) {
        if (token == "string") break;
        if (token == "depth" || token == "seldepth" || token == "multipv") {
            std::string number; int value;
            if (!(input >> number) || !parse_uci_number(number, value) || value < 0) return std::nullopt;
            if (token == "depth") info.depth = value;
            else if (token == "seldepth") info.seldepth = value;
            else { if (!value) return std::nullopt; info.multipv = value; }
        } else if (token == "nodes" || token == "time") {
            std::string number; uint64_t value;
            if (!(input >> number) || !parse_uci_number(number, value)) return std::nullopt;
            if (token == "nodes") { info.nodes = value; info.nodes_valid = true; }
            else { info.time_ms = value; info.time_valid = true; }
        } else if (token == "score") {
            std::string kind, number;
            if (!(input >> kind >> number) || (kind != "cp" && kind != "mate") ||
                !parse_uci_number(number, info.score)) return std::nullopt;
            info.kind = kind == "cp" ? UciScoreKind::Cp : UciScoreKind::Mate;
            info.score_valid = true;
        } else if (token == "lowerbound") info.bound = UciScoreBound::Lower;
        else if (token == "upperbound") info.bound = UciScoreBound::Upper;
        else if (token == "pv") {
            if (input >> info.root_move && !valid_uci_move(info.root_move)) return std::nullopt;
            break;
        }
    }
    return info;
}
// Reports are outside HG's search path. Associate by root move, not print order.
class UciSearchReports {
    std::vector<UciInfo> roots_;
public:
    uint64_t nodes = 0;
    bool nodes_valid = false;
    void ingest(const std::string& line) {
        auto info = parse_uci_info(line);
        if (!info) return;
        if (info->nodes_valid) { nodes = info->nodes; nodes_valid = true; }
        if (!info->score_valid || info->root_move.empty()) return;
        auto found = std::find_if(roots_.begin(), roots_.end(), [&](const auto& old) { return old.root_move == info->root_move; });
        if (found != roots_.end()) { if (info->depth >= found->depth) *found = *info; }
        else if (roots_.size() < 256) roots_.push_back(*info);
    }
    std::optional<UciInfo> for_move(const std::string& move) const {
        for (const auto& report : roots_) if (report.root_move == move) return report;
        return std::nullopt;
    }
};
inline Move legal_uci_move(const Board& board, std::string_view text) {
    if (!valid_uci_move(text)) return Move{};
    MoveList legal; MoveGenerator::generate_legal_moves(board, legal);
    for (Move move : legal) if (move_to_uci(move) == text) return move;
    return Move{}; // No coordinate-only promotion fallback.
}
inline std::string match_position_command(const std::string& initial_fen, const std::vector<Move>& moves) {
    std::string command = "position fen " + initial_fen;
    if (!moves.empty()) {
        command += " moves";
        for (Move move : moves) command += " " + move_to_uci(move);
    }
    return command;
}
inline std::string match_go_command(const TournamentTimeControl& tc, int white_ms, int black_ms) {
    if (tc.bank_ms) return "go wtime " + std::to_string(std::max(1, white_ms)) + " btime " +
        std::to_string(std::max(1, black_ms)) + " winc " + std::to_string(tc.increment_ms) + " binc " + std::to_string(tc.increment_ms);
    if (tc.movetime_ms) return "go movetime " + std::to_string(tc.movetime_ms);
    return "go depth " + std::to_string(tc.fixed_depth);
}
struct MatchEnding { std::string result, reason; };
inline std::optional<MatchEnding> board_match_ending(const Board& board) {
    MoveList legal; MoveGenerator::generate_legal_moves(board, legal);
    if (legal.empty()) {
        if (MoveGenerator::in_check(board, board.side_to_move()))
            return MatchEnding{board.side_to_move() == Color::White ? "0-1" : "1-0", "Checkmate"};
        return MatchEnding{"1/2-1/2", "Stalemate"};
    }
    if (board.is_repetition(3)) return MatchEnding{"1/2-1/2", "Threefold Repetition"};
    if (board.halfmove_clock() >= 100) return MatchEnding{"1/2-1/2", "50-Move Rule"};
    if (board.is_insufficient_material()) return MatchEnding{"1/2-1/2", "Insufficient Material"};
    return std::nullopt; // Scores deliberately cannot adjudicate a game.
}
} // namespace heavensgate
