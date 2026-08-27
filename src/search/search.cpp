#include "search.hpp"
#include "syzygy.hpp"
#include "search_params.hpp"
#include "../core/fen.hpp"
#include "../evaluation/eval.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#if defined(_OPENMP)
#include <omp.h>
#endif

namespace heavensgate {

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

int SearchEngine::quiescence_search(Board& board, int alpha, int beta, int ply) {
    if (ply >= 64) {
        return Evaluator::evaluate_fast(board);
    }

    metrics_tracker_.add_nodes(1);
    q_nodes_++;
    node_count_++;

    if (time_stop_flag_ || ((node_count_ & 2047) == 0 && is_time_up())) return 0;

    Color us = board.side_to_move();
    bool in_chk = MoveGenerator::in_check(board, us);

    // Hybrid Eval Strategy:
    // PV nodes (full window: alpha+1 < beta) use full SpectralTropical eval for quality
    // Non-PV nodes (zero window: alpha+1 == beta) use fast Material+PST eval for speed
    bool is_pv = (beta - alpha) > 1;
    int stand_pat = is_pv ? Evaluator::evaluate(board) : Evaluator::evaluate_fast(board);
    if (!in_chk) {
        if (stand_pat >= beta) {
            return beta;
        }

        if (stand_pat > alpha) {
            alpha = stand_pat;
        }
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

    move_picker_.score_and_sort_moves(board, moves, ply);

    for (const auto& m : moves) {
        if (!in_chk) {
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

        if (time_stop_flag_) return 0;

        if (score >= beta) {
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

int SearchEngine::negamax_minimax(Board& board, int depth, int ply, TreeNodeJSON* json_node) {
    metrics_tracker_.add_nodes(1);

    if (depth <= 0) {
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

        TreeNodeJSON* child_node = nullptr;
        if (json_node) {
            json_node->children.push_back(TreeNodeJSON{});
            child_node = &json_node->children.back();
            child_node->move_uci = move_to_uci(m);
            child_node->fen = FEN::to_string(board);
            child_node->depth = depth - 1;
            child_node->ply = ply + 1;
        }

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

int SearchEngine::negamax_alphabeta(Board& board, int depth, int ply, int alpha, int beta, bool use_move_ordering, bool use_tt, Move pv_move, Move prev_move, Move prev2_move, TreeNodeJSON* json_node, int prev_eval, Move excluded_move) {
    metrics_tracker_.add_nodes(1);
    node_count_++;

    if (time_stop_flag_ || ((node_count_ & 2047) == 0 && is_time_up())) return 0;

    if (ply > 0 && board.is_repetition(2)) {
        return ScoreDraw;
    }

    Color us = board.side_to_move();
    bool in_chk = MoveGenerator::in_check(board, us);

    // Check Extension (ply < 16) to prevent checkmate blind spots in middlegame/endgame
    if (in_chk && ply < 16 && depth > 1) {
        depth++;
    }

    int orig_alpha = alpha;
    Move tt_move = pv_move;
    int tt_score = -ScoreInfinity;
    TTEntry* tt_entry = nullptr;
    bool is_non_pv = (beta - alpha == 1);

    // 1. Transposition Table Probing
    if (use_tt) {
        tt().prefetch(board.zobrist_key());
        tt_entry = tt().probe(board.zobrist_key());
        if (tt_entry) {
            if (static_cast<bool>(tt_entry->move)) {
                tt_move = tt_entry->move;
            }

            if (!excluded_move && tt_entry->depth >= depth) {
                tt_score = tt_entry->score;
                if (tt_score > ScoreMate - 1000) tt_score -= ply;
                else if (tt_score < -ScoreMate + 1000) tt_score += ply;

                if (tt_entry->bound == TTBound::Exact) {
                    return tt_score;
                } else if (tt_entry->bound == TTBound::Lower && tt_score >= beta) {
                    metrics_tracker_.add_cut();
                    return tt_score;
                } else if (tt_entry->bound == TTBound::Upper && tt_score <= alpha) {
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

    // 1.5 Syzygy Tablebase Probe (6 or fewer pieces, 0.00ms overhead)
    if (popcount(board.occupied()) <= 6) {
        int tb_score = SyzygyTablebase::instance().probe_wdl(board, ply);
        if (tb_score != SyzygyTablebase::NO_SCORE) {
            metrics_tracker_.add_cut();
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
        raw_static_eval = Evaluator::evaluate_fast(board);
        int pawn_corr = corr_history_[c_idx][pawn_hash];
        int non_pawn_corr = non_pawn_corr_history_[c_idx][non_pawn_hash];
        int major_corr = major_corr_history_[c_idx][major_hash];
        int total_corr = std::clamp((pawn_corr + non_pawn_corr + major_corr) / 256, -1024, 1024);
        static_eval = raw_static_eval + total_corr;

        if (ply >= 2 && prev_eval != -ScoreInfinity) {
            improving = (static_eval > prev_eval);
        }

        // Reverse Futility Pruning (Static Null Move Pruning)
        if (depth <= 3 && !excluded_move) {
            int margin = g_search_params.rfp_margin * depth;
            if (static_eval - margin >= beta) {
                metrics_tracker_.add_cut();
                return static_eval - margin;
            }
        }

        // Forward Futility Pruning flag (checked per-move in the loop below)
        if (depth <= 2 && !excluded_move && static_eval + g_search_params.futility_margin * depth <= alpha) {
            can_futility_prune = true;
        }
    }

    // 3. Adaptive Null Move Pruning (NMP) (R=2 for shallow depth, R=3 for depth >= 6, R=4 if static_eval >= beta + nmp_eval_margin, +1 if !improving)
    if (depth >= 3 && !in_chk && !excluded_move && board.has_non_pawn_material(us)) {
        int R = (depth >= 6) ? 3 : 2;
        if (static_eval - beta >= g_search_params.nmp_eval_margin) {
            R += 1;
        }
        if (!improving && depth >= 4) {
            R += 1;
        }
        R = std::min(R, depth - 1);

        board.make_null_move();

        int null_score = -negamax_alphabeta(board, depth - 1 - R, ply + 1, -beta, -beta + 1, use_move_ordering, use_tt, Move(), Move(), Move(), nullptr, static_eval);

        board.unmake_null_move();

        if (null_score >= beta) {
            metrics_tracker_.add_cut();
            return beta;
        }
    }

    // 3.5 ProbCut (Probability-Based Cutoffs)
    if (depth >= 5 && !in_chk && !excluded_move && std::abs(beta) < ScoreMate - 1000 && board.has_non_pawn_material(us)) {
        int prob_beta = beta + 200;
        int prob_depth = depth - 4;
        MoveList tactical_moves;
        MoveGenerator::generate_capture_moves(board, tactical_moves);
        move_picker_.score_and_sort_moves(board, tactical_moves, ply);

        for (const auto& tm : tactical_moves) {
            if (!MovePicker::see_ge(board, tm, prob_beta - static_eval)) continue;
            board.make_move(tm);
            int prob_score = -negamax_alphabeta(board, prob_depth, ply + 1, -prob_beta, -prob_beta + 1, use_move_ordering, use_tt, Move(), tm, Move(), nullptr, static_eval);
            board.unmake_move(tm);
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
        int singular_margin = 2 * depth;
        int singular_beta = tt_score - singular_margin;
        int singular_depth = (depth - 1) / 2;

        int singular_score = negamax_alphabeta(board, singular_depth, ply, singular_beta - 1, singular_beta,
                                               use_move_ordering, use_tt, Move(), prev_move, prev2_move, nullptr, static_eval, tt_move);

        if (singular_score < singular_beta) {
            singular_extension = 1;
            // Double extension on PV nodes if failed low by a substantial margin
            if (!is_non_pv && singular_score < singular_beta - 25) {
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

    // 4. Move Generation & Ordering
    MoveList moves;
    MoveGenerator::generate_legal_moves(board, moves);

    if (moves.empty()) {
        if (in_chk) {
            return -ScoreMate + ply;
        } else {
            return ScoreDraw;
        }
    }

    if (use_move_ordering) {
        move_picker_.score_and_sort_moves(board, moves, ply, tt_move, prev_move, prev2_move);
    }

    int best_score = -ScoreInfinity;
    Move best_move = moves[0];
    int quiets_searched = 0;

    for (size_t i = 0; i < moves.size(); ++i) {
        Move m = moves[i];
        if (m == excluded_move) continue;

        bool is_quiet = !m.is_capture() && !m.is_promotion();

        // Late Move Pruning (LMP): At shallow depth non-PV nodes, prune quiet moves after threshold
        if (is_non_pv && depth <= 5 && !in_chk && is_quiet) {
            int lmp_threshold = 3 + 2 * depth * depth;
            if (quiets_searched >= lmp_threshold) {
                continue;
            }
        }

        // Futility Pruning: Skip quiet moves at low depth if static eval is far below alpha
        if (can_futility_prune && i >= 1 && is_quiet) {
            board.make_move(m);
            bool gives_chk = MoveGenerator::in_check(board, ~us);
            board.unmake_move(m);
            if (!gives_chk) {
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
        if (depth <= 4 && !in_chk && i >= 1 && is_quiet) {
            if (!MovePicker::see_ge(board, m, -g_search_params.see_quiet_slope * depth)) {
                continue;
            }
        }

        if (is_quiet) quiets_searched++;

        board.make_move(m);

        TreeNodeJSON* child_node = nullptr;
        if (json_node) {
            json_node->children.push_back(TreeNodeJSON{});
            child_node = &json_node->children.back();
            child_node->move_uci = move_to_uci(m);
            child_node->fen = FEN::to_string(board);
            child_node->depth = depth - 1;
            child_node->ply = ply + 1;
        }

        int score = 0;

        if (i == 0) {
            int search_depth = depth - 1 + singular_extension;
            score = -negamax_alphabeta(board, search_depth, ply + 1, -beta, -alpha, use_move_ordering, use_tt, Move(), m, prev_move, child_node, static_eval);
        } else {
            // History-Based Late Move Reductions (LMR) for quiet moves (extra reduction when !improving)
            if (i >= 3 && depth >= 3 && !m.is_capture() && !m.is_promotion() && !in_chk) {
                int reduction = lmr_table[std::min(depth, 63)][std::min(i + 1, static_cast<size_t>(63))];
                int history_val = move_picker_.get_history_score(us, m);
                int cont_val = move_picker_.get_continuation_history(board, prev_move, m);
                int cont2_val = move_picker_.get_continuation_history_2(board, prev2_move, m);
                int total_hist = history_val + cont_val + cont2_val;

                if (total_hist > g_search_params.lmr_hist_bonus) reduction = std::max(1, reduction - 1);
                if (total_hist < g_search_params.lmr_hist_malus && i >= 6) reduction += 1;
                if (!improving) reduction += 1;
                if (!is_non_pv) reduction = std::max(0, reduction - 1);
                reduction = std::clamp(reduction, 0, depth - 2);
                int reduced_depth = std::max(1, depth - 1 - reduction);

                score = -negamax_alphabeta(board, reduced_depth, ply + 1, -alpha - 1, -alpha, use_move_ordering, use_tt, Move(), m, prev_move, child_node, static_eval);
            } else {
                // Zero-window search
                score = -negamax_alphabeta(board, depth - 1, ply + 1, -alpha - 1, -alpha, use_move_ordering, use_tt, Move(), m, prev_move, child_node, static_eval);
            }

            // PVS Re-Search: If zero-window search raised alpha, re-search with full [alpha, beta] window!
            if (score > alpha && score < beta) {
                score = -negamax_alphabeta(board, depth - 1, ply + 1, -beta, -alpha, use_move_ordering, use_tt, Move(), m, prev_move, child_node, static_eval);
            }
        }

        board.unmake_move(m);

        if (time_stop_flag_) return 0;

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
                    move_picker_.add_capture_history(attacker, m.to(), piece_type_of(victim), depth);
                } else {
                    move_picker_.add_killer_move(ply, m);
                    move_picker_.add_history_score(board.side_to_move(), m, depth);
                    if (static_cast<bool>(prev_move)) {
                        move_picker_.add_countermove(prev_move, m);
                        move_picker_.add_continuation_history(board, prev_move, m, depth);
                    }
                    if (static_cast<bool>(prev2_move)) {
                        move_picker_.add_continuation_history_2(board, prev2_move, m, depth);
                    }
                }
            }
            if (use_tt && !excluded_move) {
                tt().store(board.zobrist_key(), m, score, depth, TTBound::Lower, ply);
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
        tt().store(board.zobrist_key(), best_move, best_score, depth, bound, ply);
    }

    if (json_node) {
        json_node->eval = best_score;
    }

    return best_score;
}

SearchResult SearchEngine::search_minimax(Board& board, int depth, bool export_tree) {
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
    if (moves.empty()) {
        result.best_move = Move();
        result.best_score = MoveGenerator::in_check(board, board.side_to_move()) ? -ScoreMate : ScoreDraw;
        metrics_tracker_.stop_timer();
        result.metrics = metrics_tracker_.get_metrics();
        return result;
    }

    int best_score = -ScoreInfinity;
    Move best_move = moves[0];

    TreeNodeJSON* root_json = export_tree ? &exporter_.root() : nullptr;

    for (const auto& m : moves) {
        board.make_move(m);

        TreeNodeJSON* child_json = nullptr;
        if (root_json) {
            root_json->children.push_back(TreeNodeJSON{});
            child_json = &root_json->children.back();
            child_json->move_uci = move_to_uci(m);
            child_json->fen = FEN::to_string(board);
            child_json->depth = depth - 1;
            child_json->ply = 1;
        }

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
    result.pv = pv_table_.get_pv(depth).to_vector();
    result.metrics = metrics_tracker_.get_metrics();

    return result;
}

SearchResult SearchEngine::search_alphabeta(Board& board, int depth, bool use_move_ordering, bool use_tt, bool export_tree) {
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

    if (polyglot_book_.is_loaded()) {
        Move book_move = polyglot_book_.probe(board);
        if (static_cast<bool>(book_move)) {
            result.best_move = book_move;
            result.best_score = 0;
            metrics_tracker_.stop_timer();
            result.metrics = metrics_tracker_.get_metrics();
            return result;
        }
    }

    if (moves.empty()) {
        result.best_move = Move();
        result.best_score = MoveGenerator::in_check(board, board.side_to_move()) ? -ScoreMate : ScoreDraw;
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

    TreeNodeJSON* root_json = export_tree ? &exporter_.root() : nullptr;

    for (const auto& m : moves) {
        board.make_move(m);

        TreeNodeJSON* child_json = nullptr;
        if (root_json) {
            root_json->children.push_back(TreeNodeJSON{});
            child_json = &root_json->children.back();
            child_json->move_uci = move_to_uci(m);
            child_json->fen = FEN::to_string(board);
            child_json->depth = depth - 1;
            child_json->ply = 1;
        }

        int score = -negamax_alphabeta(board, depth - 1, 1, -beta, -alpha, use_move_ordering, use_tt, Move(), m, Move(), child_json);

        board.unmake_move(m);

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
    result.pv = pv_table_.get_pv(depth).to_vector();
    result.metrics = metrics_tracker_.get_metrics();
    result.tt_hits = tt().hits();
    result.q_nodes = q_nodes_;

    return result;
}

SearchResult SearchEngine::search_iterative_deepening(Board& board, int max_depth, double max_time_ms, uint64_t max_nodes) {
    if (polyglot_book_.is_loaded()) {
        Move book_move = polyglot_book_.probe(board);
        if (static_cast<bool>(book_move)) {
            SearchResult res;
            res.best_move = book_move;
            res.best_score = Evaluator::evaluate_fast(board);
            res.depth = 1;
            res.completed_depth = 1;
            res.pv = {book_move};
            metrics_tracker_.reset();
            metrics_tracker_.start_timer();
            metrics_tracker_.stop_timer();
            res.metrics = metrics_tracker_.get_metrics();
            return res;
        }
    }

    pv_table_.clear();
    move_picker_.age_history();
    tt().new_search();
    q_nodes_ = 0;

    search_start_time_ = std::chrono::high_resolution_clock::now();
    max_time_ms_ = max_time_ms;
    time_stop_flag_ = false;

    metrics_tracker_.reset();
    metrics_tracker_.start_timer();
    metrics_tracker_.set_version("v10.0 (Master Lazy SMP)");

    SearchResult final_result;
    Move best_pv_move = Move();
    int last_score = 0;
    int stable_move_count = 0;

#if defined(_OPENMP)
    int n_threads = num_threads_;
    if (n_threads > 1) {
        #pragma omp parallel num_threads(n_threads)
        {
            int tid = omp_get_thread_num();
            if (tid == 0) {
                for (int d = 1; d <= max_depth; ++d) {
                    if (max_nodes > 0 && metrics_tracker_.get_metrics().total_nodes >= max_nodes) break;
                    metrics_tracker_.set_depth(d);

                    MoveList moves;
                    MoveGenerator::generate_legal_moves(board, moves);

                    if (moves.empty()) break;
                    if (!static_cast<bool>(final_result.best_move)) {
                        final_result.best_move = moves[0];
                    }

                    move_picker_.score_and_sort_moves(board, moves, 0, best_pv_move);

                    int alpha = -ScoreInfinity;
                    int beta  =  ScoreInfinity;
                    int window_delta = 25;

                    if (d >= 4 && std::abs(last_score) < ScoreMate - 1000) {
                        alpha = std::max(-ScoreInfinity, last_score - window_delta);
                        beta  = std::min(ScoreInfinity, last_score + window_delta);
                    }

                    int current_best_score = -ScoreInfinity;
                    Move current_best_move = moves[0];
                    bool interrupted = false;

                    while (true) {
                        current_best_score = -ScoreInfinity;
                        int orig_alpha = alpha;

                        for (const auto& m : moves) {
                            board.make_move(m);

                            int score = -negamax_alphabeta(board, d - 1, 1, -beta, -alpha, true, true, Move(), m, Move(), nullptr);

                            board.unmake_move(m);

                            if (time_stop_flag_) {
                                interrupted = true;
                                break;
                            }

                            if (score > current_best_score) {
                                current_best_score = score;
                                current_best_move = m;
                            }

                            if (score > alpha) {
                                alpha = score;
                                pv_table_.set_move(0, m);
                                pv_table_.update(0, m);
                            }
                        }

                        if (interrupted) break;

                        if (current_best_score <= orig_alpha) {
                            beta = (orig_alpha + beta) / 2;
                            alpha = std::max(-ScoreInfinity, orig_alpha - window_delta);
                            window_delta += window_delta / 2;
                        } else if (current_best_score >= beta) {
                            alpha = (orig_alpha + beta) / 2;
                            beta = std::min(ScoreInfinity, beta + window_delta);
                            window_delta += window_delta / 2;
                        } else {
                            break;
                        }
                    }

                    if (interrupted) break;

                    if (current_best_move == best_pv_move) {
                        stable_move_count++;
                    } else {
                        stable_move_count = 0;
                    }

                    best_pv_move = current_best_move;
                    last_score   = current_best_score;

                    final_result.best_move = current_best_move;
                    final_result.best_score = current_best_score;
                    final_result.pv = pv_table_.get_pv(d).to_vector();
                    final_result.completed_depth = d;
                    final_result.tt_hits = tt().hits();
                    final_result.q_nodes = q_nodes_;

                    if (uci_output_) {
                        auto now = std::chrono::high_resolution_clock::now();
                        uint64_t elapsed_ms = std::max<uint64_t>(1, std::chrono::duration_cast<std::chrono::milliseconds>(now - search_start_time_).count());
                        uint64_t total_nodes = node_count_ + q_nodes_;
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
                            std::cout << " " << move_to_uci(pv_m);
                        }
                        std::cout << std::endl;
                    }

                    if (max_time_ms > 0.0 && d >= 10 && stable_move_count >= 5) {
                        auto now = std::chrono::high_resolution_clock::now();
                        double elapsed = std::chrono::duration<double, std::milli>(now - search_start_time_).count();
                        if (elapsed >= max_time_ms * 0.85) {
                            break;
                        }
                    }

                    if (is_time_up()) break;
                }
                time_stop_flag_.store(true, std::memory_order_relaxed);
            } else {
                // Helper thread (private board and engine instance sharing master TT)
                Board helper_board = board;
                SearchEngine helper(tt_ptr_);
                helper.search_start_time_ = search_start_time_;
                helper.max_time_ms_ = max_time_ms_;

                int depth_offset = (tid % 2);
                for (int d = 1 + depth_offset; d <= max_depth; ++d) {
                    if (time_stop_flag_ || helper.time_stop_flag_) break;

                    MoveList moves;
                    MoveGenerator::generate_legal_moves(helper_board, moves);
                    if (moves.empty()) break;

                    helper.move_picker_.score_and_sort_moves(helper_board, moves, 0);

                    for (const auto& m : moves) {
                        if (time_stop_flag_ || helper.time_stop_flag_) break;
                        helper_board.make_move(m);
                        helper.negamax_alphabeta(helper_board, d - 1, 1, -ScoreInfinity, ScoreInfinity, true, true, Move(), m, Move(), nullptr);
                        helper_board.unmake_move(m);
                    }
                }
            }
        }
    } else
#endif
    {
        for (int d = 1; d <= max_depth; ++d) {
            if (max_nodes > 0 && metrics_tracker_.get_metrics().total_nodes >= max_nodes) break;
            metrics_tracker_.set_depth(d);

            MoveList moves;
            MoveGenerator::generate_legal_moves(board, moves);

            if (moves.empty()) break;
            if (!static_cast<bool>(final_result.best_move)) {
                final_result.best_move = moves[0];
            }

            move_picker_.score_and_sort_moves(board, moves, 0, best_pv_move);

            int alpha = -ScoreInfinity;
            int beta  =  ScoreInfinity;
            int window_delta = 25;

            if (d >= 4 && std::abs(last_score) < ScoreMate - 1000) {
                alpha = std::max(-ScoreInfinity, last_score - window_delta);
                beta  = std::min(ScoreInfinity, last_score + window_delta);
            }

            int current_best_score = -ScoreInfinity;
            Move current_best_move = moves[0];
            bool interrupted = false;

            while (true) {
                current_best_score = -ScoreInfinity;
                int orig_alpha = alpha;

                for (const auto& m : moves) {
                    board.make_move(m);

                    int score = -negamax_alphabeta(board, d - 1, 1, -beta, -alpha, true, true, Move(), m, Move(), nullptr);

                    board.unmake_move(m);

                    if (time_stop_flag_) {
                        interrupted = true;
                        break;
                    }

                    if (score > current_best_score) {
                        current_best_score = score;
                        current_best_move = m;
                    }

                    if (score > alpha) {
                        alpha = score;
                        pv_table_.set_move(0, m);
                        pv_table_.update(0, m);
                    }
                }

                if (interrupted) break;

                if (current_best_score <= orig_alpha) {
                    beta = (orig_alpha + beta) / 2;
                    alpha = std::max(-ScoreInfinity, orig_alpha - window_delta);
                    window_delta += window_delta / 2;
                } else if (current_best_score >= beta) {
                    alpha = (orig_alpha + beta) / 2;
                    beta = std::min(ScoreInfinity, beta + window_delta);
                    window_delta += window_delta / 2;
                } else {
                    break;
                }
            }

            if (interrupted) break;

            if (current_best_move == best_pv_move) {
                stable_move_count++;
            } else {
                stable_move_count = 0;
            }

            best_pv_move = current_best_move;
            last_score   = current_best_score;

            final_result.best_move = current_best_move;
            final_result.best_score = current_best_score;
            final_result.pv = pv_table_.get_pv(d).to_vector();
            final_result.completed_depth = d;
            final_result.tt_hits = tt().hits();
            final_result.q_nodes = q_nodes_;

            if (uci_output_) {
                auto now = std::chrono::high_resolution_clock::now();
                uint64_t elapsed_ms = std::max<uint64_t>(1, std::chrono::duration_cast<std::chrono::milliseconds>(now - search_start_time_).count());
                uint64_t total_nodes = node_count_ + q_nodes_;
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
                    std::cout << " " << move_to_uci(pv_m);
                }
                std::cout << std::endl;
            }

            if (max_time_ms > 0.0 && d >= 10 && stable_move_count >= 5) {
                auto now = std::chrono::high_resolution_clock::now();
                double elapsed = std::chrono::duration<double, std::milli>(now - search_start_time_).count();
                if (elapsed >= max_time_ms * 0.85) {
                    break;
                }
            }

            if (is_time_up()) break;
        }
    }

    metrics_tracker_.stop_timer();
    final_result.metrics = metrics_tracker_.get_metrics();

    return final_result;
}

SearchResult SearchEngine::search_smp(Board& board, int max_depth, int num_threads) {
    set_threads(num_threads);
    return search_iterative_deepening(board, max_depth, 0.0);
}

} // namespace heavensgate
