#pragma once
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <string>

namespace heavensgate {
struct TournamentTimeControl {
    int bank_ms = 0;
    int increment_ms = 0;
    int fixed_depth = 12;
    int movetime_ms = 0;
    static TournamentTimeControl parse(int bank_seconds, int increment_seconds, bool fixed_movetime = false) {
        if (bank_seconds < 0 || increment_seconds < -1)
            throw std::invalid_argument("Invalid time control");
        TournamentTimeControl tc;
        if (increment_seconds == -1) {
            if (bank_seconds < 1 || bank_seconds > 64) throw std::invalid_argument("Depth must be 1..64");
            tc.fixed_depth = bank_seconds;
            return tc;
        }
        constexpr int max_seconds = std::numeric_limits<int>::max() / 1000;
        if (bank_seconds > max_seconds || increment_seconds > max_seconds)
            throw std::invalid_argument("Clock overflow");
        if (fixed_movetime) {
            if (!bank_seconds || increment_seconds) throw std::invalid_argument("Movetime must be positive");
            tc.movetime_ms = bank_seconds * 1000;
        } else {
            if (!bank_seconds && increment_seconds) throw std::invalid_argument("Increment needs a bank");
            tc.bank_ms = bank_seconds * 1000;
            tc.increment_ms = increment_seconds * 1000;
        }
        return tc;
    }
};

struct MatchClock {
    double remaining_ms = 0;
    // Flag fall is tested BEFORE adding the increment. Never clamp a dead clock.
    bool finish_move(double elapsed_ms, double increment_ms) noexcept {
        if (!std::isfinite(elapsed_ms) || elapsed_ms < 0 || !std::isfinite(remaining_ms) ||
            !std::isfinite(increment_ms) || increment_ms < 0) return false;
        remaining_ms -= elapsed_ms;
        if (remaining_ms <= 0) return false;
        remaining_ms += increment_ms;
        return true;
    }
};

enum class MatchFailure { None, TimeForfeit, EngineFailure };
struct MatchRunStatus {
    int time_forfeits = 0;
    int master_time_forfeits = 0;
    int opponent_time_forfeits = 0;
    bool engine_failure = false;
    bool clean() const noexcept { return !time_forfeits && !engine_failure; }
    bool record(MatchFailure failure, bool fail_fast_timeouts = false, bool master_flagged = true) noexcept {
        if (failure == MatchFailure::EngineFailure) { engine_failure = true; return false; }
        if (failure == MatchFailure::TimeForfeit) {
            ++time_forfeits;
            if (master_flagged) ++master_time_forfeits;
            else ++opponent_time_forfeits;
            return !fail_fast_timeouts;
        }
        return true;
    }
};

inline std::string pgn_clock(double milliseconds) {
    const auto ms = static_cast<long long>(std::max(0.0, milliseconds));
    char text[64];
    std::snprintf(text, sizeof(text), "%lld:%02lld:%02lld.%03lld",
                  ms / 3600000, (ms / 60000) % 60, (ms / 1000) % 60, ms % 1000);
    return text;
}
} // namespace heavensgate
