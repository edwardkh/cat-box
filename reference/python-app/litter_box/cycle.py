import time

from litter_box import state, rotate, sense
from litter_box.settings import get_cycle_eating_time, get_cycle_overshoot_time, get_cycle_start_ignore_hall_sensor_time, get_timed_cycle_delay_hours


def timed_cycle_tripped():
    next_timed_cycle_time = state.get_last_cycle() + get_timed_cycle_delay_hours() * 3600
    return next_timed_cycle_time < time.time()


def do_cycle():
    if state.get_state() == state.IDLE and timed_cycle_tripped():
        state.start_cycle()
    if state.get_state() == state.IDLE:
        rotate.stop()
    if state.get_state() == state.PAUSED:
        rotate.stop()
    if state.get_state() == state.WAITING_TO_CYCLE and state.ok_to_move():
        state.start_cycle()
    if state.get_state() == state.SIFTING:
        rotate.counter_clock_wise()
        if sense.hall_sensor_triggered() and state.delay_over(get_cycle_start_ignore_hall_sensor_time()):
            state.set_state(state.EATING_SHIT)
            rotate.stop()
            state.start_delay()
    if state.get_state() == state.EATING_SHIT and state.delay_over(get_cycle_eating_time()):
        state.set_state(state.MOVING_BACK)
    if state.get_state() == state.MOVING_BACK:
        rotate.clock_wise()
        if sense.hall_sensor_triggered():
            state.set_state(state.LEVELING_LITTER)
            state.start_delay()
    if state.get_state() == state.LEVELING_LITTER:
        rotate.clock_wise()
        if state.delay_over(get_cycle_overshoot_time()):
            rotate.stop()
            state.set_state(state.LEVELING_GLOBE)
            state.start_delay()
    # add a 3 second delay to buffer the clockwise rotation to counterclockwise rotation
    # as this was causing a power drip
    if state.get_state() == state.LEVELING_GLOBE and state.delay_over(3):
        rotate.counter_clock_wise()

        # there is a weak magnet at the level litter position, so we need an ignore delay here
        if sense.hall_sensor_triggered() and state.delay_over(get_cycle_start_ignore_hall_sensor_time()): 
            rotate.stop()
            state.set_state(state.IDLE)
            sense.calibrate_load()

