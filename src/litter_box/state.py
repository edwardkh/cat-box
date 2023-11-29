from micropython import const
import time
from litter_box.settings import get_cycle_wait_time
import alert.bin_full as bin_full
from litter_box.sense import calibrate_load

IDLE = const('Waiting for shit')

WAITING_TO_CYCLE = const("Shit detected waiting to cycle")
SIFTING = const("Sifting shit")
EATING_SHIT = const("Eating shit")
MOVING_BACK = const("Moving back")
LEVELING_LITTER = const("Leveling litter")
LEVELING_GLOBE = const("Leveling the globe")

EMPTYING = const("Dumping this mess")
OPENING_THROAT = const("Opening Throat")
EATING_LITTER = const("Eating litter")
SWALLOWING = const("Swallowing")
RESETTING = const("Resetting")

PAUSED = const("Paused")

current_state = IDLE
resume_state = IDLE
last_detection = 0
delay_start = 0
connected = False


def set_state(state):
    global current_state
    current_state = state


def get_state():
    global current_state
    return current_state


def cat_detected():
    global last_detection, current_state
    print("Cat detected")
    last_detection = time.time()
    if current_state == IDLE:
        current_state = WAITING_TO_CYCLE
    else:
        pause()


def pause():
    global current_state, resume_state
    if current_state != PAUSED:
        resume_state = current_state
        current_state = PAUSED


def unpause():
    global current_state, resume_state
    current_state = resume_state


def ok_to_move():
    global last_detection
    return last_detection + get_cycle_wait_time() < time.time()


def start_delay():
    global delay_start
    delay_start = time.time()


def delay_over(dwell_time):
    global delay_start
    return delay_start + dwell_time < time.time()


def reset():
    global current_state
    current_state = IDLE
    bin_full.reset()
    calibrate_load()


def start_cycle():
    set_state(SIFTING)
    start_delay()
    bin_full.react_to_cycle()
