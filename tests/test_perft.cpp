#include "test.hpp"
#include "../src/movegen/perft.hpp"
#include <iostream>

namespace heavensgate::test {

static bool test_perft_suite_depth_3() {
    return Perft::run_verification_suite(3);
}

static bool dummy_perft_init = []() {
    register_test("Perft: Verification Suite Depth 1-3", test_perft_suite_depth_3);
    return true;
}();

} // namespace heavensgate::test
