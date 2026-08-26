#include "test.hpp"
#include "../src/movegen/perft.hpp"
#include <iostream>

namespace heavensgate::test {

static bool test_perft_suite_depth_5() {
    return Perft::run_verification_suite(5);
}

static bool dummy_perft_init = []() {
    register_test("Perft: Verification Suite Depth 1-5 (All 6 Standard CPW Positions)", test_perft_suite_depth_5);
    return true;
}();

} // namespace heavensgate::test
