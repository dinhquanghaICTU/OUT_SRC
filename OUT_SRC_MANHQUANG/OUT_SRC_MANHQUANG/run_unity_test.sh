#!/usr/bin/env bash
set -e

SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}" 2>/dev/null || echo "$0")"
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_PATH")" && pwd)"

if [ -d "$SCRIPT_DIR/unity_test" ]; then
    TEST_DIR="$SCRIPT_DIR/unity_test"
elif [ -d "$SCRIPT_DIR/../unity_test" ]; then
    TEST_DIR="$(cd "$SCRIPT_DIR/.." && pwd)/unity_test"
elif [ -d "$(pwd)/unity_test" ]; then
    TEST_DIR="$(pwd)/unity_test"
else
    echo -e "[1;31mError: Cannot find unity_test directory![0m"
    exit 1
fi

echo -e "[1;34m============================================================[0m"
echo -e "[1;34m  BUILDING & RUNNING UNITY TEST: Manh Quang Smart Door[0m"
echo -e "[1;34m  Project: OUT_SRC_MANHQUANG[0m"
echo -e "[1;34m  Test dir: $TEST_DIR[0m"
echo -e "[1;34m============================================================[0m"

cd "$TEST_DIR"
rm -f run_test_bin *.o

echo -e "[1;33m[1/2] Compiling Unity test suite with GCC...[0m"
gcc -Wall -Wextra -O2 -I. -o run_test_bin unity.c mock_hal.c test_manhquang_firmware.c -lm

echo -e "[1;33m[2/2] Executing Unity Test Suite...[0m"
./run_test_bin
TEST_RESULT=$?

if [ $TEST_RESULT -eq 0 ]; then
    echo -e "[1;32m✓ [OUT_SRC_MANHQUANG] ALL UNITY TESTS PASSED SUCCESSFULLY![0m
"
else
    echo -e "[1;31m✗ [OUT_SRC_MANHQUANG] UNITY TESTS FAILED (code $TEST_RESULT)![0m
"
    exit $TEST_RESULT
fi
