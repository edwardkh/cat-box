from litter_box import state
from litter_box.settings import get_neopixel_power_pin, get_neopixel_pin
from machine import Pin
from neopixel import NeoPixel
import asyncio

np = NeoPixel(Pin(get_neopixel_pin()), 1)

np_power = Pin(get_neopixel_power_pin(), Pin.OUT)
np_power.on()

blink_task = 0

def blink(period_ms):
    global np
    cached_np = np

    try:
        while True:
            np[0] = (0,0,0)
            np.write()

            np = cached_np
            np.write()
    finally:
        np[0] = (0,128,0)
        np.write()

def lumos():
    global blink_task

    # idle = green
    if state.get_state() == state.IDLE:
        # if blink_task != 0:
        #     blink_task.cancel()
        #     blink_task = 0

        np[0] = (0,128,0)

    # cat entered / waiting to cycle = yellow
    elif state.get_state() == state.WAITING_TO_CYCLE:
        np[0] = (128,128,0)

    # paused = red
    elif state.get_state() == state.PAUSED:
        np[0] = (128,0,0)

    # rotating = blue
    #elif (state.get_state() == state.SIFTING or state.get_state() == state.EATING_SHIT or state.get_state() == state.MOVING_BACK or state.get_state() == state.LEVELING_LITTER or state.get_state() == state.LEVELING_GLOBE) and blink_task == 0:
    elif state.get_state() == state.SIFTING or state.get_state() == state.EATING_SHIT or state.get_state() == state.MOVING_BACK or state.get_state() == state.LEVELING_LITTER or state.get_state() == state.LEVELING_GLOBE == 0:
        np[0] = (0,0,128)
        #blink_task = asyncio.create_task(blink(250))
    # purple
    elif state.get_state() == state.EMPTYING or state.get_state() == state.OPENING_THROAT or state.get_state() == state.EATING_LITTER or state.get_state() == state.SWALLOWING or state.get_state() == state.RESETTING:
        np[0] = (128,0,128)
    
 
    np.write()