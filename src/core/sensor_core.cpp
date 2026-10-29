#include "core/sensor_core.h"

void sensor_state_init(SensorState* s) {
    s->old_hall1 = true;  // Python module init: old_hall_sensor1 = True
    s->old_hall2 = true;
    s->average_load = DEFAULT_LOAD_SEED;
    s->rolling_load = DEFAULT_LOAD_SEED;
}

bool sensor_hall_triggered(SensorState* s, CbDigitalRead hall1, CbDigitalRead hall2) {
    int v1 = hall1();
    int v2 = hall2();
    bool t1 = !v1 && s->old_hall1;
    bool t2 = !v2 && s->old_hall2;
    s->old_hall1 = v1 != 0;
    s->old_hall2 = v2 != 0;
    return t1 || t2;
}

int sensor_median3(int a, int b, int c) {
    if (a > b) {
        int t = a;
        a = b;
        b = t;
    }
    if (b > c) {
        int t = b;
        b = c;
        c = t;
    }
    if (a > b) {
        int t = a;
        a = b;
        b = t;
    }
    return b;
}

void sensor_calibrate(SensorState* s, CbAdcRead read_raw, CbSleepMs sleep_ms) {
    double load = 0;
    int valid = 0;
    for (int i = 0; i < CALIBRATE_MAX_RAW_READS && valid < CALIBRATE_VALID_SAMPLES; i++) {
        int v = read_raw();
        if (v == 0) {  // D6 spurious-zero guard: skip, keep sampling
            if (sleep_ms) sleep_ms(5);
            continue;
        }
        load += v;
        valid++;
        if (sleep_ms) sleep_ms(5);
    }
    if (valid > 0) {
        s->average_load = (double)load / (double)valid;  // Python: load / count (true division)
    }
}

bool sensor_cat_in_the_box(SensorState* s, CbAdcRead sample_filtered, double threshold) {
    int load = sample_filtered();
    if (load != 0) {  // D6: a still-zero sample is discarded; rolling unchanged
        s->rolling_load = (2.0 * s->rolling_load + (double)load) / 3.0;
    }
    return s->rolling_load + threshold < s->average_load;
}
