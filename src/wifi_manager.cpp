#include "wifi_manager.h"

#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "settings.h"

// Core auto-reconnect is switched off: the Python app owned its retry policy
// (3 attempts, then a 60 s backoff window) and stay_connected() polls at a
// fixed cadence — replicated here event-driven.
static volatile bool g_wifi_up = false;
static volatile bool g_got_ip_edge = false;  // handled in wifi_tick (loop task)
static int g_attempts = 0;
static unsigned long g_last_attempt_ms = 0;  // monotonic analogue of
                                             // last_connection_attempt
static bool g_sntp_started = false;
static char g_url[64];

static void on_wifi_event(WiFiEvent_t event) {
    // Runs on the ESP-IDF WiFi driver task (priority 23): ONLY volatile flag
    // writes here. Serial, WiFi accessors and configTime() belong in the
    // loop-task edge handling below.
    if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
        g_wifi_up = true;
        // NOTE: g_attempts is deliberately NOT reset here — connection.py
        // resets the attempt counter only via the backoff window in
        // ready_to_connect().
        g_got_ip_edge = true;
    } else if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
        g_wifi_up = false;
    }
}

void wifi_setup(void) {
    char ssid[64], pass[64], host[32];
    settings_get_s(SETTING_KEY_WIFI_SSID, "", ssid, sizeof(ssid));
    settings_get_s(SETTING_KEY_WIFI_PASSWORD, "", pass, sizeof(pass));
    settings_get_s(SETTING_KEY_WIFI_HOSTNAME, DEFAULT_WIFI_HOSTNAME, host, sizeof(host));
    if (!ssid[0] || !pass[0]) return;  // configured() gate

    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(host);  // wlan.config(dhcp_hostname=...)
    // wifi_ip_config stays unread: the static-IP branch was commented out in
    // Python, so applying it would be a behavior change.
    WiFi.setAutoReconnect(false);
    WiFi.onEvent(on_wifi_event);
    // Deviation D2: begin() is non-blocking; the keepalive tick reports state.
    g_last_attempt_ms = millis();
    g_attempts = 1;
    WiFi.begin(ssid, pass);
}

// connection.py ready_to_connect(): attempts reset after the backoff window;
// connect() then allows attempts <= retries.
static bool ready_to_connect(void) {
    unsigned long backoff =
        (unsigned long)(settings_get_f(SETTING_KEY_WIFI_BACKOFF, DEFAULT_WIFI_BACKOFF) * 1000.0);
    if (millis() - g_last_attempt_ms > backoff) g_attempts = 0;
    return g_attempts <= (int)settings_get_i(SETTING_KEY_WIFI_RETRIES, DEFAULT_WIFI_RETRIES);
}

void wifi_tick(void) {
    if (g_got_ip_edge) {  // loop-task half of the GOT_IP event (see on_wifi_event)
        g_got_ip_edge = false;
        Serial.print("Successfully connected to ");
        Serial.println(WiFi.SSID());
        Serial.println(WiFi.localIP());
        if (!g_sntp_started) {  // deviation D3
            g_sntp_started = true;
            configTime(0, 0, "pool.ntp.org");
        }
    }
    char ssid[64], pass[64];
    settings_get_s(SETTING_KEY_WIFI_SSID, "", ssid, sizeof(ssid));
    settings_get_s(SETTING_KEY_WIFI_PASSWORD, "", pass, sizeof(pass));
    if (!ssid[0] || !pass[0]) return;  // not configured
    if (g_wifi_up || WiFi.status() == WL_CONNECTED) return;
    if (!ready_to_connect()) return;
    g_last_attempt_ms = millis();
    g_attempts++;
    Serial.println("connecting to network...");
    WiFi.disconnect(false);  // Python called wlan.connect() directly; core
                             // requires a fresh begin() to restart an attempt
    WiFi.begin(ssid, pass);
}

bool wifi_is_up(void) { return g_wifi_up || WiFi.status() == WL_CONNECTED; }

const char* wifi_base_url(void) {
    char configured[64];
    settings_get_s(SETTING_KEY_BASE_URL, "", configured, sizeof(configured));
    if (configured[0]) {
        strncpy(g_url, configured, sizeof(g_url) - 1);
        g_url[sizeof(g_url) - 1] = '\0';
        return g_url;
    }
    if (!wifi_is_up()) {
        g_url[0] = '\0';  // Python get_ip() -> None -> alert suppressed
        return g_url;
    }
    snprintf(g_url, sizeof(g_url), "http://%s", WiFi.localIP().toString().c_str());
    return g_url;
}
