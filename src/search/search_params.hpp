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
    int nmp_eval_margin = 218;

    void reset() noexcept {
        lmr_divisor = 3.2000f;
        lmr_hist_bonus = 425;
        lmr_hist_malus = 72;
        rfp_margin = 163;
        futility_margin = 180;
        see_bad_capture_slope = 124;
        see_quiet_slope = 15;
        nmp_eval_margin = 218;
    }
};

extern SearchParams g_search_params;

} // namespace heavensgate
