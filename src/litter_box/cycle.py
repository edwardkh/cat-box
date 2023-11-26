from litter_box import state, rotate, sense
from litter_box.settings import get_cycle_eating_time, get_cycle_overshoot_time


def do_cycle():
    if state.get_state() == state.PAUSED:
        rotate.stop()
    if state.get_state() == state.WAITING_TO_CYCLE and state.ok_to_move():
        state.start_cycle()
    if state.get_state() == state.SIFTING:
        rotate.counter_clock_wise()
        if sense.hall_sensor_triggered():
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
            state.set_state(state.LEVELING_GLOBE)
    if state.get_state() == state.LEVELING_GLOBE:
        rotate.counter_clock_wise()
        if sense.hall_sensor_triggered():
            rotate.stop()
            state.set_state(state.IDLE)
            sense.calibrate_load()
