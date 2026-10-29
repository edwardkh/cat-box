#include "led_status.h"

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "config.h"
#include "settings.h"

// NeoPixel backend is isolated here on purpose: if the on-device soak
// (checklist #3, Adafruit_NeoPixel#429 class on core 3.2+) shows RMT/interrupt
// corruption, swap this file to NeoPixelBus NeoEsp32Rmt0Ws2812xMethod without
// touching anything else.
static Adafruit_NeoPixel g_px;  // 1 pixel, NEO_GRB + NEO_KHZ800
static bool g_blink_phase = false;
static uint32_t g_shown = 0xFFFFFFFF;

static bool is_yellow_blink(LbState s) {
    return s == LbState::SIFTING || s == LbState::EATING_SHIT || s == LbState::MOVING_BACK ||
           s == LbState::LEVELING_LITTER || s == LbState::LEVELING_GLOBE;
}

static bool is_purple_blink(LbState s) {
    return s == LbState::EMPTYING || s == LbState::OPENING_THROAT || s == LbState::EATING_LITTER ||
           s == LbState::SWALLOWING || s == LbState::RESETING;
}

void led_setup(void) {
    int power_pin = (int)settings_get_i(SETTING_KEY_NEOPIXEL_POWER, DEFAULT_NEOPIXEL_POWER);
    int data_pin = (int)settings_get_i(SETTING_KEY_NEOPIXEL_PIN, DEFAULT_NEOPIXEL_PIN);
    pinMode(power_pin, OUTPUT);
    digitalWrite(power_pin, HIGH);
    delay(NEOPIXEL_POWER_SETTLE_MS);  // power settle before the first frame
    g_px = Adafruit_NeoPixel(1, data_pin, NEO_GRB + NEO_KHZ800);
    g_px.begin();
    g_px.clear();
    g_px.show();
}

void led_tick(LbState s) {
    uint32_t color;
    bool blink;
    switch (s) {
        case LbState::IDLE:
            color = g_px.Color(0, 128, 0);  // green
            blink = false;
            break;
        case LbState::WAITING_TO_CYCLE:
            color = g_px.Color(128, 128, 0);  // yellow
            blink = false;
            break;
        case LbState::PAUSED:
            color = g_px.Color(128, 0, 0);  // red
            blink = false;
            break;
        default:
            if (is_yellow_blink(s)) {
                color = g_px.Color(128, 128, 0);
                blink = true;
            } else if (is_purple_blink(s)) {
                color = g_px.Color(128, 0, 128);
                blink = true;
            } else {
                color = g_px.Color(0, 0, 0);
                blink = false;
            }
            break;
    }
    g_blink_phase = !g_blink_phase;
    uint32_t px = (blink && g_blink_phase) ? g_px.Color(0, 0, 0) : color;
    if (px != g_shown) {  // show() only on change or blink-tick
        g_px.setPixelColor(0, px);
        g_px.show();
        g_shown = px;
    }
}
