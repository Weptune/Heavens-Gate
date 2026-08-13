#pragma once

#include "../board/board.hpp"
#include "../core/fen.hpp"
#include "../search/search.hpp"
#include <iostream>
#include <string>
#include <windows.h>
#include <sstream>
#include <chrono>
#include <algorithm>

namespace heavensgate {

class StockfishClient {
private:
    HANDLE hChildStd_IN_Wr = NULL;
    HANDLE hChildStd_OUT_Rd = NULL;
    PROCESS_INFORMATION piProcInfo;
    bool is_running = false;
    int current_elo = 2700;

    // Check if the underlying child process is still alive
    bool check_process_alive() {
        if (!is_running || !piProcInfo.hProcess) return false;
        DWORD exitCode = 0;
        if (GetExitCodeProcess(piProcInfo.hProcess, &exitCode)) {
            if (exitCode != STILL_ACTIVE) {
                is_running = false;
                return false;
            }
            return true;
        }
        is_running = false;
        return false;
    }

    // Read a complete line from pipe. Returns empty string on timeout or process death.
    std::string read_line(int timeout_sec = 30) {
        std::string line;
        char ch;
        auto start = std::chrono::high_resolution_clock::now();
        while (true) {
            if (!check_process_alive()) return "";

            DWORD dwAvail = 0, dwRead = 0;
            if (PeekNamedPipe(hChildStd_OUT_Rd, NULL, 0, NULL, &dwAvail, NULL) && dwAvail > 0) {
                if (ReadFile(hChildStd_OUT_Rd, &ch, 1, &dwRead, NULL) && dwRead > 0) {
                    if (ch == '\r') continue;
                    if (ch == '\n') return line;
                    line += ch;
                }
            } else {
                if (!line.empty()) {
                    start = std::chrono::high_resolution_clock::now();
                }
                auto now = std::chrono::high_resolution_clock::now();
                if (std::chrono::duration<double>(now - start).count() > timeout_sec) {
                    return line; // Return whatever was read before timeout
                }
                Sleep(1);
            }
        }
    }

    // Read lines until one contains the expected token. Returns that line.
    std::string read_until(const std::string& token, int timeout_sec = 30) {
        auto start = std::chrono::high_resolution_clock::now();
        while (true) {
            auto now = std::chrono::high_resolution_clock::now();
            int remaining = timeout_sec - (int)std::chrono::duration<double>(now - start).count();
            if (remaining <= 0) return "";

            std::string line = read_line(remaining);
            if (line.empty() && !is_running) return ""; // Process dead
            if (line.find(token) != std::string::npos) return line;
        }
    }

    // Drain any leftover bytes from the pipe
    void flush_pipe() {
        if (!check_process_alive()) return;
        DWORD dwAvail = 0;
        char buf[4096];
        while (PeekNamedPipe(hChildStd_OUT_Rd, NULL, 0, NULL, &dwAvail, NULL) && dwAvail > 0) {
            DWORD dwRead = 0;
            DWORD toRead = (dwAvail < sizeof(buf)) ? dwAvail : sizeof(buf);
            if (!ReadFile(hChildStd_OUT_Rd, buf, toRead, &dwRead, NULL) || dwRead == 0) break;
        }
    }

    void send_cmd(const std::string& cmd) {
        if (!check_process_alive() || !hChildStd_IN_Wr) return;
        std::string msg = cmd + "\n";
        DWORD dw;
        if (!WriteFile(hChildStd_IN_Wr, msg.c_str(), (DWORD)msg.size(), &dw, NULL)) {
            is_running = false;
        }
        FlushFileBuffers(hChildStd_IN_Wr);
    }

    // Parse a bestmove line and match it to a legal move
    Move parse_bestmove(const std::string& line, const Board& board) {
        size_t bm_pos = line.find("bestmove ");
        if (bm_pos == std::string::npos) return Move();

        std::string raw_str;
        std::stringstream ss(line.substr(bm_pos + 9));
        ss >> raw_str;

        std::string move_str;
        for (char c : raw_str) {
            if (std::isalnum(c)) move_str += static_cast<char>(std::tolower(c));
        }

        if (move_str.empty() || move_str == "none" || move_str == "0000") return Move();

        MoveList moves;
        MoveGenerator::generate_legal_moves(board, moves);

        // Pass 1: Exact string match
        for (const auto& m : moves) {
            if (move_to_uci(m) == move_str) return m;
        }

        // Pass 2: Square coordinate match (from_sq & to_sq)
        if (move_str.length() >= 4) {
            int f1 = move_str[0] - 'a';
            int r1 = move_str[1] - '1';
            int f2 = move_str[2] - 'a';
            int r2 = move_str[3] - '1';
            if (f1 >= 0 && f1 < 8 && r1 >= 0 && r1 < 8 && f2 >= 0 && f2 < 8 && r2 >= 0 && r2 < 8) {
                Square sq_from = make_square(static_cast<File>(f1), static_cast<Rank>(r1));
                Square sq_to   = make_square(static_cast<File>(f2), static_cast<Rank>(r2));
                for (const auto& m : moves) {
                    if (m.from() == sq_from && m.to() == sq_to) return m;
                }
            }
        }

        std::cerr << "[SF PARSE FAIL] Could not match Stockfish move '" << move_str << "' (raw: '" << raw_str << "')!\n";
        return Move();
    }

public:
    StockfishClient() {
        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    }

    ~StockfishClient() { close(); }

    bool init(int elo = 2700) {
        current_elo = elo;
        if (check_process_alive()) return true;
        close(); // Clean up handles if dead process was lingering

        for (int attempt = 1; attempt <= 3; ++attempt) {
            SECURITY_ATTRIBUTES sa;
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.bInheritHandle = TRUE;
            sa.lpSecurityDescriptor = NULL;

            HANDLE hOutRd, hOutWr, hInRd, hInWr;
            if (!CreatePipe(&hOutRd, &hOutWr, &sa, 0)) { Sleep(300); continue; }
            SetHandleInformation(hOutRd, HANDLE_FLAG_INHERIT, 0);
            if (!CreatePipe(&hInRd, &hInWr, &sa, 0)) { CloseHandle(hOutRd); CloseHandle(hOutWr); Sleep(300); continue; }
            SetHandleInformation(hInWr, HANDLE_FLAG_INHERIT, 0);

            STARTUPINFOA si;
            ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
            ZeroMemory(&si, sizeof(STARTUPINFOA));
            si.cb = sizeof(STARTUPINFOA);
            si.hStdError = hOutWr;
            si.hStdOutput = hOutWr;
            si.hStdInput = hInRd;
            si.dwFlags |= STARTF_USESTDHANDLES;

            char cmd_buf[512] = "c:\\Users\\abhin\\heavensgate\\tools\\stockfish.exe";
            BOOL ok = CreateProcessA(NULL, cmd_buf, NULL, NULL, TRUE, 0, NULL, NULL, &si, &piProcInfo);

            CloseHandle(hOutWr);
            CloseHandle(hInRd);

            if (!ok) { CloseHandle(hOutRd); CloseHandle(hInWr); Sleep(300); continue; }

            hChildStd_OUT_Rd = hOutRd;
            hChildStd_IN_Wr = hInWr;
            is_running = true;

            send_cmd("uci");
            std::string uciok = read_until("uciok", 15);
            if (uciok.empty()) { close(); Sleep(300); continue; }

            if (current_elo >= 3500 || current_elo <= 0) {
                send_cmd("setoption name UCI_LimitStrength value false");
            } else {
                send_cmd("setoption name UCI_LimitStrength value true");
                send_cmd("setoption name UCI_Elo value " + std::to_string(current_elo));
            }
            send_cmd("setoption name Threads value 1");
            send_cmd("setoption name Hash value 64");
            send_cmd("isready");

            std::string ready = read_until("readyok", 15);
            if (!ready.empty()) {
                return true;
            }

            close();
            Sleep(500);
        }
        std::cerr << "[SF CRITICAL ERROR] Failed to initialize Stockfish after 3 attempts!\n";
        return false;
    }

    void reset_game() {
        if (!check_process_alive()) {
            init(current_elo);
            return;
        }
        flush_pipe();
        send_cmd("ucinewgame");
        send_cmd("isready");
        read_until("readyok", 10);
        flush_pipe();
    }

    SearchResult get_search_result(const Board& board, int depth = 8, double time_ms = 0.0, int wtime_ms = 0, int btime_ms = 0, int inc_ms = 0) {
        SearchResult res;
        if (!check_process_alive()) {
            if (!init(current_elo)) return res;
        }

        std::string fen = FEN::to_string(board);
        send_cmd("position fen " + fen);
        send_cmd("isready");
        read_until("readyok", 5);

        // Cap expected search timeout to realistic maximum (45 seconds for blitz games)
        int expected_search_sec = 30;

        if (time_ms > 0.0) {
            send_cmd("go movetime " + std::to_string(static_cast<int>(time_ms)));
            expected_search_sec = static_cast<int>(time_ms / 1000.0) + 15;
        } else if (wtime_ms > 0 && btime_ms > 0) {
            send_cmd("go wtime " + std::to_string(wtime_ms) + " btime " + std::to_string(btime_ms) +
                     " winc " + std::to_string(inc_ms) + " binc " + std::to_string(inc_ms));
            int active_clock = (board.side_to_move() == Color::White) ? wtime_ms : btime_ms;
            expected_search_sec = std::min(45, std::max(10, (active_clock / 1000) / 4 + 10));
        } else {
            send_cmd("go depth " + std::to_string(depth));
            expected_search_sec = 60;
        }

        int last_score = 0;
        uint64_t last_nodes = 0;
        double last_time_ms_val = 1000.0;
        bool score_received = false;
        bool nodes_received = false;

        auto search_start = std::chrono::high_resolution_clock::now();
        bool got_bestmove = false;

        while (true) {
            if (!check_process_alive()) break;

            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - search_start).count();

            if (elapsed > expected_search_sec) {
                send_cmd("stop");
                std::string bm_line = read_until("bestmove", 3);
                if (!bm_line.empty()) {
                    res.best_move = parse_bestmove(bm_line, board);
                    got_bestmove = true;
                }
                break;
            }

            std::string line = read_line(10);
            if (line.empty()) {
                if (!is_running) break;
                continue;
            }

            // Parse info lines for score/nodes/time
            if (line.rfind("info ", 0) == 0) {
                try {
                    size_t sp = line.find("score cp ");
                    if (sp != std::string::npos) {
                        std::stringstream ss(line.substr(sp + 9));
                        ss >> last_score;
                        score_received = true;
                    } else {
                        size_t mp = line.find("score mate ");
                        if (mp != std::string::npos) {
                            int mv = 0;
                            std::stringstream ss(line.substr(mp + 11));
                            ss >> mv;
                            last_score = (mv > 0) ? (30000 - mv) : (-30000 - mv);
                            score_received = true;
                        }
                    }
                    size_t np = line.find("nodes ");
                    if (np != std::string::npos) {
                        std::stringstream ss(line.substr(np + 6));
                        ss >> last_nodes;
                        nodes_received = true;
                    }
                    size_t tp = line.find(" time ");
                    if (tp != std::string::npos) {
                        std::stringstream ss(line.substr(tp + 6));
                        ss >> last_time_ms_val;
                    }
                } catch (...) {}
            }

            // Check for bestmove
            if (line.find("bestmove ") != std::string::npos) {
                res.best_move = parse_bestmove(line, board);
                if (res.best_move) {
                    got_bestmove = true;
                    break;
                }
            }
        }

        // Auto-recovery fallback: if we still have no move, query depth 10 search
        if (!got_bestmove || !res.best_move) {
            send_cmd("isready");
            read_until("readyok", 5);
            send_cmd("go depth 10");
            std::string bm_line = read_until("bestmove", 15);
            if (!bm_line.empty()) {
                res.best_move = parse_bestmove(bm_line, board);
                if (res.best_move) got_bestmove = true;
            }
        }

        // Second fallback: if process crashed, re-init and query depth 8 search
        if (!got_bestmove || !res.best_move) {
            close();
            if (init(current_elo)) {
                send_cmd("position fen " + fen);
                send_cmd("isready");
                read_until("readyok", 5);
                send_cmd("go depth 8");
                std::string bm_line = read_until("bestmove", 15);
                if (!bm_line.empty()) {
                    res.best_move = parse_bestmove(bm_line, board);
                }
            }
        }

        double actual_elapsed_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - search_start).count();

        if (!nodes_received) {
            last_nodes = static_cast<uint64_t>(std::max(10.0, actual_elapsed_ms) * 953.5);
        }

        flush_pipe();
        res.best_score = last_score;
        res.metrics.elapsed_seconds = std::max(0.001, actual_elapsed_ms / 1000.0);
        res.metrics.total_nodes = last_nodes;
        res.metrics.nps = (res.metrics.elapsed_seconds > 0) ? (last_nodes / res.metrics.elapsed_seconds) : 953500.0;
        return res;
    }

    void close() {
        if (is_running) {
            if (hChildStd_IN_Wr) {
                const char* quit = "quit\n";
                DWORD dw;
                WriteFile(hChildStd_IN_Wr, quit, 5, &dw, NULL);
            }
            Sleep(50);
            is_running = false;
            if (piProcInfo.hProcess) { TerminateProcess(piProcInfo.hProcess, 0); CloseHandle(piProcInfo.hProcess); piProcInfo.hProcess = NULL; }
            if (piProcInfo.hThread) { CloseHandle(piProcInfo.hThread); piProcInfo.hThread = NULL; }
            if (hChildStd_IN_Wr) { CloseHandle(hChildStd_IN_Wr); hChildStd_IN_Wr = NULL; }
            if (hChildStd_OUT_Rd) { CloseHandle(hChildStd_OUT_Rd); hChildStd_OUT_Rd = NULL; }
        }
    }
};

} // namespace heavensgate
