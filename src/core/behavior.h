// behavior — mechanical translation of litter_box detect.py/cycle.py/empty.py
// plus main_loop.run() tick order (plan §4.5). Pure: all hardware and
// settings access arrives via BehaviorDeps so host tests can script ticks.
//
// Parity note: the Python state chain calls sense.hall_sensor_triggered()
// at EACH evaluation point within one tick (the falling-edge latch is
// consumed by the first call). A pre-polled boolean would change that, so
// hall is invoked as a callback at each site, exactly like Python.
#pragma once

#include "core/app_state.h"
#include "core/clock.h"

typedef void (*CbMotorFn)(void);
typedef bool (*CbBoolFn)(void);

typedef struct {
    CbMotorFn motor_cw;
    CbMotorFn motor_ccw;
    CbMotorFn motor_stop;
    CbVoidVoid calibrate;      // sense.calibrate_load (LEVELING completion)
    CbBoolFn hall_triggered;   // sense.hall_sensor_triggered()
    CbBoolFn cat_in_the_box;   // sense.cat_in_the_box()
    // Settings getters (get_state_of(key, default) parity on device):
    double (*cycle_ignore)();      // cycle_start_ignore_hall_sensor_time
    double (*cycle_eating)();      // cycle_eating_time
    double (*cycle_overshoot)();   // cycle_overshoot_time
    double (*empty_overshoot)();   // empty_overshoot_time
    double (*empty_eating)();      // empty_eating_time
    double (*timed_delay_hours)(); // timed_cycle_delay_hours
} BehaviorDeps;

void behavior_init(const BehaviorDeps* deps);

// main_loop.run() order: detect.cat() -> cycle.do_cycle() -> empty.do_empty()
void do_detect();  // detect.cat()
void do_cycle();   // cycle.do_cycle()
void do_empty();   // empty.do_empty()
void behavior_tick(void);  // the three above, in order

// Deviation D3/D8 support: wall clock counts as synced at/after
// CLOCK_SYNCED_EPOCH.
bool clock_synced();
