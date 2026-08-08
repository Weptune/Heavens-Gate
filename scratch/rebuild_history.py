import glob
import json
import re

def rebuild_clean_history_summary():
    pgn_files = sorted(glob.glob('pgn_history/tournament_round_*.pgn') + glob.glob('tournament_round_*.pgn'), key=lambda x: int(re.search(r'\d+', x).group()))
    
    summary_path = "tournament_history_summary.txt"
    with open(summary_path, 'w', encoding='utf-8') as sf:
        sf.write("=========================================================================\n")
        sf.write("            HEAVEN'S GATE — CONTINUOUS TRAINING HISTORY SUMMARY\n")
        sf.write("=========================================================================\n\n")

        current_phase = 0

        for pgn in pgn_files:
            r_num = int(re.search(r'\d+', pgn).group())
            if r_num <= 8:
                phase = 1
                phase_round = r_num
            elif r_num <= 29:
                phase = 2
                phase_round = r_num - 8
            else:
                phase = 3
                phase_round = r_num - 29

            if phase != current_phase:
                current_phase = phase
                sf.write("=========================================================================\n")
                if phase == 1:
                    sf.write("  PHASE 1: 10 Spatial King-Bucket Partitioning (16 Features, 5,440 Params)\n")
                elif phase == 2:
                    sf.write("  PHASE 2: Hand-Selected Feature Cross-Terms (22 Features, 7,480 Params)\n")
                else:
                    sf.write("  PHASE 3: 4-Zone Localized Fiedler & AVX2 SIMD (28 Features, 9,520 Params)\n")
                sf.write("=========================================================================\n\n")

            with open(pgn, 'r', encoding='utf-8', errors='ignore') as f:
                text = f.read()

            games = text.split('[Event ')
            total_games = 0
            master_wins = 0
            baseline_wins = 0
            draws = 0

            opening_evals = []
            middle_evals = []
            endgame_evals = []

            games_with_adv = 0
            converted_wins = 0
            squandered_losses = 0
            promotions = 0

            for g in games:
                g_str = g.strip()
                if not g_str: continue
                total_games += 1

                white_is_master = 'White "Master Edition"' in g_str
                black_is_master = 'Black "Master Edition"' in g_str

                lines = [l.strip() for l in g_str.split('\n') if l.strip()]
                last = lines[-1] if lines else ''

                master_won = False
                master_lost = False
                if last.endswith('1-0'):
                    if white_is_master: master_won = True
                    else: master_lost = True
                elif last.endswith('0-1'):
                    if black_is_master: master_won = True
                    else: master_lost = True
                else:
                    draws += 1

                if master_won: master_wins += 1
                elif master_lost: baseline_wins += 1

                promotions += len(re.findall(r'\b[a-h][27][a-h][18][qrbnQRBN]\b', g_str))
                eval_matches = re.findall(r'\[%eval (-?\d+)\]', g_str)
                consecutive_adv = 0
                had_adv = False

                for idx, ev_str in enumerate(eval_matches):
                    ev = int(ev_str)
                    move_num = (idx // 2) + 1
                    is_white = (idx % 2 == 0)
                    is_master = (is_white and white_is_master) or (not is_white and black_is_master)

                    if is_master:
                        if ev >= 150: consecutive_adv += 1
                        else: consecutive_adv = 0

                        if consecutive_adv >= 3 or ev >= 300: had_adv = True

                        if abs(ev) <= 400:
                            if move_num <= 15: opening_evals.append(ev)
                            elif move_num <= 40: middle_evals.append(ev)
                            else: endgame_evals.append(ev)

                if had_adv:
                    games_with_adv += 1
                    if master_won: converted_wins += 1
                    else: squandered_losses += 1

            score_pct = (master_wins + 0.5 * draws) / total_games * 100.0 if total_games > 0 else 0.0
            conversion_rate = (converted_wins / games_with_adv * 100.0) if games_with_adv > 0 else 0.0

            avg_op = sum(opening_evals)/len(opening_evals) if opening_evals else 0
            avg_mid = sum(middle_evals)/len(middle_evals) if middle_evals else 0
            avg_end = sum(endgame_evals)/len(endgame_evals) if endgame_evals else 0

            sf.write(f"=== PHASE {phase} ROUND {phase_round} (Overall Round {r_num}) SUMMARY ===\n")
            sf.write(f"  * Win Rate / Score  : {score_pct:.1f}% ({master_wins} Wins / {baseline_wins} Losses / {draws} Draws in {total_games} games)\n")
            sf.write(f"  * Sustained Leads   : Held >=+150cp in {games_with_adv}/{total_games} games ({games_with_adv/total_games*100:.0f}%)\n")
            sf.write(f"  * Conversion Rate   : Converted {converted_wins}/{games_with_adv} ({conversion_rate:.1f}% Conversion Efficiency)\n")
            sf.write(f"  * Phase Avg Evals   : Opening={avg_op:+.0f}cp | Middlegame={avg_mid:+.0f}cp | Endgame={avg_end:+.0f}cp\n")
            sf.write(f"  * Pawn Promotions   : {promotions} promotions\n\n")

    print("[REBUILD COMPLETE] Rebuilt clean tournament_history_summary.txt with Phase 1 vs Phase 2 sections")

if __name__ == '__main__':
    rebuild_clean_history_summary()
