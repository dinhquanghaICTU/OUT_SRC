#!/usr/bin/env bash
set -e

# Find true OUT_SRC root directory
SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}")"
BASE_DIR="$(cd "$(dirname "$SCRIPT_PATH")" && pwd)"

PROJECTS=(
    "OUT_SRC_HOANGANH"
    "OUT_SRC_LENAM"
    "OUT_SRC_TRUNGKIEN"
    "OUT_SRC_SON"
    "OUT_SRC_THEANH"
    "OUT_SRC_TUANANH"
)

echo -e "\033[1;36m========================================================================\033[0m"
echo -e "\033[1;36m       RUNNING ALL UNITY FIRMWARE TEST SUITES ACROSS 6 PROJECTS         \033[0m"
echo -e "\033[1;36m========================================================================\033[0m\n"

PASSED_COUNT=0
TOTAL_COUNT=${#PROJECTS[@]}

for prj in "${PROJECTS[@]}"; do
    PRJ_PATH="$BASE_DIR/$prj"
    if [ -f "$PRJ_PATH/run_unity_test.sh" ]; then
        echo -e "\033[1;33m>>> Running Unity test for: $prj ...\033[0m"
        bash "$PRJ_PATH/run_unity_test.sh"
        PASSED_COUNT=$((PASSED_COUNT + 1))
    else
        echo -e "\033[1;31mError: $PRJ_PATH/run_unity_test.sh not found!\033[0m"
        exit 1
    fi
done

echo -e "\033[1;32m========================================================================\033[0m"
echo -e "\033[1;32m  FINAL RESULT: ALL $PASSED_COUNT / $TOTAL_COUNT PROJECT TEST SUITES PASSED! (100% PASS)  \033[0m"
echo -e "\033[1;32m========================================================================\033[0m"
