#pragma once

#include "../board/board.hpp"
#include "../core/fen.hpp"
#include <iostream>
#include <string>
#include <windows.h>
#include <sstream>

namespace heavensgate {

class StockfishClient {
private:
    HANDLE hChildStd_IN_Rd = NULL;
    HANDLE hChildStd_IN_Wr = NULL;
    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;
    PROCESS_INFORMATION piProcInfo;
    bool is_running = false;

public:
    ~StockfishClient() {
        close();
    }

    bool init(int elo = 2400) {
        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0)) return false;
        if (!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) return false;

        if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0)) return false;
        if (!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) return false;

        STARTUPINFOA siStartInfo;
        ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&siStartInfo, sizeof(STARTUPINFOA));
        siStartInfo.cb = sizeof(STARTUPINFOA);
        siStartInfo.hStdError = hChildStd_OUT_Wr;
        siStartInfo.hStdOutput = hChildStd_OUT_Wr;
        siStartInfo.hStdInput = hChildStd_IN_Rd;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

        std::string cmd = "tools\\stockfish.exe";
        if (GetFileAttributesA(cmd.c_str()) == INVALID_FILE_ATTRIBUTES) {
            cmd = "c:\\Users\\abhin\\heavensgate\\tools\\stockfish.exe";
        }
        BOOL bSuccess = CreateProcessA(
            NULL,
            cmd.data(),
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &siStartInfo,
            &piProcInfo
        );

        if (!bSuccess) return false;
        is_running = true;

        send_cmd("uci");
        send_cmd("setoption name UCI_LimitStrength value true");
        send_cmd("setoption name UCI_Elo value " + std::to_string(elo));
        send_cmd("isready");

        wait_for("readyok");
        return true;
    }

    bool is_available() const {
        return is_running;
    }

    void send_cmd(const std::string& cmd) {
        if (!is_running) return;
        std::string full_cmd = cmd + "\n";
        DWORD dwWritten;
        WriteFile(hChildStd_IN_Wr, full_cmd.c_str(), static_cast<DWORD>(full_cmd.length()), &dwWritten, NULL);
    }

    std::string wait_for(const std::string& expected_prefix) {
        std::string buffer;
        char ch;
        DWORD dwRead;

        while (is_running) {
            if (ReadFile(hChildStd_OUT_Rd, &ch, 1, &dwRead, NULL) && dwRead > 0) {
                buffer += ch;
                if (ch == '\n') {
                    if (buffer.find(expected_prefix) != std::string::npos) {
                        return buffer;
                    }
                    buffer.clear();
                }
            } else {
                break;
            }
        }
        return "";
    }

    Move get_bestmove(const Board& board, int depth = 8) {
        auto res = get_search_result(board, depth);
        return res.best_move;
    }

    SearchResult get_search_result(const Board& board, int depth = 8, double time_ms = 0.0, int wtime_ms = 0, int btime_ms = 0, int inc_ms = 0) {
        SearchResult res;
        if (!is_running) {
            if (!init(2400)) return res;
        }

        std::string fen = FEN::to_string(board);
        send_cmd("position fen " + fen);
        if (wtime_ms > 0 && btime_ms > 0) {
            send_cmd("go wtime " + std::to_string(wtime_ms) + " btime " + std::to_string(btime_ms) + " winc " + std::to_string(inc_ms) + " binc " + std::to_string(inc_ms));
        } else if (time_ms > 0.0) {
            send_cmd("go movetime " + std::to_string(static_cast<int>(time_ms)));
        } else {
            send_cmd("go depth " + std::to_string(depth));
        }
        
        std::string current_line;
        char ch;
        DWORD dwRead;
        int last_score = 0;
        uint64_t last_nodes = 50000;
        double last_time_ms = (time_ms > 0.0) ? time_ms : 100.0;

        while (is_running) {
            if (ReadFile(hChildStd_OUT_Rd, &ch, 1, &dwRead, NULL) && dwRead > 0) {
                current_line += ch;
                if (ch == '\n') {
                    if (current_line.rfind("info ", 0) == 0) {
                        try {
                            size_t score_pos = current_line.find("score cp ");
                            if (score_pos != std::string::npos) {
                                last_score = std::stoi(current_line.substr(score_pos + 9));
                            } else {
                                size_t mate_pos = current_line.find("score mate ");
                                if (mate_pos != std::string::npos) {
                                    int m_val = std::stoi(current_line.substr(mate_pos + 11));
                                    last_score = (m_val > 0) ? (30000 - m_val) : (-30000 - m_val);
                                }
                            }

                            size_t nodes_pos = current_line.find("nodes ");
                            if (nodes_pos != std::string::npos) {
                                last_nodes = std::stoull(current_line.substr(nodes_pos + 6));
                            }

                            size_t time_pos = current_line.find("time ");
                            if (time_pos != std::string::npos) {
                                last_time_ms = std::stod(current_line.substr(time_pos + 5));
                            }
                        } catch (...) {}
                    }

                    if (current_line.rfind("bestmove", 0) == 0) {
                        try {
                            std::stringstream ss(current_line);
                            std::string tag, move_str;
                            ss >> tag >> move_str;

                            MoveList moves;
                            MoveGenerator::generate_legal_moves(board, moves);
                            for (const auto& m : moves) {
                                if (move_to_uci(m) == move_str) {
                                    res.best_move = m;
                                    break;
                                }
                            }
                            if (!res.best_move && !moves.empty()) res.best_move = moves[0];
                        } catch (...) {
                            MoveList moves;
                            MoveGenerator::generate_legal_moves(board, moves);
                            if (!moves.empty()) res.best_move = moves[0];
                        }

                        res.best_score = last_score;
                        res.metrics.elapsed_seconds = std::max(0.001, last_time_ms / 1000.0);
                        res.metrics.total_nodes = last_nodes;
                        res.metrics.nps = (res.metrics.elapsed_seconds > 0) ? (last_nodes / res.metrics.elapsed_seconds) : 1000000.0;
                        return res;
                    }
                    current_line.clear();
                }
            } else {
                close();
                break;
            }
        }

        MoveList moves;
        MoveGenerator::generate_legal_moves(board, moves);
        if (!moves.empty()) res.best_move = moves[0];

        return res;
    }

    void close() {
        if (is_running) {
            is_running = false;
            send_cmd("quit");
            if (piProcInfo.hProcess) { CloseHandle(piProcInfo.hProcess); piProcInfo.hProcess = NULL; }
            if (piProcInfo.hThread) { CloseHandle(piProcInfo.hThread); piProcInfo.hThread = NULL; }
            if (hChildStd_IN_Rd) { CloseHandle(hChildStd_IN_Rd); hChildStd_IN_Rd = NULL; }
            if (hChildStd_IN_Wr) { CloseHandle(hChildStd_IN_Wr); hChildStd_IN_Wr = NULL; }
            if (hChildStd_OUT_Rd) { CloseHandle(hChildStd_OUT_Rd); hChildStd_OUT_Rd = NULL; }
            if (hChildStd_OUT_Wr) { CloseHandle(hChildStd_OUT_Wr); hChildStd_OUT_Wr = NULL; }
        }
    }
};

} // namespace heavensgate
