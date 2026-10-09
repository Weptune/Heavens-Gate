#include "test.hpp"
#include "../src/core/tournament_runtime.hpp"

namespace heavensgate::test {
static bool future_clock_reserve() {
    const auto normal = bank_search_budget(120000, 0, 20);
    if (std::abs(normal.optimum_ms - (120000 - 875) / 45.0) > 1e-9 ||
        std::abs(normal.maximum_ms - normal.optimum_ms * 1.8) > 1e-9) return false;
    const auto increment = bank_search_budget(120000, 1000, 20);
    const double old_inc = (120000 - 25) / 35.0 + 800;
    if (std::abs(increment.optimum_ms - old_inc) > 1e-9 ||
        std::abs(increment.maximum_ms - old_inc * 3.5) > 1e-9) return false;
    // Adversarial spending up to each hard limit plus 2ms of reply overhead,
    // with a 40ms scheduler delay every 20th move. Model, not an OS guarantee.
    for (const double bank : {60000.0, 120000.0}) {
        double remaining = bank;
        for (int move = 1; move <= 200; ++move) {
            const auto budget = bank_search_budget(remaining, 0, move);
            if (!(budget.optimum_ms > 0 && budget.optimum_ms <= budget.maximum_ms && budget.maximum_ms <= remaining)) return false;
            remaining -= budget.maximum_ms + 2.0 + (move % 20 == 0 ? 40.0 : 0.0);
            if (remaining <= 0) return false;
        }
        if (remaining < 50) return false;
    }
    const auto fractional = bank_search_budget(0.005, 0, 141);
    return fractional.maximum_ms <= .0025 && fractional.optimum_ms > 0;
}
static bool suspend_accounting() {
    const SuspendSample start{1000, 10000000, true};
    if (suspended_ms(start, {1100, 11000000, true}) != 0 ||
        suspended_ms(start, {61100, 11000000, true}) != 60000 ||
        suspended_ms(start, {61100, 11000000, false}) != 0) return false;
    SuspendMonitor monitor;
    ScopedTournamentPower guard;
    return guard.active() && monitor.supported() && monitor.observe() < 2000;
}
static const bool clock_registered = register_test("Sudden-death future reserve and 200-move budget stress model", future_clock_reserve);
static const bool suspend_registered = register_test("Scoped idle-sleep inhibition and suspend accounting", suspend_accounting);
} // namespace heavensgate::test
