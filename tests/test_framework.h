#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

void test_assert(int condition, const char *expression, const char *file, int line);
void test_report(void);

#define TEST_ASSERT(condition) \
    test_assert((condition), #condition, __FILE__, __LINE__)

#endif
