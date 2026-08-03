#include "test.hpp"
#include <iostream>

namespace heavensgate::test {

std::vector<TestCase>& get_tests() {
    static std::vector<TestCase> tests;
    return tests;
}

bool register_test(const std::string& name, std::function<bool()> func) {
    get_tests().push_back({name, func});
    return true;
}

int run_all_tests() {
    int passed = 0;
    int failed = 0;

    std::cout << "\n======================================================\n";
    std::cout << "          HEAVEN'S GATE UNIT TEST RUNNER              \n";
    std::cout << "======================================================\n\n";

    for (const auto& test : get_tests()) {
        std::cout << "[RUN] " << test.name << " ... ";
        try {
            bool result = test.func();
            if (result) {
                std::cout << "PASSED\n";
                passed++;
            } else {
                std::cout << "FAILED\n";
                failed++;
            }
        } catch (const std::exception& e) {
            std::cout << "FAILED (Exception: " << e.what() << ")\n";
            failed++;
        } catch (...) {
            std::cout << "FAILED (Unknown exception)\n";
            failed++;
        }
    }

    std::cout << "\n------------------------------------------------------\n";
    std::cout << "SUMMARY: " << passed << " PASSED, " << failed << " FAILED\n";
    std::cout << "------------------------------------------------------\n\n";

    return failed == 0 ? 0 : 1;
}

} // namespace heavensgate::test

int main() {
    std::cout << "\n======================================================\n";
    std::cout << "          RUNNING ALL SYSTEM TEST MODULES             \n";
    std::cout << "======================================================\n";
    heavensgate::test_fen();
    heavensgate::test_movegen();
    heavensgate::test_eval();
    heavensgate::test_search();
    return heavensgate::test::run_all_tests();
}
