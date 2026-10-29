// clock — injectable time source so core logic is deterministic under test.
// Device build installs the real implementations in main.cpp (deviation D5:
// dwell/ok_to_move timers use monotonic millis(); wall clock is only used for
// last_cycle timestamps and the timed-cycle check).
#pragma once
#include <stdint.h>

typedef double (*CbTimeFn)();

void cb_set_time_sources(CbTimeFn millis_now, CbTimeFn epoch_now);

// Device adapter helper: Arduino's millis() is a uint32_t that wraps every
// ~49.7 days. Dwell timers compare against a stored start value, so a wrap
// would freeze a state machine mid-dwell until reboot. This extends the
// wrapping counter into a monotonically increasing 64-bit count; feed it the
// raw millis() value and use the result as the monotonic time source.
uint64_t clock_extend_millis(uint32_t raw_millis);
double cb_millis_now();   // monotonic milliseconds
double cb_epoch_now();    // wall clock seconds; < CLOCK_SYNCED_EPOCH until SNTP synced
