from wifi.connection import stay_connected
from litter_box import main_loop as litter_box
import server
import asyncio
from machine import Pin
from litter_box import neopixel_status

print("Let's start eating some shit!")
loop = asyncio.get_event_loop()
led = Pin(15, Pin.OUT)


async def main():
    litter_box_task = loop.create_task(litter_box.loop())
    server_task = loop.create_task(server.start())
    led_task = loop.create_task(neopixel_status.lumos())
    wifi_task = loop.create_task(stay_connected())
    await litter_box_task
    await server_task
    await led_task
    await wifi_task


loop.run_until_complete(main())

