#include "sensors.h"

#include <Arduino.h>

#include "config.h"
#include "settings.h"

static SensorState g_state;
static int g_hall1_pin = DEFAULT_HALL_PIN1;
static int g_hall2_pin = DEFAULT_HALL_PIN2;
static int g_load_pin = DEFAULT_LOAD_SENSOR_PIN;

static int read_hall1(void) { return digitalRead(g_hall1_pin); }
static int read_hall2(void) { return digitalRead(g_hall2_pin); }
static int read_adc(void) { return analogRead(g_load_pin); }
static void delay_ms(int ms) { delay(ms); }

void sensors_setup(void) {
    g_hall1_pin = (int)settings_get_i(SETTING_KEY_HALL_PIN1, DEFAULT_HALL_PIN1);
    g_hall2_pin = (int)settings_get_i(SETTING_KEY_HALL_PIN2, DEFAULT_HALL_PIN2);
    g_load_pin = (int)settings_get_i(SETTING_KEY_LOAD_SENSOR_PIN, DEFAULT_LOAD_SENSOR_PIN);
    sensor_state_init(&g_state);
    pinMode(g_hall1_pin, INPUT_PULLUP);  // external pull-ups no longer needed
    pinMode(g_hall2_pin, INPUT_PULLUP);
    analogSetPinAttenuation(g_load_pin, ADC_11db);  // Python: ATTN_11DB
    analogReadResolution(16);  // 16-bit scale => threshold 6000 carries over
}

SensorState* sensors_state(void) { return &g_state; }

void sensors_print_halls(void) {
    Serial.printf("  Hall1: %d, Hall2: %d\n", digitalRead(g_hall1_pin), digitalRead(g_hall2_pin));
}

bool hall_sensor_triggered(void) {
    return sensor_hall_triggered(&g_state, read_hall1, read_hall2);
}

void calibrate_load(void) { sensor_calibrate(&g_state, read_adc, delay_ms); }

// D6 spurious-zero guard: up to ADC_SAMPLE_MAX_ATTEMPTS raw reads 5 ms
// apart, first non-zero wins, else median of the batch (a still-zero median
// is discarded inside sensor_core).
static int filtered_sample(void) {
    int reads[ADC_SAMPLE_MAX_ATTEMPTS];
    int n = 0;
    for (int i = 0; i < ADC_SAMPLE_MAX_ATTEMPTS; i++) {
        if (n > 0) delay(5);  // Python-port cadence: 5 ms between raw reads
        reads[n] = read_adc();
        if (reads[n] != 0) return reads[n];  // first non-zero wins
        n++;
    }
    return sensor_median3(reads[0], reads[1], reads[2]);
}

bool cat_in_the_box(void) {
    double threshold = settings_get_f(SETTING_KEY_LOAD_SENSOR_THRESHOLD,
                                      DEFAULT_LOAD_SENSOR_THRESHOLD);
    return sensor_cat_in_the_box(&g_state, filtered_sample, threshold);
}
