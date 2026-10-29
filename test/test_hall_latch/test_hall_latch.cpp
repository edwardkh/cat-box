// Hall falling-edge latch parity (sense.py hall_sensor_triggered):
// trig = !now && was, both stored values update every poll.
#include <unity.h>

#include "core/sensor_core.h"

static int g_h1, g_h2;
static int read_h1(void) { return g_h1; }
static int read_h2(void) { return g_h2; }

static SensorState st;

static void test_idle_high_stays_false(void) {
    sensor_state_init(&st);
    g_h1 = 1;
    g_h2 = 1;
    for (int i = 0; i < 5; i++) TEST_ASSERT_FALSE(sensor_hall_triggered(&st, read_h1, read_h2));
}

static void test_falling_edge_fires_once(void) {
    sensor_state_init(&st);
    g_h1 = 1;
    g_h2 = 1;
    TEST_ASSERT_FALSE(sensor_hall_triggered(&st, read_h1, read_h2));
    g_h1 = 0;  // magnet approaches
    TEST_ASSERT_TRUE(sensor_hall_triggered(&st, read_h1, read_h2));
    TEST_ASSERT_FALSE(sensor_hall_triggered(&st, read_h1, read_h2));  // latch consumed
    TEST_ASSERT_FALSE(sensor_hall_triggered(&st, read_h1, read_h2));
}

// Python inits old_hall_sensor* = True, so a pin that is already low at the
// first poll counts as a falling edge on that first poll.
static void test_low_at_first_poll_fires_once(void) {
    sensor_state_init(&st);
    g_h1 = 1;
    g_h2 = 0;
    TEST_ASSERT_TRUE(sensor_hall_triggered(&st, read_h1, read_h2));
    TEST_ASSERT_FALSE(sensor_hall_triggered(&st, read_h1, read_h2));
}

static void test_both_pins_same_tick(void) {
    sensor_state_init(&st);
    g_h1 = 0;
    g_h2 = 0;
    TEST_ASSERT_TRUE(sensor_hall_triggered(&st, read_h1, read_h2));
    TEST_ASSERT_FALSE(sensor_hall_triggered(&st, read_h1, read_h2));
}

static void test_rearm_after_pass(void) {
    sensor_state_init(&st);
    g_h1 = 1;
    g_h2 = 1;
    sensor_hall_triggered(&st, read_h1, read_h2);
    g_h1 = 0;
    TEST_ASSERT_TRUE(sensor_hall_triggered(&st, read_h1, read_h2));
    g_h1 = 1;  // magnet leaves
    TEST_ASSERT_FALSE(sensor_hall_triggered(&st, read_h1, read_h2));
    g_h1 = 0;  // next magnet
    TEST_ASSERT_TRUE(sensor_hall_triggered(&st, read_h1, read_h2));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_idle_high_stays_false);
    RUN_TEST(test_falling_edge_fires_once);
    RUN_TEST(test_low_at_first_poll_fires_once);
    RUN_TEST(test_both_pins_same_tick);
    RUN_TEST(test_rearm_after_pass);
    return UNITY_END();
}
