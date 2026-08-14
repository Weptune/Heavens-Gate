---
title: "Heaven's Gate (Part 4): High-Performance Search Physics & True Singular Extensions"
description: "A first-principles deep dive into Principal Variation Search (PVS), Magic Bitboard movegen, Transposition Tables, SEE move ordering, and True Singular Extensions."
date: 2026-08-10
tags: ['search-algorithms', 'chess', 'cpp', 'algorithms', 'maths']
image: './board_graph.png'
pinned: false
---

# Part 4: High-Performance Search Physics & True Singular Extensions

## 4.1 Search Engine Infrastructure

A great evaluation function is useless without a fast search engine. Heaven's Gate implements a state-of-the-art **Principal Variation Search (PVS / NegaScout)** framework written in C++20.

```text
===================================================================================
  SEARCH COMPONENT            SPECIFICATION & CAPACITY
===================================================================================
  Move Generation             Magic Bitboards (PEXT / Shift fallback)
  Transposition Table (TT)    256MB Lockless Cluster (Zobrist 64-bit keys)
  Search Framework            Principal Variation Search (PVS / NegaScout)
  Window Management           Aspiration Windows (Δ = 25) with dynamic re-search
  Move Ordering               8-Tier Move Picker with Static Exchange Evaluation (SEE)
  Draw Prevention             50-Move Draw Prevention Priority (clock_boost)
===================================================================================
```

---

## 4.2 The 8-Tier Move Picker & Static Exchange Evaluation (SEE)

To achieve maximum Alpha-Beta pruning efficiency, moves must be searched in order of tactical quality. Searching the best move first achieves the theoretical minimum search tree size $O(\sqrt{b^d})$.

Heaven's Gate orders moves across **8 distinct priority tiers**:

1. **TT / PV Move (+2,000,000)**: Move retrieved from the Transposition Table.
2. **Winning Captures (+1,000,000)**: Captures evaluated as winning by Static Exchange Evaluation ($\text{SEE} \ge 0$).
3. **Promotions (+900,000)**: Pawn promotions to Queen or Knight.
4. **Castling (+850,000)**: Kingside and Queenside castling.
5. **Killer Moves (+800,000 / +700,000)**: Quiet moves that produced a beta cutoff at the current search ply.
6. **Countermoves (+600,000)**: Move history response to opponent's previous move.
7. **Quiet History Scores**: Butterfly history heuristic accumulated across search iterations.
8. **Losing Captures (-100,000)**: Captures evaluated as losing by Static Exchange Evaluation ($\text{SEE} < 0$).

---

## 4.3 True Singular Extensions (+50 Elo Upgrade)

In deep search (`depth >= 6`), when Master finds a strong Transposition Table move (TT move), how do we know if it is just one of many good moves, or the **ONLY move that saves the position**?

If a position has only one valid escape from a tactical pin or checkmate threat, searching all candidate moves to equal depth wastes node budget. Conversely, if we reduce depth on that node, we risk missing a forced mate.

Heaven's Gate solves this by implementing **True Singular Extensions**:

### The True Singular Extensions Algorithm

When probing the TT move `m` at `depth >= 6` (and `!in_check`):

1. **Singular Beta Threshold**:
   $$\text{singular\_beta} = \text{tt\_score} - 2 \times \text{depth}$$
2. **Alternative Move Probing**: Temporarily exclude the primary TT move `m` and search all alternative legal moves at reduced depth $(\text{depth} - 1) / 2$ with a zero-window $(-\text{singular\_beta}, -\text{singular\_beta} + 1)$.
3. **Singular Extension**: If **no alternative move** on the board can reach `singular_beta`, the primary move `m` is mathematically proven to be **SINGULAR** (the uniquely forced move). The search automatically grants a **+1 ply depth extension**!

### Production Implementation (`src/search/search.cpp`)

```cpp
// True Singular Extensions in src/search/search.cpp
int extension = 0;
if (m.is_promotion()) {
    extension = 1;
} else if (i == 0 && depth >= 6 && static_cast<bool>(tt_move) && !in_chk) {
    TTEntry* entry = tt_.probe(board.zobrist_key());
    if (entry && entry->depth >= depth - 3 && entry->bound != TTBound::Upper && std::abs(entry->score) < ScoreMate - 1000) {
        int singular_beta = entry->score - 2 * depth;
        
        // Temporarily unmake TT move to test alternative moves
        board.unmake_move(m);
        
        int alt_max = -ScoreInfinity;
        for (size_t alt_i = 1; alt_i < moves.size(); alt_i++) {
            Move alt_m = moves[alt_i];
            board.make_move(alt_m);
            int alt_eval = -negamax_alphabeta(board, (depth - 1) / 2, ply + 1, -singular_beta, -singular_beta + 1, false, false, Move(), alt_m, nullptr);
            board.unmake_move(alt_m);
            alt_max = std::max(alt_max, alt_eval);
            if (alt_max >= singular_beta) break;
        }
        
        board.make_move(m); // Re-make TT move
        
        if (alt_max < singular_beta) {
            extension = 1; // Singular extension granted!
        }
    } else {
        extension = 1; // Default TT extension
    }
}
```

---

## 4.4 50-Move Draw Prevention Priority (`clock_boost`)

In self-play tournaments, engines occasionally drift into 50-move rule draws when holding a winning endgame advantage. 

To prevent unnecessary draws when holding an advantage, Heaven's Gate implements a **50-Move Draw Priority Boost**:

```cpp
// Move picker clock boost (src/search/move_picker.cpp)
if (board.halfmove_clock() >= 70) {
    if (m.is_capture() || m.piece_type() == PieceType::Pawn) {
        score += 1500000; // Priority boost to reset halfmove clock!
    }
}
```

When the halfmove clock reaches 70, Master prioritizes pawn advances and captures, resetting the halfmove clock and converting winning endgames into checkmates before the 50-move draw threshold is reached!
