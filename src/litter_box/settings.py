from persistent_state.state import get_state_of

cycle_start_ignore_hall_sensor_time = 5
cycle_wait_time = 30
cycle_eating_time = 7
cycle_overshoot_time = 7
empty_overshoot_time = 5
empty_eating_time = 7
hall_pin1 = 37
hall_pin2 = 39
load_sensor_pin = 16
load_sensor_threshold = 3000
l298n_ena = 18
l298n_in1 = 33
l298n_in2 = 35
loop_sleep = 0.1
rotate_direction_reversed = False


def get_cycle_start_ignore_hall_sensor_time():
    return get_state_of("cycle_start_ignore_hall_sensor_time", cycle_start_ignore_hall_sensor_time)


def get_cycle_wait_time():
    return get_state_of("cycle_wait_time", cycle_wait_time)


def get_cycle_eating_time():
    return get_state_of("cycle_eating_time", cycle_eating_time)


def get_cycle_overshoot_time():
    return get_state_of("cycle_overshoot_time", cycle_overshoot_time)


def get_empty_overshoot_time():
    return get_state_of("empty_overshoot_time", empty_overshoot_time)


def get_empty_eating_time():
    return get_state_of("empty_eating_time", empty_eating_time)


def get_hall_pin1():
    return get_state_of("hall_pin1", hall_pin1)


def get_hall_pin2():
    return get_state_of("hall_pin2", hall_pin2)


def get_load_sensor_pin():
    return get_state_of("load_sensor_pin", load_sensor_pin)


def get_load_sensor_threshold():
    return get_state_of("load_sensor_threshold", load_sensor_threshold)


def get_l298n_ena():
    return get_state_of("l298n_ena", l298n_ena)


def get_l298n_in1():
    return get_state_of("l298n_in1", l298n_in1)


def get_l298n_in2():
    return get_state_of("l298n_in2", l298n_in2)


def get_loop_sleep():
    return get_state_of("loop_sleep", loop_sleep)


def get_rotate_direction_reversed():
    return get_state_of("rotate_direction_reversed", rotate_direction_reversed)
