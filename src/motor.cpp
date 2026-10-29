#include "motor.h"

#include <Arduino.h>

#include "config.h"
#include "settings.h"

static int g_in1 = DEFAULT_L298N_IN1;  // "backward" in rotate.py
static int g_in2 = DEFAULT_L298N_IN2;  // "forward" in rotate.py
static bool g_reversed = DEFAULT_ROTATE_REVERSED;

void motor_setup(void) {
    g_in1 = (int)settings_get_i(SETTING_KEY_L298N_IN1, DEFAULT_L298N_IN1);
    g_in2 = (int)settings_get_i(SETTING_KEY_L298N_IN2, DEFAULT_L298N_IN2);
    g_reversed = settings_get_b(SETTING_KEY_ROTATE_REVERSED, DEFAULT_ROTATE_REVERSED);
    pinMode(g_in1, OUTPUT);
    pinMode(g_in2, OUTPUT);
    motor_stop();
}

static void write_pins(bool forward, bool backward) {
    digitalWrite(g_in2, forward ^ g_reversed);
    digitalWrite(g_in1, backward ^ g_reversed);
}

void motor_clock_wise(void) { write_pins(true, false); }
void motor_counter_clock_wise(void) { write_pins(false, true); }
void motor_stop(void) { write_pins(false, false); }
