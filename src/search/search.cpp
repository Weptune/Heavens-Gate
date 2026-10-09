#include "search.hpp"
#include "syzygy.hpp"
#include "search_params.hpp"
#include "null_move_policy.hpp"
#include "../core/fen.hpp"
#include "../evaluation/eval.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#if defined(_OPENMP)
#include <omp.h>
#endif

namespace heavensgate {

SearchEngine::SearchEngine(TranspositionTable* shared_tt, MovePicker* shared_picker)
    : tt_ptr_(shared_tt ? shared_tt : &local_tt_),
      local_tt_(shared_tt ? 0 : 64),
      move_picker_ptr_(shared_picker ? shared_picker : &local_move_picker_),
      move_picker_(*move_picker_ptr_), exporter_(shared_tt == nullptr) {
    eval_stack_.fill(-ScoreInfinity);
    if (shared_tt) {
        num_threads_ = 1;
    } else {
        polyglot_book_.load("performance.bin");
        if (!polyglot_book_.is_loaded()) polyglot_book_.load("tools/performance.bin");
        set_threads(num_threads_);
    }
}

void SearchEngine::set_threads(int threads) {
    num_threads_ = std::clamp(threads, 1, MaxThreads);
    for (int i = 0; i < num_threads_ - 1; ++i) {
        if (!workers_[i]) workers_[i] = std::make_unique<SearchEngine>(tt_ptr_);
        workers_[i]->set_master_stop_flag(&time_stop_flag_);
    }
}

void SearchEngine::prepare_search(double max_time_ms, double opt_time_ms) {
    // The hard deadline includes all root preparation, not just recursive work.
    search_start_time_ = std::chrono::steady_clock::now();
    max_time_ms_ = max_time_ms;
    opt_time_ms_ = opt_time_ms > 0.0 ? opt_time_ms : max_time_ms;
    time_poll_mask_ = max_time_ms > 0.0 && max_time_ms <= 5.0 ? 7 :
                      max_time_ms > 0.0 && max_time_ms <= 50.0 ? 63 : 2047;
    time_stop_flag_.store(false, std::memory_order_relaxed);
    max_nodes_ = 0;
    metrics_tracker_.reset();
    metrics_tracker_.start_timer();
    pv_table_.clear();
    node_count_ = 0; // Includes qsearch nodes exactly once; q_nodes_ is a subset.
    q_nodes_ = 0;
    tt_statistics_.reset();
    // Helper preparation must not publish into its master's shared table.
    if (!master_stop_flag_) tt().publish_statistics(tt_statistics_);
    eval_stack_.fill(-ScoreInfinity);
    move_stack_.fill(Move{});
    piece_stack_.fill(Piece::None);
    // Defer the multi-megabyte history sweep when it would dominate the budget.
    // Keep learned ordering and reset per-search moves; normal searches age as before.
    if (max_time_ms > 0.0 && max_time_ms <= timing_options_.minimum_smp_time_ms)
        move_picker_.reset_search_moves();
    else move_picker_.age_history();
    metrics_tracker_.set_version("v10.0 (Master Lazy SMP)");
}

// Precomputed logarithmic LMR reduction lookup table
static int lmr_table[64][64];

void SearchEngine::init_lmr_table(float divisor) {
    if (divisor <= 0.1f) divisor = 3.20f;
    for (int d = 0; d < 64; ++d) {
        for (int m = 0; m < 64; ++m) {
            if (d == 0 || m == 0) {
                lmr_table[d][m] = 0;
            } else {
                lmr_table[d][m] = 1 + static_cast<int>(std::log(d) * std::log(m) / divisor);
            }
        }
    }
}

static struct LMRTableInit {
    LMRTableInit() {
        SearchEngine::init_lmr_table(3.20f);
    }
} g_lmr_table_init;

// Existing engine/match policy takes available rule-50 claims. A position
// already checkmated is not claimable, even if its last move reached 100.
static int rule50_score(const Board& board, bool in_check, int ply) {
    return in_check && !MoveGenerator::has_legal_move(board) ? -ScoreMate + ply : ScoreDraw;
}

int SearchEngine::quiescence_search(Board& board, int alpha, int beta, int ply) {
    if (max_nodes_ && node_count_ >= max_nodes_) {
        time_stop_flag_.store(true, std::memory_order_relaxed);
        return 0;
    }
    metrics_tracker_.add_nodes(1);
    q_nodes_++;
    node_count_++;
    if (!board.can_push_history()) return Evaluator::evaluate_fast(board, alpha, beta);

    if (is_stopped() || ((node_count_ & time_poll_mask_) == 0 && is_time_up())) return 0;

    if (ply > 0 && board.is_repetition(2)) {
        return ScoreDraw;
    }

    const Color us = board.side_to_move();
    const bool in_chk = MoveGenerator::in_check(board, us);
    if (board.halfmove_clock() >= 100) return rule50_score(board, in_chk, ply);
    // Stand-pat is unavailable in stalemate. Check quiet mobility as well as
    // captures, before either evaluation or a cached nonterminal cutoff.
    if (!in_chk && !MoveGenerator::has_legal_move(board)) return ScoreDraw;
    if (ply >= 64) return Evaluator::evaluate_fast(board, alpha, beta);

    int orig_alpha = alpha;
    Move tt_move = Move();
    TTEntry snapshot;
    TTEntry* tt_entry = tt().probe(board.zobrist_key(), snapshot, tt_statistics_, board.halfmove_clock()) ? &snapshot : nullptr;
    if (tt_entry) {
        if (static_cast<bool>(tt_entry->move)) {
            tt_move = tt_entry->move;
        }
        const bool compatible = tt_entry->score_matches_rule50(board.halfmove_clock());
#if defined(HG_TT_DIAGNOSTICS)
        auto& diagnostic = tt_statistics_.diagnostics;
        diagnostic.compatible_hits += compatible;
        if (!compatible) {
            ++diagnostic.rejected_clock_hits;
            diagnostic.hint_only_hits += static_cast<bool>(tt_move);
            ++diagnostic.rejected_clock_deciles[std::clamp(board.halfmove_clock() / 10, 0, 9)];
        }
#endif
        if (!compatible) tt_entry = nullptr;
    }
    if (tt_entry) {
        int tt_score = tt_entry->score;
        if (tt_score > ScoreMate - 1000) tt_score -= ply;
        else if (tt_score < -ScoreMate + 1000) tt_score += ply;

        if (tt_entry->bound == TTBound::Exact) {
#if defined(HG_TT_DIAGNOSTICS)
            ++tt_statistics_.diagnostics.score_cutoffs;
#endif
            return tt_score;
        } else if (tt_entry->bound == TTBound::Lower && tt_score >= beta) {
#if defined(HG_TT_DIAGNOSTICS)
            ++tt_statistics_.diagnostics.score_cutoffs;
#endif
            return tt_score;
        } else if (tt_entry->bound == TTBound::Upper && tt_score <= alpha) {
#if defined(HG_TT_DIAGNOSTICS)
            ++tt_statistics_.diagnostics.score_cutoffs;
#endif
            return tt_score;
        }
    }

    int stand_pat = Evaluator::evaluate_fast(board, alpha, beta);
    int best_score = stand_pat;
    Move best_move = Move();

    if (!in_chk) {
        if (stand_pat >= beta) {
            return beta;
        }

        if (stand_pat > alpha) {
            alpha = stand_pat;
        }
    } else {
        best_score = -ScoreInfinity;
    }

    MoveList moves;
    if (in_chk) {
        MoveGenerator::generate_legal_moves(board, moves);
    } else {
        MoveGenerator::generate_capture_moves(board, moves);
    }

    if (moves.empty()) {
        if (in_chk) return -ScoreMate + ply;
        return stand_pat;
    }

    std::array<int, 256> scores{};
    if (in_chk) {
        move_picker_.score_moves(board, moves, scores, ply, tt_move);
    } else {
        move_picker_.score_captures_only(board, moves, scores, tt_move);
    }

    for (size_t i = 0; i < moves.size(); ++i) {
        MovePicker::pick_best(moves, scores, i);
        const auto& m = moves[i];

        if (!in_chk) {
            // SEE Pruning in QSearch: Skip clearly losing captures, unless promotion or check
            if (!m.is_promotion() && !MoveGenerator::gives_check(board, m) && !MovePicker::see_ge(board, m, 0)) {
                continue;
            }

            Piece victim = board.piece_at(m.to());
            int victim_val = (victim != Piece::None || m.is_ep()) ? PawnValue : 0;
            switch (piece_type_of(victim)) {
                case PieceType::Pawn:   victim_val = PawnValue; break;
                case PieceType::Knight: victim_val = KnightValue; break;
                case PieceType::Bishop: victim_val = BishopValue; break;
                case PieceType::Rook:   victim_val = RookValue; break;
                case PieceType::Queen:  victim_val = QueenValue; break;
                default: break;
            }
            if (m.is_ep()) victim_val = PawnValue;

            if (stand_pat + victim_val + 200 < alpha && !m.is_promotion()) {
                continue;
            }
        }

        board.make_move(m);

        int score = -quiescence_search(board, -beta, -alpha, ply + 1);

        board.unmake_move(m);

        if (is_stopped()) return 0;

        if (score > best_score) {
            best_score = score;
            best_move = m;
        }

        if (score >= beta) {
            tt().store(board.zobrist_key(), m, score, 0, TTBound::Lower, ply, board.halfmove_clock(), &tt_statistics_);
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    TTBound bound = (best_score > orig_alpha) ? (in_chk ? TTBound::Exact : TTBound::Lower) : TTBound::Upper;
    tt().store(board.zobrist_key(), best_move, best_score, 0, bound, ply, board.halfmove_clock(), &tt_statistics_);

    return alpha;
}

int SearchEngine::negamax_minimax(Board& board, int depth, int ply, SearchTreeNode* json_node) {
    metrics_tracker_.add_nodes(1);
    node_count_++;

    if (depth <= 0 || ply >= 255 || !board.can_push_history()) {
        int eval = Evaluator::evaluate(board);
        if (json_node) {
            json_node->eval = eval;
            json_node->is_terminal = true;
        }
        return eval;
    }

    MoveList moves;
    MoveGenerator::generate_legal_moves(board, moves);

    if (moves.empty()) {
        int score = 0;
        if (MoveGenerator::in_check(board, board.side_to_move())) {
            score = -ScoreMate + ply;
        } else {
            score = ScoreDraw;
        }
        if (json_node) {
            json_node->eval = score;
            json_node->is_terminal = true;
        }
        return score;
    }

    int best_score = -ScoreInfinity;

    for (const auto& m : moves) {
        board.make_move(m);

        SearchTreeNode* child_node = exporter_.add_child(json_node, board, m, depth - 1, ply + 1);

        int score = -negamax_minimax(board, depth - 1, ply + 1, child_node);

        board.unmake_move(m);

        if (score > best_score) {
            best_score = score;
            pv_table_.update(ply, m);
        }
    }

    if (json_node) {
        json_node->eval = best_score;
    }

    return best_score;
}

int SearchEngine::negamax_alphabeta(Board& board, int depth, int ply, int alpha, int beta, bool use_move_ordering, bool use_tt, Move pv_move, SearchTreeNode* json_node, int /*prev_eval*/, Move excluded_move, bool previous_was_null) {
    if (max_nodes_ && node_count_ >= max_nodes_) {
        time_stop_flag_.store(true, std::memory_order_relaxed);
        return 0;
    }
    metrics_tracker_.add_nodes(1);
    node_count_++;
    // Stack safety, not an extension budget. Check-extension rule below is unchanged.
    if (ply >= 255 || !board.can_push_history()) return Evaluator::evaluate_fast(board, alpha, beta);

    if (is_stopped() || ((node_count_ & time_poll_mask_) == 0 && is_time_up())) return 0;

    if (ply > 0 && board.is_repetition(2)) {
        return ScoreDraw;
    }

    Color us = board.side_to_move();
    bool in_chk = MoveGenerator::in_check(board, us);
    if (board.halfmove_clock() >= 100) return rule50_score(board, in_chk, ply);

    // Check Extension (ply < 64) to prevent checkmate blind spots in middlegame/endgame
    if (in_chk && ply < 64 && depth > 1) {
        depth++;
    }

    if (ply < MaxSearchDepth) {
        pv_table_.init_ply(ply);
    }

    int orig_alpha = alpha;
    Move tt_move = pv_move;
    int tt_score = -ScoreInfinity;
    TTEntry snapshot;
    TTEntry* tt_entry = nullptr;
    bool is_non_pv = (beta - alpha == 1);

    Move prev_move  = (ply >= 1 && ply < 256) ? move_stack_[ply - 1] : Move();
    Move prev2_move = (ply >= 2 && ply < 256) ? move_stack_[ply - 2] : Move();
    Move prev4_move = (ply >= 4 && ply < 256) ? move_stack_[ply - 4] : Move();
    Move prev6_move = (ply >= 6 && ply < 256) ? move_stack_[ply - 6] : Move();

    Piece prev_piece  = (ply >= 1 && ply < 256) ? piece_stack_[ply - 1] : Piece::None;
    Piece prev2_piece = (ply >= 2 && ply < 256) ? piece_stack_[ply - 2] : Piece::None;
    Piece prev4_piece = (ply >= 4 && ply < 256) ? piece_stack_[ply - 4] : Piece::None;
    Piece prev6_piece = (ply >= 6 && ply < 256) ? piece_stack_[ply - 6] : Piece::None;

    // 1. Transposition Table Probing
    if (use_tt) {
        tt().prefetch(board.zobrist_key());
        tt_entry = tt().probe(board.zobrist_key(), snapshot, tt_statistics_, board.halfmove_clock()) ? &snapshot : nullptr;
        if (tt_entry) {
            if (static_cast<bool>(tt_entry->move)) {
                tt_move = tt_entry->move;
            }
            const bool compatible = tt_entry->score_matches_rule50(board.halfmove_clock());
#if defined(HG_TT_DIAGNOSTICS)
            auto& diagnostic = tt_statistics_.diagnostics;
            diagnostic.compatible_hits += compatible;
            if (!compatible) {
                ++diagnostic.rejected_clock_hits;
                diagnostic.hint_only_hits += static_cast<bool>(tt_move);
                ++diagnostic.rejected_clock_deciles[std::clamp(board.halfmove_clock() / 10, 0, 9)];
            }
#endif
            if (!compatible) tt_entry = nullptr;
        }
        if (tt_entry) {
            if (!excluded_move && tt_entry->depth >= depth) {
                tt_score = tt_entry->score;
                if (tt_score > ScoreMate - 1000) tt_score -= ply;
                else if (tt_score < -ScoreMate + 1000) tt_score += ply;

                if (tt_entry->bound == TTBound::Exact) {
#if defined(HG_TT_DIAGNOSTICS)
                    ++tt_statistics_.diagnostics.score_cutoffs;
#endif
                    return tt_score;
                } else if (tt_entry->bound == TTBound::Lower && tt_score >= beta) {
#if defined(HG_TT_DIAGNOSTICS)
                    ++tt_statistics_.diagnostics.score_cutoffs;
#endif
                    metrics_tracker_.add_cut();
                    return tt_score;
                } else if (tt_entry->bound == TTBound::Upper && tt_score <= alpha) {
#if defined(HG_TT_DIAGNOSTICS)
                    ++tt_statistics_.diagnostics.score_cutoffs;
#endif
                    return tt_score;
                }
            } else if (tt_entry->depth >= depth - 3) {
                tt_score = tt_entry->score;
                if (tt_score > ScoreMate - 1000) tt_score -= ply;
                else if (tt_score < -ScoreMate + 1000) tt_score += ply;
            }
        }
    }

    // 1.2 Internal Iterative Reduction (IIR)
    // If no TT move is found at depth >= 5 at non-PV nodes, reduce search depth by 1 ply to prevent search tree bloat.
    if (is_non_pv && use_tt && !static_cast<bool>(tt_move) && depth >= 5 && !in_chk) {
        depth--;
    }

    // Only proven results may cut: probe_wdl establishes terminal legality first
    // and rejects unproven material-pattern heuristics with NO_SCORE.
    if (!in_chk && popcount(board.occupied()) <= 6) {
        int tb_score = SyzygyTablebase::instance().probe_wdl(board, ply);
        if (tb_score != SyzygyTablebase::NO_SCORE) {
            if (tb_score >= beta) {
                metrics_tracker_.add_cut();
            }
            return tb_score;
        }
    }

    // Reach search horizon -> enter Quiescence Search!
    if (depth <= 0) {
        int q_eval = quiescence_search(board, alpha, beta, ply);
        if (json_node) {
            json_node->eval = q_eval;
        }
        return q_eval;
    }

    // 2. Static Eval for Pruning Decisions (computed once, reused by RFP + Futility + Multi-Table CorrHist)
    int raw_static_eval = 0;
    int static_eval = 0;
    int eval = 0;
    bool can_futility_prune = false;
    bool improving = false;
    size_t c_idx = static_cast<size_t>(us);

    uint64_t w_pawns = board.pieces(Piece::WhitePawn);
    uint64_t b_pawns = board.pieces(Piece::BlackPawn);
    size_t pawn_hash = static_cast<size_t>((w_pawns ^ (b_pawns * 0x9e3779b97f4a7c15ULL)) % 4096);

    uint64_t w_minors = board.pieces(Piece::WhiteKnight) | board.pieces(Piece::WhiteBishop);
    uint64_t b_minors = board.pieces(Piece::BlackKnight) | board.pieces(Piece::BlackBishop);
    size_t non_pawn_hash = static_cast<size_t>((w_minors ^ (b_minors * 0x9e3779b97f4a7c15ULL)) % 4096);

    uint64_t w_majors = board.pieces(Piece::WhiteRook) | board.pieces(Piece::WhiteQueen);
    uint64_t b_majors = board.pieces(Piece::BlackRook) | board.pieces(Piece::BlackQueen);
    size_t major_hash = static_cast<size_t>((w_majors ^ (b_majors * 0x9e3779b97f4a7c15ULL)) % 4096);

    if (!in_chk && std::abs(beta) < ScoreMate - 1000) {
        raw_static_eval = Evaluator::evaluate_fast(board, alpha, beta);
        int pawn_corr = corr_history_[c_idx][pawn_hash];
        int non_pawn_corr = non_pawn_corr_history_[c_idx][non_pawn_hash];
        int major_corr = major_corr_history_[c_idx][major_hash];
        int total_corr = std::clamp((pawn_corr + non_pawn_corr + major_corr) / 256, -1024, 1024);
        static_eval = raw_static_eval + total_corr;

        // TT Evaluation Refinement: Tighten static evaluation bounds using Transposition Table score
        eval = static_eval;
        if (tt_entry && std::abs(tt_score) < ScoreMate - 1000) {
            if (tt_entry->bound == TTBound::Exact) {
                eval = tt_score;
            } else if (tt_entry->bound == TTBound::Lower && tt_score > eval) {
                eval = tt_score;
            } else if (tt_entry->bound == TTBound::Upper && tt_score < eval) {
                eval = tt_score;
            }
        }

        if (ply < 256) eval_stack_[ply] = static_eval;
        if (ply >= 2 && eval_stack_[ply - 2] != -ScoreInfinity) {
            improving = (eval > eval_stack_[ply - 2]);
        }

        // Reverse Futility Pruning (Static Null Move Pruning)
        if (depth <= 6 && !excluded_move) {
            int margin = std::max(60, g_search_params.rfp_margin) * depth - (improving ? 30 : 0);
            if (eval - margin >= beta) {
                metrics_tracker_.add_cut();
                return eval - margin;
            }
        }

        // Forward Futility Pruning flag (checked per-move in the loop below)
        if (depth <= 4 && !excluded_move && eval + g_search_params.futility_margin * depth <= alpha) {
            can_futility_prune = true;
        }
    }

    // 3. Adaptive Null Move Pruning (NMP) with continuous depth & score scaling
    if (g_search_params.enable_nmp && depth >= 3 && !in_chk && !excluded_move && board.has_non_pawn_material(us) &&
        (!g_search_params.enable_nmp_guards || guarded_null_move_allowed(is_non_pv, static_eval, beta, previous_was_null))) {
        int nmp_margin = std::max(1, g_search_params.nmp_eval_margin);
        int R = 3 + depth / 4 + std::min(3, (eval - beta) / nmp_margin);
        if (!improving) {
            R += 1;
        }
        R = std::clamp(R, 1, depth - 1);

        const Move saved_move = move_stack_[ply];
        const Piece saved_piece = piece_stack_[ply];
        if (g_search_params.enable_nmp_guards) {
            move_stack_[ply] = Move();
            piece_stack_[ply] = Piece::None;
        }
        board.make_null_move();

        int null_score = -negamax_alphabeta(board, depth - 1 - R, ply + 1, -beta, -beta + 1, use_move_ordering, use_tt, Move(), nullptr, static_eval, Move(), true);

        board.unmake_null_move();
        if (g_search_params.enable_nmp_guards) {
            move_stack_[ply] = saved_move;
            piece_stack_[ply] = saved_piece;
        }

        if (null_score >= beta) {
            // NMP Verification Search at High Depths (depth >= 12)
            // Eliminates high-depth zugzwang miscalculations and false cutoffs
            if (depth >= 12 && null_score < ScoreMate - 1000) {
                // Sentinel disables NMP at this verification node, not throughout
                // the descendants. Preserve predecessor state at the same ply.
                int verify_score = negamax_alphabeta(board, depth - 1 - R, ply, beta - 1, beta, use_move_ordering, use_tt, Move(), nullptr, static_eval, Move::nmp_verify_sentinel(), previous_was_null);
                if (verify_score >= beta) {
                    metrics_tracker_.add_cut();
                    return beta;
                }
            } else {
                metrics_tracker_.add_cut();
                return beta;
            }
        }
    }

    // 3.5 ProbCut (Probability-Based Cutoffs)
    if (depth >= 5 && !in_chk && !excluded_move && std::abs(beta) < ScoreMate - 1000 && board.has_non_pawn_material(us)) {
        int prob_beta = beta + 200;
        int prob_depth = depth - 4;
        MoveList tactical_moves;
        MoveGenerator::generate_capture_moves(board, tactical_moves);
        move_picker_.score_and_sort_moves(board, tactical_moves, ply, Move(), prev_move, prev2_move, prev4_move, prev6_move, prev_piece, prev2_piece, prev4_piece, prev6_piece);

        for (const auto& tm : tactical_moves) {
            if (!MovePicker::see_ge(board, tm, prob_beta - static_eval)) continue;
            if (ply < 256) {
                move_stack_[ply] = tm;
                piece_stack_[ply] = board.piece_at(tm.from());
            }
            board.make_move(tm);
            int prob_score = -negamax_alphabeta(board, prob_depth, ply + 1, -prob_beta, -prob_beta + 1, use_move_ordering, use_tt, Move(), nullptr, static_eval);
            board.unmake_move(tm);
            if (ply < 256) {
                move_stack_[ply] = Move();
                piece_stack_[ply] = Piece::None;
            }
            if (prob_score >= prob_beta) {
                metrics_tracker_.add_cut();
                return prob_beta;
            }
        }
    }

    // 3.8 Singular Extensions (SE)
    // If TT move is promising at depth >= 7, check if all alternative moves fail low by at least singular_margin.
    int singular_extension = 0;
    if (depth >= 7 && !in_chk && !excluded_move && tt_entry && tt_entry->bound != TTBound::Upper &&
        tt_entry->depth >= depth - 3 && std::abs(tt_score) < ScoreMate - 1000 && static_cast<bool>(tt_move)) {
        int singular_margin = std::max(1, g_search_params.singular_margin) * depth;
        int singular_beta = tt_score - singular_margin;
        int singular_depth = (depth - 1) / 2;

        int singular_score = negamax_alphabeta(board, singular_depth, ply, singular_beta - 1, singular_beta,
                                               use_move_ordering, use_tt, Move(), nullptr, static_eval, tt_move, previous_was_null);

        if (singular_score < singular_beta) {
            singular_extension = 1;
            // Double extension on PV nodes if failed low by a substantial scaled margin
            if (!is_non_pv && singular_score < singular_beta - std::max(50, 3 * singular_margin)) {
                singular_extension = 2;
            }
        } else if (singular_beta >= beta) {
            // Multi-cut: another move exceeded singular_beta >= beta -> immediate cutoff!
            return singular_beta;
        } else if (tt_score >= beta) {
            // Negative extension if TT move was not singular
            singular_extension = -1;
        }
    }

    // 3.9 Multi-Cut Pruning (MC) at high-depth non-PV nodes (depth >= 8)
    MoveList moves;
    bool moves_generated = false;

    if (is_non_pv && depth >= 8 && !in_chk && !excluded_move && std::abs(beta) < ScoreMate - 1000) {
        int mc_depth = depth - 1 - 3;
        MoveGenerator::generate_legal_moves(board, moves);
        moves_generated = true;
        if (moves.size() >= 4) {
            move_picker_.score_and_sort_moves(board, moves, ply, tt_move, prev_move, prev2_move, prev4_move, prev6_move, prev_piece, prev2_piece, prev4_piece, prev6_piece);
            int cutoffs = 0;
            int mc_count = 0;
            for (const auto& mc_m : moves) {
                if (mc_m == tt_move) continue;
                if (++mc_count > 8) break; // Check up to 8 moves

                if (ply < 256) {
                    move_stack_[ply] = mc_m;
                    piece_stack_[ply] = board.piece_at(mc_m.from());
                }
                board.make_move(mc_m);
                int mc_score = -negamax_alphabeta(board, mc_depth, ply + 1, -beta, -beta + 1, use_move_ordering, use_tt, Move(), nullptr, static_eval);
                board.unmake_move(mc_m);
                if (ply < 256) {
                    move_stack_[ply] = Move();
                    piece_stack_[ply] = Piece::None;
                }

                if (mc_score >= beta) {
                    cutoffs++;
                    if (cutoffs >= 3) {
                        metrics_tracker_.add_cut();
                        return beta;
                    }
                }
            }
        }
    }

    // 4. Move Generation & Ordering
    if (!moves_generated) {
        MoveGenerator::generate_legal_moves(board, moves);
    }

    if (moves.empty()) {
        if (in_chk) {
            return -ScoreMate + ply;
        } else {
            return ScoreDraw;
        }
    }

    std::array<int, 256> scores{};
    if (use_move_ordering) {
        move_picker_.score_moves(board, moves, scores, ply, tt_move, prev_move, prev2_move, prev4_move, prev6_move, prev_piece, prev2_piece, prev4_piece, prev6_piece);
    }

    int best_score = -ScoreInfinity;
    Move best_move = moves[0];
    int quiets_searched = 0;
    std::array<Move, 64> quiets_tried{};
    int num_quiets_tried = 0;
    std::array<Move, 32> captures_tried{};
    int num_captures_tried = 0;

    for (size_t i = 0; i < moves.size(); ++i) {
        if (use_move_ordering) {
            MovePicker::pick_best(moves, scores, i);
        }
        Move m = moves[i];
        if (m == excluded_move) continue;

        bool is_quiet = !m.is_capture() && !m.is_promotion();

        int cont1_val = 0;
        int cont2_val = 0;
        int cont4_val = 0;
        int cont6_val = 0;
        int history_val = 0;
        int total_hist = 0;
        bool is_killer = false;
        bool is_counter = false;

        if (is_quiet) {
            is_killer = (m == move_picker_.get_killer_move(ply, 0) || m == move_picker_.get_killer_move(ply, 1));
            is_counter = (static_cast<bool>(prev_move) && m == move_picker_.get_countermove(prev_move));
            history_val = move_picker_.get_history_score(us, m);
            cont1_val = move_picker_.get_continuation_history(board, prev_move, m, prev_piece);
            cont2_val = move_picker_.get_continuation_history_2(board, prev2_move, m, prev2_piece);
            cont4_val = move_picker_.get_continuation_history_4(board, prev4_move, m, prev4_piece);
            cont6_val = move_picker_.get_continuation_history_6(board, prev6_move, m, prev6_piece);
            total_hist = history_val + 2 * cont1_val + cont2_val + cont4_val + (cont6_val / 2);
        }

        // History-Driven Late Move Pruning (LMP): At shallow depth non-PV nodes, prune quiet moves after threshold
        // Killer moves and countermoves are strictly protected from LMP.
        // Moves with positive history receive a higher threshold; negative history moves are pruned earlier.
        if (is_non_pv && depth <= 5 && !in_chk && is_quiet && !is_killer && !is_counter) {
            int lmp_threshold = (3 + 2 * depth * depth) / (improving ? 1 : 2);
            if (total_hist < -2048) {
                lmp_threshold = std::max(1, lmp_threshold - 1);
            } else if (total_hist > 2048) {
                lmp_threshold += 1;
            }
            if (quiets_searched >= lmp_threshold) {
                continue;
            }
        }

        // Fast Futility Pruning: Skip quiet moves at low depth if static eval is far below alpha (Zero make/unmake overhead)
        // Killer moves and countermoves are also protected from futility pruning.
        if (can_futility_prune && i >= 1 && is_quiet && !is_killer && !is_counter) {
            if (!MoveGenerator::gives_check(board, m)) {
                continue;
            }
        }

        // SEE Bad Capture Pruning: Skip losing captures at shallow depth (depth <= 4)
        if (depth <= 4 && !in_chk && i >= 1 && m.is_capture() && !m.is_promotion()) {
            if (!MovePicker::see_ge(board, m, -g_search_params.see_bad_capture_slope * depth)) {
                continue;
            }
        }

        // SEE Quiet Pruning: Skip quiet moves that drop material at shallow depth
        if (depth <= 4 && !in_chk && i >= 1 && is_quiet && !is_killer) {
            if (!MovePicker::see_ge(board, m, -g_search_params.see_quiet_slope * depth)) {
                continue;
            }
        }

        Piece attacker = board.piece_at(m.from());
        Piece victim = board.piece_at(m.to());
        PieceType vic_pt = m.is_ep() ? PieceType::Pawn : piece_type_of(victim);

        bool is_bad_capture = m.is_capture() && i >= 4 && depth >= 4 && !in_chk;
        if (is_bad_capture) {
            int cap_hist = move_picker_.get_capture_history(attacker, m.to(), vic_pt);
            bool see_negative = !MovePicker::see_ge(board, m, 0);
            if (!see_negative && cap_hist >= 0) is_bad_capture = false;
        }

        if (m.is_capture()) {
            if (num_captures_tried < 32) {
                captures_tried[num_captures_tried++] = m;
            }
        } else if (is_quiet) {
            quiets_searched++;
            if (num_quiets_tried < 64) {
                quiets_tried[num_quiets_tried++] = m;
            }
        }

        if (ply < 256) {
            move_stack_[ply] = m;
            piece_stack_[ply] = board.piece_at(m.from());
        }
        board.make_move(m);
        tt().prefetch(board.zobrist_key());

        SearchTreeNode* child_node = exporter_.add_child(json_node, board, m, depth - 1, ply + 1);

        int score = 0;

        if (i == 0) {
            int search_depth = depth - 1 + singular_extension;
            score = -negamax_alphabeta(board, search_depth, ply + 1, -beta, -alpha, use_move_ordering, use_tt, Move(), child_node, static_eval);
        } else {
            int reduction = 0;
            // History-Based Late Move Reductions (LMR) for quiet moves & bad captures
            if (g_search_params.enable_lmr && ((i >= 3 && depth >= 3 && is_quiet && !in_chk) || is_bad_capture)) {
                reduction = lmr_table[std::min(depth, 63)][std::min(i + 1, static_cast<size_t>(63))];
                if (is_quiet) {
                    // Smooth continuous history reduction scaling
                    reduction -= total_hist / 8192;
                    if (!improving) reduction += 1;
                    if (!is_non_pv) reduction = std::max(0, reduction - 1);
                } else {
                    reduction = std::max(1, reduction / 2);
                }
                reduction = std::clamp(reduction, 0, depth - 2);
                int reduced_depth = std::max(1, depth - 1 - reduction);

                score = -negamax_alphabeta(board, reduced_depth, ply + 1, -alpha - 1, -alpha, use_move_ordering, use_tt, Move(), child_node, static_eval);
            } else {
                // Zero-window search
                score = -negamax_alphabeta(board, depth - 1, ply + 1, -alpha - 1, -alpha, use_move_ordering, use_tt, Move(), child_node, static_eval);
            }

            // If reduced search raised alpha (including score >= beta), re-search at full depth with zero window:
            if (reduction > 0 && score > alpha) {
                score = -negamax_alphabeta(board, depth - 1, ply + 1, -alpha - 1, -alpha, use_move_ordering, use_tt, Move(), child_node, static_eval);
            }

            // PVS Re-Search: If zero-window search raised alpha at a PV node (score < beta), re-search with full [alpha, beta] window!
            if (score > alpha && score < beta) {
                score = -negamax_alphabeta(board, depth - 1, ply + 1, -beta, -alpha, use_move_ordering, use_tt, Move(), child_node, static_eval);
            }
        }

        board.unmake_move(m);
        if (ply < 256) {
            move_stack_[ply] = Move();
            piece_stack_[ply] = Piece::None;
        }

        if (is_stopped()) return 0;

        if (score > best_score) {
            best_score = score;
            best_move = m;
        }

        if (score >= beta) {
            metrics_tracker_.add_cut();
            if (use_move_ordering) {
                if (m.is_capture()) {
                    Piece attacker = board.piece_at(m.from());
                    Piece victim   = board.piece_at(m.to());
                    PieceType vic_pt = m.is_ep() ? PieceType::Pawn : piece_type_of(victim);
                    move_picker_.add_capture_history(attacker, m.to(), vic_pt, depth);

                    // Penalize any prior captures that failed to produce a beta cutoff
                    for (int c = 0; c < num_captures_tried; ++c) {
                        Move failed_c = captures_tried[c];
                        if (failed_c != m) {
                            Piece f_att = board.piece_at(failed_c.from());
                            Piece f_vic = board.piece_at(failed_c.to());
                            PieceType f_vic_pt = failed_c.is_ep() ? PieceType::Pawn : piece_type_of(f_vic);
                            move_picker_.sub_capture_history(f_att, failed_c.to(), f_vic_pt, depth);
                        }
                    }
                } else {
                    move_picker_.add_killer_move(ply, m);
                    move_picker_.add_history_score(board.side_to_move(), m, depth);
                    if (static_cast<bool>(prev_move)) {
                        move_picker_.add_countermove(prev_move, m);
                        move_picker_.add_continuation_history(board, prev_move, m, depth, prev_piece);
                    }
                    if (static_cast<bool>(prev2_move)) {
                        move_picker_.add_continuation_history_2(board, prev2_move, m, depth, prev2_piece);
                    }
                    if (static_cast<bool>(prev4_move)) {
                        move_picker_.add_continuation_history_4(board, prev4_move, m, depth, prev4_piece);
                    }
                    if (static_cast<bool>(prev6_move)) {
                        move_picker_.add_continuation_history_6(board, prev6_move, m, depth, prev6_piece);
                    }

                    // Penalize any prior captures that failed to produce a cutoff
                    for (int c = 0; c < num_captures_tried; ++c) {
                        Move failed_c = captures_tried[c];
                        Piece f_att = board.piece_at(failed_c.from());
                        Piece f_vic = board.piece_at(failed_c.to());
                        PieceType f_vic_pt = failed_c.is_ep() ? PieceType::Pawn : piece_type_of(f_vic);
                        move_picker_.sub_capture_history(f_att, failed_c.to(), f_vic_pt, depth);
                    }

                    // History Malus: Penalize all quiet moves searched prior to this cutoff
                    for (int q = 0; q < num_quiets_tried; ++q) {
                        Move failed_q = quiets_tried[q];
                        if (failed_q != m) {
                            move_picker_.sub_history_score(us, failed_q, depth);
                            if (static_cast<bool>(prev_move)) {
                                move_picker_.sub_continuation_history(board, prev_move, failed_q, depth, prev_piece);
                            }
                            if (static_cast<bool>(prev2_move)) {
                                move_picker_.sub_continuation_history_2(board, prev2_move, failed_q, depth, prev2_piece);
                            }
                            if (static_cast<bool>(prev4_move)) {
                                move_picker_.sub_continuation_history_4(board, prev4_move, failed_q, depth, prev4_piece);
                            }
                            if (static_cast<bool>(prev6_move)) {
                                move_picker_.sub_continuation_history_6(board, prev6_move, failed_q, depth, prev6_piece);
                            }
                        }
                    }
                }
            }
            if (use_tt && !excluded_move) {
                tt().store(board.zobrist_key(), m, score, depth, TTBound::Lower, ply, board.halfmove_clock(), &tt_statistics_);
            }
            if (!in_chk && raw_static_eval != 0 && std::abs(score) < ScoreMate - 1000 && !excluded_move) {
                int err = score - raw_static_eval;
                err = std::clamp(err, -1024, 1024);
                int bonus = (err * depth) / 8;
                bonus = std::clamp(bonus, -256, 256);
                corr_history_[c_idx][pawn_hash] = std::clamp(corr_history_[c_idx][pawn_hash] + bonus, -1024, 1024);
                non_pawn_corr_history_[c_idx][non_pawn_hash] = std::clamp(non_pawn_corr_history_[c_idx][non_pawn_hash] + bonus, -1024, 1024);
                major_corr_history_[c_idx][major_hash] = std::clamp(major_corr_history_[c_idx][major_hash] + bonus, -1024, 1024);
            }
            if (child_node) {
                child_node->is_pruned = true;
            }
            return beta;
        }

        if (score > alpha) {
            alpha = score;
            pv_table_.update(ply, m);
        }
    }

    if (!in_chk && raw_static_eval != 0 && std::abs(best_score) < ScoreMate - 1000 && !excluded_move) {
        int err = best_score - raw_static_eval;
        err = std::clamp(err, -1024, 1024);
        int bonus = (err * depth) / 8;
        bonus = std::clamp(bonus, -256, 256);
        corr_history_[c_idx][pawn_hash] = std::clamp(corr_history_[c_idx][pawn_hash] + bonus, -1024, 1024);
        non_pawn_corr_history_[c_idx][non_pawn_hash] = std::clamp(non_pawn_corr_history_[c_idx][non_pawn_hash] + bonus, -1024, 1024);
        major_corr_history_[c_idx][major_hash] = std::clamp(major_corr_history_[c_idx][major_hash] + bonus, -1024, 1024);
    }

    if (use_tt && !time_stop_flag_ && !excluded_move) {
        TTBound bound = (best_score <= orig_alpha) ? TTBound::Upper : TTBound::Exact;
        tt().store(board.zobrist_key(), best_move, best_score, depth, bound, ply, board.halfmove_clock(), &tt_statistics_);
    }

    if (json_node) {
        json_node->eval = best_score;
    }

    return best_score;
}

SearchResult SearchEngine::search_minimax(Board& board, int depth, bool export_tree) {
    prepare_search(0.0, 0.0);
    pv_table_.clear();
    metrics_tracker_.start_timer();
    metrics_tracker_.set_version("v1.0 (Minimax)");
    metrics_tracker_.set_depth(depth);

    if (export_tree) {
        exporter_.reset(board);
    }

    MoveList moves;
    MoveGenerator::generate_legal_moves(board, moves);

    SearchResult result;

    if (moves.empty() || !board.can_push_history()) {
        result.best_move = Move();
        result.best_score = moves.empty() ? (MoveGenerator::in_check(board, board.side_to_move()) ? -ScoreMate : ScoreDraw) : Evaluator::evaluate_fast(board);
        metrics_tracker_.stop_timer();
        result.metrics = metrics_tracker_.get_metrics();
        return result;
    }

    int best_score = -ScoreInfinity;
    Move best_move = moves[0];

    SearchTreeNode* root_json = export_tree ? exporter_.trace_root() : nullptr;

    for (const auto& m : moves) {
        board.make_move(m);

        SearchTreeNode* child_json = exporter_.add_child(root_json, board, m, depth - 1, 1);

        int score = -negamax_minimax(board, depth - 1, 1, child_json);

        board.unmake_move(m);

        if (score > best_score) {
            best_score = score;
            best_move = m;
            pv_table_.set_move(0, m);
            pv_table_.update(0, m);
        }
    }

    metrics_tracker_.stop_timer();

    result.best_move = best_move;
    result.best_score = best_score;
    if (root_json) { root_json->eval = best_score; root_json->depth = depth; }
    result.pv = pv_table_.get_pv(depth);
    result.depth = result.completed_depth = depth;
    result.metrics = metrics_tracker_.get_metrics();

    return result;
}

SearchResult SearchEngine::search_alphabeta(Board& board, int depth, bool use_move_ordering, bool use_tt, bool export_tree) {
    prepare_search(0.0, 0.0);
    pv_table_.clear();
    move_picker_.clear();
    if (use_tt) tt().clear();
    q_nodes_ = 0;
    Evaluator::reset_incremental_cache();

    metrics_tracker_.start_timer();
    metrics_tracker_.set_version("v10.0 (Master Edition)");
    metrics_tracker_.set_depth(depth);

    time_stop_flag_ = false;
    max_time_ms_ = 0.0;

    if (export_tree) {
        exporter_.reset(board);
    }

    MoveList moves;
    MoveGenerator::generate_legal_moves(board, moves);

    SearchResult result;

    if (board.halfmove_clock() >= 100 && board.can_push_history()) {
        result.best_move = moves.empty() ? Move{} : moves[0];
        result.best_score = moves.empty() && MoveGenerator::in_check(board, board.side_to_move()) ? -ScoreMate : ScoreDraw;
        metrics_tracker_.stop_timer();
        result.metrics = metrics_tracker_.get_metrics();
        finish_tt_statistics(result);
        return result;
    }

    if (book_enabled_ && polyglot_book_.is_loaded()) {
        Move book_move = polyglot_book_.probe(board);
        if (static_cast<bool>(book_move)) {
            result.best_move = book_move;
            result.best_score = 0;
            metrics_tracker_.stop_timer();
            result.metrics = metrics_tracker_.get_metrics();
            return result;
        }
    }

    if (moves.empty() || !board.can_push_history()) {
        result.best_move = Move();
        result.best_score = moves.empty() ? (MoveGenerator::in_check(board, board.side_to_move()) ? -ScoreMate : ScoreDraw) : Evaluator::evaluate_fast(board);
        metrics_tracker_.stop_timer();
        result.metrics = metrics_tracker_.get_metrics();
        return result;
    }

    if (use_move_ordering) {
        move_picker_.score_and_sort_moves(board, moves, 0);
    }

    int alpha = -ScoreInfinity;
    int beta  =  ScoreInfinity;
    int best_score = -ScoreInfinity;
    Move best_move = moves[0];

    SearchTreeNode* root_json = export_tree ? exporter_.trace_root() : nullptr;

    for (size_t i = 0; i < moves.size(); ++i) {
        const auto& m = moves[i];
        move_stack_[0] = m;
        piece_stack_[0] = board.piece_at(m.from());
        board.make_move(m);

        SearchTreeNode* child_json = exporter_.add_child(root_json, board, m, depth - 1, 1);

        int score = 0;
        if (i == 0) {
            score = -negamax_alphabeta(board, depth - 1, 1, -beta, -alpha, use_move_ordering, use_tt, Move(), child_json);
        } else {
            score = -negamax_alphabeta(board, depth - 1, 1, -alpha - 1, -alpha, use_move_ordering, use_tt, Move(), child_json);
            if (score > alpha && score < beta) {
                score = -negamax_alphabeta(board, depth - 1, 1, -beta, -alpha, use_move_ordering, use_tt, Move(), child_json);
            }
        }

        board.unmake_move(m);
        move_stack_[0] = Move();
        piece_stack_[0] = Piece::None;

        if (score > best_score) {
            best_score = score;
            best_move = m;
        }

        if (score > alpha) {
            alpha = score;
            pv_table_.set_move(0, m);
            pv_table_.update(0, m);
        }
    }

    metrics_tracker_.stop_timer();

    result.best_move = best_move;
    result.best_score = best_score;
    if (root_json) { root_json->eval = best_score; root_json->depth = depth; }
    result.pv = pv_table_.get_pv(depth);
    result.depth = result.completed_depth = depth;
    result.metrics = metrics_tracker_.get_metrics();
    finish_tt_statistics(result);
    result.q_nodes = q_nodes_;

    return result;
}

void SearchEngine::iterative_deepening_root(Board& board, int max_depth, uint64_t max_nodes, SearchResult& final_result) {
    Move best_pv_move = Move();
    int last_score = 0;
    int stable_move_count = 0;

    for (int d = 1; d <= max_depth; ++d) {
        if (d > 1 && is_time_up()) break;
        if (max_nodes > 0 && metrics_tracker_.get_metrics().total_nodes >= max_nodes) break;
        metrics_tracker_.set_depth(d);

        MoveList moves;
        MoveGenerator::generate_legal_moves(board, moves);

        if (moves.empty()) {
            final_result.best_score = MoveGenerator::in_check(board, board.side_to_move()) ? -ScoreMate : ScoreDraw;
            break;
        }
        if (!board.can_push_history()) {
            final_result.best_score = Evaluator::evaluate_fast(board);
            break;
        }
        move_picker_.score_and_sort_moves(board, moves, 0, best_pv_move);
        if (!final_result.best_move) final_result.best_move = moves[0];

        int alpha = -ScoreInfinity;
        int beta  =  ScoreInfinity;
        int window_delta = std::max(4, g_search_params.aspiration_window_delta);

        if (d >= 4 && std::abs(last_score) < ScoreMate - 1000) {
            alpha = std::max(-ScoreInfinity, last_score - window_delta);
            beta  = std::min(ScoreInfinity, last_score + window_delta);
        }

        int current_best_score = -ScoreInfinity;
        Move current_best_move = moves[0];
        bool interrupted = false;

        while (true) {
            current_best_score = -ScoreInfinity;
            int search_alpha = alpha;

            for (size_t i = 0; i < moves.size(); ++i) {
                const auto& m = moves[i];
                move_stack_[0] = m;
                piece_stack_[0] = board.piece_at(m.from());
                board.make_move(m);

                int score = 0;
                if (i == 0) {
                    score = -negamax_alphabeta(board, d - 1, 1, -beta, -search_alpha, true, true, Move(), nullptr);
                } else {
                    score = -negamax_alphabeta(board, d - 1, 1, -search_alpha - 1, -search_alpha, true, true, Move(), nullptr);
                    if (score > search_alpha && score < beta) {
                        score = -negamax_alphabeta(board, d - 1, 1, -beta, -search_alpha, true, true, Move(), nullptr);
                    }
                }

                board.unmake_move(m);
                move_stack_[0] = Move();
                piece_stack_[0] = Piece::None;

                if (is_stopped()) {
                    interrupted = true;
                    break;
                }

                if (score > current_best_score) {
                    current_best_score = score;
                    current_best_move = m;
                }

                if (score > search_alpha) {
                    search_alpha = score;
                    pv_table_.set_move(0, m);
                    pv_table_.update(0, m);
                }

                // Check every root move. Never replace a completed iteration
                // with an interrupted deeper one, or claim a partial root as exact.
                if (max_time_ms_ > 0.0 && is_time_up()) {
                    interrupted = i + 1 < moves.size();
                    break;
                }

                // Fail-High Early Break & Hoisting in Aspiration Window:
                // If score >= beta, the aspiration window is exceeded.
                // Break immediately and hoist the cut move to index 0 so it is searched first on widened re-search.
                if (score >= beta) {
                    if (i > 0) {
                        std::swap(moves[0], moves[i]);
                    }
                    break;
                }
            }

            if (interrupted) break;

            if (current_best_score <= alpha) {
                opt_time_ms_ = std::min(max_time_ms_, opt_time_ms_ * 1.5);
                if (alpha <= -ScoreInfinity) break;
                alpha = std::max(-ScoreInfinity, alpha - window_delta);
                window_delta += window_delta / 2;
            } else if (current_best_score >= beta) {
                if (beta >= ScoreInfinity) break;
                beta = std::min(ScoreInfinity, beta + window_delta);
                window_delta += window_delta / 2;
            } else {
                break;
            }
        }

        if (interrupted) {
            if (!final_result.completed_depth && current_best_score > -ScoreInfinity)
                final_result.best_move = current_best_move;
            break;
        }

        if (current_best_move == best_pv_move) {
            stable_move_count++;
        } else {
            stable_move_count = 0;
        }

        best_pv_move = current_best_move;
        last_score   = current_best_score;

        final_result.best_move = current_best_move;
        final_result.best_score = current_best_score;
        final_result.pv = pv_table_.get_pv(d);
        final_result.depth = d;
        final_result.completed_depth = d;
        final_result.tt_hits = tt_statistics_.hits;
        final_result.tt_probes = tt_statistics_.probes;
        final_result.q_nodes = q_nodes_;

        if (uci_output_) {
            auto now = std::chrono::steady_clock::now();
            uint64_t elapsed_ms = std::max<uint64_t>(1, std::chrono::duration_cast<std::chrono::milliseconds>(now - search_start_time_).count());
            uint64_t total_nodes = node_count_;
            uint64_t nps = (total_nodes * 1000) / elapsed_ms;

            std::cout << "info depth " << d
                      << " score ";
            if (std::abs(current_best_score) >= ScoreMate - 1000) {
                int mate_plies = ScoreMate - std::abs(current_best_score);
                int mate_moves = (mate_plies + 1) / 2;
                if (current_best_score < 0) mate_moves = -mate_moves;
                std::cout << "mate " << mate_moves;
            } else {
                std::cout << "cp " << current_best_score;
            }
            std::cout << " nodes " << total_nodes
                      << " nps " << nps
                      << " time " << elapsed_ms
                      << " hashfull " << tt().hashfull()
                      << " pv";
            for (const auto& pv_m : final_result.pv) {
                std::cout << " " << move_to_uci_buffer(pv_m).data();
            }
            std::cout << std::endl;
        }

        // High-depth stable move early exit (saves clock time safely on rock-solid moves)
        if (opt_time_ms_ > 0.0 && d >= 12 && stable_move_count >= 5) {
            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double, std::milli>(now - search_start_time_).count();
            if (elapsed >= opt_time_ms_ * 0.85) {
                break;
            }
        }

        // Soft Time Allocation: Stop iterative deepening between iterations if optimum time expired
        if (opt_time_ms_ > 0.0 && d >= 5) {
            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double, std::milli>(now - search_start_time_).count();
            if (elapsed >= opt_time_ms_ || (stable_move_count >= 4 && elapsed >= opt_time_ms_ * 0.70)) {
                break;
            }
        }

        if (is_time_up() || is_stopped()) break;
    }
}

SearchResult SearchEngine::search_iterative_deepening(Board& board, int max_depth, double max_time_ms, uint64_t max_nodes, double opt_time_ms) {
    prepare_search(max_time_ms, opt_time_ms);
    max_nodes_ = max_nodes;
    if (board.halfmove_clock() >= 100 && board.can_push_history()) {
        MoveList legal;
        MoveGenerator::generate_legal_moves(board, legal);
        SearchResult result;
        result.best_move = legal.empty() ? Move{} : legal[0];
        result.best_score = legal.empty() && MoveGenerator::in_check(board, board.side_to_move()) ? -ScoreMate : ScoreDraw;
        metrics_tracker_.stop_timer();
        result.metrics = metrics_tracker_.get_metrics();
        finish_tt_statistics(result);
        return result;
    }
    if (book_enabled_ && polyglot_book_.is_loaded()) {
        Move book_move = polyglot_book_.probe(board);
        if (static_cast<bool>(book_move)) {
            SearchResult res;
            res.best_move = book_move;
            res.best_score = Evaluator::evaluate_fast(board);
            res.depth = 1;
            res.completed_depth = 1;
            res.pv = {book_move};
            metrics_tracker_.stop_timer();
            res.metrics = metrics_tracker_.get_metrics();
            return res;
        }
    }

    tt().new_search();

    SearchResult final_result;

#if defined(_OPENMP)
    const double elapsed_ms = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - search_start_time_).count();
    int n_threads = max_nodes_ > 0 || (max_time_ms_ > 0.0 &&
        max_time_ms_ - elapsed_ms <= timing_options_.minimum_smp_time_ms) ? 1 : num_threads_;
    if (n_threads > 1) {
        // No thread can read the mutable root after this immutable snapshot phase.
        const EvalMode mode = Evaluator::mode();
        for (int i = 0; i < n_threads - 1; ++i) {
            auto& worker = *workers_[i];
            worker.worker_board_ = board;
            worker.timing_options_ = timing_options_;
            worker.prepare_search(max_time_ms, opt_time_ms);
            worker.search_start_time_ = search_start_time_;
        }
        #pragma omp parallel num_threads(n_threads)
        {
            int tid = omp_get_thread_num();
            if (tid == 0) {
                final_result.threads_used = omp_get_num_threads();
                iterative_deepening_root(board, max_depth, max_nodes, final_result);
                time_stop_flag_.store(true, std::memory_order_relaxed);
            } else {
                // Helper thread (sharing master TT, private thread-local MovePicker history tables)
                SearchEngine& helper = *workers_[tid - 1];
                Board& helper_board = helper.worker_board_;
                Evaluator::set_mode(mode);

                int depth_offset = (tid % 2);
                for (int d = 1 + depth_offset; d <= max_depth; ++d) {
                    if (helper.is_time_up() || !helper_board.can_push_history()) break;

                    MoveList moves;
                    MoveGenerator::generate_legal_moves(helper_board, moves);
                    if (moves.empty()) break;

                    helper.move_picker_.score_and_sort_moves(helper_board, moves, 0);

                    for (const auto& m : moves) {
                        if (helper.is_stopped()) break;
                        helper.move_stack_[0] = m;
                        helper.piece_stack_[0] = helper_board.piece_at(m.from());
                        helper_board.make_move(m);
                        helper.negamax_alphabeta(helper_board, d - 1, 1, -ScoreInfinity, ScoreInfinity, true, true, Move(), nullptr);
                        helper_board.unmake_move(m);
                        helper.move_stack_[0] = Move();
                        helper.piece_stack_[0] = Piece::None;
                    }
                }
            }
        }
        // The implicit barrier makes aggregation safe; workers count privately.
        for (int i = 0; i < n_threads - 1; ++i) {
            metrics_tracker_.add_nodes(workers_[i]->node_count_);
            node_count_ += workers_[i]->node_count_;
            q_nodes_ += workers_[i]->q_nodes_;
            tt_statistics_.add(workers_[i]->tt_statistics_);
        }
    } else
#endif
    {
        iterative_deepening_root(board, max_depth, max_nodes, final_result);
    }

    metrics_tracker_.stop_timer();
    final_result.metrics = metrics_tracker_.get_metrics();
    final_result.q_nodes = q_nodes_;
    finish_tt_statistics(final_result);
    if (uci_output_) {
        // After the SMP barrier this is an exact all-worker, current-go total.
        const auto elapsed_ms = static_cast<uint64_t>(std::max(1.0, final_result.metrics.elapsed_seconds * 1000.0));
        std::cout << "info nodes " << node_count_ << " nps " << (node_count_ * 1000 / elapsed_ms)
                  << " time " << elapsed_ms << std::endl;
#if defined(HG_TT_DIAGNOSTICS)
        const auto& diagnostic = final_result.tt_diagnostics;
        std::cout << "info string tt_audit {\"probes\":" << final_result.tt_probes
                  << ",\"hits\":" << final_result.tt_hits
                  << ",\"compatible_hits\":" << diagnostic.compatible_hits
                  << ",\"rejected_clock_hits\":" << diagnostic.rejected_clock_hits
                  << ",\"hint_only_hits\":" << diagnostic.hint_only_hits
                  << ",\"score_cutoffs\":" << diagnostic.score_cutoffs
                  << ",\"store_attempts\":" << diagnostic.store_attempts
                  << ",\"stored\":" << diagnostic.stored
                  << ",\"context_replacements\":" << diagnostic.context_replacements
                  << ",\"shallower_context_replacements\":" << diagnostic.shallower_context_replacements
                  << ",\"qsearch_context_replacements_of_deeper\":" << diagnostic.qsearch_context_replacements_of_deeper
                  << ",\"rejected_clock_deciles\":[";
        for (size_t i = 0; i < diagnostic.rejected_clock_deciles.size(); ++i) {
            if (i) std::cout << ',';
            std::cout << diagnostic.rejected_clock_deciles[i];
        }
        std::cout << "]}" << std::endl;
#endif
    }

    return final_result;
}

SearchResult SearchEngine::search_smp(Board& board, int max_depth, int num_threads) {
    set_threads(num_threads);
    return search_iterative_deepening(board, max_depth, 0.0);
}

} // namespace heavensgate
