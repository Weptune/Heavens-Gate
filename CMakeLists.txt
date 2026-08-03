cmake_minimum_required(VERSION 3.20)
project(heavensgate LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source files
set(ENGINE_SOURCES
    src/board/board.cpp
    src/core/fen.cpp
    src/core/zobrist.cpp
    src/movegen/magic.cpp
    src/movegen/attack_masks.cpp
    src/movegen/movegen.cpp
    src/movegen/perft.cpp
    src/evaluation/pst.cpp
    src/evaluation/eval_features.cpp
    src/evaluation/nnue.cpp
    src/evaluation/tensor_eval.cpp
    src/evaluation/tensor_train.cpp
    src/evaluation/tensor_quant.cpp
    src/evaluation/tensor_nnue.cpp
    src/evaluation/spectral_graph.cpp
    src/evaluation/tropical_eval.cpp
    src/evaluation/eval.cpp
    src/search/move_picker.cpp
    src/search/tt.cpp
    src/search/search.cpp
    src/visualization/exporter.cpp
    src/benchmark/metrics.cpp
    src/uci/uci.cpp
)

# Main engine CLI binary
add_executable(heavensgate
    src/main.cpp
    ${ENGINE_SOURCES}
)
target_include_directories(heavensgate PRIVATE src)

# Unit test suite binary
add_executable(heavensgate_tests
    tests/main_test.cpp
    tests/test_fen.cpp
    tests/test_movegen.cpp
    tests/test_perft.cpp
    tests/test_eval.cpp
    tests/test_search.cpp
    ${ENGINE_SOURCES}
)
target_include_directories(heavensgate_tests PRIVATE src tests)

if(MSVC)
    target_compile_options(heavensgate PRIVATE /W4 /O2 /permissive-)
    target_compile_options(heavensgate_tests PRIVATE /W4 /O2 /permissive-)
else()
    target_compile_options(heavensgate PRIVATE -Wall -Wextra -O3)
    target_compile_options(heavensgate_tests PRIVATE -Wall -Wextra -O3)
endif()
