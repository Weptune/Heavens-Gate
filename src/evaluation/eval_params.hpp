#pragma once

#include <array>
#include <vector>
#include <string>

namespace heavensgate {

struct EvalParams {
    // 1. Base Material (Pawn MG is anchored at 100)
    int pawn_mg = 100;
    int pawn_eg = 120;
    int knight_mg = 320;
    int knight_eg = 310;
    int bishop_mg = 330;
    int bishop_eg = 340;
    int rook_mg = 500;
    int rook_eg = 530;
    int queen_mg = 900;
    int queen_eg = 950;

    // 2. Pawn Structure
    int doubled_mg = 14;
    int doubled_eg = 24;
    int connected_mg = 10;
    int connected_eg = 16;
    int phalanx_mg = 12;
    int phalanx_eg = 18;
    int isolated_mg = 16;
    int isolated_eg = 26;
    int backward_mg = 12;
    int backward_eg = 22;
    int central_lever_mg = 16;
    int central_lever_eg = 10;
    int pawn_tension_mg = 8;
    int pawn_tension_eg = 5;

    // 3. Passed Pawns by Relative Rank (Ranks 2..7)
    std::array<int, 6> passed_mg = { 5, 14, 28, 48, 82, 135 };
    std::array<int, 6> passed_eg = { 14, 30, 60, 105, 175, 275 };
    int protected_passed_mg = 20;
    int protected_passed_eg = 40;

    // 4. Piece Activity & Outposts
    int minor_undeveloped_mg = 15;
    int bishop_pair_mg = 32;
    int bishop_pair_eg = 52;
    int knight_outpost_mg = 28;
    int knight_outpost_eg = 38;
    int central_knight_outpost_mg = 12;
    int central_knight_outpost_eg = 14;
    int bishop_outpost_mg = 20;
    int bishop_outpost_eg = 28;
    int rook_7th_mg = 25;
    int rook_7th_eg = 40;
    int rook_open_mg = 20;
    int rook_open_eg = 25;
    int rook_semi_open_mg = 10;
    int rook_semi_open_eg = 15;

    // 5. Tactical Threats
    int minor_threat_rook_mg = 35;
    int minor_threat_rook_eg = 45;
    int minor_threat_queen_mg = 48;
    int minor_threat_queen_eg = 62;
    int rook_threat_queen_mg = 30;
    int rook_threat_queen_eg = 35;
    int pawn_push_threat_mg = 16;
    int pawn_push_threat_eg = 22;
    int pawn_attack_threat_mg = 22;
    int pawn_attack_threat_eg = 28;

    // 6. Safe Mobility Slopes
    int knight_mob_mg = 4;
    int knight_mob_eg = 4;
    int bishop_mob_mg = 4;
    int bishop_mob_eg = 4;
    int rook_mob_mg = 2;
    int rook_mob_eg = 3;
    int queen_mob_mg = 1;
    int queen_mob_eg = 2;

    // 7. King Safety & Tempo
    int pawn_shield_mg = 15;
    int uncastled_king_mg = 120;
    int uncastled_king_eg = 20;
    int tempo_mg = 18;
    int tempo_eg = 4;

    // 8. Eval v2 terms. Every term below enters the evaluation linearly in its
    // parameter (a count times a weight, or a table entry selected by a count),
    // so the HCE tuner can extract exact coefficients per position.
    std::array<int, 9>  knight_mob_tab_mg{}, knight_mob_tab_eg{};
    std::array<int, 14> bishop_mob_tab_mg{}, bishop_mob_tab_eg{};
    std::array<int, 15> rook_mob_tab_mg{}, rook_mob_tab_eg{};
    std::array<int, 28> queen_mob_tab_mg{}, queen_mob_tab_eg{};
    // Indexed by weighted king-zone attackers (N3 B3 R5 Q8), applied with >= 2 attackers.
    std::array<int, 32> king_danger_mg{}, king_danger_eg{};
    std::array<int, 3> pawn_storm_mg = {25, 55, 105};          // Enemy pawn 3, 2, 1 ranks from our shield.
    int king_open_file_mg = 50, king_open_file_eg = 15;
    int king_semi_open_file_mg = 32, king_semi_open_file_eg = 10;
    int trapped_bishop_deep_mg = 70, trapped_bishop_deep_eg = 110;
    int trapped_bishop_mg = 60, trapped_bishop_eg = 90;
    int trapped_rook_mg = 45;
    int pawn_threat_minor_mg = 22, pawn_threat_minor_eg = 28;  // Our pawn attacks an enemy minor.
    int pawn_threat_major_mg = 22, pawn_threat_major_eg = 28;  // Our pawn attacks an enemy rook or queen.
    int hanging_mg = 0, hanging_eg = 0;                        // Enemy non-pawn attacked by us, undefended.
    int safe_check_knight_mg = 0, safe_check_knight_eg = 0;    // Squares from which the enemy can give a safe check.
    int safe_check_bishop_mg = 0, safe_check_bishop_eg = 0;
    int safe_check_rook_mg = 0, safe_check_rook_eg = 0;
    int safe_check_queen_mg = 0, safe_check_queen_eg = 0;
    std::array<int, 6> passed_blocked_mg{}, passed_blocked_eg{};       // Stop square occupied, by relative rank.
    std::array<int, 6> passed_own_king_mg{}, passed_own_king_eg{};     // Per square of own-king distance.
    std::array<int, 6> passed_opp_king_mg{}, passed_opp_king_eg{};     // Per square of enemy-king distance.

    // Seed the v2 tables from the scalar weights they replace (used only before a tune).
    void init_v2_tables() {
        for (size_t c = 0; c < knight_mob_tab_mg.size(); ++c) { knight_mob_tab_mg[c] = int(c) * knight_mob_mg; knight_mob_tab_eg[c] = int(c) * knight_mob_eg; }
        for (size_t c = 0; c < bishop_mob_tab_mg.size(); ++c) { bishop_mob_tab_mg[c] = int(c) * bishop_mob_mg; bishop_mob_tab_eg[c] = int(c) * bishop_mob_eg; }
        for (size_t c = 0; c < rook_mob_tab_mg.size(); ++c) { rook_mob_tab_mg[c] = int(c) * rook_mob_mg; rook_mob_tab_eg[c] = int(c) * rook_mob_eg; }
        for (size_t c = 0; c < queen_mob_tab_mg.size(); ++c) { queen_mob_tab_mg[c] = int(c) * queen_mob_mg; queen_mob_tab_eg[c] = int(c) * queen_mob_eg; }
        // King danger starts at zero: configurations absent from the data add nothing.
    }

    struct ParamRef {
        std::string name;
        int* ptr;
        int min_val;
        int max_val;
    };

    std::vector<ParamRef> get_tunable_params() {
        std::vector<ParamRef> refs = {
            {"pawn_eg", &pawn_eg, 80, 180},
            {"knight_mg", &knight_mg, 250, 400},
            {"knight_eg", &knight_eg, 240, 390},
            {"bishop_mg", &bishop_mg, 260, 420},
            {"bishop_eg", &bishop_eg, 260, 420},
            {"rook_mg", &rook_mg, 400, 620},
            {"rook_eg", &rook_eg, 420, 650},
            {"queen_mg", &queen_mg, 750, 1100},
            {"queen_eg", &queen_eg, 780, 1150},

            {"doubled_mg", &doubled_mg, 0, 40},
            {"doubled_eg", &doubled_eg, 0, 50},
            {"connected_mg", &connected_mg, 0, 30},
            {"connected_eg", &connected_eg, 0, 40},
            {"phalanx_mg", &phalanx_mg, 0, 30},
            {"phalanx_eg", &phalanx_eg, 0, 40},
            {"isolated_mg", &isolated_mg, 0, 40},
            {"isolated_eg", &isolated_eg, 0, 50},
            {"backward_mg", &backward_mg, 0, 35},
            {"backward_eg", &backward_eg, 0, 45},
            {"central_lever_mg", &central_lever_mg, 0, 35},
            {"central_lever_eg", &central_lever_eg, 0, 30},
            {"pawn_tension_mg", &pawn_tension_mg, 0, 25},
            {"pawn_tension_eg", &pawn_tension_eg, 0, 20},

            {"passed_mg[0]", &passed_mg[0], 0, 25},
            {"passed_mg[1]", &passed_mg[1], 0, 40},
            {"passed_mg[2]", &passed_mg[2], 5, 70},
            {"passed_mg[3]", &passed_mg[3], 15, 110},
            {"passed_mg[4]", &passed_mg[4], 30, 180},
            {"passed_mg[5]", &passed_mg[5], 60, 280},

            {"passed_eg[0]", &passed_eg[0], 0, 40},
            {"passed_eg[1]", &passed_eg[1], 5, 70},
            {"passed_eg[2]", &passed_eg[2], 15, 130},
            {"passed_eg[3]", &passed_eg[3], 30, 200},
            {"passed_eg[4]", &passed_eg[4], 60, 320},
            {"passed_eg[5]", &passed_eg[5], 100, 450},

            {"protected_passed_mg", &protected_passed_mg, 0, 50},
            {"protected_passed_eg", &protected_passed_eg, 0, 80},

            {"minor_undeveloped_mg", &minor_undeveloped_mg, 0, 35},
            {"bishop_pair_mg", &bishop_pair_mg, 10, 65},
            {"bishop_pair_eg", &bishop_pair_eg, 15, 95},
            {"knight_outpost_mg", &knight_outpost_mg, 5, 60},
            {"knight_outpost_eg", &knight_outpost_eg, 10, 75},
            {"central_knight_outpost_mg", &central_knight_outpost_mg, 0, 30},
            {"central_knight_outpost_eg", &central_knight_outpost_eg, 0, 35},
            {"bishop_outpost_mg", &bishop_outpost_mg, 5, 50},
            {"bishop_outpost_eg", &bishop_outpost_eg, 5, 60},
            {"rook_7th_mg", &rook_7th_mg, 5, 55},
            {"rook_7th_eg", &rook_7th_eg, 10, 80},
            {"rook_open_mg", &rook_open_mg, 5, 45},
            {"rook_open_eg", &rook_open_eg, 5, 50},
            {"rook_semi_open_mg", &rook_semi_open_mg, 0, 30},
            {"rook_semi_open_eg", &rook_semi_open_eg, 0, 35},

            {"minor_threat_rook_mg", &minor_threat_rook_mg, 10, 70},
            {"minor_threat_rook_eg", &minor_threat_rook_eg, 10, 80},
            {"minor_threat_queen_mg", &minor_threat_queen_mg, 15, 90},
            {"minor_threat_queen_eg", &minor_threat_queen_eg, 15, 110},
            {"rook_threat_queen_mg", &rook_threat_queen_mg, 10, 60},
            {"rook_threat_queen_eg", &rook_threat_queen_eg, 10, 70},
            {"pawn_push_threat_mg", &pawn_push_threat_mg, 0, 40},
            {"pawn_push_threat_eg", &pawn_push_threat_eg, 0, 50},


            {"pawn_shield_mg", &pawn_shield_mg, 0, 35},
            {"uncastled_king_mg", &uncastled_king_mg, 40, 200},
            {"uncastled_king_eg", &uncastled_king_eg, 0, 50},
            {"tempo_mg", &tempo_mg, 0, 35},
            {"tempo_eg", &tempo_eg, 0, 15},
            {"king_open_file_mg", &king_open_file_mg, -400, 400}, {"king_open_file_eg", &king_open_file_eg, -400, 400},
            {"king_semi_open_file_mg", &king_semi_open_file_mg, -400, 400}, {"king_semi_open_file_eg", &king_semi_open_file_eg, -400, 400},
            {"trapped_bishop_deep_mg", &trapped_bishop_deep_mg, -400, 400}, {"trapped_bishop_deep_eg", &trapped_bishop_deep_eg, -400, 400},
            {"trapped_bishop_mg", &trapped_bishop_mg, -400, 400}, {"trapped_bishop_eg", &trapped_bishop_eg, -400, 400},
            {"trapped_rook_mg", &trapped_rook_mg, -400, 400},
            {"pawn_threat_minor_mg", &pawn_threat_minor_mg, -400, 400}, {"pawn_threat_minor_eg", &pawn_threat_minor_eg, -400, 400},
            {"pawn_threat_major_mg", &pawn_threat_major_mg, -400, 400}, {"pawn_threat_major_eg", &pawn_threat_major_eg, -400, 400},
            {"hanging_mg", &hanging_mg, -400, 400}, {"hanging_eg", &hanging_eg, -400, 400},
            {"safe_check_knight_mg", &safe_check_knight_mg, -400, 400}, {"safe_check_knight_eg", &safe_check_knight_eg, -400, 400},
            {"safe_check_bishop_mg", &safe_check_bishop_mg, -400, 400}, {"safe_check_bishop_eg", &safe_check_bishop_eg, -400, 400},
            {"safe_check_rook_mg", &safe_check_rook_mg, -400, 400}, {"safe_check_rook_eg", &safe_check_rook_eg, -400, 400},
            {"safe_check_queen_mg", &safe_check_queen_mg, -400, 400}, {"safe_check_queen_eg", &safe_check_queen_eg, -400, 400},
        };
        auto table = [&](const char* name, int* data, size_t n) {
            for (size_t i = 0; i < n; ++i) refs.push_back({std::string(name) + "[" + std::to_string(i) + "]", data + i, -2000, 2000});
        };
        table("pawn_storm_mg", pawn_storm_mg.data(), pawn_storm_mg.size());
        table("knight_mob_tab_mg", knight_mob_tab_mg.data(), knight_mob_tab_mg.size());
        table("knight_mob_tab_eg", knight_mob_tab_eg.data(), knight_mob_tab_eg.size());
        table("bishop_mob_tab_mg", bishop_mob_tab_mg.data(), bishop_mob_tab_mg.size());
        table("bishop_mob_tab_eg", bishop_mob_tab_eg.data(), bishop_mob_tab_eg.size());
        table("rook_mob_tab_mg", rook_mob_tab_mg.data(), rook_mob_tab_mg.size());
        table("rook_mob_tab_eg", rook_mob_tab_eg.data(), rook_mob_tab_eg.size());
        table("queen_mob_tab_mg", queen_mob_tab_mg.data(), queen_mob_tab_mg.size());
        table("queen_mob_tab_eg", queen_mob_tab_eg.data(), queen_mob_tab_eg.size());
        table("king_danger_mg", king_danger_mg.data(), king_danger_mg.size());
        table("king_danger_eg", king_danger_eg.data(), king_danger_eg.size());
        table("passed_blocked_mg", passed_blocked_mg.data(), passed_blocked_mg.size());
        table("passed_blocked_eg", passed_blocked_eg.data(), passed_blocked_eg.size());
        table("passed_own_king_mg", passed_own_king_mg.data(), passed_own_king_mg.size());
        table("passed_own_king_eg", passed_own_king_eg.data(), passed_own_king_eg.size());
        table("passed_opp_king_mg", passed_opp_king_mg.data(), passed_opp_king_mg.size());
        table("passed_opp_king_eg", passed_opp_king_eg.data(), passed_opp_king_eg.size());
        return refs;
    }
};

inline EvalParams g_eval_params;

} // namespace heavensgate
