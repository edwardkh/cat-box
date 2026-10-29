#include "core/app_state.h"

#include "core/clock.h"
#include "settings.h"

static LbState g_state = LbState::IDLE;
static LbState g_resume_state = LbState::IDLE;
// Python state.py initializes last_detection = 0 — a WALL epoch (1970), so
// ok_to_move() is TRUE at boot and until cycle_wait_time after the first
// cat_detected(). On the monotonic clock (D5) the boot value must therefore
// be an "epoch 0" far behind uptime, not 0 ms: seeding the full window base
// keeps ok_to_move() true from t=0 through the same post-boot window Python
// had (detect.py can auto-unpause then, exactly as before).
#define MONOTONIC_EPOCH_BASE_MS 86400000.0  // 24 h behind t=0 (>> any cycle_wait)
static double g_last_detection_ms = -MONOTONIC_EPOCH_BASE_MS;

static double g_delay_start_ms = 0;  // monotonic ms (D5)

void app_state_init_boot_time(void) {
    // Python module-import semantics (state.py lines 26-28): resume_state =
    // IDLE, delay_start = 0, last_detection = wall epoch 0 (=> the
    // far-behind monotonic equivalent; see MONOTONIC_EPOCH_BASE_MS).
    g_resume_state = LbState::IDLE;
    g_last_detection_ms = -MONOTONIC_EPOCH_BASE_MS;
    g_delay_start_ms = 0;
}

static CbVoidVoid g_calibrate = nullptr;
static CbVoidVoid g_alert_reset = nullptr;
static CbVoidVoid g_alert_react = nullptr;

void app_state_init(CbVoidVoid calibrate,
                    CbVoidVoid alert_reset,
                    CbVoidVoid alert_react) {
    g_calibrate = calibrate;
    g_alert_reset = alert_reset;
    g_alert_react = alert_react;
}

const char* state_str(LbState s) {
    switch (s) {
        case LbState::IDLE: return "Idle";
        case LbState::WAITING_TO_CYCLE: return "Waiting to cycle";
        case LbState::SIFTING: return "Sifting";
        case LbState::EATING_SHIT: return "Eating shit";
        case LbState::MOVING_BACK: return "Moving back";
        case LbState::LEVELING_LITTER: return "Leveling litter";
        case LbState::LEVELING_GLOBE: return "Leveling the globe";
        case LbState::EMPTYING: return "Dumping this mess";
        case LbState::OPENING_THROAT: return "Opening Throat";
        case LbState::EATING_LITTER: return "Eating litter";
        case LbState::SWALLOWING: return "Swallowing";
        case LbState::RESETING: return "Resetting";
        case LbState::PAUSED: return "Paused";
    }
    return "Idle";
}

LbState lb_get_state() { return g_state; }

void lb_set_state(LbState s) { g_state = s; }

double get_last_cycle() {
    // persistent_state.get_state_of("last_cycle", 0)
    return settings_get_f(SETTING_KEY_LAST_CYCLE, 0);
}

void set_last_cycle(double epoch) {
    // persistent_state.set_state_of("last_cycle", value) + immediate save
    settings_set_f_and_save(SETTING_KEY_LAST_CYCLE, epoch);
}

void cat_detected() {
    g_last_detection_ms = cb_millis_now();  // Python: last_detection = time.time()
    if (g_state == LbState::IDLE || g_state == LbState::WAITING_TO_CYCLE) {
        g_state = LbState::WAITING_TO_CYCLE;
    } else {
        lb_pause();
    }
}

void lb_pause() {
    if (g_state != LbState::PAUSED) {
        g_resume_state = g_state;
        g_state = LbState::PAUSED;
    }
}

void lb_unpause() { g_state = g_resume_state; }

bool ok_to_move() {
    // last_detection + cycle_wait_time*60 < now (monotonic ms both sides, D5;
    // the boot seed reproduces Python's epoch-0 "true at boot" behaviour)
    return g_last_detection_ms +
               settings_get_f(SETTING_KEY_CYCLE_WAIT_TIME, DEFAULT_CYCLE_WAIT_TIME) * 60000.0
           < cb_millis_now();
}

void start_delay() { g_delay_start_ms = cb_millis_now(); }

bool delay_over(double dwell_secs) {
    return g_delay_start_ms + dwell_secs * 1000.0 < cb_millis_now();
}

void lb_reset() {
    g_state = LbState::IDLE;
    if (g_alert_reset) g_alert_reset();  // bin_full.reset(): cycles = 0
    if (g_calibrate) g_calibrate();
}

void lb_start_cycle() {
    // Python: persistent_state.set_state_of("last_cycle", time.time())
    set_last_cycle(cb_epoch_now());
    lb_set_state(LbState::SIFTING);
    start_delay();
    if (g_alert_react) g_alert_react();  // bin_full.react_to_cycle()
}
