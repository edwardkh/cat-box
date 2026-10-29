# On-device bring-up checklist (not covered by CI — no hardware attached)

The offline gates are `pio run` (compile + link + size + no TLS in our objects)
and `pio test -e native` (34 host unit tests). Everything below needs the
board wired up. Items are ordered by how much they can bite you.

1. **Free heap after WiFi associates** — `heap_caps_get_free_size(MALLOC_CAP_INTERNAL)`
   and `heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL)`. Budget from
   research: ~60–75 KB free on the no-PSRAM S2 once WiFi + WebServer are up.
   Fragmentation over weeks of uptime is the thing to watch (checklist 9 too).
2. **loopTask stack high-water** — `uxTaskGetStackHighWaterMark(NULL)` after
   exercising every JSON endpoint and a `/reset` redirect. `ARDUINO_LOOP_STACK_SIZE`
   is raised to 8192 in `platformio.ini`; confirm the margin.
3. **NeoPixel soak ≥ 1 h** — 1-px `show()` every 250 ms while WiFi is
   associated and the web UI is polling. This is the Adafruit_NeoPixel #429
   crash/corruption class (reported on S3 with core ≥ 3.2, unverified on S2).
   Fallback if it misbehaves: swap `src/led_status.cpp` to NeoPixelBus
   `NeoEsp32Rmt0Ws2812xMethod` — the backend is isolated in that file.
4. **NeoPixel power settle** — GPIO21 high → first frame is 80 ms today
   (`NEOPIXEL_POWER_SETTLE_MS`). A glitchy first frame means raising it.
5. **ADC zero rate** — log 1 h at 4 Hz under web traffic and count discarded
   zero samples (D6 guard in `src/sensors.cpp` / `core/sensor_core.cpp`).
   Cross-check the trigger threshold against the old MicroPython build on the
   same board (both use the 16-bit scale, so 6000 should mean the same thing).
   Also sanity-check `raw % 4 == 0` on every sample: if always true, the ADC is
   reporting 12-bit shifted values and the silicon rev needs a different read.
   Long-term option: move the load cell to a GPIO1–10 (ADC1) pin — it is only a
   `load_sensor_pin` settings change.
6. **Hall double-latch / bounce at 4 Hz** — watch for a single magnet pass
   producing two transitions. If it happens, add a two-sample confirm inside
   `sensor_hall_triggered()`.
7. **WiFi policy** — kill the AP for 10 minutes: the 3-attempt budget should
   close, then the 60 s backoff window should reopen it, and it should
   reconnect when the AP returns. `catbox` should appear in the router's DHCP
   table (that is `wifi_hostname`).
8. **Web parity** — `PATCH /settings` with JSON from the browser and confirm
   the body arrives whole through `arg("plain")` (the settings page does this);
   `GET /` serves from PROGMEM; `GET /reset` redirects to `base_url`; and the
   control tick must not fall more than ~50 ms behind while a client is being
   served (timestamp the tick over serial while the UI polls).
9. **settings.json durability** — reboot normally, then yank power 20× during a
   save. The tmp+rename write in `src/settings_store.cpp` should leave either
   the old or the new file, never a truncated one.
10. **SNTP** — measure time-to-first-fix on your AP, and confirm no timed cycle
     fires before the clock is synced (D3/D8; `clock_synced()` in
     `src/core/behavior.cpp`).
11. **L298N thresholds** — the driver's VIH vs the S2's 3.3 V GPIOs, and set
     `rotate_direction_reversed` to match how you actually wired in1/in2.
12. **Timed cycle across a reboot** — with and without NTP reachable, verify
     the behaviour matches D3/D8 expectations (and revisit whether "skip while
     unsynced" is the product behaviour you want).
