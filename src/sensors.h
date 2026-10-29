// sensors — device-side ADC/hall wiring over the pure sensor_core logic.
#pragma once

#include "core/sensor_core.h"

// Boot: reads hall/load pins + threshold from settings (Python read them at
// import time; changes via PATCH apply after reboot).
void sensors_setup(void);

SensorState* sensors_state(void);

// sense.py hall_sensor_triggered()
bool hall_sensor_triggered(void);

// sense.py calibrate_load()
void calibrate_load(void);

// sense.py cat_in_the_box()
bool cat_in_the_box(void);

// Debug print of both hall levels (uses the configured pins).
void sensors_print_halls(void);
