#include "core/behavior.h"

#include "config.h"

static BehaviorDeps g;

void behavior_init(const BehaviorDeps* deps) { g = *deps; }

bool clock_synced() { return cb_epoch_now() >= CLOCK_SYNCED_EPOCH; }

// detect.py cat()
void do_detect() {
    if (lb_get_state() == LbState::IDLE && g.cat_in_the_box()) {
        cat_detected();
    }
    if (lb_get_state() == LbState::PAUSED && ok_to_move()) {
        lb_unpause();
    }
}

// cycle.py timed_cycle_tripped() with deviations D3+D8: while the wall clock
// is unsynced the check is skipped (Python fired a timed cycle immediately on
// every reboot because its 2000-era epoch < last_cycle).
static bool timed_cycle_tripped() {
    if (!clock_synced()) return false;
    double next = get_last_cycle() + g.timed_delay_hours() * 3600.0;
    return next < cb_epoch_now();
}

// cycle.py do_cycle() — if-chain order and per-state rotate-before-hall
// ordering preserved exactly (states fall through within one tick like Python).
void do_cycle() {
    if (lb_get_state() == LbState::IDLE && timed_cycle_tripped()) {
        lb_start_cycle();
    }
    if (lb_get_state() == LbState::IDLE) {
        g.motor_stop();
    }
    if (lb_get_state() == LbState::PAUSED) {
        g.motor_stop();
    }
    if (lb_get_state() == LbState::WAITING_TO_CYCLE && ok_to_move()) {
        lb_start_cycle();
    }
    if (lb_get_state() == LbState::SIFTING) {
        g.motor_ccw();
        if (g.hall_triggered() && delay_over(g.cycle_ignore())) {
            lb_set_state(LbState::EATING_SHIT);
            g.motor_stop();
            start_delay();
        }
    }
    if (lb_get_state() == LbState::EATING_SHIT && delay_over(g.cycle_eating())) {
        lb_set_state(LbState::MOVING_BACK);
    }
    if (lb_get_state() == LbState::MOVING_BACK) {
        g.motor_cw();
        if (g.hall_triggered()) {
            lb_set_state(LbState::LEVELING_LITTER);
            start_delay();
        }
    }
    if (lb_get_state() == LbState::LEVELING_LITTER) {
        g.motor_cw();
        if (delay_over(g.cycle_overshoot())) {
            g.motor_stop();
            lb_set_state(LbState::LEVELING_GLOBE);
            start_delay();
        }
    }
    // add a 3 second delay to buffer the clockwise rotation to counterclockwise rotation
    // as this was causing a power drip
    if (lb_get_state() == LbState::LEVELING_GLOBE && delay_over(3)) {
        g.motor_ccw();
        // there is a weak magnet at the level litter position, so we need an ignore delay here
        if (g.hall_triggered() && delay_over(g.cycle_ignore())) {
            g.motor_stop();
            lb_set_state(LbState::IDLE);
            g.calibrate();
        }
    }
}

// empty.py do_empty() — Python assigned state.current_state directly
// (equivalent to set_state; normalized per plan §4.5).
void do_empty() {
    if (lb_get_state() == LbState::EMPTYING) {
        g.motor_cw();
        if (g.hall_triggered()) {
            g.motor_stop();
            start_delay();
            lb_set_state(LbState::OPENING_THROAT);
        }
    }
    if (lb_get_state() == LbState::OPENING_THROAT) {
        g.motor_cw();
        if (delay_over(g.empty_overshoot())) {
            g.motor_stop();
            start_delay();
            lb_set_state(LbState::EATING_LITTER);
        }
    }
    if (lb_get_state() == LbState::EATING_LITTER && delay_over(g.empty_eating())) {
        lb_set_state(LbState::SWALLOWING);
    }
    if (lb_get_state() == LbState::SWALLOWING) {
        g.motor_ccw();
        if (g.hall_triggered()) {
            lb_set_state(LbState::RESETING);
        }
    }
    if (lb_get_state() == LbState::RESETING) {
        g.motor_ccw();
        if (g.hall_triggered()) {
            g.motor_stop();
            lb_set_state(LbState::IDLE);
        }
    }
}

// main_loop.run() body order (print parity lives device-side behind debug).
void behavior_tick() {
    do_detect();
    do_cycle();
    do_empty();
}
