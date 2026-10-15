// Heaven's Gate HCE tuner: exact linear model of the production evaluation.
//
// Every evaluation parameter enters the eval linearly, so for each training
// position the eval is
//     ((sum_j cm_j*w_j + rm) * phase + (sum_j ce_j*w_j + re) * (24 - phase)) / 24
// where cm/ce are per-position coefficients. Material, piece-square and tempo
// coefficients come from piece placement; every other parameter's coefficient is
// measured by a +1 finite difference on Evaluator::positional_terms (the same code
// the search runs), so the model cannot drift from the engine. rm/re absorb the
// terms that are not parameters (imbalance literals).
//
//   tune_hce --parity <positions>                       verify model == engine eval
//   tune_hce --tune <data> [<data>...] --out <dir>      fit and write source files
//            [--epochs 3000] [--lr 0.5] [--limit N] [--val 0.05] [--threads N]
// Data: "FEN c9 \"1-0\";" (zurichess quiet-labeled) or "FEN|1.0" (white score).
#include "../src/board/board.hpp"
#include "../src/core/zobrist.hpp"
#include "../src/evaluation/eval.hpp"
#include "../src/evaluation/eval_params.hpp"
#include "../src/evaluation/pst.hpp"
#include "../src/movegen/movegen.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#if defined(_OPENMP)
#include <omp.h>
#endif

using namespace heavensgate;

namespace {

struct Param { std::string name; int* engine_ptr; double value; bool fixed; };
struct Feature { uint16_t idx; int16_t cm, ce; };
struct Sample { uint32_t first, count; float rm, re, result; uint8_t phase, scale; int8_t stm; };

std::vector<Param> params;
std::map<std::string, size_t> param_index;
const char* kPieceNames[6] = {"Pawn", "Knight", "Bishop", "Rook", "Queen", "King"};

std::array<int, 64>* pst_table(PieceType pt, bool eg) {
    static std::array<std::array<int, 64>, 12> copy = [] {
        std::array<std::array<int, 64>, 12> t{};
        const std::array<int, 64>* src[12] = {
            &PST::PawnMG, &PST::PawnEG, &PST::KnightMG, &PST::KnightEG, &PST::BishopMG, &PST::BishopEG,
            &PST::RookMG, &PST::RookEG, &PST::QueenMG, &PST::QueenEG, &PST::KingMG, &PST::KingEG};
        for (int i = 0; i < 12; ++i) t[static_cast<size_t>(i)] = *src[i];
        return t;
    }();
    return &copy[static_cast<size_t>(static_cast<int>(pt) * 2 + (eg ? 1 : 0))];
}

size_t add_param(const std::string& name, int* ptr, double value, bool fixed = false) {
    param_index[name] = params.size();
    params.push_back({name, ptr, value, fixed});
    return params.size() - 1;
}

// Indices: material, PST, tempo first (direct coefficients), then finite-difference params.
size_t material_idx[6][2], pst_idx[6][64][2], tempo_idx[2];
size_t fd_begin = 0;

void build_params() {
    auto& p = g_eval_params;
    int* mat[5][2] = {{&p.pawn_mg, &p.pawn_eg}, {&p.knight_mg, &p.knight_eg}, {&p.bishop_mg, &p.bishop_eg},
                      {&p.rook_mg, &p.rook_eg}, {&p.queen_mg, &p.queen_eg}};
    const char* mat_names[5] = {"pawn", "knight", "bishop", "rook", "queen"};
    for (int t = 0; t < 5; ++t)
        for (int e = 0; e < 2; ++e)
            material_idx[t][e] = add_param(std::string(mat_names[t]) + (e ? "_eg" : "_mg"), mat[t][e], *mat[t][e],
                                           t == 0 && e == 0); // PawnMG = 100 anchors the scale.
    for (int t = 0; t < 6; ++t)
        for (int sq = 0; sq < 64; ++sq)
            for (int e = 0; e < 2; ++e) {
                auto* table = pst_table(static_cast<PieceType>(t), e);
                pst_idx[t][sq][e] = add_param(std::string("pst_") + kPieceNames[t] + (e ? "EG" : "MG") + "[" + std::to_string(sq) + "]",
                                              &(*table)[static_cast<size_t>(sq)], (*table)[static_cast<size_t>(sq)]);
            }
    tempo_idx[0] = add_param("tempo_mg", &p.tempo_mg, p.tempo_mg);
    tempo_idx[1] = add_param("tempo_eg", &p.tempo_eg, p.tempo_eg);
    fd_begin = params.size();
    for (const auto& ref : p.get_tunable_params()) {
        if (param_index.count(ref.name)) continue; // material/tempo handled directly
        add_param(ref.name, ref.ptr, *ref.ptr);
    }
}

ScorePair white_terms(const Board& b) {
    const ScorePair w = Evaluator::positional_terms(b, Color::White), k = Evaluator::positional_terms(b, Color::Black);
    return {w.mg - k.mg, w.eg - k.eg};
}

// Extracts coefficients for one position. Returns false for unusable positions.
bool extract(Board& b, std::vector<Feature>& out, float& rm, float& re, uint8_t& phase, uint8_t& scale, int8_t& stm_out) {
    out.clear();
    phase = static_cast<uint8_t>(std::min(24, b.game_phase()));
    std::vector<std::pair<size_t, int>> direct_mg, direct_eg; // (param, coef)
    std::map<size_t, std::pair<int, int>> acc;
    for (int sq = 0; sq < 64; ++sq) {
        const Piece pc = b.piece_at(static_cast<Square>(sq));
        if (pc == Piece::None) continue;
        const int t = static_cast<int>(piece_type_of(pc));
        const bool white = color_of(pc) == Color::White;
        const int sign = white ? 1 : -1, rel = white ? sq : (sq ^ 56);
        if (t < 5) { acc[material_idx[t][0]].first += sign; acc[material_idx[t][1]].second += sign; }
        acc[pst_idx[t][rel][0]].first += sign; acc[pst_idx[t][rel][1]].second += sign;
    }
    stm_out = b.side_to_move() == Color::White ? 1 : -1; // Tempo is outside the scaled EG sum.

    const ScorePair base = white_terms(b);
    double lin_mg = 0, lin_eg = 0;
    for (size_t j = fd_begin; j < params.size(); ++j) {
        int* ptr = params[j].engine_ptr;
        *ptr += 1;
        const ScorePair bumped = white_terms(b);
        *ptr -= 1;
        const int cm = bumped.mg - base.mg, ce = bumped.eg - base.eg;
        if (cm || ce) { acc[j].first += cm; acc[j].second += ce; lin_mg += cm * double(*ptr); lin_eg += ce * double(*ptr); }
    }
    rm = static_cast<float>(base.mg - lin_mg);
    re = static_cast<float>(base.eg - lin_eg);
    const int eg_total = b.eg_material(Color::White) + b.eg_pst(Color::White) - b.eg_material(Color::Black) - b.eg_pst(Color::Black) + base.eg;
    scale = static_cast<uint8_t>(Evaluator::eg_scale(b, eg_total));
    for (const auto& [idx, c] : acc) {
        if (!c.first && !c.second) continue;
        if (std::abs(c.first) > 32767 || std::abs(c.second) > 32767) return false;
        out.push_back({static_cast<uint16_t>(idx), static_cast<int16_t>(c.first), static_cast<int16_t>(c.second)});
    }
    return true;
}

double model_eval(const Feature* f, uint32_t n, float rm, float re, int phase, int scale, int stm, const std::vector<double>& w) {
    double mg = rm, eg = re;
    for (uint32_t i = 0; i < n; ++i) { mg += f[i].cm * w[f[i].idx]; eg += f[i].ce * w[f[i].idx]; }
    const double tempo = stm * (w[tempo_idx[0]] * phase + w[tempo_idx[1]] * (24 - phase)) / 24.0;
    return (mg * phase + eg * (scale / 64.0) * (24 - phase)) / 24.0 + tempo;
}

// Engine eval (white relative) without the per-side integer rounding.
double engine_unrounded(const Board& b) {
    const int phase = std::min(24, b.game_phase());
    auto totals = [&](Color c) {
        const ScorePair pos = Evaluator::positional_terms(b, c);
        return ScorePair(b.mg_material(c) + b.mg_pst(c) + pos.mg, b.eg_material(c) + b.eg_pst(c) + pos.eg);
    };
    const ScorePair w = totals(Color::White), k = totals(Color::Black);
    const int mg = w.mg - k.mg, eg = w.eg - k.eg;
    const double tempo = (g_eval_params.tempo_mg * phase + g_eval_params.tempo_eg * (24 - phase)) / 24.0;
    return (mg * double(phase) + eg * (Evaluator::eg_scale(b, eg) / 64.0) * (24 - phase)) / 24.0
         + (b.side_to_move() == Color::White ? tempo : -tempo);
}

bool parse_line(const std::string& line, std::string& fen, float& result) {
    const auto bar = line.find('|');
    if (bar != std::string::npos) { fen = line.substr(0, bar); result = std::stof(line.substr(bar + 1)); return true; }
    const auto quote = line.find('"');
    if (quote == std::string::npos) return false;
    const std::string r = line.substr(quote + 1, line.find('"', quote + 1) - quote - 1);
    if (r == "1-0") result = 1; else if (r == "0-1") result = 0; else if (r == "1/2-1/2") result = 0.5f; else return false;
    std::istringstream in(line.substr(0, quote)); std::string part; std::vector<std::string> fields;
    while (in >> part && fields.size() < 4) fields.push_back(part);
    if (fields.size() < 4) return false;
    fen = fields[0] + " " + fields[1] + " " + fields[2] + " " + fields[3] + " 0 1";
    return true;
}

int parity(const std::string& path) {
    std::ifstream in(path); std::string line; double worst = 0; int n = 0;
    std::vector<Feature> f; std::vector<double> w(params.size());
    for (size_t i = 0; i < params.size(); ++i) w[i] = params[i].value;
    std::mt19937 rng(7);
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::string fen = line; float r = 0.5f;
        if (line.find('"') != std::string::npos || line.find('|') != std::string::npos) parse_line(line, fen, r);
        Board b; b.load_fen(fen);
        float rm, re; uint8_t phase, scale; int8_t stm;
        if (!extract(b, f, rm, re, phase, scale, stm)) { std::cerr << "extract failed: " << fen << "\n"; return 1; }
        worst = std::max(worst, std::abs(model_eval(f.data(), uint32_t(f.size()), rm, re, phase, scale, stm, w) - engine_unrounded(b)));
        // Linearity: random perturbations of finite-difference params must be predicted exactly.
        for (int trial = 0; trial < 3; ++trial) {
            std::vector<std::pair<size_t, int>> bumps;
            for (int k = 0; k < 12; ++k) bumps.push_back({fd_begin + rng() % (params.size() - fd_begin), int(rng() % 7) - 3});
            std::vector<double> w2 = w;
            for (auto [j, d] : bumps) { *params[j].engine_ptr += d; w2[j] += d; }
            const double predicted = model_eval(f.data(), uint32_t(f.size()), rm, re, phase, scale, stm, w2), actual = engine_unrounded(b);
            for (auto [j, d] : bumps) *params[j].engine_ptr -= d;
            worst = std::max(worst, std::abs(predicted - actual));
        }
        ++n;
    }
    std::cout << "[HCE] parity: " << n << " positions, " << params.size() << " parameters, max |model - engine| = "
              << std::setprecision(9) << worst << " cp\n";
    return n > 0 && worst < 1e-6 ? 0 : 1;
}

double sigmoid(double eval, double K) { return 1.0 / (1.0 + std::pow(10.0, -K * eval / 400.0)); }

void write_outputs(const std::string& dir, const std::vector<double>& w, const std::string& provenance) {
    std::vector<int> v(w.size());
    for (size_t i = 0; i < w.size(); ++i) v[i] = static_cast<int>(std::lround(w[i]));
    // PST: move each table's mean (over squares a piece can stand on) into material for readability.
    for (int t = 0; t < 6; ++t)
        for (int e = 0; e < 2; ++e) {
            if (t == 5) continue; // king has no material term
            long sum = 0; int cnt = 0;
            for (int sq = 0; sq < 64; ++sq) { if (t == 0 && (sq < 8 || sq >= 56)) continue; sum += v[pst_idx[t][sq][e]]; ++cnt; }
            const int mean = static_cast<int>(std::lround(double(sum) / cnt));
            if (t == 0 && e == 0) continue; // keep the PawnMG anchor exact
            for (int sq = 0; sq < 64; ++sq) if (!(t == 0 && (sq < 8 || sq >= 56))) v[pst_idx[t][sq][e]] -= mean;
            v[material_idx[t][e]] += mean;
        }
    std::ofstream pst(dir + "/pst_tables.inc");
    pst << "// Piece-square tables (white perspective, a1 = index 0, rank 1 first).\n// Generated by tools/tune_hce.cpp: " << provenance << "\n";
    const char* names[6] = {"Pawn", "Knight", "Bishop", "Rook", "Queen", "King"};
    for (int t = 0; t < 6; ++t)
        for (int e = 0; e < 2; ++e) {
            pst << "const std::array<int, 64> PieceSquareTables::" << names[t] << (e ? "EG" : "MG") << " = {\n";
            for (int r = 0; r < 8; ++r) {
                pst << "   ";
                for (int f = 0; f < 8; ++f) {
                    const int sq = r * 8 + f;
                    const int val = (t == 0 && (r == 0 || r == 7)) ? 0 : v[pst_idx[t][sq][e]];
                    pst << std::setw(5) << val << (sq == 63 ? "" : ",");
                }
                pst << " // Rank " << r + 1 << "\n";
            }
            pst << "};\n\n";
        }
    std::ofstream inc(dir + "/tuned_eval_params.inc");
    inc << "// Generated by tools/tune_hce.cpp: " << provenance << "\n#pragma once\n\nnamespace heavensgate {\n\n"
        << "inline void apply_tuned_eval_params(EvalParams& p) {\n";
    for (size_t i = 0; i < params.size(); ++i) {
        if (params[i].name.rfind("pst_", 0) == 0) continue;
        std::string field = params[i].name;
        inc << "    p." << field << " = " << v[i] << ";\n";
    }
    inc << "}\n\n} // namespace heavensgate\n";
}

} // namespace

int main(int argc, char** argv) {
    Zobrist::init(); MoveGenerator::init(); Evaluator::init();
    build_params();
    std::vector<std::string> data; std::string out_dir = ".", parity_path;
    int epochs = 3000, threads = 1; double lr = 0.5, val_frac = 0.05; size_t limit = 0;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--parity" && i + 1 < argc) parity_path = argv[++i];
        else if (a == "--tune") { while (i + 1 < argc && argv[i + 1][0] != '-') data.push_back(argv[++i]); }
        else if (a == "--out" && i + 1 < argc) out_dir = argv[++i];
        else if (a == "--epochs" && i + 1 < argc) epochs = std::stoi(argv[++i]);
        else if (a == "--lr" && i + 1 < argc) lr = std::stod(argv[++i]);
        else if (a == "--limit" && i + 1 < argc) limit = std::stoul(argv[++i]);
        else if (a == "--val" && i + 1 < argc) val_frac = std::stod(argv[++i]);
        else if (a == "--threads" && i + 1 < argc) threads = std::stoi(argv[++i]);
        else { std::cerr << "unknown argument: " << a << "\n"; return 2; }
    }
    if (!parity_path.empty()) return parity(parity_path);
    if (data.empty()) { std::cerr << "usage: tune_hce --parity FILE | --tune DATA... --out DIR\n"; return 2; }
#if defined(_OPENMP)
    omp_set_num_threads(threads);
#endif

    // 1. Load and extract.
    std::vector<Sample> samples; std::vector<Feature> feats, f; std::vector<uint8_t> is_val;
    auto t0 = std::chrono::steady_clock::now();
    for (const auto& path : data) {
        std::ifstream in(path); std::string line;
        while (std::getline(in, line)) {
            if (limit && samples.size() >= limit) break;
            std::string fen; float r;
            if (!parse_line(line, fen, r)) continue;
            Board b; b.load_fen(fen);
            if (MoveGenerator::in_check(b, b.side_to_move())) continue;
            Sample s{}; uint8_t phase, scale; int8_t stm;
            if (!extract(b, f, s.rm, s.re, phase, scale, stm)) continue;
            s.scale = scale; s.stm = stm;
            s.first = static_cast<uint32_t>(feats.size()); s.count = static_cast<uint32_t>(f.size());
            s.result = r; s.phase = phase;
            feats.insert(feats.end(), f.begin(), f.end());
            samples.push_back(s);
            is_val.push_back((std::hash<std::string>{}(fen) % 10000) < val_frac * 10000);
            if (samples.size() % 50000 == 0)
                std::cout << "[HCE] extracted " << samples.size() << " ("
                          << std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count() << " s)\n" << std::flush;
        }
    }
    std::cout << "[HCE] " << samples.size() << " positions, " << feats.size() << " coefficients, " << params.size() << " parameters\n";

    std::vector<double> w(params.size());
    for (size_t i = 0; i < params.size(); ++i) w[i] = params[i].value;
    auto loss = [&](const std::vector<double>& wv, double K, bool val) {
        double total = 0; size_t n = 0;
        #pragma omp parallel for reduction(+:total, n) schedule(static)
        for (size_t i = 0; i < samples.size(); ++i) {
            if (bool(is_val[i]) != val) continue;
            const auto& s = samples[i];
            const double e = model_eval(&feats[s.first], s.count, s.rm, s.re, s.phase, s.scale, s.stm, wv);
            const double d = s.result - sigmoid(e, K); total += d * d; ++n;
        }
        return n ? total / n : 0.0;
    };

    // 2. Fit K on the starting weights.
    double lo = 0.5, hi = 3.0;
    for (int it = 0; it < 60; ++it) {
        const double m1 = lo + (hi - lo) / 3, m2 = hi - (hi - lo) / 3;
        if (loss(w, m1, false) < loss(w, m2, false)) hi = m2; else lo = m1;
    }
    const double K = (lo + hi) / 2;
    std::cout << std::setprecision(8) << "[HCE] K = " << K << "  start train MSE " << loss(w, K, false)
              << "  val MSE " << loss(w, K, true) << "\n" << std::flush;

    // 3. Full-batch Adam.
    std::vector<double> m(w.size()), v(w.size());
    const double b1 = 0.9, b2 = 0.999, ln10k = std::log(10.0) * K / 400.0;
    double best_val = loss(w, K, true); std::vector<double> best_w = w; int best_epoch = 0;
    for (int epoch = 1; epoch <= epochs; ++epoch) {
        std::vector<double> grad(w.size(), 0.0);
        #pragma omp parallel
        {
            std::vector<double> local(w.size(), 0.0);
            #pragma omp for schedule(static)
            for (size_t i = 0; i < samples.size(); ++i) {
                if (is_val[i]) continue;
                const auto& s = samples[i];
                const double e = model_eval(&feats[s.first], s.count, s.rm, s.re, s.phase, s.scale, s.stm, w);
                const double p = sigmoid(e, K);
                const double g = -2.0 * (s.result - p) * p * (1 - p) * ln10k;
                const double gm = g * s.phase / 24.0, ge = g * (24 - s.phase) / 24.0;
                const double ges = ge * s.scale / 64.0;
                for (uint32_t k = 0; k < s.count; ++k) {
                    const auto& ft = feats[s.first + k];
                    local[ft.idx] += gm * ft.cm + ges * ft.ce;
                }
                local[tempo_idx[0]] += gm * s.stm;
                local[tempo_idx[1]] += ge * s.stm;
            }
            #pragma omp critical
            for (size_t j = 0; j < w.size(); ++j) grad[j] += local[j];
        }
        size_t n_train = 0; for (auto x : is_val) n_train += !x;
        for (size_t j = 0; j < w.size(); ++j) {
            if (params[j].fixed) continue;
            const double g = grad[j] / double(n_train);
            m[j] = b1 * m[j] + (1 - b1) * g; v[j] = b2 * v[j] + (1 - b2) * g * g;
            const double mh = m[j] / (1 - std::pow(b1, epoch)), vh = v[j] / (1 - std::pow(b2, epoch));
            w[j] -= lr * mh / (std::sqrt(vh) + 1e-8);
        }
        if (epoch % 100 == 0 || epoch == epochs) {
            const double tr = loss(w, K, false), va = loss(w, K, true);
            if (va < best_val) { best_val = va; best_w = w; best_epoch = epoch; }
            std::cout << "[HCE] epoch " << epoch << "  train " << tr << "  val " << va << (best_epoch == epoch ? "  *" : "") << "\n" << std::flush;
        }
    }
    std::ostringstream prov;
    prov << samples.size() << " positions";
    for (const auto& d : data) prov << ", " << d.substr(d.find_last_of("/\\") + 1);
    prov << "; K=" << std::setprecision(5) << K << "; best val MSE " << std::setprecision(8) << best_val << " at epoch " << best_epoch;
    write_outputs(out_dir, best_w, prov.str());
    std::cout << "[HCE] wrote " << out_dir << "/pst_tables.inc and tuned_eval_params.inc (" << prov.str() << ")\n";
    return 0;
}
