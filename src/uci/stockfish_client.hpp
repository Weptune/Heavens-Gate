#pragma once
#include "match_protocol.hpp"
#include "../search/search.hpp"
#include <windows.h>
#include <chrono>
#include <filesystem>
namespace heavensgate {
enum class EngineReplyStatus { Ok, ProcessExited, Timeout, ProtocolError, IllegalMove };
inline const char* reply_status_name(EngineReplyStatus status) {
    switch (status) {
        case EngineReplyStatus::Ok: return "ok";
        case EngineReplyStatus::ProcessExited: return "process-exited";
        case EngineReplyStatus::Timeout: return "protocol-timeout";
        case EngineReplyStatus::ProtocolError: return "protocol-error";
        case EngineReplyStatus::IllegalMove: return "illegal-move";
    }
    return "unknown";
}
struct StockfishReply {
    SearchResult search;
    std::optional<UciInfo> info;
    EngineReplyStatus status = EngineReplyStatus::ProtocolError;
    bool nodes_valid = false;
    std::string diagnostic;
};
class StockfishClient {
    using Clock = std::chrono::steady_clock;
    HANDLE input_ = nullptr, output_ = nullptr;
    PROCESS_INFORMATION process_{};
    std::string executable_, pending_, error_, identity_;
    int elo_ = 0, threads_ = 1, hash_mb_ = 64;
    bool alive() const {
        DWORD code = 0;
        return process_.hProcess && GetExitCodeProcess(process_.hProcess, &code) && code == STILL_ACTIVE;
    }
    bool send(const std::string& command) {
        if (!alive() || !input_) { error_ = "Opponent process exited"; return false; }
        const std::string bytes = command + '\n'; DWORD written = 0;
        if (!WriteFile(input_, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr) || written != bytes.size()) {
            error_ = "Opponent pipe write failed"; return false;
        }
        return true;
    }
    bool read_line(std::string& line, Clock::time_point deadline) {
        while (Clock::now() < deadline) {
            const auto newline = pending_.find('\n');
            if (newline != std::string::npos) {
                line = pending_.substr(0, newline); pending_.erase(0, newline + 1);
                if (!line.empty() && line.back() == '\r') line.pop_back();
                return true;
            }
            DWORD available = 0;
            if (!output_ || !PeekNamedPipe(output_, nullptr, 0, nullptr, &available, nullptr)) {
                error_ = "Opponent pipe closed"; return false;
            }
            if (available) {
                char buffer[4096]; DWORD count = 0;
                if (!ReadFile(output_, buffer, std::min<DWORD>(available, sizeof(buffer)), &count, nullptr) || !count) {
                    error_ = "Opponent pipe read failed"; return false;
                }
                pending_.append(buffer, count);
                if (pending_.size() > 1024 * 1024) { error_ = "Oversized opponent protocol line"; return false; }
            } else {
                if (!alive()) { error_ = "Opponent process exited"; return false; }
                Sleep(1);
            }
        }
        error_ = "Opponent protocol deadline expired"; return false;
    }
    bool wait_for(const std::string& token, int timeout_ms) {
        const auto deadline = Clock::now() + std::chrono::milliseconds(timeout_ms);
        std::string line;
        while (read_line(line, deadline)) {
            if (line.rfind("id name ", 0) == 0) identity_ = line.substr(8);
            if (line == token) return true;
        }
        return false;
    }
public:
    explicit StockfishClient(std::string executable = "tools/stockfish.exe")
        : executable_(std::filesystem::absolute(executable).string()) {}
    ~StockfishClient() { close(); }
    StockfishClient(const StockfishClient&) = delete;
    StockfishClient& operator=(const StockfishClient&) = delete;
    const std::string& executable() const { return executable_; }
    const std::string& identity() const { return identity_; }
    const std::string& last_error() const { return error_; }
    int effective_elo() const { return elo_; }
    int threads() const { return threads_; }
    int hash_mb() const { return hash_mb_; }
    bool init(int elo = 2700, int threads = 6, int hash_mb = 64) {
        close(); error_.clear(); identity_.clear();
        if (threads < 1 || threads > 64 || hash_mb < 1 || hash_mb > 16384) {
            error_ = "Invalid opponent configuration"; return false;
        }
        SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
        HANDLE child_output = nullptr, child_input = nullptr;
        if (!CreatePipe(&output_, &child_output, &security, 65536) || !CreatePipe(&child_input, &input_, &security, 65536)) {
            if (child_output) CloseHandle(child_output);
            if (child_input) CloseHandle(child_input);
            error_ = "Cannot create opponent pipes"; close(); return false;
        }
        SetHandleInformation(output_, HANDLE_FLAG_INHERIT, 0); SetHandleInformation(input_, HANDLE_FLAG_INHERIT, 0);
        STARTUPINFOA startup{}; startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES;
        startup.hStdInput = child_input; startup.hStdOutput = startup.hStdError = child_output;
        std::string command = '"' + executable_ + '"';
        const BOOL launched = CreateProcessA(executable_.c_str(), command.data(), nullptr, nullptr, TRUE,
                                             CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process_);
        CloseHandle(child_input); CloseHandle(child_output);
        if (!launched) { error_ = "Cannot launch opponent"; close(); return false; }
        elo_ = elo > 0 && elo < 3200 ? std::clamp(elo, 1320, 3190) : 0;
        threads_ = threads; hash_mb_ = hash_mb;
        if (!send("uci") || !wait_for("uciok", 10000) || identity_.empty() ||
            !send("setoption name Threads value " + std::to_string(threads_)) ||
            !send("setoption name Hash value " + std::to_string(hash_mb_)) ||
            !send("setoption name MultiPV value 1") ||
            (elo_ && !send("setoption name UCI_Elo value " + std::to_string(elo_))) ||
            !send(std::string("setoption name UCI_LimitStrength value ") + (elo_ ? "true" : "false")) ||
            !send("isready") || !wait_for("readyok", 10000)) { close(); return false; }
        return true;
    }
    bool reset_game() {
        error_.clear();
        return send("ucinewgame") && send("isready") && wait_for("readyok", 5000);
    }
    StockfishReply get_search_result(const Board& board, const std::string& position_command,
                                    const std::string& go_command, int deadline_ms = 60000) {
        StockfishReply reply; error_.clear(); const auto started = Clock::now();
        auto failed = [&]() {
            reply.diagnostic = error_;
            reply.status = !alive() ? EngineReplyStatus::ProcessExited :
                error_ == "Opponent protocol deadline expired" ? EngineReplyStatus::Timeout : EngineReplyStatus::ProtocolError;
        };
        if (!send(position_command) || !send(go_command)) { failed(); return reply; }
        UciSearchReports reports;
        const auto deadline = started + std::chrono::milliseconds(std::max(1, deadline_ms));
        std::string line; bool received = false;
        while (read_line(line, deadline)) {
            if (line.rfind("info ", 0) == 0) reports.ingest(line);
            if (line.rfind("bestmove ", 0) != 0) continue;
            std::istringstream input(line); std::string token, text; input >> token >> text;
            reply.search.best_move = legal_uci_move(board, text);
            if (!reply.search.best_move) {
                reply.status = EngineReplyStatus::IllegalMove;
                reply.diagnostic = "Illegal or empty opponent bestmove: " + text;
            } else {
                reply.status = EngineReplyStatus::Ok; reply.info = reports.for_move(text);
                if (reply.info) {
                    reply.search.depth = reply.search.completed_depth = reply.info->depth;
                    if (reply.info->kind == UciScoreKind::Cp) reply.search.best_score = reply.info->score;
                    else reply.search.best_score = reply.info->score > 0 ? ScoreMate - std::min(reply.info->score, 999) :
                        -ScoreMate - std::max(reply.info->score, -999);
                }
            }
            received = true; break;
        }
        if (!received) { failed(); close(); } // No retry/restart or synthetic fallback.
        reply.nodes_valid = reports.nodes_valid; reply.search.metrics.total_nodes = reports.nodes;
        reply.search.metrics.elapsed_seconds = std::chrono::duration<double>(Clock::now() - started).count();
        if (reply.search.metrics.elapsed_seconds > 0 && reports.nodes_valid)
            reply.search.metrics.nps = reports.nodes / reply.search.metrics.elapsed_seconds;
        return reply;
    }
    void close() {
        if (process_.hProcess) {
            if (alive() && input_) send("quit");
            if (WaitForSingleObject(process_.hProcess, 100) == WAIT_TIMEOUT) {
                TerminateProcess(process_.hProcess, 1); WaitForSingleObject(process_.hProcess, 100);
            }
            CloseHandle(process_.hProcess);
        }
        if (process_.hThread) CloseHandle(process_.hThread);
        if (input_) CloseHandle(input_);
        if (output_) CloseHandle(output_);
        process_ = {}; input_ = output_ = nullptr; pending_.clear();
    }
};
} // namespace heavensgate
