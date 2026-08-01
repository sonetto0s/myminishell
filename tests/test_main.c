#include <stdio.h>

void test_config();
void test_parser();
void test_log();

int main()
{
    printf("======= MiniShell Test =======\n");
    test_config();
    test_parser();
    test_log();
    printf("======= Test Finished =======\n");
    return 0;
}