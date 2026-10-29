// config.h — central defaults and pins.
//
// DEFAULT_* values mirror the Python module defaults in
// reference/python-app/litter_box/settings.py, wifi/settings.py,
// alert/settings.py. The settings.json key strings are byte-identical to the
// keys the Python app reads via persistent_state.get_state_of(key, default)
// (note: the neopixel power key is literally "neopixel_power" in Python).
#pragma once

// ---- settings.json keys + defaults (typed getters live in settings_store) ----
#define SETTING_KEY_CYCLE_START_IGNORE "cycle_start_ignore_hall_sensor_time"
#define DEFAULT_CYCLE_START_IGNORE 3.0

#define SETTING_KEY_CYCLE_WAIT_TIME "cycle_wait_time"
#define DEFAULT_CYCLE_WAIT_TIME 7.0

#define SETTING_KEY_CYCLE_EATING_TIME "cycle_eating_time"
#define DEFAULT_CYCLE_EATING_TIME 7.0

#define SETTING_KEY_CYCLE_OVERSHOOT_TIME "cycle_overshoot_time"
#define DEFAULT_CYCLE_OVERSHOOT_TIME 9.0

#define SETTING_KEY_EMPTY_OVERSHOOT_TIME "empty_overshoot_time"
#define DEFAULT_EMPTY_OVERSHOOT_TIME 5.0

#define SETTING_KEY_EMPTY_EATING_TIME "empty_eating_time"
#define DEFAULT_EMPTY_EATING_TIME 7.0

#define SETTING_KEY_HALL_PIN1 "hall_pin1"
#define DEFAULT_HALL_PIN1 11

#define SETTING_KEY_HALL_PIN2 "hall_pin2"
#define DEFAULT_HALL_PIN2 12

#define SETTING_KEY_LOAD_SENSOR_PIN "load_sensor_pin"
#define DEFAULT_LOAD_SENSOR_PIN 18

#define SETTING_KEY_LOAD_SENSOR_THRESHOLD "load_sensor_threshold"
#define DEFAULT_LOAD_SENSOR_THRESHOLD 6000.0

#define SETTING_KEY_L298N_IN1 "l298n_in1"
#define DEFAULT_L298N_IN1 4

#define SETTING_KEY_L298N_IN2 "l298n_in2"
#define DEFAULT_L298N_IN2 3

#define SETTING_KEY_LOOP_SLEEP "loop_sleep"
#define DEFAULT_LOOP_SLEEP 0.25

#define SETTING_KEY_ROTATE_REVERSED "rotate_direction_reversed"
#define DEFAULT_ROTATE_REVERSED false

#define SETTING_KEY_TIMED_CYCLE_DELAY_HOURS "timed_cycle_delay_hours"
#define DEFAULT_TIMED_CYCLE_DELAY_HOURS 24

#define SETTING_KEY_NEOPIXEL_POWER "neopixel_power"  // Python key, value = GPIO 21
#define DEFAULT_NEOPIXEL_POWER 21

#define SETTING_KEY_NEOPIXEL_PIN "neopixel_pin"
#define DEFAULT_NEOPIXEL_PIN 33

#define SETTING_KEY_BIN_FULL_CYCLES "bin_full_cycles"
#define DEFAULT_BIN_FULL_CYCLES 20

// Python bin_full.react_to_cycle reads this key with get_bin_full_cycles() default.
#define SETTING_KEY_ALERT_BIN_FULL_CYCLES "alert_bin_full_cycles"

#define SETTING_KEY_WIFI_RETRIES "wifi_connection_retries"
#define DEFAULT_WIFI_RETRIES 3

#define SETTING_KEY_WIFI_BACKOFF "wifi_connection_backoff"
#define DEFAULT_WIFI_BACKOFF 60

// Creds/config (inert smtp/alert keys — deviation D1)
#define SETTING_KEY_WIFI_SSID "wifi_ssid"
#define SETTING_KEY_WIFI_PASSWORD "wifi_password"
#define SETTING_KEY_WIFI_HOSTNAME "wifi_hostname"
#define DEFAULT_WIFI_HOSTNAME "catbox"
#define SETTING_KEY_WIFI_IP_CONFIG "wifi_ip_config"  // read/kept, never applied (parity: commented out in Python)
#define SETTING_KEY_BASE_URL "base_url"

// Mutable state keys
#define SETTING_KEY_LAST_CYCLE "last_cycle"
#define SETTING_KEY_CYCLES "cycles"

// ---- timing ----
// Deviation D7: loop_sleep floored at 20 ms to prevent a settings=0 busy loop.
#define MIN_LOOP_SLEEP_MS 20

// Deviation D3/D8: wall clock counts as synced at/after this epoch (2023-11-15).
#define CLOCK_SYNCED_EPOCH 1700000000.0

// ADC calibration sample handling (deviation D6 spurious-zero guard).
#define CALIBRATE_VALID_SAMPLES 10
#define CALIBRATE_MAX_RAW_READS 30
#define ADC_SAMPLE_MAX_ATTEMPTS 3

// ---- boot / misc ----
// settings.json read/serialize buffer. Must exceed the largest plausible
// file (settings + creds + unknown keys); overflow is refused, never
// truncated to disk (see settings.cpp persist_now).
#define SETTINGS_JSON_MAX 4096
#define SETTINGS_JSON_MAX_STR "4096"

#define DEFAULT_LOAD_SEED 44000  // Python sense.py average_load/rolling_load seed
#define NEOPIXEL_POWER_SETTLE_MS 80
#define CONTROL_PERIOD_MS_DEFAULT 250
