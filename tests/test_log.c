#include "log.h"
#include <stdio.h>

void test_log()
{
    log_init();
    log_debug("debug message");
    log_info("info message");
    log_error("error message");
    log_setlevel(LOG_INFO);
    log_debug("debug");
    log_info("info");
    log_error("error");
}
