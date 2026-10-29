// Arduino millis() is a wrapping uint32_t; the dwell/ok_to_move timers store a
// start value and compare later, so the firmware must not see the wrap.
#include <unity.h>

#include "core/clock.h"

void setUp(void) {}
void tearDown(void) {}

void test_extends_monotonically(void) {
    TEST_ASSERT_EQUAL_UINT64(0, clock_extend_millis(0));
    TEST_ASSERT_EQUAL_UINT64(1000, clock_extend_millis(1000));
    TEST_ASSERT_EQUAL_UINT64(4294967295ULL, clock_extend_millis(4294967295u));
}

void test_survives_the_49_day_wrap(void) {
    clock_extend_millis(4294967295u);
    // millis() restarts at 0; the extended value keeps climbing
    TEST_ASSERT_EQUAL_UINT64(4294967296ULL, clock_extend_millis(0));
    TEST_ASSERT_EQUAL_UINT64(4294967296ULL + 5000, clock_extend_millis(5000));
}

void test_repeated_reads_are_stable(void) {
    uint64_t a = clock_extend_millis(6000);
    uint64_t b = clock_extend_millis(6000);
    TEST_ASSERT_EQUAL_UINT64(a, b);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_extends_monotonically);
    RUN_TEST(test_survives_the_49_day_wrap);
    RUN_TEST(test_repeated_reads_are_stable);
    return UNITY_END();
}
