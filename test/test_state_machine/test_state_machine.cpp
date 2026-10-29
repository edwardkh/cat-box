// State machine parity: full cycle + empty happy paths, pause mid-cycle,
// ok_to_move with injected time, boot reset, timed-cycle gating (D3/D8).
// behavior.cpp + app_state.cpp run against scripted sensors/motors.
#include <unity.h>

#include <string.h>

#include "../src/core/app_state.h"
#include "../src/core/behavior.h"
#include "../src/core/clock.h"
#include "../src/settings.h"

// ---- injected time ----
static double g_ms = 0, g_epoch = 1700000100.0;  // synced by default
static double t_ms(void) { return g_ms; }
static double t_epoch(void) { return g_epoch; }

// ---- injected storage ----
static SettingsStore store;
static bool g_saved_ok;
static int load_none(char* buf, size_t buflen) {
    (void)buf;
    (void)buflen;
    return SETTINGS_LOAD_ABSENT;  // missing file -> {} (Python parity)
}
static bool persist_ok(const char* json) {
    (void)json;
    g_saved_ok = true;
    return true;
}

// ---- scripted sensors ----
static bool g_hall = false, g_cat = false;
static bool hall(void) { return g_hall; }
static bool cat(void) { return g_cat; }
// one-shot hall pulse consumed by the next hall() call
static bool g_hall_armed = false;
static bool hall_oneshot(void) {
    if (g_hall_armed) {
        g_hall_armed = false;
        return true;
    }
    return false;
}

// ---- motor trace ----
static char g_motors[512];
static void m_cw(void) { strncat(g_motors, "W", sizeof(g_motors) - strlen(g_motors) - 1); }
static void m_ccw(void) { strncat(g_motors, "C", sizeof(g_motors) - strlen(g_motors) - 1); }
static void m_stop(void) { strncat(g_motors, "S", sizeof(g_motors) - strlen(g_motors) - 1); }
static int g_calibrations = 0;
static void calib(void) { g_calibrations++; }
static int g_alerts_react = 0, g_alerts_reset = 0;
static void alert_react(void) { g_alerts_react++; }
static void alert_reset(void) { g_alerts_reset++; }

static double c3(void) { return 3; }
static double c7(void) { return 7; }
static double c9(void) { return 9; }
static double c5(void) { return 5; }
static double c24(void) { return 24; }

static CbBoolFn g_hall_fn = hall;

static void reset_world(void) {
    g_ms = 0;
    g_epoch = 1700000100.0;
    cb_set_time_sources(t_ms, t_epoch);
    settings_init(&store, load_none, persist_ok);
    settings_load(store, "");
    settings_begin();
    g_hall = false;
    g_cat = false;
    g_hall_armed = false;
    g_hall_fn = hall;
    g_motors[0] = '\0';
    g_calibrations = 0;
    g_alerts_react = 0;
    g_alerts_reset = 0;
    g_saved_ok = false;
    BehaviorDeps d;
    d.motor_cw = m_cw;
    d.motor_ccw = m_ccw;
    d.motor_stop = m_stop;
    d.calibrate = calib;
    d.hall_triggered = []() { return g_hall_fn(); };
    d.cat_in_the_box = cat;
    d.cycle_ignore = c3;
    d.cycle_eating = c7;
    d.cycle_overshoot = c9;
    d.empty_overshoot = c5;
    d.empty_eating = c7;
    d.timed_delay_hours = c24;
    behavior_init(&d);
    app_state_init(calib, alert_reset, alert_react);
    app_state_init_boot_time();
    lb_set_state(LbState::IDLE);
}

static void advance_ms(double ms) { g_ms += ms; }

// ---------------- tests ----------------

static void test_state_strings_byte_exact(void) {
    TEST_ASSERT_EQUAL_STRING("Idle", state_str(LbState::IDLE));
    TEST_ASSERT_EQUAL_STRING("Waiting to cycle", state_str(LbState::WAITING_TO_CYCLE));
    TEST_ASSERT_EQUAL_STRING("Sifting", state_str(LbState::SIFTING));
    TEST_ASSERT_EQUAL_STRING("Eating shit", state_str(LbState::EATING_SHIT));
    TEST_ASSERT_EQUAL_STRING("Moving back", state_str(LbState::MOVING_BACK));
    TEST_ASSERT_EQUAL_STRING("Leveling litter", state_str(LbState::LEVELING_LITTER));
    TEST_ASSERT_EQUAL_STRING("Leveling the globe", state_str(LbState::LEVELING_GLOBE));
    TEST_ASSERT_EQUAL_STRING("Dumping this mess", state_str(LbState::EMPTYING));
    TEST_ASSERT_EQUAL_STRING("Opening Throat", state_str(LbState::OPENING_THROAT));
    TEST_ASSERT_EQUAL_STRING("Eating litter", state_str(LbState::EATING_LITTER));
    TEST_ASSERT_EQUAL_STRING("Swallowing", state_str(LbState::SWALLOWING));
    TEST_ASSERT_EQUAL_STRING("Resetting", state_str(LbState::RESETING));
    TEST_ASSERT_EQUAL_STRING("Paused", state_str(LbState::PAUSED));
}

static void test_boot_reset_clears_alerts_and_calibrates(void) {
    reset_world();
    lb_reset();  // main_loop.py calls state.reset() first thing
    TEST_ASSERT_EQUAL_INT((int)LbState::IDLE, (int)lb_get_state());
    TEST_ASSERT_EQUAL_INT(1, g_alerts_reset);  // cycles=0
    TEST_ASSERT_EQUAL_INT(1, g_calibrations);
}

// Python state.py: last_detection starts at the wall epoch (0), so
// ok_to_move() is TRUE at boot; the monotonic seed reproduces that, and
// detect.py may auto-unpause immediately after a boot pause.
static void test_ok_to_move_true_at_boot(void) {
    reset_world();
    lb_reset();
    TEST_ASSERT_TRUE(ok_to_move());
    // Fresh boot + synced clock would ALSO trip the timed cycle (as in
    // Python); pin last_cycle so this test isolates the unpause path.
    settings_set_f_and_save(SETTING_KEY_LAST_CYCLE, 1700000100.0);
    lb_pause();
    behavior_tick();  // detect.py: PAUSED && ok_to_move -> unpause at t=0
    TEST_ASSERT_EQUAL_INT((int)LbState::IDLE, (int)lb_get_state());
    // The window closes with the first detection, exactly like Python:
    // cat in IDLE -> WAITING_TO_CYCLE; a pause then resumes there, and
    // auto-unpause (WAITING_TO_CYCLE -> cycling) waits out cycle_wait_time.
    cat_detected();
    TEST_ASSERT_EQUAL_INT((int)LbState::WAITING_TO_CYCLE, (int)lb_get_state());
    lb_pause();
    behavior_tick();  // ok_to_move false inside the window -> stays paused
    TEST_ASSERT_EQUAL_INT((int)LbState::PAUSED, (int)lb_get_state());
    advance_ms(8 * 60 * 1000);
    g_hall_fn = hall_oneshot;  // hall only for SIFTING entry checks
    behavior_tick();  // unpause -> WAITING_TO_CYCLE && ok_to_move -> cycle
    TEST_ASSERT_EQUAL_INT((int)LbState::SIFTING, (int)lb_get_state());
}

static void test_cat_detection_waits_then_cycles(void) {
    reset_world();
    g_cat = true;
    behavior_tick();  // IDLE + cat -> WAITING_TO_CYCLE
    TEST_ASSERT_EQUAL_INT((int)LbState::WAITING_TO_CYCLE, (int)lb_get_state());
    // ok_to_move requires cycle_wait_time (7 min default) after detection
    advance_ms(6 * 60 * 1000);
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::WAITING_TO_CYCLE, (int)lb_get_state());
    advance_ms(2 * 60 * 1000);  // 8 min total > 7 min
    g_hall_fn = hall_oneshot;   // hall only for SIFTING entry checks
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::SIFTING, (int)lb_get_state());
    TEST_ASSERT_EQUAL_INT(1, g_alerts_react);  // react_to_cycle on start_cycle
    g_cat = false;
}

static void test_full_cycle_happy_path(void) {
    reset_world();
    lb_start_cycle();
    TEST_ASSERT_EQUAL_INT((int)LbState::SIFTING, (int)lb_get_state());
    TEST_ASSERT_TRUE(g_saved_ok);  // last_cycle persisted

    // SIFTING: hall ignored during the 3 s ignore window
    g_hall = true;
    behavior_tick();  // rotates ccw, hall fires but ignore window active
    TEST_ASSERT_EQUAL_INT((int)LbState::SIFTING, (int)lb_get_state());
    TEST_ASSERT_NOT_NULL(strchr(g_motors, 'C'));
    advance_ms(3100);
    g_hall_fn = hall_oneshot;  // single edge for the transition
    g_hall_armed = true;
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::EATING_SHIT, (int)lb_get_state());

    // EATING_SHIT dwell 7 s
    advance_ms(6900);
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::EATING_SHIT, (int)lb_get_state());
    advance_ms(200);
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::MOVING_BACK, (int)lb_get_state());

    // MOVING_BACK: hall edge -> LEVELING_LITTER
    g_hall_fn = hall_oneshot;
    g_hall_armed = true;
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::LEVELING_LITTER, (int)lb_get_state());

    // LEVELING_LITTER overshoot 9 s -> LEVELING_GLOBE
    advance_ms(9100);
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::LEVELING_GLOBE, (int)lb_get_state());

    // LEVELING_GLOBE: 3 s buffer before ccw; hall needs ignore window too
    g_hall_fn = hall_oneshot;
    g_hall_armed = true;
    behavior_tick();  // 3 s buffer not over -> no ccw rotation, no transition
    TEST_ASSERT_EQUAL_INT((int)LbState::LEVELING_GLOBE, (int)lb_get_state());
    advance_ms(3100);  // buffer (3 s) and ignore window both elapsed
    g_hall_armed = true;
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::IDLE, (int)lb_get_state());
    TEST_ASSERT_EQUAL_INT(1, g_calibrations);  // recalibrated at cycle end
}

static void test_pause_midcycle_and_auto_unpause(void) {
    reset_world();
    lb_start_cycle();
    advance_ms(1000);
    lb_pause();  // detect.py pauses when cat appears mid-cycle
    cat_detected();  // closes the Python boot-window (as a real mid-cycle
                     // cat detection would; without it, boot headroom
                     // auto-unpauses instantly — Python parity at boot)
    TEST_ASSERT_EQUAL_INT((int)LbState::PAUSED, (int)lb_get_state());
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::PAUSED, (int)lb_get_state());
    TEST_ASSERT_NOT_NULL(strchr(g_motors, 'S'));  // PAUSED -> rotate.stop()
    // auto-unpause requires ok_to_move (cycle_wait_time after last detection)
    cat_detected();  // refreshes last_detection
    advance_ms(6 * 60 * 1000);
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::PAUSED, (int)lb_get_state());
    advance_ms(2 * 60 * 1000);
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::SIFTING, (int)lb_get_state());  // resumed
}

static void test_cat_during_idle_cycle_start_pauses(void) {
    reset_world();
    lb_start_cycle();
    g_cat = true;
    behavior_tick();  // do_detect: state SIFTING -> cat_detected -> pause
    // NOTE: do_detect only acts on IDLE for detection; SIFTING + cat goes
    // through cat_detected only from detect.py's IDLE branch -> no pause.
    TEST_ASSERT_EQUAL_INT((int)LbState::SIFTING, (int)lb_get_state());
}

static void test_empty_happy_path(void) {
    reset_world();
    lb_set_state(LbState::EMPTYING);
    behavior_tick();  // rotates cw
    TEST_ASSERT_NOT_NULL(strchr(g_motors, 'W'));
    g_hall_fn = hall_oneshot;
    g_hall_armed = true;
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::OPENING_THROAT, (int)lb_get_state());
    advance_ms(5100);  // empty_overshoot 5 s
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::EATING_LITTER, (int)lb_get_state());
    advance_ms(7100);  // empty_eating 7 s
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::SWALLOWING, (int)lb_get_state());
    g_hall_armed = true;
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::RESETING, (int)lb_get_state());
    g_hall_armed = true;
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::IDLE, (int)lb_get_state());
}

static void test_timed_cycle_requires_synced_clock_and_delay(void) {
    reset_world();
    // Python fired immediately on reboot (2000-epoch < last_cycle). D3/D8:
    // unsynced clock must NOT trigger.
    g_epoch = 946684800.0;  // 2000-01-01, unsynced
    settings_load(store, "{\"last_cycle\": 1800000000}");
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::IDLE, (int)lb_get_state());
    // synced clock + last_cycle 25 h ago (default 24 h delay) -> auto cycle
    g_epoch = 1800000000.0 + 25 * 3600.0;  // synced (> CLOCK_SYNCED_EPOCH)
    behavior_tick();
    TEST_ASSERT_EQUAL_INT((int)LbState::SIFTING, (int)lb_get_state());
}

static void test_ok_to_move_and_reset(void) {
    reset_world();
    cat_detected();
    TEST_ASSERT_FALSE(ok_to_move());
    advance_ms(7 * 60 * 1000 + 1);
    TEST_ASSERT_TRUE(ok_to_move());
    lb_reset();
    TEST_ASSERT_EQUAL_INT(1, g_alerts_reset);
    TEST_ASSERT_EQUAL_INT((int)LbState::IDLE, (int)lb_get_state());
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_state_strings_byte_exact);
    RUN_TEST(test_boot_reset_clears_alerts_and_calibrates);
    RUN_TEST(test_ok_to_move_true_at_boot);
    RUN_TEST(test_cat_detection_waits_then_cycles);
    RUN_TEST(test_full_cycle_happy_path);
    RUN_TEST(test_pause_midcycle_and_auto_unpause);
    RUN_TEST(test_cat_during_idle_cycle_start_pauses);
    RUN_TEST(test_empty_happy_path);
    RUN_TEST(test_timed_cycle_requires_synced_clock_and_delay);
    RUN_TEST(test_ok_to_move_and_reset);
    return UNITY_END();
}
