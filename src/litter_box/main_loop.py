import asyncio
import time
from litter_box.state import get_state
from litter_box.settings import get_loop_sleep
import litter_box.detect
import litter_box.cycle
import litter_box.empty
import litter_box.neopixel_status


def run():
    litter_box.detect.cat()
    litter_box.cycle.do_cycle()
    litter_box.empty.do_empty()
    litter_box.neopixel_status.lumos()
    print(get_state())


async def loop():
    while True:
        run()
        await asyncio.sleep(get_loop_sleep())
