#pragma once

#include "../board/board.hpp"
#include "../core/types.hpp"
#include <string>
#include <vector>
#include <map>

namespace heavensgate {

struct STSPosition {
    std::string fen;
    std::string id;
    std::string theme;
    // Pairs of (move_str, points) e.g. ("e4", 100), ("d4", 60)
    std::vector<std::pair<std::string, int>> scored_moves;
};

struct ThemeResult {
    std::string theme_name;
    int total_positions{0};
    int max_points{0};
    int points_earned{0};
    double percentage{0.0};
};

struct STSOverallResult {
    int total_positions{0};
    int max_points{0};
    int points_earned{0};
    double percentage{0.0};
    int estimated_elo{0};
    double total_time_ms{0.0};
    uint64_t total_nodes{0};
    std::map<std::string, ThemeResult> theme_results;

    std::string format_report() const;
};

class STSRunner {
public:
    static STSOverallResult run_suite(
        const std::string& epd_file_path = "",
        int time_per_pos_ms = 1000,
        int fixed_depth = 0,
        int threads = 6
    );

    static std::vector<STSPosition> load_builtin_suite();
    static std::vector<STSPosition> load_epd_file(const std::string& file_path);

    static Move parse_move_san_or_uci(const Board& board, const std::string& move_str);
    static bool move_matches(const Board& board, Move engine_move, const std::string& target_str);
};

} // namespace heavensgate
