#pragma once

namespace heavensgate {

struct SearchParams {
    float lmr_divisor = 2.9500f;
    int lmr_hist_bonus = 425;
    int lmr_hist_malus = 72;
    int rfp_margin = 70;
    int futility_margin = 185;
    int see_bad_capture_slope = 102;
    int see_quiet_slope = 15;
    int nmp_eval_margin = 184;
    int singular_margin = 2;
    int aspiration_window_delta = 19;
    bool enable_nmp = true; // Diagnostic ablation; production default is unchanged.
    bool enable_lmr = true;
    bool enable_nmp_guards = false; // Isolated experimental candidate; never auto-promoted.

    void reset() noexcept {
        lmr_divisor = 2.9500f;
        lmr_hist_bonus = 425;
        lmr_hist_malus = 72;
        rfp_margin = 70;
        futility_margin = 185;
        see_bad_capture_slope = 102;
        see_quiet_slope = 15;
        nmp_eval_margin = 184;
        singular_margin = 2;
        aspiration_window_delta = 19;
        enable_nmp = true;
        enable_lmr = true;
        enable_nmp_guards = false;
    }
};

extern SearchParams g_search_params;

} // namespace heavensgate
