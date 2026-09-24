/* Unity Project - Test Framework for C */
#ifndef UNITY_FRAMEWORK_H
#define UNITY_FRAMEWORK_H

#include <setjmp.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UNITY_DISPLAY_STYLE_INT = 0,
    UNITY_DISPLAY_STYLE_UINT,
    UNITY_DISPLAY_STYLE_HEX8,
    UNITY_DISPLAY_STYLE_HEX16,
    UNITY_DISPLAY_STYLE_HEX32
} UNITY_DISPLAY_STYLE_T;

struct UNITY_STORAGE_T {
    const char* TestFile;
    const char* CurrentTestName;
    unsigned int NumberOfTests;
    unsigned int TestFailures;
    unsigned int TestIgnores;
    unsigned int CurrentTestLineNumber;
    unsigned int CurrentTestIgnored;
    jmp_buf AbortFrame;
};

extern struct UNITY_STORAGE_T Unity;

void UnityBegin(const char* filename);
int  UnityEnd(void);
void UnityConcludeTest(void);
void UnityDefaultTestRun(void (*Func)(void), const char* FuncName, const int FuncLineNum);

void UnityAssertEqualNumber(const intptr_t expected,
                            const intptr_t actual,
                            const char* msg,
                            const unsigned short lineNumber,
                            const UNITY_DISPLAY_STYLE_T style);

void UnityAssertEqualString(const char* expected,
                            const char* actual,
                            const char* msg,
                            const unsigned short lineNumber);

void UnityAssertFloatsWithin(const float delta,
                             const float expected,
                             const float actual,
                             const char* msg,
                             const unsigned short lineNumber);

void UnityAssertDoublesWithin(const double delta,
                              const double expected,
                              const double actual,
                              const char* msg,
                              const unsigned short lineNumber);

void UnityFail(const char* message, const unsigned short line);
void UnityIgnore(const char* message, const unsigned short line);

#define TEST_PROTECT() (setjmp(Unity.AbortFrame) == 0)
#define TEST_ABORT() longjmp(Unity.AbortFrame, 1)

#define TEST_ASSERT(condition) do { if (!(condition)) { UnityFail(#condition, (unsigned short)__LINE__); } } while(0)
#define TEST_ASSERT_TRUE(condition) TEST_ASSERT(condition)
#define TEST_ASSERT_FALSE(condition) TEST_ASSERT(!(condition))
#define TEST_ASSERT_NULL(pointer) TEST_ASSERT((pointer) == NULL)
#define TEST_ASSERT_NOT_NULL(pointer) TEST_ASSERT((pointer) != NULL)
#define TEST_ASSERT_EQUAL_INT(expected, actual) UnityAssertEqualNumber((intptr_t)(expected), (intptr_t)(actual), NULL, (unsigned short)__LINE__, UNITY_DISPLAY_STYLE_INT)
#define TEST_ASSERT_EQUAL_UINT(expected, actual) UnityAssertEqualNumber((intptr_t)(expected), (intptr_t)(actual), NULL, (unsigned short)__LINE__, UNITY_DISPLAY_STYLE_UINT)
#define TEST_ASSERT_EQUAL_STRING(expected, actual) UnityAssertEqualString((const char*)(expected), (const char*)(actual), NULL, (unsigned short)__LINE__)
#define TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual) UnityAssertFloatsWithin((float)(delta), (float)(expected), (float)(actual), NULL, (unsigned short)__LINE__)
#define TEST_ASSERT_DOUBLE_WITHIN(delta, expected, actual) UnityAssertDoublesWithin((double)(delta), (double)(expected), (double)(actual), NULL, (unsigned short)__LINE__)
#define TEST_FAIL_MESSAGE(message) UnityFail((message), (unsigned short)__LINE__)

#define RUN_TEST(func) UnityDefaultTestRun(func, #func, __LINE__)
#define UNITY_BEGIN() UnityBegin(__FILE__)
#define UNITY_END() UnityEnd()

#ifdef __cplusplus
}
#endif

#endif
