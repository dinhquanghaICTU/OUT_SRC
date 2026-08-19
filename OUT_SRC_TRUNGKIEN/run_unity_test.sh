#!/usr/bin/env bash
set -e

# Find unity_test directory flexibly
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ -d "$SCRIPT_DIR/unity_test" ]; then
    TEST_DIR="$SCRIPT_DIR/unity_test"
elif [ -d "$SCRIPT_DIR/../unity_test" ]; then
    TEST_DIR="$(cd "$SCRIPT_DIR/.." && pwd)/unity_test"
elif [ -d "$SCRIPT_DIR/OUT_SRC_TRUNGKIEN/unity_test" ]; then
    TEST_DIR="$SCRIPT_DIR/OUT_SRC_TRUNGKIEN/unity_test"
else
    echo "Cannot find unity_test directory!"
    exit 1
fi

echo -e "\033[1;34m============================================================\033[0m"
echo -e "\033[1;34m  BUILDING & RUNNING UNITY TEST\033[0m"
echo -e "\033[1;34m  Project: OUT_SRC_TRUNGKIEN\033[0m"
echo -e "\033[1;34m  Test dir: $TEST_DIR\033[0m"
echo -e "\033[1;34m============================================================\033[0m"

cd "$TEST_DIR"
make clean > /dev/null 2>&1 || true
make all

echo -e "\n\033[1;35m>>> Executing Unity Test Suite...\033[0m"
./run_test_bin
TEST_RESULT=$?

if [ $TEST_RESULT -eq 0 ]; then
    echo -e "\033[1;32m✓ [OUT_SRC_TRUNGKIEN] ALL UNITY TESTS PASSED SUCCESSFULLY!\033[0m\n"
else
    echo -e "\033[1;31m✗ [OUT_SRC_TRUNGKIEN] UNITY TESTS FAILED (code $TEST_RESULT)!\033[0m\n"
    exit $TEST_RESULT
fi
