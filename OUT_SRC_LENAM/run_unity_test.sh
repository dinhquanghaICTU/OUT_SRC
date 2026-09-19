#!/usr/bin/env bash
set -e

# Find true path of script
SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}" 2>/dev/null || echo "$0")"
SCRIPT_DIR="$(cd "$(dirname "$SCRIPT_PATH")" && pwd)"

# Resolve unity_test directory
if [ -d "$SCRIPT_DIR/unity_test" ]; then
    TEST_DIR="$SCRIPT_DIR/unity_test"
elif [ -d "$SCRIPT_DIR/../unity_test" ]; then
    TEST_DIR="$(cd "$SCRIPT_DIR/.." && pwd)/unity_test"
elif [ -d "$SCRIPT_DIR/OUT_SRC_LENAM/unity_test" ]; then
    TEST_DIR="$SCRIPT_DIR/OUT_SRC_LENAM/unity_test"
elif [ -d "$(pwd)/unity_test" ]; then
    TEST_DIR="$(pwd)/unity_test"
else
    echo -e "\033[1;31mError: Cannot find unity_test directory for OUT_SRC_LENAM!\033[0m"
    exit 1
fi

echo -e "\033[1;34m============================================================\033[0m"
echo -e "\033[1;34m  BUILDING & RUNNING UNITY TEST: Le Nam (LM35 + Sound Microphone)\033[0m"
echo -e "\033[1;34m  Project: OUT_SRC_LENAM\033[0m"
echo -e "\033[1;34m  Test dir: $TEST_DIR\033[0m"
echo -e "\033[1;34m============================================================\033[0m"

cd "$TEST_DIR"

# Clean old artifacts if any
rm -f run_test_bin *.o

# Direct compilation using GCC
echo -e "\033[1;33m[1/2] Compiling Unity test suite with GCC...\033[0m"
gcc -Wall -Wextra -O2 -I. -o run_test_bin unity.c mock_hal.c test_lenam_firmware.c -lm

if [ ! -f "run_test_bin" ]; then
    echo -e "\033[1;31mCompilation failed!\033[0m"
    exit 1
fi

echo -e "\033[1;33m[2/2] Executing Unity Test Suite...\033[0m"
./run_test_bin
TEST_RESULT=$?

if [ $TEST_RESULT -eq 0 ]; then
    echo -e "\033[1;32m✓ [OUT_SRC_LENAM] ALL UNITY TESTS PASSED SUCCESSFULLY!\033[0m\n"
else
    echo -e "\033[1;31m✗ [OUT_SRC_LENAM] UNITY TESTS FAILED (code $TEST_RESULT)!\033[0m\n"
    exit $TEST_RESULT
fi
