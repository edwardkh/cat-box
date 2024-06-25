from litter_box import state
from litter_box.settings import get_neopixel_power_pin, get_neopixel_pin
from machine import Pin
from neopixel import NeoPixel
import asyncio

np = NeoPixel(Pin(get_neopixel_pin()), 1)

np_power = Pin(get_neopixel_power_pin(), Pin.OUT)
np_power.on()


async def dolight(color, blink = False, period_ms = 250):
    np[0] = color
    np.write()
    await asyncio.sleep_ms(period_ms)
    
    if blink == True:
        np[0] = (0,0,0)
        np.write()
    await asyncio.sleep_ms(period_ms)

async def lumos():
    while True:

        # idle = green
        if state.get_state() == state.IDLE:
            asyncio.run(dolight((0,255,0)))

        # cat entered / waiting to cycle = yellow
        elif state.get_state() == state.WAITING_TO_CYCLE:
            asyncio.run(dolight((255,255,0)))

        # paused = red
        elif state.get_state() == state.PAUSED:
            asyncio.run(dolight((255,0,0)))

        # rotating = blinking yellow
        elif state.get_state() == state.SIFTING or state.get_state() == state.EATING_SHIT or state.get_state() == state.MOVING_BACK or state.get_state() == state.LEVELING_LITTER or state.get_state() == state.LEVELING_GLOBE:
            asyncio.run(dolight((255,255,0), True))

        # empty = blinking purple
        elif state.get_state() == state.EMPTYING or state.get_state() == state.OPENING_THROAT or state.get_state() == state.EATING_LITTER or state.get_state() == state.SWALLOWING or state.get_state() == state.RESETTING:
            asyncio.run(dolight((255,0,255), True))
        else:
            await asyncio.sleep_ms(250)
        
