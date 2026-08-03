#pragma once

#include <string>
#include <functional>
#include <vector>

namespace heavensgate::test {

struct TestCase {
    std::string name;
    std::function<bool()> func;
};

std::vector<TestCase>& get_tests();
bool register_test(const std::string& name, std::function<bool()> func);

} // namespace heavensgate::test
