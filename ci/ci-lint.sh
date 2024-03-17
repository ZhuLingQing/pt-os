#!/bin/bash
set -e
set -o pipefail

REPO_DIR=$(cd "$(dirname $0)";pwd)/..
CURR_DIR=${REPO_DIR}
LIB_DIR=${REPO_DIR}
TESTS_DIR=${REPO_DIR}/tests

clang-format --version
if [ $# -lt 1 ]; then
    #  just print the list of files
    echo "Lint test: C++ Lint (clang-format) check for check start"
    python3 "${CURR_DIR}"/ci/run_clang_format.py --clang-format-executable clang-format -r "${LIB_DIR}" --exclude "${REPO_DIR}/build"
    python3 "${CURR_DIR}"/ci/run_clang_format.py --clang-format-executable clang-format -r "${TESTS_DIR}" --exclude "${REPO_DIR}/build"
    echo "Lint test: C++ Lint (clang-format) check for check passed"
elif [ $1 == "fix" ]; then
    #  format file instead of printing differences
    echo "Lint test: C++ Lint check for (clang-format) auto fix start"
    python3 "${CURR_DIR}"/ci/run_clang_format.py --clang-format-executable clang-format -r -i "${LIB_DIR}" --exclude "${REPO_DIR}/build"
    python3 "${CURR_DIR}"/ci/run_clang_format.py --clang-format-executable clang-format -r -i "${TESTS_DIR}" --exclude "${REPO_DIR}/build"
    echo "Lint test: C++ Lint check for (clang-format) auto fix passed"
fi
