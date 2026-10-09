#pragma once
#include "match_clock.hpp"
#include "time_budget.hpp"
#include "../evaluation/eval_params.hpp"
#include "../search/search_params.hpp"
#include <windows.h>
#include <bcrypt.h>
#include <array>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#ifndef HG_SOURCE_SHA256
#define HG_SOURCE_SHA256 "unavailable"
#endif
#ifndef HG_GIT_REVISION
#define HG_GIT_REVISION "unavailable"
#endif
namespace heavensgate {
inline std::string json_string(const std::string& value) {
    std::ostringstream out; out << '"';
    for (unsigned char ch : value) {
        if (ch == '"' || ch == '\\') out << '\\' << ch;
        else if (ch < 32) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << unsigned(ch);
        else out << ch;
    }
    out << '"'; return out.str();
}
inline std::string running_executable() {
    std::array<char, 32768> path{};
    const auto size = GetModuleFileNameA(nullptr, path.data(), static_cast<DWORD>(path.size()));
    if (!size || size >= path.size()) throw std::runtime_error("Cannot identify running executable");
    return std::string(path.data(), size);
}
inline std::string file_sha256(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::runtime_error("Cannot fingerprint file: " + path);
    BCRYPT_ALG_HANDLE algorithm = nullptr; BCRYPT_HASH_HANDLE hash = nullptr;
    std::vector<unsigned char> object;
    struct Cleanup {
        BCRYPT_ALG_HANDLE& algorithm; BCRYPT_HASH_HANDLE& hash;
        ~Cleanup() { if (hash) BCryptDestroyHash(hash); if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0); }
    } cleanup{algorithm, hash};
    auto require = [](NTSTATUS status) { if (status < 0) throw std::runtime_error("SHA256 provider failed"); };
    require(BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, 0));
    DWORD bytes = 0, object_size = 0;
    require(BCryptGetProperty(algorithm, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&object_size), sizeof(object_size), &bytes, 0));
    object.resize(object_size);
    require(BCryptCreateHash(algorithm, &hash, object.data(), object_size, nullptr, 0, 0));
    std::array<unsigned char, 65536> buffer{};
    while (input) {
        input.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
        const auto count = input.gcount();
        if (count) require(BCryptHashData(hash, buffer.data(), static_cast<ULONG>(count), 0));
    }
    if (!input.eof()) throw std::runtime_error("Fingerprint file read failed");
    std::array<unsigned char, 32> digest{};
    require(BCryptFinishHash(hash, digest.data(), digest.size(), 0));
    // The provider object storage must outlive the hash handle.
    BCryptDestroyHash(hash); hash = nullptr;
    std::ostringstream result;
    for (auto byte : digest) result << std::hex << std::setw(2) << std::setfill('0') << unsigned(byte);
    return result.str();
}
inline std::string utc_date(bool timestamp = false) {
    const auto now = std::time(nullptr); std::tm parts{};
    gmtime_s(&parts, &now);
    char output[32];
    std::strftime(output, sizeof(output), timestamp ? "%Y-%m-%dT%H:%M:%SZ" : "%Y.%m.%d", &parts);
    return output;
}
struct TournamentOptions {
    int hash_mb = 64;
    bool paired = false, book = true;
    bool fail_fast_timeouts = false;
    SearchTimingOptions timing{};
    std::string opponent_path = "tools/stockfish.exe";
};
inline void write_match_manifest(const std::string& filename, const std::string& opponent,
                                 const std::string& identity, int effective_elo, int threads,
                                 int games, const TournamentTimeControl& tc, const TournamentOptions& options,
                                 const std::string& book_path, const std::vector<std::string>& openings) {
    const auto engine = running_executable();
    const auto engine_hash = file_sha256(engine), opponent_hash = file_sha256(opponent);
    const auto book_hash = options.book ? file_sha256(book_path) : std::string{};
    std::ofstream out(filename);
    if (!out) throw std::runtime_error("Cannot create manifest");
    out << "{\n\"schema\":1,\"started_utc\":" << json_string(utc_date(true))
        << ",\"engine_path\":" << json_string(engine) << ",\"engine_sha256\":" << json_string(engine_hash)
        << ",\"source_sha256\":" << json_string(HG_SOURCE_SHA256) << ",\"git_revision\":" << json_string(HG_GIT_REVISION)
        << ",\"compiler\":" << json_string(__VERSION__) << ",\"opponent_path\":" << json_string(opponent)
        << ",\"opponent_sha256\":" << json_string(opponent_hash) << ",\"opponent_identity\":" << json_string(identity)
        << ",\"uci_limit_strength\":" << (effective_elo ? "true" : "false") << ",\"uci_elo\":" << effective_elo
        << ",\"threads_each\":" << threads << ",\"hash_mb_each\":" << options.hash_mb << ",\"games\":" << games
        << ",\"hg_move_overhead_ms\":" << options.timing.move_overhead_ms
        << ",\"hg_clock_policy\":\"sudden-death-45-move-horizon-1.8-hard-future-overhead-35\""
        << ",\"hg_minimum_smp_time_ms\":" << options.timing.minimum_smp_time_ms
        << ",\"continue_after_time_forfeit\":" << (options.fail_fast_timeouts ? "false" : "true")
        << ",\"bank_ms\":" << tc.bank_ms << ",\"increment_ms\":" << tc.increment_ms << ",\"movetime_ms\":" << tc.movetime_ms
        << ",\"fixed_depth\":" << tc.fixed_depth << ",\"book_enabled\":" << (options.book ? "true" : "false")
        << ",\"book_path\":" << json_string(book_path) << ",\"book_sha256\":" << json_string(book_hash)
        << ",\"book_seed\":1337,\"opponent_seed\":null,\"smp_deterministic\":false,\"paired_openings\":" << (options.paired ? "true" : "false")
        << ",\"adjudication\":\"board-terminal-or-rule-draw-only\",\"max_plies\":400,\"eval_parameters\":{\"pawn_mg\":" << g_eval_params.pawn_mg;
    for (const auto& parameter : g_eval_params.get_tunable_params()) out << ',' << json_string(parameter.name) << ':' << *parameter.ptr;
    const auto& p = g_search_params;
    out << "},\"search_parameters\":{\"LMR_Divisor\":" << p.lmr_divisor << ",\"LMR_HistBonus\":" << p.lmr_hist_bonus
        << ",\"LMR_HistMalus\":" << p.lmr_hist_malus << ",\"RFP_Margin\":" << p.rfp_margin << ",\"Futility_Margin\":" << p.futility_margin
        << ",\"SEE_BadCaptureSlope\":" << p.see_bad_capture_slope << ",\"SEE_QuietSlope\":" << p.see_quiet_slope
        << ",\"NMP_EvalMargin\":" << p.nmp_eval_margin << ",\"Singular_Margin\":" << p.singular_margin
        << ",\"Aspiration_Window_Delta\":" << p.aspiration_window_delta
        << ",\"EnableNMP\":" << (p.enable_nmp ? "true" : "false")
        << ",\"EnableLMR\":" << (p.enable_lmr ? "true" : "false")
        << ",\"NMPGuards\":" << (p.enable_nmp_guards ? "true" : "false") << "},\"openings\":[";
    for (size_t i = 0; i < openings.size(); ++i) out << (i ? "," : "") << json_string(openings[i]);
    out << "]}\n"; out.close();
    if (!out) throw std::runtime_error("Manifest write failed");
}
} // namespace heavensgate
