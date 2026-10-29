#include "web_server.h"

#include <Arduino.h>
#include <WebServer.h>

#include "alert_stub.h"
#include "assets/web_assets.h"
#include "core/app_state.h"
#include "settings.h"
#include "wifi_manager.h"

static WebServer g_server(80);

// Compact JSON reply, microdot ujson parity (no spaces, same content-type).
static void send_json(int code, const char* json) {
    g_server.send(code, "application/json", json);
}

static void route_index(void) { g_server.send_P(200, "text/html", MAIN_HTML); }

static void route_main_js(void) { g_server.send_P(200, "text/javascript", MAIN_JS); }

static void route_current_state(void) {
    char buf[64];
    snprintf(buf, sizeof(buf), "{\"state\":\"%s\"}", state_str(lb_get_state()));
    send_json(200, buf);
}

static void route_cycle(void) {
    lb_start_cycle();
    char buf[64];
    snprintf(buf, sizeof(buf), "{\"state\":\"%s\"}", state_str(lb_get_state()));
    send_json(200, buf);
}

static void route_empty(void) {
    lb_set_state(LbState::EMPTYING);
    send_json(200, "{\"state\":\"Dumping this mess\"}");
}

static void route_pause(void) {
    lb_pause();
    send_json(200, "{\"state\":\"Paused\"}");
}

static void route_unpause(void) {
    lb_unpause();
    char buf[64];
    snprintf(buf, sizeof(buf), "{\"state\":\"%s\"}", state_str(lb_get_state()));
    send_json(200, buf);
}

static void route_reset_get(void) {
    lb_reset();
    // Deviation (registered in README): Python raised TypeError when
    // get_base_url() was None (microdot turned it into a 500); we send a 302
    // with an empty Location when offline. When online the parity is exact:
    // 302 + Location: base_url.
    g_server.sendHeader("Location", wifi_base_url());
    g_server.send(302, "application/json", "{\"state\":\"Reset\"}");
}

static void route_reset_post(void) {
    lb_reset();
    send_json(200, "{\"state\":\"Reset\"}");
}

static void route_settings_patch(void) {
    // microdot read request.json (raw body for application/json); WebServer
    // exposes the same body as arg("plain"). An unparsable body would have
    // crashed the Python app; here it is a no-op and the client still sees the
    // same success payload.
    settings_merge_and_save(g_server.arg("plain").c_str());
    send_json(200, "{\"state\":\"Updated\"}");
}

static void route_settings_get(void) {
    // Field order matches server.py dict insertion order exactly.
    char buf[512];
    snprintf(buf, sizeof(buf),
             "{\"load_sensor_threshold\":%g,\"timed_cycle_delay_hours\":%g,"
             "\"bin_full_cycles\":%lld,\"current_cycles\":%lld,\"last_cycle\":%.3f,"
             "\"cycle_wait_time\":%g}",
             settings_get_f(SETTING_KEY_LOAD_SENSOR_THRESHOLD, DEFAULT_LOAD_SENSOR_THRESHOLD),
             settings_get_f(SETTING_KEY_TIMED_CYCLE_DELAY_HOURS, DEFAULT_TIMED_CYCLE_DELAY_HOURS),
             (long long)settings_get_i(SETTING_KEY_BIN_FULL_CYCLES, DEFAULT_BIN_FULL_CYCLES),
             (long long)alert_current_cycles(), get_last_cycle(),
             settings_get_f(SETTING_KEY_CYCLE_WAIT_TIME, DEFAULT_CYCLE_WAIT_TIME));
    send_json(200, buf);
}

static void route_not_found(void) {
    // microdot's default 404 body (microdot.py:1233 `res = 'Not found', f`),
    // byte-exact.
    g_server.send(404, "text/plain", "Not found");
}

void web_setup(void) {
    g_server.on("/", HTTP_GET, route_index);
    g_server.on("/main.js", HTTP_GET, route_main_js);
    g_server.on("/current-state", HTTP_GET, route_current_state);
    g_server.on("/cycle", HTTP_POST, route_cycle);
    g_server.on("/empty", HTTP_POST, route_empty);
    g_server.on("/pause", HTTP_POST, route_pause);
    g_server.on("/unpause", HTTP_POST, route_unpause);
    g_server.on("/reset", HTTP_GET, route_reset_get);
    g_server.on("/reset", HTTP_POST, route_reset_post);
    g_server.on("/settings", HTTP_PATCH, route_settings_patch);
    g_server.on("/settings", HTTP_GET, route_settings_get);
    g_server.onNotFound(route_not_found);
    g_server.begin();
}

void web_tick(void) { g_server.handleClient(); }
