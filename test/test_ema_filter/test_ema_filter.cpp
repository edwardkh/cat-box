// EMA + spurious-zero filter parity (sense.py cat_in_the_box/calibrate_load,
// deviation D6). Python: rolling = (2*rolling + load) / 3 with true division.
#include <unity.h>

#include "../src/core/sensor_core.h"

static SensorState st;

static int g_seq[32];
static int g_seq_i, g_seq_n;
static int next_seq(void) { return g_seq_i < g_seq_n ? g_seq[g_seq_i++] : g_seq[g_seq_n - 1]; }

static void no_sleep(int) {}

static void test_seeds_match_python(void) {
    sensor_state_init(&st);
    TEST_ASSERT_EQUAL_INT(44000, (int)st.average_load);
    TEST_ASSERT_EQUAL_INT(44000, (int)st.rolling_load);
    TEST_ASSERT_TRUE(st.old_hall1 && st.old_hall2);
}

static void test_ema_math_true_division(void) {
    sensor_state_init(&st);  // rolling 44000
    g_seq[0] = 40000;
    g_seq_i = 0;
    g_seq_n = 1;
    // (2*44000 + 40000)/3 = 42666.666...
    sensor_cat_in_the_box(&st, next_seq, 6000);
    TEST_ASSERT_DOUBLE_WITHIN(1e-9, (2.0 * 44000.0 + 40000.0) / 3.0, st.rolling_load);
}

static void test_trigger_condition(void) {
    sensor_state_init(&st);
    st.rolling_load = 37000;  // 37000 + 6000 = 43000 < 44000 -> triggered
    st.average_load = 44000;
    g_seq[0] = 37000;
    g_seq_i = 0;
    g_seq_n = 1;
    TEST_ASSERT_TRUE(sensor_cat_in_the_box(&st, next_seq, 6000));
    // exactly at the boundary: rolling + threshold == average -> False (strict <)
    st.rolling_load = 38000;
    g_seq[0] = 38000;  // sample equal to rolling keeps it fixed
    g_seq_i = 0;
    g_seq_n = 1;
    TEST_ASSERT_FALSE(sensor_cat_in_the_box(&st, next_seq, 6000));
}

static void test_zero_sample_discarded(void) {
    sensor_state_init(&st);
    st.rolling_load = 12345.0;
    g_seq[0] = 0;  // filtered sample still zero -> discard, rolling unchanged
    g_seq_i = 0;
    g_seq_n = 1;
    sensor_cat_in_the_box(&st, next_seq, 6000);
    TEST_ASSERT_EQUAL_DOUBLE(12345.0, st.rolling_load);
}

static void test_calibrate_mean_and_zero_skip(void) {
    sensor_state_init(&st);
    for (int i = 0; i < 32; i++) g_seq[i] = 50000;
    g_seq[0] = 0;  // two spurious zeros: skipped, still 10 valid -> mean 50000
    g_seq[1] = 0;
    g_seq_i = 0;
    g_seq_n = 32;
    sensor_calibrate(&st, next_seq, no_sleep);
    TEST_ASSERT_EQUAL_DOUBLE(50000.0, st.average_load);
}

static void test_calibrate_all_zeros_keeps_previous(void) {
    sensor_state_init(&st);
    st.average_load = 42424.0;
    for (int i = 0; i < 32; i++) g_seq[i] = 0;
    g_seq_i = 0;
    g_seq_n = 32;
    sensor_calibrate(&st, next_seq, no_sleep);
    TEST_ASSERT_EQUAL_DOUBLE(42424.0, st.average_load);  // never divide by zero
}

static void test_median3(void) {
    TEST_ASSERT_EQUAL_INT(2, sensor_median3(1, 2, 3));
    TEST_ASSERT_EQUAL_INT(2, sensor_median3(3, 2, 1));
    TEST_ASSERT_EQUAL_INT(5, sensor_median3(5, 5, 9));
    TEST_ASSERT_EQUAL_INT(0, sensor_median3(0, 7, 0));  // two zeros -> median 0
    TEST_ASSERT_EQUAL_INT(100, sensor_median3(100, 0, 200));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_seeds_match_python);
    RUN_TEST(test_ema_math_true_division);
    RUN_TEST(test_trigger_condition);
    RUN_TEST(test_zero_sample_discarded);
    RUN_TEST(test_calibrate_mean_and_zero_skip);
    RUN_TEST(test_calibrate_all_zeros_keeps_previous);
    RUN_TEST(test_median3);
    return UNITY_END();
}
