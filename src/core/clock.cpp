#include "core/clock.h"
#include "config.h"

static CbTimeFn g_millis_fn = nullptr;
static CbTimeFn g_epoch_fn = nullptr;

void cb_set_time_sources(CbTimeFn millis_now, CbTimeFn epoch_now) {
    g_millis_fn = millis_now;
    g_epoch_fn = epoch_now;
}

uint64_t clock_extend_millis(uint32_t raw_millis) {
    static uint32_t s_prev = 0;
    static uint64_t s_laps = 0;
    if (raw_millis < s_prev) s_laps += (uint64_t)1 << 32;  // counter wrapped
    s_prev = raw_millis;
    return s_laps + raw_millis;
}

double cb_millis_now() { return g_millis_fn ? g_millis_fn() : 0.0; }

double cb_epoch_now() { return g_epoch_fn ? g_epoch_fn() : 0.0; }
