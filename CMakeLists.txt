cmake_minimum_required(VERSION 3.20)
project(heavensgate LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Offline diagnostics only: counters are private to each worker and compile
# completely out of normal builds. Never compare their timing to release NPS.
option(HG_TT_DIAGNOSTICS "Collect per-worker TT clock/context diagnostics" OFF)
if(HG_TT_DIAGNOSTICS)
    add_compile_definitions(HG_TT_DIAGNOSTICS=1)
endif()

# Fingerprint the actual dirty source tree, not just HEAD. Header edits also
# trigger reconfiguration so the manifest identifies the compiled sources.
file(GLOB_RECURSE HG_FINGERPRINT_FILES CONFIGURE_DEPENDS "src/*.cpp" "src/*.hpp" "src/*.inc")
list(APPEND HG_FINGERPRINT_FILES "${CMAKE_SOURCE_DIR}/CMakeLists.txt")
list(SORT HG_FINGERPRINT_FILES)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${HG_FINGERPRINT_FILES})
set(HG_FINGERPRINT "")
foreach(HG_FILE IN LISTS HG_FINGERPRINT_FILES)
    file(SHA256 "${HG_FILE}" HG_FILE_HASH)
    file(RELATIVE_PATH HG_RELATIVE "${CMAKE_SOURCE_DIR}" "${HG_FILE}")
    string(APPEND HG_FINGERPRINT "${HG_RELATIVE}:${HG_FILE_HASH}\n")
endforeach()
string(SHA256 HG_SOURCE_SHA256 "${HG_FINGERPRINT}")
execute_process(COMMAND git rev-parse HEAD WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                OUTPUT_VARIABLE HG_GIT_REVISION OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
add_compile_definitions(HG_SOURCE_SHA256="${HG_SOURCE_SHA256}" HG_GIT_REVISION="${HG_GIT_REVISION}")

# Source files
set(ENGINE_SOURCES
    src/board/board.cpp
    src/core/fen.cpp
    src/core/zobrist.cpp
    src/core/polyglot.cpp
    src/movegen/magic.cpp
    src/movegen/attack_masks.cpp
    src/movegen/movegen.cpp
    src/movegen/perft.cpp
    src/evaluation/pst.cpp
    src/evaluation/eval_features.cpp
    src/evaluation/spectral_graph.cpp
    src/evaluation/tropical_eval.cpp
    src/evaluation/eval.cpp
    src/search/move_picker.cpp
    src/search/tt.cpp
    src/search/search_params.cpp
    src/search/search.cpp
    src/search/syzygy.cpp
    src/visualization/exporter.cpp
    src/benchmark/metrics.cpp
    src/benchmark/sts.cpp
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
    tests/test_foundations.cpp
    tests/test_match_protocol.cpp
    tests/test_search_timing.cpp
    tests/test_tournament_runtime.cpp
    tests/test_null_move_policy.cpp
    tests/test_qsearch_contract.cpp
    tests/test_rule50_contract.cpp
    tests/test_tt_contexts.cpp
    ${ENGINE_SOURCES}
)
target_include_directories(heavensgate_tests PRIVATE src tests)

# Keep the offline tuner tied to the same evaluator implementation. Its
# parity mode checks both the exact baseline and local parameter slopes.
add_executable(heavensgate_texel_tuner
    tools/tune_classical_texel.cpp
    src/board/board.cpp
    src/core/fen.cpp
    src/core/zobrist.cpp
    src/movegen/magic.cpp
    src/movegen/attack_masks.cpp
    src/movegen/movegen.cpp
    src/evaluation/pst.cpp
    src/evaluation/eval_features.cpp
    src/evaluation/spectral_graph.cpp
    src/evaluation/tropical_eval.cpp
    src/evaluation/eval.cpp
)
target_include_directories(heavensgate_texel_tuner PRIVATE src)

find_package(OpenMP)

if(MSVC)
    target_compile_options(heavensgate PRIVATE /W4 /O2 /permissive- /arch:AVX2)
    target_compile_options(heavensgate_tests PRIVATE /W4 /O2 /permissive- /arch:AVX2)
    target_compile_options(heavensgate_texel_tuner PRIVATE /W4 /O2 /permissive- /arch:AVX2)
else()
    target_compile_options(heavensgate PRIVATE -Wall -Wextra -O3 -march=native -mavx2 -mfma)
    target_compile_options(heavensgate_tests PRIVATE -Wall -Wextra -O3 -march=native -mavx2 -mfma)
    target_compile_options(heavensgate_texel_tuner PRIVATE -Wall -Wextra -O3 -march=native -mavx2 -mfma)
endif()

if(OpenMP_CXX_FOUND)
    target_link_libraries(heavensgate PRIVATE OpenMP::OpenMP_CXX)
    target_link_libraries(heavensgate_tests PRIVATE OpenMP::OpenMP_CXX)
    target_link_libraries(heavensgate_texel_tuner PRIVATE OpenMP::OpenMP_CXX)
endif()

if(WIN32)
    target_link_libraries(heavensgate PRIVATE bcrypt)
    target_link_libraries(heavensgate_tests PRIVATE bcrypt)
    add_executable(heavensgate_fake_uci tests/fake_uci.cpp)
    add_dependencies(heavensgate_tests heavensgate_fake_uci)
    target_compile_definitions(heavensgate_tests PRIVATE HG_FAKE_UCI_PATH="$<TARGET_FILE:heavensgate_fake_uci>")
endif()

enable_testing()
add_test(NAME AllTests COMMAND heavensgate_tests)
add_test(NAME QsearchContracts COMMAND heavensgate_tests --filter Qsearch)
add_test(NAME Rule50Contracts COMMAND heavensgate_tests --filter Rule50)
add_test(NAME TTContextContracts COMMAND heavensgate_tests --filter TTContexts)
add_test(NAME TexelParity COMMAND heavensgate_texel_tuner 0 0.5
    ${CMAKE_SOURCE_DIR}/tests/texel_parity_positions.txt --parity-only)
find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_test(NAME TexelTrainingContract COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/tools/test_texel_contract.py $<TARGET_FILE:heavensgate_texel_tuner>)
    add_test(NAME TuningTools COMMAND ${Python3_EXECUTABLE} -m unittest discover
        -s ${CMAKE_SOURCE_DIR}/tools -p "test_*tuning*.py" -v)
    add_test(NAME TournamentSupervisor COMMAND ${Python3_EXECUTABLE} -m unittest discover
        -s ${CMAKE_SOURCE_DIR}/tools -p "test_tournament_supervisor.py" -v)
    add_test(NAME PairedStatistics COMMAND ${Python3_EXECUTABLE} -m unittest discover
        -s ${CMAKE_SOURCE_DIR}/tools -p "test_paired_statistics.py" -v)
endif()
