import struct
import math

def generate_phase5_trm():
    num_sec = 320
    num_feat = 25

    feature_floors = [
        10.00, # x[ 0] Material (1 Pawn = 100 cp)
        0.40, # x[ 1] Fiedler
        0.20, # x[ 2] Cohesion
        0.15, # x[ 3] Spectral Gap
        0.50, # x[ 4] PST
        0.40, # x[ 5] King Press
        0.25, # x[ 6] Battery
        0.35, # x[ 7] Pawn Coh
        0.15, # x[ 8] Trace Energy
        0.35, # x[ 9] Mobility
        0.35, # x[10] Center Control
        0.25, # x[11] Phase
        0.30, # x[12] King Shield
        0.50, # x[13] Passed Pawns
        0.50, # x[14] EG Passed Pawns
        0.25, # x[15] Attack Ratio
        0.25, # x[16] BatXCenter
        0.20, # x[17] FiedXPWN
        0.20, # x[18] EG_Mobility
        0.20, # x[19] PassXCenter
        0.20, # x[20] KingXBat
        0.20, # x[21] ShldXPWN
        0.20, # x[22] Cheb T2 Us
        0.15, # x[23] Cheb T2 Them
        0.25  # x[24] Cheb King Threat
    ]

    t1_sectors = []
    for b in range(10):
        for j in range(16):
            jf = float(j)
            bf = float(b)
            b_val = -5.0 + 0.3 * jf + 0.5 * bf
            w = [
                1.0,
                0.5 + 0.2 * abs(math.sin(jf * 0.5 + bf)),
                0.4 + 0.15 * abs(math.cos(jf * 0.7 + bf)),
                0.3 + 0.1 * abs(math.sin(jf * 1.1)),
                0.8 + 0.2 * abs(math.cos(jf * 0.3)),
                1.0 + 0.3 * abs(math.sin(jf * 0.9)),
                0.7 + 0.2 * abs(math.cos(jf * 1.3)),
                0.6 + 0.15 * abs(math.sin(jf * 0.6)),
                0.3 + 0.1 * abs(math.cos(jf * 0.4)),
                0.5 + 0.2 * abs(math.sin(jf * 0.8)),
                0.6 + 0.2 * abs(math.cos(jf * 1.0)),
                0.3 + 0.1 * abs(math.sin(jf * 1.2)),
                0.5 + 0.2 * abs(math.sin(jf * 0.7)),
                0.8 + 0.2 * abs(math.cos(jf * 0.5)),
                0.9 + 0.2 * abs(math.sin(jf * 0.4)),
                0.7 + 0.2 * abs(math.cos(jf * 0.8)),
                0.4 + 0.15 * abs(math.sin(jf * 0.9 + bf)),
                0.3 + 0.1 * abs(math.cos(jf * 1.1 + bf)),
                0.6 + 0.2 * abs(math.sin(jf * 0.6 + bf)),
                0.5 + 0.2 * abs(math.cos(jf * 0.7 + bf)),
                0.4 + 0.15 * abs(math.sin(jf * 1.3 + bf)),
                0.3 + 0.1 * abs(math.cos(jf * 0.8 + bf)),
                0.4 + 0.15 * abs(math.sin(jf * 0.9)),
                0.3 + 0.10 * abs(math.cos(jf * 1.1)),
                0.5 + 0.20 * abs(math.sin(jf * 1.3))
            ]
            for i in range(num_feat):
                w[i] = max(feature_floors[i], w[i])
            t1_sectors.append((w, b_val))

    t2_sectors = []
    for b in range(10):
        for j in range(16):
            jf = float(j)
            bf = float(b)
            b_val = -15.0 + 0.2 * jf + 0.3 * bf
            w = [
                0.0,
                0.2 + 0.1 * abs(math.sin(jf * 0.4 + bf)),
                0.15 + 0.08 * abs(math.cos(jf * 0.6)),
                0.1 + 0.05 * abs(math.sin(jf * 1.0)),
                0.2 + 0.1 * abs(math.cos(jf * 0.2)),
                0.4 + 0.15 * abs(math.sin(jf * 0.8)),
                0.25 + 0.1 * abs(math.cos(jf * 1.2)),
                0.2 + 0.1 * abs(math.sin(jf * 0.5)),
                0.1 + 0.05 * abs(math.cos(jf * 0.3)),
                0.15 + 0.08 * abs(math.sin(jf * 0.7)),
                0.2 + 0.1 * abs(math.cos(jf * 0.9)),
                0.1 + 0.05 * abs(math.sin(jf * 1.1)),
                0.3 + 0.12 * abs(math.sin(jf * 0.6)),
                0.2 + 0.1 * abs(math.cos(jf * 0.4)),
                0.25 + 0.1 * abs(math.sin(jf * 0.3)),
                0.2 + 0.1 * abs(math.cos(jf * 0.7)),
                0.15 + 0.08 * abs(math.sin(jf * 0.8)),
                0.1 + 0.05 * abs(math.cos(jf * 1.0)),
                0.2 + 0.1 * abs(math.sin(jf * 0.5)),
                0.15 + 0.08 * abs(math.cos(jf * 0.6)),
                0.2 + 0.1 * abs(math.sin(jf * 1.2)),
                0.1 + 0.05 * abs(math.cos(jf * 0.7)),
                0.15 + 0.08 * abs(math.sin(jf * 0.9)),
                0.10 + 0.05 * abs(math.cos(jf * 1.1)),
                0.20 + 0.10 * abs(math.sin(jf * 1.3))
            ]
            t2_sectors.append((w, b_val))

    with open('heavensgate_tropical.trm', 'wb') as out:
        out.write(struct.pack('<II', num_sec, num_feat))
        for w, b in t1_sectors:
            out.write(struct.pack(f'<{num_feat}f', *w))
            out.write(struct.pack('<f', b))
        for w, b in t2_sectors:
            out.write(struct.pack(f'<{num_feat}f', *w))
            out.write(struct.pack('<f', b))

    print(f"SUCCESSFULLY GENERATED PHASE 5 25D DUAL-SURFACE WEIGHT FILE (320 Sectors, {33288} bytes)!")

if __name__ == "__main__":
    generate_phase5_trm()
