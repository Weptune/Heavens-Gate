import subprocess
import struct
import json
import re
import os

FEAT_NAMES = [
    "Material", "Fiedler", "Cohesion", "Gap", "PST", "KingPress",
    "Battery", "PawnCoh", "Trace", "Mobility", "Center", "Phase",
    "Shield", "Passed", "EG_Passed", "Attack_Ratio",
    "BatXCenter", "FiedXPWN", "EG_Mobility", "PassXCenter", "KingXBat", "ShldXPWN",
    "KS_Fiedler", "QS_Fiedler", "CTR_Fiedler", "KSFiedXPress", "CTRFiedXCtr", "BR_Fiedler"
]

def parse_trm_bytes(raw_bytes, r_num):
    num_sectors = struct.unpack('<i', raw_bytes[:4])[0]
    if len(raw_bytes) >= 36488:
        num_features = struct.unpack('<i', raw_bytes[4:8])[0]
        offset = 8
    else:
        num_features = 16 if r_num <= 8 else 22
        offset = 4

    weights_per_feat = [[] for _ in range(num_features)]

    for s in range(num_sectors):
        for f_idx in range(num_features):
            w = struct.unpack('<f', raw_bytes[offset:offset+4])[0]
            offset += 4
            weights_per_feat[f_idx].append(w)
        b = struct.unpack('<f', raw_bytes[offset:offset+4])[0]
        offset += 4

    return {FEAT_NAMES[i]: round(sum(weights_per_feat[i]) / len(weights_per_feat[i]), 4) for i in range(num_features)}

def recover_weight_history():
    history_data = []
    
    # 1. Static historical commits (Rounds 1-18)
    STATIC_COMMITS = [
        (1, "1ac7f61"), (2, "155026d"), (3, "9e7a766"), (4, "02db950"),
        (5, "5a9b2f6"), (6, "ae18ca2"), (7, "0e128e5"), (8, "53accfb"),
        (9, "faa8847"), (10, "cc57933"), (11, "048c7f1"), (12, "048c7f1"),
        (13, "048c7f1"), (14, "048c7f1"), (15, "048c7f1"), (16, "048c7f1"),
        (17, "048c7f1"), (18, "048c7f1")
    ]

    # Dynamically find round commits in git log
    rounds_found = {}
    try:
        git_log = subprocess.check_output("git log --oneline -- heavensgate_tropical.trm", shell=True).decode('utf-8', errors='ignore')
        for line in git_log.split('\n'):
            line = line.strip()
            if not line: continue
            parts = line.split(' ', 1)
            commit_hash = parts[0]
            msg = parts[1] if len(parts) > 1 else ""
            
            m = re.search(r'Round\s+(\d+)', msg, re.IGNORECASE)
            if m:
                r_val = int(m.group(1))
                if r_val not in rounds_found:
                    rounds_found[r_val] = commit_hash
    except Exception as e:
        print(f"Git log scan warning: {e}")

    # Determine max round from detailed history if available
    max_r = 26
    if os.path.exists("tournament_detailed_history.json"):
        try:
            with open("tournament_detailed_history.json", 'r', encoding='utf-8') as f:
                d_hist = json.load(f)
                max_r = max(r.get("round", 1) for r in d_hist)
        except Exception:
            pass

    for r_num in range(1, max_r + 1):
        commit = rounds_found.get(r_num, "LOCAL" if r_num >= 18 else "048c7f1")
        try:
            if commit == "LOCAL" or r_num == max_r:
                if os.path.exists("heavensgate_tropical.trm"):
                    with open("heavensgate_tropical.trm", "rb") as f:
                        raw_bytes = f.read()
                else:
                    continue
            else:
                cmd = f"git show {commit}:heavensgate_tropical.trm"
                raw_bytes = subprocess.check_output(cmd, shell=True)

            avgs = parse_trm_bytes(raw_bytes, r_num)

            history_data.append({
                "round": r_num,
                "phase": 1 if r_num <= 8 else 2,
                "phase_round": r_num if r_num <= 8 else (r_num - 8),
                "commit": commit,
                "weights": avgs
            })
            print(f"  [SUCCESS] Recovered Round {r_num} Weights!")
        except Exception as e:
            print(f"  [WARNING] Round {r_num} extraction: {e}")

    with open("model_weight_history.json", 'w', encoding='utf-8') as f:
        json.dump(history_data, f, indent=2)

    print("\n[RECOVERY COMPLETE] Rebuilt model_weight_history.json with all rounds!")

if __name__ == '__main__':
    recover_weight_history()
