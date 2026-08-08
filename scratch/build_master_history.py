import glob
import json
import re

def build_master_history():
    pgn_files = sorted(glob.glob('pgn_history/tournament_round_*.pgn') + glob.glob('tournament_round_*.pgn'), key=lambda x: int(re.search(r'\d+', x).group()))
    
    all_rounds_detailed = []

    for pgn in pgn_files:
        r_num = int(re.search(r'\d+', pgn).group())
        if r_num <= 8:
            phase = 1
            phase_round = r_num
            phase_name = "Phase 1: 10 Spatial King Buckets (16 Features)"
            num_features = 16
        elif r_num <= 29:
            phase = 2
            phase_round = r_num - 8
            phase_name = "Phase 2: Hand-Selected Feature Cross-Terms (22 Features)"
            num_features = 22
        else:
            phase = 3
            phase_round = r_num - 29
            phase_name = "Phase 3: 4-Zone Localized Fiedler & AVX2 SIMD (28 Features)"
            num_features = 28

        with open(pgn, 'r', encoding='utf-8', errors='ignore') as f:
            text = f.read()

        games = text.split('[Event ')
        total_games = 0
        master_wins = 0
        baseline_wins = 0
        draws = 0

        white_wins = 0
        white_losses = 0
        black_wins = 0
        black_losses = 0

        blunders_by_phase = {'Opening': 0, 'Middlegame': 0, 'Endgame': 0}
        draw_reasons = {'Stalemate': 0, '50MoveRule': 0, 'Material': 0, 'Safety400': 0, 'Other': 0}

        opening_evals = []
        middle_evals = []
        endgame_evals = []

        games_with_adv = 0
        converted_wins = 0
        squandered_losses = 0

        blunders_total = 0
        blunders_recovered = 0

        promotions = 0
        win_lengths = []
        loss_lengths = []
        calibration_errors = []

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
            is_draw = False

            if last.endswith('1-0'):
                if white_is_master: master_won = True
                else: master_lost = True
            elif last.endswith('0-1'):
                if black_is_master: master_won = True
                else: master_lost = True
            elif '1/2-1/2' in last:
                is_draw = True

            eval_matches = re.findall(r'\[%eval (-?\d+)\]', g_str)
            num_plies = len(eval_matches)

            if master_won:
                master_wins += 1
                win_lengths.append(num_plies // 2)
                if white_is_master: white_wins += 1
                else: black_wins += 1
            elif master_lost:
                baseline_wins += 1
                loss_lengths.append(num_plies // 2)
                if white_is_master: white_losses += 1
                else: black_losses += 1
            elif is_draw:
                draws += 1
                if 'Stalemate' in g_str: draw_reasons['Stalemate'] += 1
                elif '50-Move' in g_str: draw_reasons['50MoveRule'] += 1
                elif 'Material' in g_str: draw_reasons['Material'] += 1
                elif '400-Move' in g_str: draw_reasons['Safety400'] += 1
                else: draw_reasons['Other'] += 1

            promotions += len(re.findall(r'\b[a-h][27][a-h][18][qrbnQRBN]\b', g_str))

            consecutive_adv = 0
            had_adv = False
            game_had_blunder = False
            recovered_from_blunder = False

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

                    if idx > 0:
                        prev_ev = int(eval_matches[idx-1])
                        drop = prev_ev - ev if is_white else ev - prev_ev
                        if drop > 150:
                            blunders_total += 1
                            game_had_blunder = True
                            phase_str = 'Opening' if move_num <= 15 else ('Middlegame' if move_num <= 40 else 'Endgame')
                            blunders_by_phase[phase_str] += 1
                    
                    if game_had_blunder and ev >= -50:
                        recovered_from_blunder = True

            if game_had_blunder and recovered_from_blunder:
                blunders_recovered += 1

            if had_adv:
                games_with_adv += 1
                if master_won: converted_wins += 1
                else: squandered_losses += 1

            if eval_matches:
                final_eval = int(eval_matches[-1])
                actual_outcome_cp = 600.0 if master_won else (-600.0 if master_lost else 0.0)
                calibration_errors.append(abs(final_eval - actual_outcome_cp))

        score_pct = (master_wins + 0.5 * draws) / total_games * 100.0 if total_games > 0 else 0.0
        conversion_rate = (converted_wins / games_with_adv * 100.0) if games_with_adv > 0 else 0.0
        recovery_rate = (blunders_recovered / blunders_total * 100.0) if blunders_total > 0 else 0.0
        avg_win_len = sum(win_lengths) / len(win_lengths) if win_lengths else 0
        avg_loss_len = sum(loss_lengths) / len(loss_lengths) if loss_lengths else 0
        avg_calib_err = sum(calibration_errors) / len(calibration_errors) if calibration_errors else 0

        round_data = {
            "round": r_num,
            "phase": phase,
            "phase_round": phase_round,
            "phase_name": phase_name,
            "num_features": 16 if phase == 1 else 22,
            "total_games": total_games,
            "score_pct": round(score_pct, 1),
            "master_wins": master_wins,
            "baseline_wins": baseline_wins,
            "draws": draws,
            "as_white": {"wins": white_wins, "losses": white_losses},
            "as_black": {"wins": black_wins, "losses": black_losses},
            "avg_eval": {
                "opening": round(sum(opening_evals)/len(opening_evals), 1) if opening_evals else 0,
                "middlegame": round(sum(middle_evals)/len(middle_evals), 1) if middle_evals else 0,
                "endgame": round(sum(endgame_evals)/len(endgame_evals), 1) if endgame_evals else 0,
            },
            "blunders": {
                "total": blunders_total,
                "by_phase": blunders_by_phase,
                "recovered_count": blunders_recovered,
                "recovery_rate_pct": round(recovery_rate, 1)
            },
            "sustained_advantage": {
                "games_gained_sustained_adv": games_with_adv,
                "converted_wins": converted_wins,
                "squandered": squandered_losses,
                "conversion_rate_pct": round(conversion_rate, 1)
            },
            "game_length": {
                "avg_win_moves": round(avg_win_len, 1),
                "avg_loss_moves": round(avg_loss_len, 1)
            },
            "calibration": {
                "avg_outcome_error_cp": round(avg_calib_err, 1)
            },
            "pawn_promotions": promotions,
            "draw_reasons": draw_reasons
        }

        all_rounds_detailed.append(round_data)

    master_json_path = "tournament_detailed_history.json"
    existing_data = []
    try:
        with open(master_json_path, 'r', encoding='utf-8') as jf:
            existing_data = json.load(jf)
    except Exception:
        existing_data = []

    # Merge by round number (new data overrides existing for same round)
    round_map = {r['round']: r for r in existing_data}
    for r in all_rounds_detailed:
        round_map[r['round']] = r

    final_rounds = [round_map[k] for k in sorted(round_map.keys())]

    with open(master_json_path, 'w', encoding='utf-8') as jf:
        json.dump(final_rounds, jf, indent=2)

    print(f"[SUCCESS] Built master history JSON with Phase tagging: {master_json_path}")

if __name__ == '__main__':
    build_master_history()
