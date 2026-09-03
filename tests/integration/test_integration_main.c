#include "test_framework.h"
#include <stdio.h>

void test_shell_echo(void);
void test_shell_pwd(void);
void test_shell_external_command(void);
void test_shell_command_not_found(void);
void test_shell_redirect_output(void);
void test_shell_redirect_append(void);
void test_shell_redirect_input(void);
void test_shell_pipeline_two_commands(void);
void test_shell_pipeline_three_commands(void);
void test_shell_exit_status_success(void);
void test_shell_exit_status_failure(void);
void test_shell_background_basic(void);
void test_shell_background_multiple(void);
void test_shell_background_reap(void);
void test_shell_background_cleanup(void);

int main(void)
{
    const TestCase tests[] =
        {
            {"shell_echo", test_shell_echo},
            {"shell_pwd", test_shell_pwd},
            {"shell_external_command", test_shell_external_command},
            {"shell_command_not_found", test_shell_command_not_found},
            {"shell_redirect_output", test_shell_redirect_output},
            {"shell_redirect_append", test_shell_redirect_append},
            {"shell_redirect_input", test_shell_redirect_input},
            {"shell_pipeline_two_commands", test_shell_pipeline_two_commands},
            {"shell_pipeline_three_commands", test_shell_pipeline_three_commands},
            {"shell_exit_status_success", test_shell_exit_status_success},
            {"shell_exit_status_failure", test_shell_exit_status_failure},
            {"shell_background_basic", test_shell_background_basic},
            {"shell_background_multiple", test_shell_background_multiple},
            {"shell_background_reap", test_shell_background_reap},
            {"shell_background_cleanup", test_shell_background_cleanup},
        };
    printf("======= MiniShell Integration Test =======\n");
    test_run(tests, (int)(sizeof(tests) / sizeof(tests[0])));
    test_report();
    printf("======= Integration Test Finished =======\n");
    return test_result();
}


