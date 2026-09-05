#include "test_framework.h"

#include <stdio.h>
#include <string.h>

static int total_cases = 0;
static int total_assertions = 0;
static int passed_assertions = 0;
static int failed_assertions = 0;

void test_assert(int condition, const char *expression, const char *file, int line)
{
    total_assertions++;
    if (condition)
    {
        passed_assertions++;
        printf("[PASS] %s\n", expression);
    }
    else
    {
        failed_assertions++;
        printf("[FAIL] %s\n", expression);
        printf("       File: %s:%d\n", file, line);
    }
}

void test_assert_eq(long actual, long expected, const char *actual_expr, const char *expected_expr, const char *file, int line)
{
    total_assertions++;
    if (actual == expected)
    {
        passed_assertions++;
        printf("[PASS] %s == %s\n", actual_expr, expected_expr);
    }
    else
    {
        failed_assertions++;
        printf("[FAIL] %s == %s\n", actual_expr, expected_expr);
        printf("       Expected: %ld\n", expected);
        printf("       Actual:   %ld\n", actual);
        printf("       File:     %s:%d\n", file, line);
    }
}

void test_assert_ne(long actual, long expected, const char *actual_expr, const char *expected_expr, const char *file, int line)
{
    total_assertions++;
    if (actual != expected)
    {
        passed_assertions++;
        printf("[PASS] %s != %s\n", actual_expr, expected_expr);
    }
    else
    {
        failed_assertions++;
        printf("[FAIL] %s != %s\n", actual_expr, expected_expr);
        printf("       Expected: not %ld\n", expected);
        printf("       Actual:   %ld\n", actual);
        printf("       File:     %s:%d\n", file, line);
    }
}

void test_assert_str_eq(const char *actual, const char *expected, const char *actual_expr, const char *expected_expr, const char *file, int line)
{
    total_assertions++;
    if (actual != NULL &&expected != NULL &&strcmp(actual, expected) == 0)
    {
        passed_assertions++;
        printf("[PASS] %s == %s\n", actual_expr, expected_expr);
    }
    else
    {
        failed_assertions++;
        printf("[FAIL] %s == %s\n", actual_expr, expected_expr);
        printf("       Expected: \"%s\"\n", expected != NULL ? expected : "(null)");
        printf("       Actual:   \"%s\"\n", actual != NULL ? actual : "(null)");
        printf("       File:     %s:%d\n", file, line);
    }
}

void test_assert_null(const void *ptr, const char *expression, const char *file, int line)
{
    total_assertions++;
    if (ptr == NULL)
    {
        passed_assertions++;
        printf("[PASS] %s == NULL\n", expression);
    }
    else
    {
        failed_assertions++;

        printf("[FAIL] %s == NULL\n", expression);
        printf("       Actual: %p\n", ptr);
        printf("       File:   %s:%d\n", file, line);
    }
}

void test_assert_not_null(const void *ptr, const char *expression, const char *file, int line)
{
    total_assertions++;
    if (ptr != NULL)
    {
        passed_assertions++;
        printf("[PASS] %s != NULL\n", expression);
    }
    else
    {
        failed_assertions++;
        printf("[FAIL] %s != NULL\n", expression);
        printf("       Actual: NULL\n");
        printf("       File:   %s:%d\n", file, line);
    }
}

void test_run(const TestCase *tests, int count)
{
    if (tests == NULL || count <= 0)
    {
        return;
    }
    for (int i = 0; i < count; ++i)
    {
        total_cases++;
        printf("\n[TEST] %s\n", tests[i].name);
        tests[i].func();
    }
}

void test_report(void)
{
    printf("\n");
    printf("Test Cases : %d\n", total_cases);
    printf("Assertions : %d\n", total_assertions);
    printf("Passed     : %d\n", passed_assertions);
    printf("Failed     : %d\n", failed_assertions);
}

int test_result(void)
{
    return failed_assertions == 0 ? 0 : 1;
}
