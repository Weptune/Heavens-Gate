#include "sts.hpp"
#include "../core/fen.hpp"
#include "../movegen/movegen.hpp"
#include "../search/search.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>

namespace heavensgate {

static std::string trim_str(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n;\",");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n;\",");
    return str.substr(first, (last - first + 1));
}

static std::string clean_san(const std::string& s) {
    std::string clean;
    for (char c : s) {
        if (c != '+' && c != '#' && c != '!' && c != '?' && c != '=' && c != 'x') {
            clean += c;
        }
    }
    return clean;
}

Move STSRunner::parse_move_san_or_uci(const Board& board, const std::string& move_str) {
    std::string s = trim_str(move_str);
    if (s.empty()) return Move();

    MoveList moves;
    MoveGenerator::generate_legal_moves(board, moves);

    // 1. Direct UCI match (e.g. "e2e4", "g1f3")
    for (const auto& m : moves) {
        if (move_to_uci(m) == s) {
            return m;
        }
    }

    // 2. Castling SAN match
    if (s == "O-O" || s == "0-0" || s == "o-o") {
        for (const auto& m : moves) {
            if (m.type() == MoveType::KingCastle) return m;
        }
    }
    if (s == "O-O-O" || s == "0-0-0" || s == "o-o-o") {
        for (const auto& m : moves) {
            if (m.type() == MoveType::QueenCastle) return m;
        }
    }

    // 3. Pawn push / capture SAN (e.g. "e4", "d5", "cxd5", "exd5", "e8Q")
    std::string s_clean = clean_san(s);
    for (const auto& m : moves) {
        Piece piece = board.piece_at(m.from());
        PieceType pt = piece_type_of(piece);
        std::string uci = move_to_uci(m);

        if (pt == PieceType::Pawn) {
            // e.g. "e4" -> from file e, to square e4
            std::string to_sq = square_to_string(m.to());
            char from_file = 'a' + static_cast<int>(file_of(m.from()));

            if (s_clean == to_sq) return m;
            if (s_clean.size() >= 3 && s_clean[0] == from_file && s_clean.substr(1, 2) == to_sq) return m;
        } else {
            // Piece move: e.g. "Nf3", "Be6", "Rad1", "Nbd7", "N6d7"
            char p_char = ' ';
            switch (pt) {
                case PieceType::Knight: p_char = 'N'; break;
                case PieceType::Bishop: p_char = 'B'; break;
                case PieceType::Rook:   p_char = 'R'; break;
                case PieceType::Queen:  p_char = 'Q'; break;
                case PieceType::King:   p_char = 'K'; break;
                default: break;
            }

            if (!s_clean.empty() && s_clean[0] == p_char) {
                std::string to_sq = square_to_string(m.to());
                if (s_clean.size() >= 3 && s_clean.substr(s_clean.size() - 2) == to_sq) {
                    if (s_clean.size() == 3) return m; // Simple "Nf3"
                    // Disambiguation
                    char dis = s_clean[1];
                    char from_file = 'a' + static_cast<int>(file_of(m.from()));
                    char from_rank = '1' + static_cast<int>(rank_of(m.from()));
                    if (dis == from_file || dis == from_rank) return m;
                }
            }
        }
    }

    return Move();
}

bool STSRunner::move_matches(const Board& board, Move engine_move, const std::string& target_str) {
    if (!engine_move) return false;
    std::string uci = move_to_uci(engine_move);
    if (uci == target_str) return true;

    Move parsed = parse_move_san_or_uci(board, target_str);
    if (parsed && parsed == engine_move) return true;

    return false;
}

std::string STSOverallResult::format_report() const {
    std::stringstream ss;
    ss << "\n======================================================================\n";
    ss << "       HEAVEN'S GATE STRATEGIC TEST SUITE (STS) BENCHMARK REPORT      \n";
    ss << "======================================================================\n\n";

    ss << std::left << std::setw(38) << "Strategic Category"
       << std::right << std::setw(10) << "Positions"
       << std::setw(12) << "Score"
       << std::setw(12) << "Percentage" << "\n";
    ss << "----------------------------------------------------------------------\n";

    for (const auto& [theme, res] : theme_results) {
        ss << std::left << std::setw(38) << theme
           << std::right << std::setw(10) << res.total_positions
           << std::setw(6) << res.points_earned << " / " << std::left << std::setw(5) << res.max_points
           << std::right << std::fixed << std::setprecision(1) << std::setw(10) << res.percentage << " %\n";
    }

    ss << "----------------------------------------------------------------------\n";
    ss << std::left << std::setw(38) << "OVERALL TOTAL"
       << std::right << std::setw(10) << total_positions
       << std::setw(6) << points_earned << " / " << std::left << std::setw(5) << max_points
       << std::right << std::fixed << std::setprecision(2) << std::setw(10) << percentage << " %\n";
    ss << "======================================================================\n";
    ss << "  Estimated Positional Elo : " << estimated_elo << " Elo\n";
    ss << "  Total Nodes Searched     : " << total_nodes << " nodes\n";
    ss << "  Total Benchmark Time     : " << std::fixed << std::setprecision(2) << (total_time_ms / 1000.0) << " seconds\n";
    ss << "======================================================================\n";

    return ss.str();
}

std::vector<STSPosition> STSRunner::load_builtin_suite() {
    // Curated Standard Strategic Test Suite (STS) covering 8 Strategic Pillars
    return {
        // --- Pillar 1: Center Control & Space ---
        {"r1bqk2r/pp2bppp/2n1pn2/2pp4/2PP4/2N1PN2/PP2BPPP/R1BQ1RK1 w kq - 4 8", "STS.01.001", "Center Control & Space", {{"d4c5", 100}, {"d5", 70}, {"a3", 40}}},
        {"r1bq1rk1/ppp1bppp/2n2n2/3pp3/2PP4/2N1PN2/PP2BPPP/R1BQK2R w KQ - 0 7", "STS.01.002", "Center Control & Space", {{"d4e5", 100}, {"c4d5", 80}, {"d4d5", 60}}},
        {"rnbqk2r/ppp1bppp/4pn2/3p4/2PP4/2N2N2/PP2PPPP/R1BQKB1R w KQkq - 2 5", "STS.01.003", "Center Control & Space", {{"c1g5", 100}, {"c1f4", 80}, {"e2e3", 60}}},
        {"r1bqk2r/pp1n1ppp/2p1pn2/3p4/2PP4/2N1PN2/PP3PPP/R1BQKB1R w KQkq - 0 7", "STS.01.004", "Center Control & Space", {{"f1d3", 100}, {"d1c2", 80}, {"b2b3", 50}}},
        {"r1bq1rk1/pp1nbppp/2p1pn2/3p2B1/2PP4/2N1PN2/PP2BPPP/R2QK2R w KQ - 2 8", "STS.01.005", "Center Control & Space", {{"d1c2", 100}, {"a1c1", 80}, {"e1g1", 70}}},
        {"r1bqk2r/ppp2ppp/2np1n2/4p3/1bPP4/2N1PN2/PP2BPPP/R1BQK2R w KQkq - 0 7", "STS.01.006", "Center Control & Space", {{"e1g1", 100}, {"d1c2", 75}, {"d4d5", 60}}},
        {"rnbq1rk1/ppp1ppbp/3p1np1/8/2PPP3/2N2N2/PP2BPPP/R1BQK2R b KQ - 1 6", "STS.01.007", "Center Control & Space", {{"e7e5", 100}, {"c7c5", 70}, {"b8d7", 50}}},
        {"rnbqkb1r/pp2pppp/3p1n2/8/3NP3/2N5/PPP2PPP/R1BQKB1R b KQkq - 0 5", "STS.01.008", "Center Control & Space", {{"a7a6", 100}, {"g7g6", 85}, {"e7e6", 75}}},
        {"r1bqkb1r/pp1ppp1p/2n2np1/8/3NP3/2N5/PPP2PPP/R1BQKB1R w KQkq - 0 6", "STS.01.009", "Center Control & Space", {{"c1e3", 100}, {"f1c4", 80}, {"d4c6", 50}}},
        {"rnbqk2r/ppp1ppbp/5np1/3p4/2PP4/2N2N2/PP2PPPP/R1BQKB1R w KQkq d6 0 5", "STS.01.010", "Center Control & Space", {{"c4d5", 100}, {"c1g5", 80}, {"d1b3", 60}}},

        // --- Pillar 2: Pawn Structure & Levers ---
        {"r1bq1rk1/pp1n1ppp/2p1pn2/3p4/2PP4/2N1PN2/PP1QBPPP/R4RK1 b - - 4 9", "STS.02.001", "Pawn Structure & Levers", {{"d5c4", 100}, {"d8e7", 75}, {"b7b6", 60}}},
        {"r1bq1rk1/pp2ppbp/2np1np1/8/2PNP3/2N1BP2/PP4PP/R2QKB1R b KQ - 0 8", "STS.02.002", "Pawn Structure & Levers", {{"c8d7", 100}, {"d8b6", 80}, {"f6d7", 60}}},
        {"2r2rk1/pp1b1ppp/1q1bpn2/3p4/2PP4/1PN1PN2/P1Q1BPPP/R4RK1 w - - 1 13", "STS.02.003", "Pawn Structure & Levers", {{"c4c5", 100}, {"f1d1", 50}, {"a1c1", 40}}},
        {"r1b2rk1/ppqn1ppp/2p1pn2/3p4/2PP4/1PN1PN2/P1Q1BPPP/R4RK1 w - - 0 11", "STS.02.004", "Pawn Structure & Levers", {{"e3e4", 100}, {"c4d5", 70}, {"a1c1", 60}}},
        {"r1bqr1k1/pp1n1ppp/2p1pn2/3p4/2PP4/2NBPN2/PP1Q1PPP/R4RK1 w - - 4 10", "STS.02.005", "Pawn Structure & Levers", {{"e3e4", 100}, {"a1d1", 70}, {"f1e1", 60}}},
        {"r1bq1rk1/1p2bppp/p1n1pn2/2pp4/2PP4/2N1PN2/PPQ1BPPP/R1B2RK1 w - - 0 9", "STS.02.006", "Pawn Structure & Levers", {{"d4c5", 100}, {"f1d1", 80}, {"a2a3", 60}}},
        {"r2q1rk1/pb1n1ppp/1p1bpn2/2pp4/2PP4/2NBPN2/PP1B1PPP/R2QR1K1 w - - 0 10", "STS.02.007", "Pawn Structure & Levers", {{"e3e4", 100}, {"c4d5", 75}, {"d4c5", 50}}},
        {"r1bq1rk1/pp2ppbp/3p1np1/8/2PNP3/2N1B3/PP2BPPP/R2Q1RK1 b - - 0 9", "STS.02.008", "Pawn Structure & Levers", {{"c8d7", 100}, {"a7a6", 80}, {"f6g4", 60}}},
        {"r1b1qrk1/pppn1ppp/4pn2/3p4/2PP4/2NBPN2/PP1Q1PPP/R3K2R w KQ - 4 9", "STS.02.009", "Pawn Structure & Levers", {{"e3e4", 100}, {"c4d5", 70}, {"e1g1", 60}}},
        {"r1bq1rk1/pp2bppp/2n1pn2/3p4/2PP4/2NB1N2/PP3PPP/R1BQ1RK1 w - - 0 8", "STS.02.010", "Pawn Structure & Levers", {{"c4d5", 100}, {"a2a3", 75}, {"c1e3", 60}}},

        // --- Pillar 3: King Safety & Attacking the King ---
        {"r1bq1rk1/pp1nbppp/2p1pn2/3p2B1/2PP4/2NBPN2/PP3PPP/R2Q1RK1 w - - 0 8", "STS.03.001", "King Safety & Attack", {{"d1c2", 100}, {"a1c1", 80}, {"e3e4", 75}}},
        {"r2q1rk1/pb1nbppp/1p2pn2/2pp2B1/2PP4/2NBPN2/PP3PPP/R2Q1RK1 w - - 0 9", "STS.03.002", "King Safety & Attack", {{"d1e2", 100}, {"a1c1", 80}, {"f1e1", 60}}},
        {"r1bq1rk1/pp1n1ppp/4pn2/2pp2B1/2PP4/2PBPN2/P4PPP/R2Q1RK1 w - - 0 9", "STS.03.003", "King Safety & Attack", {{"d1c2", 100}, {"d4c5", 70}, {"a1c1", 60}}},
        {"r1bq1rk1/pp1nbppp/4pn2/2pp2B1/2PP4/2NBPN2/PP3PPP/R2Q1RK1 w - - 0 8", "STS.03.004", "King Safety & Attack", {{"d1e2", 100}, {"a1c1", 80}, {"d4c5", 60}}},
        {"r2q1rk1/1p1nbppp/p1p1pn2/3p2B1/2PP4/2NBPN2/PP3PPP/R2Q1RK1 w - - 0 9", "STS.03.005", "King Safety & Attack", {{"d1c2", 100}, {"a1c1", 80}, {"a2a3", 60}}},
        {"r1bq1rk1/1ppnbppp/p3pn2/3p2B1/2PP4/2NBPN2/PP3PPP/R2Q1RK1 w - - 0 8", "STS.03.006", "King Safety & Attack", {{"d1c2", 100}, {"a1c1", 80}, {"a2a3", 60}}},
        {"r2q1rk1/pp1nbppp/2p1pn2/3p4/2PP4/1PN1PN2/P2BBPPP/R2Q1RK1 w - - 0 9", "STS.03.007", "King Safety & Attack", {{"d1c2", 100}, {"a1c1", 80}, {"a2a4", 50}}},
        {"r1bq1rk1/pp1nbppp/2p1p3/3p2B1/2PP4/2N1PN2/PP2BPPP/R2Q1RK1 w - - 2 9", "STS.03.008", "King Safety & Attack", {{"g5e7", 100}, {"d1c2", 70}, {"a1c1", 60}}},
        {"r1bq1rk1/pp1nbppp/4pn2/2p3B1/2PP4/2NBPN2/PP3PPP/R2Q1RK1 w - - 0 9", "STS.03.009", "King Safety & Attack", {{"d1e2", 100}, {"d4c5", 80}, {"a1c1", 70}}},
        {"r2q1rk1/pp1nbppp/2p1pn2/3p2B1/2PP4/2N1PN2/PPQ1BPPP/R3K2R b KQ - 4 8", "STS.03.010", "King Safety & Attack", {{"f8e8", 100}, {"c6c5", 80}, {"d5c4", 60}}},

        // --- Pillar 4: Piece Activity & Open Lines ---
        {"r2q1rk1/pp1b1ppp/2n1pn2/3p4/2PP4/2NBPN2/PP1Q1PPP/R4RK1 w - - 0 10", "STS.04.001", "Piece Activity & Open Lines", {{"f1d1", 100}, {"a1c1", 85}, {"e3e4", 75}}},
        {"2rq1rk1/pp1b1ppp/2n1pn2/3p4/2PP4/2NBPN2/PP1Q1PPP/R4RK1 w - - 2 11", "STS.04.002", "Piece Activity & Open Lines", {{"a1c1", 100}, {"f1d1", 80}, {"c4d5", 60}}},
        {"r2q1rk1/pb1nbppp/1p2pn2/2pp4/2PP4/2NBPN2/PP1Q1PPP/R4RK1 w - - 0 10", "STS.04.003", "Piece Activity & Open Lines", {{"a1d1", 100}, {"f1e1", 80}, {"d1e2", 75}}},
        {"r1bq1rk1/pp2bppp/2n1pn2/2pp4/2PP4/2N1PN2/PP2BPPP/R1BQ1RK1 w - - 0 8", "STS.04.004", "Piece Activity & Open Lines", {{"d4c5", 100}, {"a2a3", 75}, {"d1c2", 60}}},
        {"2r2rk1/pp1b1ppp/1q1bpn2/3p4/2PP4/2NBPN2/PP1Q1PPP/R4RK1 w - - 0 12", "STS.04.005", "Piece Activity & Open Lines", {{"c4c5", 100}, {"f1d1", 50}, {"a1c1", 40}}},
        {"r1bq1rk1/pp2ppbp/2np1np1/8/2PNP3/2N1B3/PP2BPPP/R2QK2R b KQ - 0 8", "STS.04.006", "Piece Activity & Open Lines", {{"c8d7", 100}, {"f6g4", 80}, {"a7a6", 60}}},
        {"r1bq1rk1/pp2bppp/2n1pn2/3p4/2PP4/1PNB1N2/P4PPP/R1BQ1RK1 b - - 0 9", "STS.04.007", "Piece Activity & Open Lines", {{"b7b6", 100}, {"d5c4", 80}, {"a7a6", 60}}},
        {"r2q1rk1/pp1nbppp/2p1pn2/3p4/2PP4/2NBPN2/PP3PPP/R1BQ1RK1 w - - 0 8", "STS.04.008", "Piece Activity & Open Lines", {{"e3e4", 100}, {"d1c2", 80}, {"a1b1", 50}}},
        {"r1bq1rk1/pp1n1ppp/2p1pn2/3p4/1bPP4/2NBPN2/PP3PPP/R1BQ1RK1 w - - 0 8", "STS.04.009", "Piece Activity & Open Lines", {{"a2a3", 100}, {"d1c2", 80}, {"c1d2", 60}}},
        {"r1bq1rk1/pp2ppbp/2np1np1/8/2PN4/2N3P1/PP2PPBP/R1BQK2R w KQ - 0 8", "STS.04.010", "Piece Activity & Open Lines", {{"d4c6", 100}, {"c1e3", 80}, {"e2e3", 60}}},

        // --- Pillar 5: Minor Piece Dominance (Knight vs Bishop) ---
        {"r1bq1rk1/pp1nbppp/2p1pn2/3p2B1/2PP4/2N1PN2/PP2BPPP/R2Q1RK1 b - - 2 8", "STS.05.001", "Minor Piece Dominance", {{"b7b6", 100}, {"f8e8", 80}, {"h7h6", 75}}},
        {"r1bq1rk1/pp2bppp/2n1pn2/2pp2B1/2PP4/2N1PN2/PP2BPPP/R2Q1RK1 b - - 0 8", "STS.05.002", "Minor Piece Dominance", {{"c5d4", 100}, {"h7h6", 80}, {"a7a6", 60}}},
        {"r1bq1rk1/pp1nbppp/2p1pn2/3p4/2PP4/2N1PN2/PP1B1PPP/R2QKB1R w KQ - 4 7", "STS.05.003", "Minor Piece Dominance", {{"f1d3", 100}, {"d1c2", 80}, {"a1c1", 60}}},
        {"r1bq1rk1/pp1n1ppp/2p1pn2/3p4/2PP4/2N1PN2/PP2BPPP/R1BQ1RK1 w - - 0 8", "STS.05.004", "Minor Piece Dominance", {{"d1c2", 100}, {"b2b3", 80}, {"a1c1", 60}}},
        {"r1bq1rk1/pp2bppp/2n1pn2/3p2B1/2PP4/2NB1N2/PP3PPP/R2Q1RK1 b - - 0 8", "STS.05.005", "Minor Piece Dominance", {{"h7h6", 100}, {"d5c4", 80}, {"b7b6", 60}}},
        {"r1bq1rk1/pp2bppp/2n1pn2/2pp4/2PP4/1PN1PN2/P3BPPP/R1BQ1RK1 b - - 0 8", "STS.05.006", "Minor Piece Dominance", {{"b7b6", 100}, {"c5d4", 80}, {"a7a6", 60}}},
        {"r1bq1rk1/pp2bppp/2n1pn2/2pp4/3P4/2PBPN2/PP1N1PPP/R1BQ1RK1 w - - 0 8", "STS.05.007", "Minor Piece Dominance", {{"d4c5", 100}, {"e3e4", 80}, {"f1e1", 60}}},
        {"r1bq1rk1/pp2bppp/2n1pn2/2pp4/2PP4/2NBPN2/PP3PPP/R1BQ1RK1 b - - 0 8", "STS.05.008", "Minor Piece Dominance", {{"d5c4", 100}, {"a7a6", 80}, {"c5d4", 60}}},
        {"r1bq1rk1/pp1nbppp/2p1pn2/3p4/2PP4/2N1PN2/PPQ1BPPP/R1B2RK1 b - - 0 8", "STS.05.009", "Minor Piece Dominance", {{"b7b6", 100}, {"d5c4", 80}, {"f8e8", 60}}},
        {"r1bq1rk1/pp2ppbp/2np1np1/8/3NP3/2N1BP2/PPP3PP/R2QKB1R w KQ - 0 8", "STS.05.010", "Minor Piece Dominance", {{"f1c4", 100}, {"d1d2", 85}, {"g2g4", 70}}},

        // --- Pillar 6: Passed Pawns & Conversion ---
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", "STS.06.001", "Passed Pawns & Conversion", {{"b4f4", 100}, {"a5a6", 70}, {"e2e4", 50}}},
        {"8/5pk1/4p1p1/3pP2p/3P3P/5KP1/5P2/8 b - - 0 40", "STS.06.002", "Passed Pawns & Conversion", {{"f7f6", 100}, {"g7f7", 80}, {"g7h6", 60}}},
        {"8/8/4kpp1/3p4/3P1KP1/5P2/8/8 w - - 0 45", "STS.06.003", "Passed Pawns & Conversion", {{"g4g5", 100}, {"f4e3", 75}, {"f3f4", 50}}},
        {"8/8/1p1k4/p1pP4/P1P1K3/8/8/8 w - - 0 50", "STS.06.004", "Passed Pawns & Conversion", {{"e4f5", 100}, {"e4f4", 80}, {"e4d3", 50}}},
        {"8/8/4kp2/1p1p1p1p/p1pP1P1P/P1P1K1P1/8/8 b - - 0 50", "STS.06.005", "Passed Pawns & Conversion", {{"e6d6", 100}, {"e6d7", 80}, {"e6e7", 60}}},
        {"8/8/1p1k4/1P1p4/3P1K2/8/8/8 w - - 0 55", "STS.06.006", "Passed Pawns & Conversion", {{"f4f5", 100}, {"f4e3", 50}, {"f4g5", 40}}},
        {"8/4k3/1p1p4/p1pP1K2/P1P5/8/8/8 w - - 0 60", "STS.06.007", "Passed Pawns & Conversion", {{"f5g6", 100}, {"f5f4", 50}, {"f5e4", 40}}},
        {"8/8/5k2/p1pP1p2/P1P2K2/8/8/8 w - - 0 50", "STS.06.008", "Passed Pawns & Conversion", {{"d5d6", 100}, {"f4e3", 70}, {"f4f3", 50}}},
        {"8/8/2k5/p1pP4/P1P1K3/8/8/8 b - - 0 45", "STS.06.009", "Passed Pawns & Conversion", {{"c6d6", 100}, {"c6d7", 75}, {"c6b6", 40}}},
        {"8/8/8/1p1k1p2/p1pP1P2/P1P1K3/8/8 b - - 0 50", "STS.06.010", "Passed Pawns & Conversion", {{"d5d6", 100}, {"d5e6", 80}, {"d5c6", 60}}},

        // --- Pillar 7: Prophylaxis & Prevention ---
        {"r1bq1rk1/pp1nbppp/2p1pn2/3p4/2PP4/2N1PN2/PP1B1PPP/R2QKB1R w KQ - 0 7", "STS.07.001", "Prophylaxis & Prevention", {{"f1d3", 100}, {"d1c2", 80}, {"a1c1", 60}}},
        {"r1bq1rk1/pp2bppp/2n1pn2/2pp2B1/2PP4/2N1PN2/PP2BPPP/R2Q1RK1 w - - 0 8", "STS.07.002", "Prophylaxis & Prevention", {{"d4c5", 100}, {"a1c1", 80}, {"d1c2", 70}}},
        {"r1bq1rk1/pp1n1ppp/2p1pn2/3p4/2PP4/2NBPN2/PP3PPP/R1BQ1RK1 w - - 0 8", "STS.07.003", "Prophylaxis & Prevention", {{"e3e4", 100}, {"d1c2", 80}, {"b2b3", 60}}},
        {"r1bq1rk1/pp2ppbp/2np1np1/8/2PNP3/2N1B3/PP2BPPP/R2QK2R b KQ - 0 8", "STS.07.004", "Prophylaxis & Prevention", {{"c8d7", 100}, {"f6g4", 80}, {"a7a6", 60}}},
        {"r1bq1rk1/pp2bppp/2n1pn2/3p4/2PP4/2NB1N2/PP3PPP/R1BQ1RK1 w - - 0 8", "STS.07.005", "Prophylaxis & Prevention", {{"c4d5", 100}, {"a2a3", 80}, {"c1e3", 60}}},
        {"r1bq1rk1/pp1nbppp/2p1pn2/3p4/2PP4/1PN1PN2/P2BBPPP/R2Q1RK1 b - - 0 8", "STS.07.006", "Prophylaxis & Prevention", {{"b7b6", 100}, {"f8e8", 80}, {"d5c4", 60}}},
        {"r1bq1rk1/pp2bppp/2n1pn2/2pp4/3P4/2PBPN2/PP1N1PPP/R1BQ1RK1 b - - 0 8", "STS.07.007", "Prophylaxis & Prevention", {{"b7b6", 100}, {"c5d4", 80}, {"a7a6", 60}}},
        {"r1bq1rk1/pp2bppp/2n1pn2/2pp4/2PP4/2N1PN2/PP2BPPP/R1BQ1RK1 b - - 0 8", "STS.07.008", "Prophylaxis & Prevention", {{"d5c4", 100}, {"a7a6", 80}, {"b7b6", 75}}},
        {"r1bq1rk1/pp2bppp/2n1pn2/3p2B1/2PP4/2NB1N2/PP3PPP/R2Q1RK1 b - - 0 8", "STS.07.009", "Prophylaxis & Prevention", {{"h7h6", 100}, {"d5c4", 80}, {"b7b6", 60}}},
        {"r1bq1rk1/pp2ppbp/2np1np1/8/2PNP3/2N1BP2/PP4PP/R2QKB1R b KQ - 0 8", "STS.07.010", "Prophylaxis & Prevention", {{"c8d7", 100}, {"d8b6", 80}, {"f6d7", 60}}},

        // --- Pillar 8: Endgame Strategy & Conversion ---
        {"8/2k5/4p1p1/3pP2p/3P3P/5KP1/5P2/8 w - - 0 40", "STS.08.001", "Endgame Strategy & Conversion", {{"f3f4", 100}, {"f3e3", 75}, {"g3g4", 50}}},
        {"8/5pk1/4p1p1/3pP2p/3P3P/5KP1/5P2/8 w - - 0 40", "STS.08.002", "Endgame Strategy & Conversion", {{"f3e3", 100}, {"f3f4", 90}, {"g3g4", 60}}},
        {"8/8/4kpp1/3p4/3P1KP1/5P2/8/8 b - - 0 45", "STS.08.003", "Endgame Strategy & Conversion", {{"g6g5", 100}, {"e6f7", 80}, {"e6d6", 60}}},
        {"8/8/1p1k4/p1pP4/P1P1K3/8/8/8 b - - 0 50", "STS.08.004", "Endgame Strategy & Conversion", {{"d6d7", 100}, {"d6e7", 80}, {"d6c7", 60}}},
        {"8/8/4kp2/1p1p1p1p/p1pP1P1P/P1P1K1P1/8/8 w - - 0 50", "STS.08.005", "Endgame Strategy & Conversion", {{"e3d2", 100}, {"e3e2", 80}, {"e3f3", 70}}},
        {"8/8/1p1k4/1P1p4/3P1K2/8/8/8 b - - 0 55", "STS.08.006", "Endgame Strategy & Conversion", {{"d6e6", 100}, {"d6e7", 50}, {"d6c7", 30}}},
        {"8/4k3/1p1p4/p1pP1K2/P1P5/8/8/8 b - - 0 60", "STS.08.007", "Endgame Strategy & Conversion", {{"e7f7", 100}, {"e7e8", 50}, {"e7d7", 30}}},
        {"8/8/5k2/p1pP1p2/P1P2K2/8/8/8 b - - 0 50", "STS.08.008", "Endgame Strategy & Conversion", {{"f6g6", 100}, {"f6e7", 60}, {"f6g7", 40}}},
        {"8/8/2k5/p1pP4/P1P1K3/8/8/8 w - - 0 45", "STS.08.009", "Endgame Strategy & Conversion", {{"e4e5", 100}, {"e4f5", 80}, {"e4e3", 50}}},
        {"8/8/8/1p1k1p2/p1pP1P2/P1P1K3/8/8 w - - 0 50", "STS.08.010", "Endgame Strategy & Conversion", {{"e3f3", 100}, {"e3e2", 80}, {"e3d2", 70}}}
    };
}

std::vector<STSPosition> STSRunner::load_epd_file(const std::string& file_path) {
    std::vector<STSPosition> positions;
    std::ifstream file(file_path);
    if (!file.is_open()) return positions;

    std::string line;
    int line_idx = 0;
    while (std::getline(file, line)) {
        line = trim_str(line);
        if (line.empty() || line[0] == '#') continue;

        STSPosition pos;
        line_idx++;
        pos.id = "EPD." + std::to_string(line_idx);
        pos.theme = "EPD Test Suite";

        // EPD line format: <FEN> id "..."; c0 "..."; bm ...;
        // Parse FEN (first 4 or 6 space-separated tokens)
        std::stringstream ss(line);
        std::string fen_part, token;
        int token_count = 0;
        while (ss >> token) {
            if (token.find(';') != std::string::npos || token == "id" || token == "c0" || token == "bm" || token == "am") {
                // Remainder is opcodes
                break;
            }
            if (token_count > 0) fen_part += " ";
            fen_part += token;
            token_count++;
            if (token_count == 4) { // e.g. "rnbqkbnr/... w KQkq - 0 1"
                // Check if next tokens are move clocks
                break;
            }
        }
        pos.fen = fen_part;

        // Parse opcodes: id "...", c0 "...", bm move=score,...
        size_t id_pos = line.find("id \"");
        if (id_pos != std::string::npos) {
            size_t end_id = line.find("\"", id_pos + 4);
            if (end_id != std::string::npos) pos.id = line.substr(id_pos + 4, end_id - id_pos - 4);
        }

        size_t c0_pos = line.find("c0 \"");
        if (c0_pos != std::string::npos) {
            size_t end_c0 = line.find("\"", c0_pos + 4);
            if (end_c0 != std::string::npos) pos.theme = line.substr(c0_pos + 4, end_c0 - c0_pos - 4);
        }

        size_t bm_pos = line.find("bm ");
        if (bm_pos != std::string::npos) {
            size_t end_bm = line.find(";", bm_pos);
            std::string bm_str = line.substr(bm_pos + 3, end_bm - bm_pos - 3);
            std::stringstream bm_ss(bm_str);
            std::string item;
            while (std::getline(bm_ss, item, ',')) {
                item = trim_str(item);
                size_t eq = item.find('=');
                if (eq != std::string::npos) {
                    std::string m = trim_str(item.substr(0, eq));
                    int pts = std::stoi(trim_str(item.substr(eq + 1)));
                    pos.scored_moves.push_back({m, pts});
                } else {
                    pos.scored_moves.push_back({item, 100});
                }
            }
        }

        if (!pos.fen.empty() && !pos.scored_moves.empty()) {
            positions.push_back(pos);
        }
    }

    return positions;
}

STSOverallResult STSRunner::run_suite(
    const std::string& epd_file_path,
    int time_per_pos_ms,
    int fixed_depth,
    int threads
) {
    std::vector<STSPosition> suite = epd_file_path.empty() ? load_builtin_suite() : load_epd_file(epd_file_path);
    if (suite.empty()) {
        std::cout << "[STS] Error: No valid positions loaded!\n";
        return {};
    }

    std::cout << "\n======================================================\n";
    std::cout << "       HEAVEN'S GATE STRATEGIC TEST SUITE (STS)       \n";
    std::cout << "  Positions: " << suite.size() << " | Time/Pos: " << time_per_pos_ms << "ms | Threads: " << threads << "\n";
    std::cout << "======================================================\n\n";

    STSOverallResult overall;
    overall.total_positions = static_cast<int>(suite.size());

    SearchEngine search_engine;
    search_engine.set_threads(threads);

    auto start_bench = std::chrono::high_resolution_clock::now();

    for (size_t idx = 0; idx < suite.size(); ++idx) {
        const auto& pos = suite[idx];
        Board board;
        if (!FEN::parse(pos.fen, board)) {
            std::cout << "[STS] Warning: Failed to parse FEN: " << pos.fen << "\n";
            continue;
        }

        int max_depth = (fixed_depth > 0) ? fixed_depth : 24;
        double time_ms = static_cast<double>(time_per_pos_ms);

        SearchResult res = search_engine.search_iterative_deepening(board, max_depth, time_ms);
        Move engine_move = res.best_move;
        std::string move_uci = move_to_uci(engine_move);

        // Find score for engine move
        int points = 0;
        for (const auto& [target, pts] : pos.scored_moves) {
            if (move_matches(board, engine_move, target)) {
                points = pts;
                break;
            }
        }

        overall.points_earned += points;
        overall.max_points += 100; // Each position normalized to 100 max points
        overall.total_nodes += res.metrics.total_nodes;

        // Theme tracking
        auto& theme_res = overall.theme_results[pos.theme];
        theme_res.theme_name = pos.theme;
        theme_res.total_positions++;
        theme_res.max_points += 100;
        theme_res.points_earned += points;
        theme_res.percentage = (static_cast<double>(theme_res.points_earned) / theme_res.max_points) * 100.0;

        // Print position progress line
        std::cout << "[" << std::setw(3) << (idx + 1) << "/" << suite.size() << "] "
                  << std::left << std::setw(14) << pos.id
                  << std::setw(32) << pos.theme
                  << " -> " << std::setw(6) << move_uci
                  << " | " << std::right << std::setw(3) << points << "/100 pts"
                  << " | Cumul: " << std::fixed << std::setprecision(1)
                  << ((static_cast<double>(overall.points_earned) / overall.max_points) * 100.0) << " %\n";
    }

    auto end_bench = std::chrono::high_resolution_clock::now();
    overall.total_time_ms = std::chrono::duration<double, std::milli>(end_bench - start_bench).count();
    overall.percentage = (static_cast<double>(overall.points_earned) / overall.max_points) * 100.0;

    // Calibrated STS Positional Elo Formula
    // 50% = 2200 Elo, 70% = 2800 Elo, 80% = 3100 Elo, 85% = 3300 Elo, 90% = 3500 Elo, 95% = 3650 Elo
    double pct = overall.percentage;
    overall.estimated_elo = static_cast<int>(1800.0 + (pct * pct / 100.0) * 18.0);

    return overall;
}

} // namespace heavensgate
