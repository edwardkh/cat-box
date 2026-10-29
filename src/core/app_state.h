// app_state — litter_box/state.py parity. State strings are byte-exact
// (the frontend renders them verbatim). Timing parity per plan §4.1/§4.5:
// dwell/ok_to_move timers are monotonic ms (deviation D5); last_cycle and the
// timed-cycle check use wall-clock epoch seconds.
#pragma once

#include <stdint.h>

enum class LbState : uint8_t {
    IDLE,
    WAITING_TO_CYCLE,
    SIFTING,
    EATING_SHIT,
    MOVING_BACK,
    LEVELING_LITTER,
    LEVELING_GLOBE,
    EMPTYING,
    OPENING_THROAT,
    EATING_LITTER,
    SWALLOWING,
    RESETING,
    PAUSED,
};

// Byte-exact Python state strings.
const char* state_str(LbState s);

typedef void (*CbVoidVoid)(void);

// Device wiring (called once from setup before lb_reset):
//  calibrate  -> sensor calibrate_load
//  alert_reset/alert_react -> alert_stub
void app_state_init(CbVoidVoid calibrate,
                    CbVoidVoid alert_reset,
                    CbVoidVoid alert_react);

// Python module-import init of resume_state/last_detection/delay_start
// (state.py lines 26-28). Call at boot after app_state_init (and in tests
// between cases).
void app_state_init_boot_time(void);

LbState lb_get_state();
void lb_set_state(LbState s);

void cat_detected();
void lb_pause();
void lb_unpause();
bool ok_to_move();          // reads cycle_wait_time setting
void start_delay();
bool delay_over(double dwell_secs);

// state.reset(): IDLE + alert reset (cycles=0) + calibrate.
void lb_reset();

// start_cycle(): last_cycle=epoch (persisted), SIFTING, start_delay, alert react.
void lb_start_cycle();

double get_last_cycle();    // from settings ("last_cycle", default 0)
void set_last_cycle(double epoch);
