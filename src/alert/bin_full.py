from alert.message import send
from persistent_state import state
from wifi.connection import get_base_url
from alert.settings import get_bin_full_cycles, get_bin_full_recipients


def react_to_cycle():
    cycles = state.increment_state_of("cycles")
    if cycles > state.get_state_of("alert_bin_full_cycles", get_bin_full_cycles()):
        bace_url = get_base_url()
        if bace_url is not None:
            message = "Empty the cat box. It has cycled " + str(cycles) + " times. Reset with " + bace_url + "/reset"
            print(message)
            send(get_bin_full_recipients(), "Empty The Cat Box", message)


def reset():
    state.set_state_of("cycles", 0)
