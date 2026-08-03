#include "search.hpp"
#include "../core/fen.hpp"
#include <iostream>
#include <algorithm>

namespace heavensgate {

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

int SearchEngine::negamax_alphabeta(Board& board, int depth, int ply, int alpha, int beta, bool use_move_ordering, Move pv_move, TreeNodeJSON* json_node) {
    metrics_tracker_.add_nodes(1);

    if (is_time_up()) return 0;

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

    if (use_move_ordering) {
        move_picker_.score_and_sort_moves(board, moves, ply, pv_move);
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

        int score = -negamax_alphabeta(board, depth - 1, ply + 1, -beta, -alpha, use_move_ordering, Move(), child_node);

        board.unmake_move(m);

        if (time_stop_flag_) return 0;

        if (score > best_score) {
            best_score = score;
        }

        if (score >= beta) {
            metrics_tracker_.add_cut();
            if (use_move_ordering && !m.is_capture()) {
                move_picker_.add_killer_move(ply, m);
                move_picker_.add_history_score(board.side_to_move(), m, depth);
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
    result.pv = pv_table_.get_pv(depth);
    result.metrics = metrics_tracker_.get_metrics();

    return result;
}

SearchResult SearchEngine::search_alphabeta(Board& board, int depth, bool use_move_ordering, bool export_tree) {
    pv_table_.clear();
    move_picker_.clear();
    metrics_tracker_.start_timer();
    metrics_tracker_.set_version(use_move_ordering ? "v4.0 (PST Positional Eval)" : "v2.0 (Alpha-Beta Raw)");
    metrics_tracker_.set_depth(depth);

    time_stop_flag_ = false;
    max_time_ms_ = 0.0;

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

        int score = -negamax_alphabeta(board, depth - 1, 1, -beta, -alpha, use_move_ordering, Move(), child_json);

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
    result.pv = pv_table_.get_pv(depth);
    result.metrics = metrics_tracker_.get_metrics();

    return result;
}

SearchResult SearchEngine::search_iterative_deepening(Board& board, int max_depth, double max_time_ms) {
    pv_table_.clear();
    move_picker_.clear();

    search_start_time_ = std::chrono::high_resolution_clock::now();
    max_time_ms_ = max_time_ms;
    time_stop_flag_ = false;

    metrics_tracker_.start_timer();
    metrics_tracker_.set_version("v5.0 (Iterative Deepening)");

    SearchResult final_result;
    Move best_pv_move = Move();

    for (int d = 1; d <= max_depth; ++d) {
        metrics_tracker_.set_depth(d);

        MoveList moves;
        MoveGenerator::generate_legal_moves(board, moves);

        if (moves.empty()) break;

        move_picker_.score_and_sort_moves(board, moves, 0, best_pv_move);

        int alpha = -ScoreInfinity;
        int beta  =  ScoreInfinity;
        int current_best_score = -ScoreInfinity;
        Move current_best_move = moves[0];

        bool interrupted = false;

        for (const auto& m : moves) {
            board.make_move(m);

            int score = -negamax_alphabeta(board, d - 1, 1, -beta, -alpha, true, Move(), nullptr);

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

        if (interrupted && d > 1) {
            break; // Stop and return best result from previous completed iteration
        }

        best_pv_move = current_best_move;
        final_result.best_move = current_best_move;
        final_result.best_score = current_best_score;
        final_result.pv = pv_table_.get_pv(d);
        final_result.completed_depth = d;

        if (is_time_up()) break;
    }

    metrics_tracker_.stop_timer();
    final_result.metrics = metrics_tracker_.get_metrics();

    return final_result;
}

} // namespace heavensgate
