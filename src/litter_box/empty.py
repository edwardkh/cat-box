from litter_box import rotate, sense, state
from litter_box.settings import empty_eating_time
from litter_box.state import EMPTYING, start_delay, EATING_LITTER, IDLE, RESETTING, delay_over


def do_empty():
    if state.get_state() == EMPTYING:
        rotate.clock_wise()
        if sense.hall_sensor_triggered():
            rotate.stop()
            start_delay()
            state.current_state = EATING_LITTER
    if state.get_state() == EATING_LITTER and delay_over(empty_eating_time):
        state.current_state = RESETTING
    if state.get_state() == RESETTING:
        rotate.counter_clock_wise()
        if sense.hall_sensor_triggered():
            rotate.stop()
            state.current_state = IDLE
