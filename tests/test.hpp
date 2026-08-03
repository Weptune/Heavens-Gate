#pragma once

#include <string>
#include <functional>
#include <vector>
#include <iostream>
#include <cstdlib>

#define HEAVENSGATE_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "\n[ASSERTION FAILED] " << (msg) << " (" << #cond << ")\n"; \
            std::exit(1); \
        } \
    } while (0)

namespace heavensgate {

void test_fen();
void test_movegen();
void test_eval();
void test_search();

namespace test {

struct TestCase {
    std::string name;
    std::function<bool()> func;
};

std::vector<TestCase>& get_tests();
bool register_test(const std::string& name, std::function<bool()> func);

} // namespace test
} // namespace heavensgate
