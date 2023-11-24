from litter_box import sense, state


def cat():
    if sense.cat_in_the_box():
        state.cat_detected()
    if state.current_state == state.PAUSED and state.ok_to_move():
        state.unpause()
