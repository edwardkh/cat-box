from alert.message import send
from persistent_state import state
from alert.settings import bin_full_cycles
from alert.secrets import bin_full_recipients
from wifi.connection import get_base_url


def react_to_cycle():
    cycles = state.increment_state_of("cycles")
    if cycles > bin_full_cycles:
        bace_url = get_base_url()
        if bace_url is not None:
            message = "Empty the cat box. It has cycled " + str(cycles) + " times. Reset with " + bace_url + "/reset"
            print(message)
            send(bin_full_recipients, "Empty The Cat Box", message)


def reset():
    state.set_state_of("cycles", 0)
