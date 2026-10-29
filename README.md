Big shout out to MrOstling for his original work.

Cat-Box firmware for a Litter Robot 3 with a dead main board: an ESP32-S2 drives
the motor through an L298N, watches two hall sensors for the rotation limits,
detects a cat on a load cell, blinks a status NeoPixel, and serves a small web
UI on the LAN.

**The firmware is now a C++ (Arduino framework) build managed by
[PlatformIO](https://platformio.org).** The original MicroPython application is
kept unmodified under `reference/python-app/` as the behavioral spec — the C++
code is a port of it (see [Deviations](#deviations-from-the-python-original)).

## Changes

* neopixel support for status on the adafruit esp32-s2 feather
* sift delay changed to minutes, since the original 30 sec delay didn't give enough time for litter to clump.  Also added this to web UI
* using internal pullup for hall sensor pins. external pullup resistors no longer needed
* added attenuation for cat sensor ADC, since the voltage range is always around the 2.7v-3v range
* light modification on status and display names for more easier debugging
* **C++ port under PlatformIO** — `pio run` builds the firmware, no MicroPython
  runtime on the device, and no TLS stack (email is a log-only stub, see below)

Sweet neopixel action

![image](https://github.com/edwardkh/cat-box/assets/1653747/d14fec79-569d-451b-96ba-f9751aeedffb)


## Stuff I broke

* email alerting is **not implemented in the C++ firmware**: when the bin-full
  cycle count is exceeded the message is printed to the serial console instead
  of being sent (the web UI `/reset` flow is unaffected). The MicroPython
  original could send it, but only with a TLS-capable firmware build.


## Hardware Used

* Adafruit ESP32-S2 Feather (https://www.microcenter.com/product/648549/adafruit-industries-esp32-s2-feather-with-bme280-sensor-stemma-qt-4mb-flash-2-mb-psram) - I was desperate for a same day solution, and bought the first avaialable ESP32 from the local microcenter.  Any ESP32-S2 board will work, although you would have to mess around with the neopixel settings.
* L298N motor controller (used to drive motor and provide 5V voltage step down)(https://amzn.to/3UtnYqj)
* One 2.2K resistor for the CAT detector (https://amzn.to/4889Zt4)
* A Litter Robot 3 with a bad main board.

## Wire it up

Slightly modified wiring diagram with corresponding wire colors from my LR3

![catbox - Page 2](https://github.com/edwardkh/cat-box/assets/1653747/d19fd6d1-b906-4889-b0a8-e811fa728c29)

| Signal | GPIO (default) |
| --- | --- |
| L298N in1 ("backward") | 4 |
| L298N in2 ("forward") | 3 |
| Hall sensor 1 (internal pull-up) | 11 |
| Hall sensor 2 (internal pull-up) | 12 |
| Load cell / cat sensor ADC (11 dB) | 18 |
| NeoPixel data | 33 |
| NeoPixel power | 21 |

All of them are settings keys, so they can be moved without reflashing (they
are read at boot, so a change applies after a reboot).


## Building and flashing (PlatformIO)

```bash
pip install platformio            # or your distro package
pio run                                    # compile (≈15 s after the one-time platform download)
pio run -t upload --upload-port /dev/ttyACM0   # flash over the USB-C UART boot ROM
pio device monitor --port /dev/ttyACM0 --baud 115200
```

* `pio run` also registers the host test environment: `pio test -e native`
  runs the 34 pure-logic unit tests (state machine, hall latch, load filter,
  settings merge, clock rollover) on your machine — no hardware needed.
* The board is `featheresp32-s2` with `platform = espressif32@8.0.0`
  (Arduino core 3.3.12 / ESP-IDF 5.5), pinned in `platformio.ini`.
* `partitions.csv` replaces the board default: one 3.75 MB app partition, no
  OTA, plus a 256 KB LittleFS `storage` partition for `settings.json`. The
  0x0–0x50000 bootloader region is left alone; if the Feather's TinyUF2
  bootloader ever has to come back, hold BOOT+RESET and use the ROM bootloader,
  or delete the `board_build.partitions` line to go back to the board default.
* Web assets (`frontend/main.html`, `frontend/main.js`) are compiled into the
  binary. After editing them, regenerate the header:
  `python3 scripts/embed_assets.py`.

### First boot / configuration

The device reads `/settings.json` from LittleFS. **It boots fine with no file
at all** (defaults + offline web UI on `http://<ip>`). Unlike the Python
version — whose `boot.py` HUNG forever whenever WiFi was missing or
credentials were wrong, leaving the device unreachable until reflash — this
firmware never blocks on WiFi: it sits retrying in the background
(deviation D2).

To join a network, prepare the LittleFS payload and flash it (this erases the
`storage` partition, so it wipes `settings.json` too):

```bash
mkdir -p data
cp reference/settings.example.json data/settings.json   # then edit the SSID/password
pio run -t uploadfs --upload-port /dev/ttyACM0
```

Alternatively — and this is how the web UI does it — the settings page can
`PATCH /settings` the same keys (WiFi credentials included); they persist
immediately. Reboot afterwards, because pins and the DHCP hostname are read
once at boot.

### Settings reference

`/settings.json` is a flat JSON object. Every key is optional; the built-in
default is used when the key is absent, and unknown keys are preserved through
the web UI's PATCH merge.

| Key | Default | Meaning |
| --- | --- | --- |
| `cycle_start_ignore_hall_sensor_time` | 3 | seconds of hall-ignore after starting a rotation (weak magnet) |
| `cycle_wait_time` | 7 | minutes a cat detection blocks motion (`ok_to_move`) |
| `cycle_eating_time` | 7 | seconds the drum stops to "eat" the clump |
| `cycle_overshoot_time` | 9 | seconds of leveling overshoot |
| `empty_overshoot_time` | 5 | seconds of throat-opening overshoot |
| `empty_eating_time` | 7 | seconds of eat during empty |
| `hall_pin1` / `hall_pin2` | 11 / 12 | hall sensor GPIOs (input pull-up) |
| `load_sensor_pin` | 18 | load cell ADC GPIO |
| `load_sensor_threshold` | 6000 | trigger delta below the calibrated empty load |
| `l298n_in1` / `l298n_in2` | 4 / 3 | motor driver GPIOs |
| `rotate_direction_reversed` | false | XOR'd into both driver pins |
| `loop_sleep` | 0.25 | control tick period in seconds (floored at 20 ms) |
| `timed_cycle_delay_hours` | 24 | hours after the last cycle before an automatic one |
| `neopixel_power` | 21 | NeoPixel power GPIO (the Python key name is kept) |
| `neopixel_pin` | 33 | NeoPixel data GPIO |
| `bin_full_cycles` | 20 | cycles before the bin-full alert fires |
| `alert_bin_full_cycles` | (falls back to `bin_full_cycles`) | explicit alert-threshold override |
| `wifi_connection_retries` | 3 | connection attempts before backing off |
| `wifi_connection_backoff` | 60 | seconds before the attempt counter resets |
| `wifi_ssid` / `wifi_password` | — | credentials, nothing else works without them |
| `wifi_hostname` | `catbox` | DHCP hostname |
| `base_url` | null | used for the `/reset` redirect and the alert text |
| `wifi_ip_config` | — | read/kept for compatibility, never applied (it was commented out in the original) |
| `smtp_server`, `smtp_port`, `smtp_email`, `smtp_password`, `bin_full_recipients` | gmail defaults | **inert** (email is a stub) |
| `last_cycle` | 0 | epoch of the last cycle (written by the firmware) |
| `cycles` | 0 | cycles since the last reset (written by the firmware) |

### HTTP API (unchanged from the Python version)

`GET /`, `GET /main.js`, `GET /current-state`, `POST /cycle`, `POST /empty`,
`POST /pause`, `POST /unpause`, `GET|POST /reset`, `GET|PATCH /settings`.
Same state strings, same JSON shapes, same `302 → base_url` on `GET /reset`.


## Deviations from the Python original

Deliberate, and each one is called out in the code:

| # | Deviation | Why |
| --- | --- | --- |
| D1 | bin-full email is a serial-log stub; no TLS code at all | out of scope; keeps ~40 KB of TLS heap and the whole mbedTLS session layer out of the image |
| D2 | boot does not block when WiFi is missing/credentials absent | `boot.py` busy-waited forever, so a device with a bad password was unreachable until reflash |
| D3 | SNTP (`pool.ntp.org`) on first connect; the timed cycle waits for a synced clock | the Python app had no time source: its 1970/2000-era clock made *every* reboot look like "time for a cycle" |
| D4 | NeoPixel blink is a non-blocking 250 ms state machine | Python nested `asyncio.run()` inside the running loop, which stalled that task each iteration |
| D5 | dwell timers use monotonic `millis()` | SNTP steps the wall clock; dwell durations are unchanged |
| D6 | spurious-zero ADC guard (up to 3 reads 5 ms apart, median-of-3, zero discarded) | GPIO18 is ADC2_CH7 on the S2, shared with WiFi; the Arduino HAL can return 0 during a radio burst |
| D7 | `loop_sleep` floored at 20 ms | `loop_sleep: 0` from the UI would otherwise spin the loop |
| D8 | no timed cycles while the clock is unsynced | policy call; remove the `clock_synced()` gate in `src/core/behavior.cpp` to restore the old behaviour |


## Known issues

* The frontend loads Materialize/axios from CDNs, so the UI chrome needs
  internet the first time you open it (unchanged from the original).
* Anything requiring hardware is not verified by CI: see
  `docs/on-device-checklist.md` for the bring-up measurements to run once a
  board is attached (heap, NeoPixel soak under WiFi, ADC zero rate, WiFi
  retry/recovery, power-loss durability of `settings.json`).
