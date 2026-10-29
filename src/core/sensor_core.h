// sensor_core — pure logic of litter_box/sense.py (host-testable).
// Hall edge latch, calibration mean (zero-guarded), EMA load filter.
#pragma once

#include <stdbool.h>

#include "config.h"
#include "core/clock.h"

typedef int (*CbAdcRead)(void);      // one raw 16-bit ADC sample
typedef int (*CbDigitalRead)(void);  // one hall pin level (pull-up inputs)
typedef void (*CbSleepMs)(int ms);      // device: delay(ms); tests: no-op

typedef struct {
    // hall latch (sense.py old_hall_sensor1/2, both init True)
    bool old_hall1;
    bool old_hall2;
    // load sensor (sense.py average_load/rolling_load seed 44000)
    double average_load;
    double rolling_load;
} SensorState;

void sensor_state_init(SensorState* s);

// hall_sensor_triggered() parity: trig = !now && was for each pin; both
// stored values update every poll; returns t1 || t2.
bool sensor_hall_triggered(SensorState* s, CbDigitalRead hall1, CbDigitalRead hall2);

// calibrate_load() parity with deviation D6 spurious-zero guard: up to
// CALIBRATE_MAX_RAW_READS raw samples, skip zeros, stop after
// CALIBRATE_VALID_SAMPLES valid; mean of the valid ones (Python: sum/10).
// If fewer than 1 valid sample: average unchanged (never divide by zero).
void sensor_calibrate(SensorState* s, CbAdcRead read_raw, CbSleepMs sleep_ms);

// cat_in_the_box() parity with D6: take a sample (device wrapper retries up
// to 3 raw reads 5 ms apart on zero, then median-of-3); a still-zero sample
// is DISCARDED — rolling value keeps its previous value and returns the
// trigger decision computed from it (Python: rolling+threshold < average).
bool sensor_cat_in_the_box(SensorState* s, CbAdcRead sample_filtered, double threshold);

// Exposed for tests: median of 3 ints.
int sensor_median3(int a, int b, int c);
