#include "test_framework.h"
#include <stdio.h>

void test_config(void);
void test_parser(void);
void test_log(void);

int main(void)
{
    const TestCase tests[] =
        {
            {"config_basic", test_config},
            {"parser_basic", test_parser},
            {"log_smoke", test_log},
        };
    printf("======= MiniShell Test =======\n");
    test_run(tests, sizeof(tests) / sizeof(tests[0]));
    test_report();
    printf("======= Test Finished =======\n");
    return test_result();
}

