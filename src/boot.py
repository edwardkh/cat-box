from litter_box import state
import wifi.connection
import gc

print("I'm here to eat shit!")
gc.enable()
wifi.connection.init()
state.set_state(state.SIFTING)
