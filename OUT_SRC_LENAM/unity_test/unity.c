/* Unity Project - Test Framework for C */
#include "unity.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

struct UNITY_STORAGE_T Unity;

extern void setUp(void);
extern void tearDown(void);

void UnityBegin(const char* filename) {
    Unity.TestFile = filename;
    Unity.CurrentTestName = NULL;
    Unity.NumberOfTests = 0;
    Unity.TestFailures = 0;
    Unity.TestIgnores = 0;
    Unity.CurrentTestLineNumber = 0;
    Unity.CurrentTestIgnored = 0;
    printf("\n\033[1;36m============================================================\033[0m\n");
    printf("\033[1;36m  UNITY TEST RUNNER: %s\033[0m\n", filename);
    printf("\033[1;36m============================================================\033[0m\n");
}

int UnityEnd(void) {
    printf("------------------------------------------------------------\n");
    printf("Test Summary: %u Tests executed | %u Failures | %u Ignored\n", 
           Unity.NumberOfTests, Unity.TestFailures, Unity.TestIgnores);
    if (Unity.TestFailures == 0) {
        printf("\033[1;32m[PASS] ALL %u UNIT TESTS PASSED SUCCESSFULLY!\033[0m\n", Unity.NumberOfTests);
    } else {
        printf("\033[1;31m[FAIL] %u UNIT TEST(S) FAILED!\033[0m\n", Unity.TestFailures);
    }
    printf("============================================================\n\n");
    return (int)Unity.TestFailures;
}

void UnityConcludeTest(void) {
    if (Unity.CurrentTestIgnored) {
        Unity.TestIgnores++;
    }
}

void UnityDefaultTestRun(void (*Func)(void), const char* FuncName, const int FuncLineNum) {
    Unity.CurrentTestName = FuncName;
    Unity.CurrentTestLineNumber = (unsigned int)FuncLineNum;
    Unity.NumberOfTests++;
    Unity.CurrentTestIgnored = 0;

    if (TEST_PROTECT()) {
        setUp();
        Func();
    }
    if (TEST_PROTECT()) {
        tearDown();
    }
    UnityConcludeTest();
    if (Unity.TestFailures == 0 || Unity.CurrentTestIgnored == 0) {
        printf("  [\033[32mPASS\033[0m] %-48s (line %3d)\n", FuncName, FuncLineNum);
    }
}

void UnityFail(const char* message, const unsigned short line) {
    Unity.TestFailures++;
    printf("  [\033[31mFAIL\033[0m] %s (line %u): %s\n",
           Unity.CurrentTestName ? Unity.CurrentTestName : "Unknown",
           (unsigned int)line,
           message ? message : "Assertion failed");
    TEST_ABORT();
}

void UnityIgnore(const char* message, const unsigned short line) {
    Unity.CurrentTestIgnored = 1;
    printf("  [\033[33mSKIP\033[0m] %s (line %u): %s\n",
           Unity.CurrentTestName ? Unity.CurrentTestName : "Unknown",
           (unsigned int)line,
           message ? message : "Test ignored");
    TEST_ABORT();
}

void UnityAssertEqualNumber(const intptr_t expected,
                            const intptr_t actual,
                            const char* msg,
                            const unsigned short lineNumber,
                            const UNITY_DISPLAY_STYLE_T style) {
    (void)style;
    if (expected != actual) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Expected %ld Was %ld%s%s",
                 (long)expected, (long)actual,
                 msg ? " - " : "", msg ? msg : "");
        UnityFail(buffer, lineNumber);
    }
}

void UnityAssertEqualString(const char* expected,
                            const char* actual,
                            const char* msg,
                            const unsigned short lineNumber) {
    if (expected == NULL && actual == NULL) return;
    if (expected == NULL || actual == NULL || strcmp(expected, actual) != 0) {
        char buffer[512];
        snprintf(buffer, sizeof(buffer), "Expected \"%s\" Was \"%s\"%s%s",
                 expected ? expected : "NULL",
                 actual ? actual : "NULL",
                 msg ? " - " : "", msg ? msg : "");
        UnityFail(buffer, lineNumber);
    }
}

void UnityAssertFloatsWithin(const float delta,
                             const float expected,
                             const float actual,
                             const char* msg,
                             const unsigned short lineNumber) {
    float diff = actual - expected;
    if (diff < 0) diff = -diff;
    if (diff > delta || isnan(actual) || isnan(expected)) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Expected %.4f (+/- %.4f) Was %.4f%s%s",
                 expected, delta, actual,
                 msg ? " - " : "", msg ? msg : "");
        UnityFail(buffer, lineNumber);
    }
}

void UnityAssertDoublesWithin(const double delta,
                              const double expected,
                              const double actual,
                              const char* msg,
                              const unsigned short lineNumber) {
    double diff = actual - expected;
    if (diff < 0) diff = -diff;
    if (diff > delta || isnan(actual) || isnan(expected)) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "Expected %.6f (+/- %.6f) Was %.6f%s%s",
                 expected, delta, actual,
                 msg ? " - " : "", msg ? msg : "");
        UnityFail(buffer, lineNumber);
    }
}
