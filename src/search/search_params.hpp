#pragma once

namespace heavensgate {

struct SearchParams {
    float lmr_divisor = 3.2000f;
    int lmr_hist_bonus = 425;
    int lmr_hist_malus = 72;
    int rfp_margin = 163;
    int futility_margin = 180;
    int see_bad_capture_slope = 124;
    int see_quiet_slope = 15;
    int nmp_eval_margin = 200;
    int singular_margin = 2;
    int aspiration_window_delta = 16;
    bool enable_nmp = true; // Diagnostic ablation; production default is unchanged.
    bool enable_lmr = true;
    bool enable_nmp_guards = false; // Isolated experimental candidate; never auto-promoted.

    void reset() noexcept {
        lmr_divisor = 3.2000f;
        lmr_hist_bonus = 425;
        lmr_hist_malus = 72;
        rfp_margin = 163;
        futility_margin = 180;
        see_bad_capture_slope = 124;
        see_quiet_slope = 15;
        nmp_eval_margin = 200;
        singular_margin = 2;
        aspiration_window_delta = 16;
        enable_nmp = true;
        enable_lmr = true;
        enable_nmp_guards = false;
    }
};

extern SearchParams g_search_params;

} // namespace heavensgate
