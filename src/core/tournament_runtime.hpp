#pragma once
#include "run_manifest.hpp"

namespace heavensgate {
// These diagnostics and file writes are outside the engine search path.
class ScopedTournamentPower {
    EXECUTION_STATE previous_;
public:
    ScopedTournamentPower() : previous_(SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED)) {}
    ~ScopedTournamentPower() { if (previous_) SetThreadExecutionState(previous_ | ES_CONTINUOUS); }
    bool active() const noexcept { return previous_ != 0; }
    ScopedTournamentPower(const ScopedTournamentPower&) = delete;
    ScopedTournamentPower& operator=(const ScopedTournamentPower&) = delete;
};

struct SuspendSample { ULONGLONG elapsed_ms = 0, awake_100ns = 0; bool valid = false; };
inline SuspendSample suspend_sample() noexcept {
    SuspendSample sample;
    sample.valid = QueryUnbiasedInterruptTime(&sample.awake_100ns) != 0;
    sample.elapsed_ms = GetTickCount64(); // Includes time spent suspended.
    return sample;
}
inline double suspended_ms(SuspendSample before, SuspendSample after) noexcept {
    if (!before.valid || !after.valid || after.elapsed_ms < before.elapsed_ms ||
        after.awake_100ns < before.awake_100ns) return 0;
    return std::max(0.0, double(after.elapsed_ms - before.elapsed_ms) -
                         double(after.awake_100ns - before.awake_100ns) / 10000.0);
}
class SuspendMonitor {
    SuspendSample previous_ = suspend_sample();
public:
    double observe() noexcept {
        const auto now = suspend_sample();
        const double gap = suspended_ms(previous_, now);
        previous_ = now;
        return gap;
    }
    bool supported() const noexcept { return previous_.valid; }
};

class TournamentJournal {
    std::ofstream events_;
    std::string status_path_;
    int games_written_ = 0;
    bool finished_ = false;
public:
    explicit TournamentJournal(const std::string& pgn) : events_(pgn + ".events.jsonl"), status_path_(pgn + ".run.json") {
        if (!events_) throw std::runtime_error("Cannot create tournament event journal");
        event("run_start", "\"pid\":" + std::to_string(GetCurrentProcessId()));
    }
    ~TournamentJournal() {
        if (!finished_) {
            // SIGKILL/power loss cannot run destructors; the supervisor records
            // those separately, and the flushed begin event identifies the reply.
            try { finish("aborted", "Unfinished run; inspect events, console and supervisor status", 0, 0); }
            catch (...) {}
        }
    }
    void event(const std::string& type, const std::string& fields = "") {
        events_ << "{\"event\":" << json_string(type) << ",\"utc\":" << json_string(utc_date(true))
                << (fields.empty() ? "" : ",") << fields << "}\n";
        events_.flush();
        if (!events_) throw std::runtime_error("Tournament journal write failed");
    }
    void game_written(int game, const std::string& result, const std::string& reason) {
        games_written_ = game;
        event("game_written", "\"game\":" + std::to_string(game) + ",\"result\":" + json_string(result) +
                              ",\"reason\":" + json_string(reason));
    }
    void finish(const std::string& status, const std::string& reason, int hg_flags, int opponent_flags) {
        event("run_end", "\"status\":" + json_string(status) + ",\"reason\":" + json_string(reason));
        std::ofstream out(status_path_);
        out << "{\"schema\":1,\"status\":" << json_string(status) << ",\"reason\":" << json_string(reason)
            << ",\"ended_utc\":" << json_string(utc_date(true)) << ",\"games_written\":" << games_written_
            << ",\"hg_time_forfeits\":" << hg_flags << ",\"opponent_time_forfeits\":" << opponent_flags << "}\n";
        out.close();
        if (!out) throw std::runtime_error("Tournament final status write failed");
        finished_ = true;
    }
};
} // namespace heavensgate
