// cat-box C++ firmware — boot.py + main.py parity.
// Single cooperative loop (plan §1): 250 ms control tick, 250 ms LED tick,
// WebServer::handleClient(), WiFi keepalive. No FreeRTOS app tasks, no
// mutexes: loopTask is the only writer of app state and the WiFi event
// callback only sets one volatile bool.
#include <Arduino.h>

#include "alert_stub.h"
#include "config.h"
#include "core/app_state.h"
#include "core/behavior.h"
#include "core/clock.h"
#include "led_status.h"
#include "motor.h"
#include "sensors.h"
#include "settings.h"
#include "settings_store.h"
#include "web_server.h"
#include "wifi_manager.h"

// Debug prints parity (Python printed state/hall/load every tick). Off by
// default: the Python build had them on, but they cost CDC bandwidth; flip
// with -D CATBOX_DEBUG=1.
#ifndef CATBOX_DEBUG
#define CATBOX_DEBUG 0
#endif

SettingsStore g_settings_store;

static double now_ms(void) { return (double)clock_extend_millis((uint32_t)millis()); }
static double now_epoch(void) { return (double)time(nullptr); }

// ---- behavior dependency wiring ----
static double set_cycle_ignore(void) {
    return settings_get_f(SETTING_KEY_CYCLE_START_IGNORE, DEFAULT_CYCLE_START_IGNORE);
}
static double set_cycle_eating(void) {
    return settings_get_f(SETTING_KEY_CYCLE_EATING_TIME, DEFAULT_CYCLE_EATING_TIME);
}
static double set_cycle_overshoot(void) {
    return settings_get_f(SETTING_KEY_CYCLE_OVERSHOOT_TIME, DEFAULT_CYCLE_OVERSHOOT_TIME);
}
static double set_empty_overshoot(void) {
    return settings_get_f(SETTING_KEY_EMPTY_OVERSHOOT_TIME, DEFAULT_EMPTY_OVERSHOOT_TIME);
}
static double set_empty_eating(void) {
    return settings_get_f(SETTING_KEY_EMPTY_EATING_TIME, DEFAULT_EMPTY_EATING_TIME);
}
static double set_timed_delay(void) {
    return settings_get_f(SETTING_KEY_TIMED_CYCLE_DELAY_HOURS, DEFAULT_TIMED_CYCLE_DELAY_HOURS);
}

static void motor_cw_dbg(void) {
#if CATBOX_DEBUG
    Serial.println("  rotating clockwise...");
#endif
    motor_clock_wise();
}
static void motor_ccw_dbg(void) {
#if CATBOX_DEBUG
    Serial.println("  rotating counter clockwise...");
#endif
    motor_counter_clock_wise();
}
static void motor_stop_dbg(void) {
#if CATBOX_DEBUG
    Serial.println("  stopping...");
#endif
    motor_stop();
}
static void calibrate_dbg(void) {
    calibrate_load();
#if CATBOX_DEBUG
    Serial.printf("  Recalibrated to: %.1f\n", sensors_state()->average_load);
#endif
}
static bool hall_dbg(void) {
    bool t = hall_sensor_triggered();
#if CATBOX_DEBUG
    sensors_print_halls();
#endif
    return t;
}

// bin_full.react_to_cycle() — the would-alert message is printed, never sent
// (deviation D1: email out of scope, zero TLS code). Like bin_full.py, the
// message is only built/printed when get_base_url() is not None (empty here).
static void alert_react(void) {
    if (alert_react_to_cycle()) {
        const char* url = wifi_base_url();
        if (url && url[0]) {
            char msg[256];
            alert_build_message(msg, sizeof(msg), url, alert_current_cycles());
            Serial.println(msg);
        }
    }
}

static unsigned long control_period_ms(void) {
    double secs = settings_get_f(SETTING_KEY_LOOP_SLEEP, DEFAULT_LOOP_SLEEP);
    unsigned long ms = (unsigned long)(secs * 1000.0);
    return ms < MIN_LOOP_SLEEP_MS ? MIN_LOOP_SLEEP_MS : ms;  // deviation D7
}

void setup() {
    Serial.begin(115200);
    Serial.println("I'm here to eat shit!");  // boot.py

    cb_set_time_sources(now_ms, now_epoch);
    settings_store_setup(&g_settings_store);
    settings_begin();

    sensors_setup();
    motor_setup();
    led_setup();

    BehaviorDeps deps;
    deps.motor_cw = motor_cw_dbg;
    deps.motor_ccw = motor_ccw_dbg;
    deps.motor_stop = motor_stop_dbg;
    deps.calibrate = calibrate_dbg;
    deps.hall_triggered = hall_dbg;
    deps.cat_in_the_box = cat_in_the_box;
    deps.cycle_ignore = set_cycle_ignore;
    deps.cycle_eating = set_cycle_eating;
    deps.cycle_overshoot = set_cycle_overshoot;
    deps.empty_overshoot = set_empty_overshoot;
    deps.empty_eating = set_empty_eating;
    deps.timed_delay_hours = set_timed_delay;
    behavior_init(&deps);
    app_state_init(calibrate_dbg, alert_reset, alert_react);
    app_state_init_boot_time();  // state.py last_detection/delay_start init

    lb_reset();  // main_loop.py: state.reset() before the loop (IDLE +
                 // cycles=0 + calibrate)

    web_setup();
    wifi_setup();
    Serial.println("Let's start eating some shit!");  // main.py
}

static void control_tick(void) {
    behavior_tick();  // detect -> cycle -> empty (main_loop.run order)
#if CATBOX_DEBUG
    Serial.println(state_str(lb_get_state()));
#endif
}

void loop() {
    static unsigned long last_ctrl = 0;
    static unsigned long last_led = 0;
    static unsigned long last_wifi = 0;
    unsigned long now = millis();

    if (now - last_ctrl >= control_period_ms()) {
        last_ctrl = now;
        control_tick();
    }
    if (now - last_led >= CONTROL_PERIOD_MS_DEFAULT) {
        last_led = now;
        led_tick(lb_get_state());
    }
    if (now - last_wifi >= 250) {  // stay_connected cadence
        last_wifi = now;
        wifi_tick();
    }
    web_tick();
    delay(1);  // yield to the IDLE/WiFi tasks (cooperative semantics)
}
