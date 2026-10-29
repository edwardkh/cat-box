// led_status — litter_box/neopixel_status.py parity on a 1-px WS2812.
// Deviation D4: blink is a non-blocking 250 ms state machine instead of
// Python's nested asyncio.run (same visual cadence, no loop stall).
#pragma once

#include "core/app_state.h"

void led_setup(void);
void led_tick(LbState state);  // call every 250 ms
