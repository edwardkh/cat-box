from litter_box import rotate, sense, state
from litter_box.settings import get_empty_eating_time, get_empty_overshoot_time
from litter_box.state import EMPTYING, SWALLOWING, start_delay, OPENING_THROAT, EATING_LITTER, IDLE, RESETTING, delay_over


def do_empty():
    if state.get_state() == EMPTYING:
        rotate.clock_wise()
        if sense.hall_sensor_triggered():
            rotate.stop()
            start_delay()
            state.current_state = OPENING_THROAT
    if state.get_state() == OPENING_THROAT:
        rotate.clock_wise()
        if delay_over(get_empty_overshoot_time()):
            rotate.stop()
            start_delay()
            state.current_state = EATING_LITTER
    if state.get_state() == EATING_LITTER and delay_over(get_empty_eating_time()):
        state.current_state = SWALLOWING
    if state.get_state() == SWALLOWING:
        rotate.counter_clock_wise()
        if sense.hall_sensor_triggered():
            state.current_state = RESETTING
    if state.get_state() == RESETTING:
        rotate.counter_clock_wise()
        if sense.hall_sensor_triggered():
            rotate.stop()
            state.current_state = IDLE
