#include "test_framework.h"
#include <stdio.h>

static int total_tests = 0;
static int failed_tests = 0;

void test_assert(int condition, const char *expression, const char *file, int line)
{
    total_tests++;
    if (condition)
    {
        printf("[PASS] %s\n", expression);
    }
    else
    {
        failed_tests++;
        printf("[FAIL] %s\n", expression);
        printf("    File:%s:%d\n", file, line);
    }
}

void test_report(void)
{
    int pass_tests = total_tests - failed_tests;
    printf("\n");
    printf("----------------------------------------\n");
    printf("Tests : %d\n", total_tests);
    printf("Passed: %d\n", pass_tests);
    printf("Failed: %d\n", failed_tests);
    printf("----------------------------------------\n");
}