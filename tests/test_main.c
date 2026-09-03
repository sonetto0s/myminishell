#include "test_framework.h"
#include <stdio.h>

void test_config_init(void);
void test_config_parse_line(void);
void test_config_load(void);
void test_config_load_missing_file(void);
void test_parser_basic(void);
void test_parser_pipeline(void);
void test_shell_context_init(void);
void test_shell_context_destroy(void);
void test_parser_redirect_out(void);
void test_builtin_count(void);
void test_builtin_lookup_known(void);
void test_builtin_lookup_unknown(void);
void test_builtin_get(void);
void test_parser_redirect_append(void);
void test_parser_redirect_in(void);
void test_parser_background(void);
void test_parser_exit_status(void);
void test_dispatcher_builtin(void);
void test_dispatcher_external(void);
void test_system_info_collect_null(void);
void test_system_info_collect(void);
void test_system_info_fields(void);
void test_dispatcher_builtin_redirect(void);
void test_dispatcher_external_not_found(void);
void test_parser_empty_input(void);
void test_parser_invalid_pipe(void);
void test_parser_invalid_redirect(void);
void test_command_init(void);
void test_command_init_null(void);
void test_command_free_single(void);
void test_command_free_chain(void);
void test_log(void);
void test_job_init(void);
void test_jobmanager_init(void);
void test_job_add(void);
void test_job_find(void);
void test_process_add(void);
void test_job_reap_stop_continue(void);
void test_job_remove(void);
void test_job_destroy(void);
void test_job_reap_exit(void);
void test_job_reap_exit_nonzero(void);
void test_job_cleanup_done(void);
void test_job_reap_signal(void);
void test_job_continue(void);
void test_job_multi_process(void);
void test_event_init(void);
void test_event_init_idempotent(void);
void test_event_notify(void);
void test_event_shut(void);
void test_event_close_in_child(void);

int main(void)
{
    const TestCase tests[] =
        {
            {"config_init", test_config_init},
            {"config_parse_line", test_config_parse_line},
            {"config_load", test_config_load},
            {"config_load_missing_file", test_config_load_missing_file},
            {"parser_basic", test_parser_basic},
            {"job_multi_process", test_job_multi_process},
            {"parser_pipeline", test_parser_pipeline},
            {"parser_redirect_out", test_parser_redirect_out},
            {"builtin_count", test_builtin_count},
            {"builtin_lookup_known", test_builtin_lookup_known},
            {"builtin_lookup_unknown", test_builtin_lookup_unknown},
            {"builtin_get", test_builtin_get},
            {"job_init", test_job_init},
            {"job_continue", test_job_continue},
            {"jobmanager_init", test_jobmanager_init},
            {"job_add", test_job_add},
            {"job_find", test_job_find},
            {"job_reap_stop_continue", test_job_reap_stop_continue},
            {"process_add", test_process_add},
            {"job_reap_signal", test_job_reap_signal},
            {"job_remove", test_job_remove},
            {"job_destroy", test_job_destroy},
            {"job_cleanup_done", test_job_cleanup_done},
            {"parser_redirect_append", test_parser_redirect_append},
            {"parser_redirect_in", test_parser_redirect_in},
            {"job_reap_exit", test_job_reap_exit},
            {"job_reap_exit_nonzero", test_job_reap_exit_nonzero},
            {"parser_background", test_parser_background},
            {"parser_exit_status", test_parser_exit_status},
            {"event_init", test_event_init},
            {"event_init_idempotent", test_event_init_idempotent},
            {"event_notify", test_event_notify},
            {"event_shut", test_event_shut},
            {"event_close_in_child", test_event_close_in_child},
            {"parser_empty_input", test_parser_empty_input},
            {"parser_invalid_pipe", test_parser_invalid_pipe},
            {"parser_invalid_redirect", test_parser_invalid_redirect},
            {"dispatcher_builtin", test_dispatcher_builtin},
            {"dispatcher_external", test_dispatcher_external},
            {"dispatcher_builtin_redirect", test_dispatcher_builtin_redirect},
            {"dispatcher_external_not_found", test_dispatcher_external_not_found},
            {"command_init", test_command_init},
            {"shell_context_init", test_shell_context_init},
            {"shell_context_destroy", test_shell_context_destroy},
            {"system_info_collect_null", test_system_info_collect_null},
            {"system_info_collect", test_system_info_collect},
            {"system_info_fields", test_system_info_fields},
            {"command_init_null", test_command_init_null},
            {"command_free_single", test_command_free_single},
            {"command_free_chain", test_command_free_chain},
            {"log_smoke", test_log},
        };

    printf("======= MiniShell Test =======\n");
    test_run(tests, sizeof(tests) / sizeof(tests[0]));
    test_report();
    printf("======= Test Finished =======\n");
    return test_result();
}

