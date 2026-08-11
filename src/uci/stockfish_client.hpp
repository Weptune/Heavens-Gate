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
        if (!is_running) return Move();
        std::string fen = FEN::to_string(board);
        send_cmd("position fen " + fen);
        send_cmd("go depth " + std::to_string(depth));
        
        std::string current_line;
        char ch;
        DWORD dwRead;

        while (is_running) {
            if (ReadFile(hChildStd_OUT_Rd, &ch, 1, &dwRead, NULL) && dwRead > 0) {
                current_line += ch;
                if (ch == '\n') {
                    if (current_line.rfind("bestmove", 0) == 0) {
                        std::stringstream ss(current_line);
                        std::string tag, move_str;
                        ss >> tag >> move_str;

                        // Parse UCI move string into Board Move struct
                        MoveList moves;
                        MoveGenerator::generate_legal_moves(board, moves);
                        for (const auto& m : moves) {
                            if (move_to_uci(m) == move_str) {
                                return m;
                            }
                        }
                        if (!moves.empty()) return moves[0];
                    }
                    current_line.clear();
                }
            } else {
                break;
            }
        }
        return Move();
    }

    void close() {
        if (is_running) {
            send_cmd("quit");
            CloseHandle(piProcInfo.hProcess);
            CloseHandle(piProcInfo.hThread);
            CloseHandle(hChildStd_IN_Rd);
            CloseHandle(hChildStd_IN_Wr);
            CloseHandle(hChildStd_OUT_Rd);
            CloseHandle(hChildStd_OUT_Wr);
            is_running = false;
        }
    }
};

} // namespace heavensgate
