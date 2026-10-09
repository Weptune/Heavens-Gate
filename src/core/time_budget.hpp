#pragma once
#include <algorithm>
namespace heavensgate {
struct SearchBudget { double optimum_ms = 0; double maximum_ms = 0; };
struct SearchTimingOptions {
    double move_overhead_ms = 25.0;
    double minimum_smp_time_ms = 20.0;
};
// Shared UCI/harness policy. In sudden death reserve overhead for a future
// horizon, rather than only one reply. This is a finite-bank risk policy, not
// a promise of a thinking floor for arbitrarily long zero-increment games.
// Fractional banks never get rounded UP to a one-millisecond search.
inline SearchBudget bank_search_budget(double remaining_ms, double increment_ms,
                                       int fullmove, int moves_to_go = 0,
                                       double move_overhead_ms = 25.0) noexcept {
    if (remaining_ms <= 0) return {0.001, 0.001};
    const bool sudden_death = increment_ms <= 0;
    const int reserve_horizon = moves_to_go > 0 ? std::min(moves_to_go, 35) : 35;
    const double reserve = std::min(std::max(0.0, move_overhead_ms) *
                                   (sudden_death ? reserve_horizon : 1), remaining_ms * 0.5);
    const double available = remaining_ms - reserve;
    const double floor = std::min(1.0, available);
    // Restore the pre-recovery zero-increment spending ceiling. Increment
    // controls retain their previous horizon and hard-budget multiplier.
    const int default_horizon = sudden_death ? 45 : 35;
    const int horizon = moves_to_go > 0 ? std::min(moves_to_go, default_horizon) : default_horizon;
    const double allocation = available / horizon + std::max(0.0, increment_ms) * 0.8;
    SearchBudget budget{std::min(allocation, available * 0.5),
                        std::min(allocation * (sudden_death ? 1.8 : 3.5), available * 0.85)};
    if (fullmove <= 5) {
        budget.optimum_ms = std::clamp(budget.optimum_ms * 0.35, 120.0, 350.0);
        budget.maximum_ms = budget.optimum_ms * 2.0;
    }
    budget.maximum_ms = std::clamp(budget.maximum_ms, floor, available);
    budget.optimum_ms = std::clamp(budget.optimum_ms, floor, budget.maximum_ms);
    return budget;
}
} // namespace heavensgate
