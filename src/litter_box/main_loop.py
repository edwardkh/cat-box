import asyncio
import time
from litter_box.state import current_state
from litter_box.settings import loop_sleep
import litter_box.detect
import litter_box.cycle
import litter_box.empty

last_loop = 0


def run():
    global last_loop
    if last_loop + loop_sleep < time.time():
        last_loop = time.time()
        litter_box.detect.cat()
        litter_box.cycle.do_cycle()
        litter_box.empty.do_empty()
        print(current_state)


async def loop():
    while True:
        run()
        await asyncio.sleep(0.1)
