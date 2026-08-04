#include "search.hpp"
#include "../core/fen.hpp"
#include "../evaluation/eval.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace heavensgate {

int SearchEngine::quiescence_search(Board& board, int alpha, int beta, int ply) {
    metrics_tracker_.add_nodes(1);
    q_nodes_++;

    if (is_time_up()) return 0;

    Color us = board.side_to_move();
    bool in_chk = MoveGenerator::in_check(board, us);

    int stand_pat = Evaluator::evaluate_incremental(board, ply);
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
            int victim_val = (victim != Piece::None) ? PawnValue : 0;
            switch (piece_type_of(victim)) {
                case PieceType::Pawn:   victim_val = PawnValue; break;
                case PieceType::Knight: victim_val = KnightValue; break;
                case PieceType::Bishop: victim_val = BishopValue; break;
                case PieceType::Rook:   victim_val = RookValue; break;
                case PieceType::Queen:  victim_val = QueenValue; break;
                default: break;
            }

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

int SearchEngine::negamax_alphabeta(Board& board, int depth, int ply, int alpha, int beta, bool use_move_ordering, bool use_tt, Move pv_move, Move prev_move, TreeNodeJSON* json_node) {
    metrics_tracker_.add_nodes(1);

    if (is_time_up()) return 0;

    if (ply > 0 && board.is_repetition()) {
        // Winning Repetition Penalty: Avoid accidental draws when significantly winning (>150 cp)
        if (alpha > 150) return -100;
        return ScoreDraw;
    }

    Color us = board.side_to_move();
    bool in_chk = MoveGenerator::in_check(board, us);

    // Bounded Check Extension (ply < 4) to prevent search depth explosion in self-play
    if (in_chk && ply < 4 && depth > 1) {
        depth++;
    }

    int orig_alpha = alpha;
    Move tt_move = pv_move;

    // 1. Transposition Table Probing
    if (use_tt) {
        TTEntry* tt_entry = tt_.probe(board.zobrist_key());
        if (tt_entry) {
            if (static_cast<bool>(tt_entry->move)) {
                tt_move = tt_entry->move;
            }

            if (tt_entry->depth >= depth) {
                int tt_score = tt_entry->score;
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
            }
        }
    }

    // Reach search horizon -> enter Quiescence Search!
    if (depth <= 0) {
        int q_eval = quiescence_search(board, alpha, beta, ply);
        if (json_node) {
            json_node->eval = q_eval;
            json_node->is_terminal = true;
        }
        return q_eval;
    }

    // 2. Reverse Futility Pruning (Static Null Move Pruning)
    if (depth <= 3 && !in_chk && std::abs(beta) < ScoreMate - 1000) {
        int eval = Evaluator::evaluate_incremental(board, ply);
        int margin = 120 * depth;
        if (eval - margin >= beta) {
            metrics_tracker_.add_cut();
            return eval - margin;
        }
    }

    // 3. Adaptive Null Move Pruning (NMP) (R=2 for shallow depth, R=3 for depth >= 6)
    if (depth >= 3 && !in_chk && board.has_non_pawn_material(us)) {
        int R = (depth >= 6) ? 3 : 2;
        board.make_null_move();

        int null_score = -negamax_alphabeta(board, depth - 1 - R, ply + 1, -beta, -beta + 1, use_move_ordering, use_tt, Move(), Move(), nullptr);

        board.unmake_null_move();

        if (null_score >= beta) {
            metrics_tracker_.add_cut();
            return beta;
        }
    }

    MoveList moves;
    MoveGenerator::generate_legal_moves(board, moves);

    if (moves.empty()) {
        int score = 0;
        if (in_chk) {
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

    if (use_move_ordering) {
        move_picker_.score_and_sort_moves(board, moves, ply, tt_move, prev_move);
    }

    int best_score = -ScoreInfinity;
    Move best_move = Move();

    for (size_t i = 0; i < moves.size(); ++i) {
        Move m = moves[i];
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

        // 4. Principal Variation Search (PVS / NegaScout) & LMR
        if (i == 0) {
            score = -negamax_alphabeta(board, depth - 1, ply + 1, -beta, -alpha, use_move_ordering, use_tt, Move(), m, child_node);
        } else {
            // Late Move Reductions (LMR) for quiet moves
            if (i >= 4 && depth >= 3 && !m.is_capture() && !m.is_promotion() && !in_chk) {
                int reduction = 1 + static_cast<int>(std::log(depth) * std::log(i + 1) / 2.5);
                int reduced_depth = std::max(1, depth - 1 - reduction);

                score = -negamax_alphabeta(board, reduced_depth, ply + 1, -alpha - 1, -alpha, use_move_ordering, use_tt, Move(), m, child_node);
            } else {
                // Zero-window search
                score = -negamax_alphabeta(board, depth - 1, ply + 1, -alpha - 1, -alpha, use_move_ordering, use_tt, Move(), m, child_node);
            }

            // PVS Re-Search: If zero-window search raised alpha, re-search with full [alpha, beta] window!
            if (score > alpha) {
                score = -negamax_alphabeta(board, depth - 1, ply + 1, -beta, -alpha, use_move_ordering, use_tt, Move(), m, child_node);
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
            if (use_move_ordering && !m.is_capture()) {
                move_picker_.add_killer_move(ply, m);
                move_picker_.add_history_score(board.side_to_move(), m, depth);
                if (static_cast<bool>(prev_move)) {
                    move_picker_.add_countermove(prev_move, m);
                }
            }
            if (use_tt) {
                tt_.store(board.zobrist_key(), m, beta, depth, TTBound::Lower, ply);
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

    if (use_tt && !time_stop_flag_) {
        TTBound bound = (best_score <= orig_alpha) ? TTBound::Upper : TTBound::Exact;
        tt_.store(board.zobrist_key(), best_move, best_score, depth, bound, ply);
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
    if (use_tt) tt_.clear();
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

        int score = -negamax_alphabeta(board, depth - 1, 1, -beta, -alpha, use_move_ordering, use_tt, Move(), m, child_json);

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
    result.tt_hits = tt_.hits();
    result.q_nodes = q_nodes_;

    return result;
}

SearchResult SearchEngine::search_iterative_deepening(Board& board, int max_depth, double max_time_ms) {
    pv_table_.clear();
    move_picker_.clear();
    tt_.clear();
    q_nodes_ = 0;

    search_start_time_ = std::chrono::high_resolution_clock::now();
    max_time_ms_ = max_time_ms;
    time_stop_flag_ = false;

    metrics_tracker_.start_timer();
    metrics_tracker_.set_version("v10.0 (Master Search Architecture)");

    SearchResult final_result;
    Move best_pv_move = Move();
    int last_score = 0;

    for (int d = 1; d <= max_depth; ++d) {
        metrics_tracker_.set_depth(d);

        MoveList moves;
        MoveGenerator::generate_legal_moves(board, moves);

        if (moves.empty()) break;

        move_picker_.score_and_sort_moves(board, moves, 0, best_pv_move);

        int alpha = -ScoreInfinity;
        int beta  =  ScoreInfinity;
        constexpr int WindowDelta = 25;

        if (d >= 4 && std::abs(last_score) < ScoreMate - 1000) {
            alpha = last_score - WindowDelta;
            beta  = last_score + WindowDelta;
        }

        int current_best_score = -ScoreInfinity;
        Move current_best_move = moves[0];
        bool interrupted = false;

        while (true) {
            current_best_score = -ScoreInfinity;

            for (const auto& m : moves) {
                board.make_move(m);

                int score = -negamax_alphabeta(board, d - 1, 1, -beta, -alpha, true, true, Move(), m, nullptr);

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

            if (current_best_score <= alpha) {
                alpha = -ScoreInfinity;
            } else if (current_best_score >= beta) {
                beta = ScoreInfinity;
            } else {
                break;
            }
        }

        if (interrupted && d > 1) {
            break;
        }

        best_pv_move = current_best_move;
        last_score   = current_best_score;

        final_result.best_move = current_best_move;
        final_result.best_score = current_best_score;
        final_result.pv = pv_table_.get_pv(d).to_vector();
        final_result.completed_depth = d;
        final_result.tt_hits = tt_.hits();
        final_result.q_nodes = q_nodes_;

        if (is_time_up()) break;
    }

    metrics_tracker_.stop_timer();
    final_result.metrics = metrics_tracker_.get_metrics();

    return final_result;
}

} // namespace heavensgate
