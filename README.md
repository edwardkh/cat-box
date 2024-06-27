## Changes

* neopixel support for status on the adafruit esp32-s2 feather
* sift delay changed to minutes, since the original 30 sec delay didn't give enough time for litter to clump.  Also added this to web UI
* using internal pullup for hall sensor pins. external pullup resistors no longer needed
* added attenuation for cat sensor ADC, since the voltage range is always around the 2.7v-3v range
* light modification on status and display names for more easier debugging

## Stuff I broke

* email currently not working for generic ESP32-S2 firmware due to missing library

## Hardware Used

* Adafruit ESP32-S2 Feather (https://www.microcenter.com/product/648549/adafruit-industries-esp32-s2-feather-with-bme280-sensor-stemma-qt-4mb-flash-2-mb-psram) - I was desperate for a same day solution, and bought the first avaialable ESP32 from the local microcenter.  Any MicroPython supported board will work, although you would have to mess around with the neopixel settings.
* L298N motor controller (used to drive motor and provide 5V voltage step down)(https://amzn.to/3UtnYqj)
* One 2.2K resistor for the CAT detector (https://amzn.to/4889Zt4)
* A Litter Robot 3 with a bad main board.

## Prepare the Adafruit ESP32-S2 Feather

Adafruit really wants you to use circuitpython, and I was not about to go porting the whole code over.  Go ahead and flash the generic ESP32-S2 micropython firmware (https://micropython.org/download/ESP32_GENERIC_S2/) and everything except for the email feature will work.  The generic firmware is missing one of the needed libaries.

Follow MrOstling's guide or MicroPython's getting started guide to get your board ready

## Wire it up

Slightly modified wiring diagram with corresponding wire colors from my LR3

![catbox - Page 2](https://github.com/edwardkh/cat-box/assets/1653747/d19fd6d1-b906-4889-b0a8-e811fa728c29)


