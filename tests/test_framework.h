#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

typedef void (*TestFunc)(void);

typedef struct
{
    const char *name;
    TestFunc func;
} TestCase;

void test_assert(int condition, const char *expression, const char *file, int line);

void test_assert_eq(long actual, long expected, const char *actual_expr, const char *expected_expr, const char *file, int line);

void test_assert_ne(long actual, long expected, const char *actual_expr, const char *expected_expr, const char *file, int line);

void test_assert_str_eq(const char *actual, const char *expected, const char *actual_expr, const char *expected_expr, const char *file, int line);

void test_assert_null(const void *ptr, const char *expression, const char *file, int line);

void test_assert_not_null(const void *ptr, const char *expression, const char *file, int line);

void test_run(const TestCase *tests, int count);

void test_report(void);

int test_result(void);

#define TEST_ASSERT(condition) \
    test_assert((condition), #condition, __FILE__, __LINE__)

#define TEST_ASSERT_EQ(actual, expected) \
    test_assert_eq((actual), (expected),  \
                   #actual, #expected,     \
                   __FILE__, __LINE__)

#define TEST_ASSERT_NE(actual, expected) \
    test_assert_ne((actual), (expected),  \
                   #actual, #expected,     \
                   __FILE__, __LINE__)

#define TEST_ASSERT_STR_EQ(actual, expected) \
    test_assert_str_eq((actual), (expected),  \
                       #actual, #expected,     \
                       __FILE__, __LINE__)

#define TEST_ASSERT_NULL(ptr) \
    test_assert_null((ptr), #ptr, __FILE__, __LINE__)

#define TEST_ASSERT_NOT_NULL(ptr) \
    test_assert_not_null((ptr), #ptr, __FILE__, __LINE__)


#endif
