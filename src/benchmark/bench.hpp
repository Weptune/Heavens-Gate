#pragma once
#include <cstdint>

namespace heavensgate {
// Fixed-depth search over built-in positions. Prints per-position results and a
// final "<nodes> nodes <nps> nps" line (the format OpenBench parses). The node
// count is a search signature: identical counts mean identical single-thread search.
uint64_t run_bench(int depth = 11);
} // namespace heavensgate
