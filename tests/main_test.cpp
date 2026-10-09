#include "test.hpp"
#include <iostream>
#include "../src/core/zobrist.hpp"
#include "../src/movegen/movegen.hpp"
#include "../src/evaluation/eval.hpp"

namespace heavensgate::test {

std::vector<TestCase>& get_tests() {
    static std::vector<TestCase> tests;
    return tests;
}

bool register_test(const std::string& name, std::function<bool()> func) {
    get_tests().push_back({name, func});
    return true;
}

int run_all_tests(const std::string& filter = {}) {
    int passed = 0;
    int failed = 0;

    std::cout << "\n======================================================\n";
    std::cout << "          HEAVEN'S GATE UNIT TEST RUNNER              \n";
    std::cout << "======================================================\n\n";

    for (const auto& test : get_tests()) {
        if (!filter.empty() && test.name.find(filter) == std::string::npos) continue;
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

    return failed == 0 && passed > 0 ? 0 : 1;
}

} // namespace heavensgate::test

int main(int argc, char** argv) {
    std::string filter;
    if (argc == 3 && std::string(argv[1]) == "--filter") filter = argv[2];
    else if (argc != 1) {
        std::cerr << "Usage: heavensgate_tests [--filter name-substring]\n";
        return 2;
    }
    heavensgate::Zobrist::init();
    heavensgate::MoveGenerator::init();
    heavensgate::Evaluator::init();
    std::cout << "\n======================================================\n";
    std::cout << "          RUNNING ALL SYSTEM TEST MODULES             \n";
    std::cout << "======================================================\n";
    if (filter.empty()) {
        heavensgate::test_fen();
        heavensgate::test_movegen();
        heavensgate::test_eval();
        heavensgate::test_search();
    }
    return heavensgate::test::run_all_tests(filter);
}
