#include "settings.h"

#include <string.h>

#ifndef UNIT_TEST
#include <Arduino.h>  // Serial for the load/persist safety warnings
#define SETTINGS_WARN(msg) Serial.println(msg)
#else
#define SETTINGS_WARN(msg) ((void)0)
#endif

static SettingsStore* g_store = nullptr;
static CbLoadFn g_loader = nullptr;
static CbPersistFn g_persister = nullptr;
static bool g_file_broken = false;  // corrupt file on disk: refuse all writes

void settings_init(SettingsStore* store, CbLoadFn loader, CbPersistFn persister) {
    g_store = store;
    g_loader = loader;
    g_persister = persister;
}

void settings_begin() {
    static char buf[SETTINGS_JSON_MAX];  // static: 4 KB does not belong on the loop stack
    g_file_broken = false;
    int code = g_loader ? g_loader(buf, sizeof(buf)) : SETTINGS_LOAD_ABSENT;
    if (code == SETTINGS_LOAD_CORRUPT) {
        // Unparseable, not an object, or larger than SETTINGS_JSON_MAX: keep
        // the bytes on disk untouched (auto-overwriting them would silently
        // destroy the user's settings) and run from {} in RAM.
        g_file_broken = true;
        SETTINGS_WARN("settings.json is corrupt or larger than the " SETTINGS_JSON_MAX_STR
                      "-byte read buffer; running from defaults and NOT writing "
                      "settings until it is fixed");
        settings_load(*g_store, "");
        return;
    }
    SettingsLoadCode loaded = settings_load(*g_store, code == SETTINGS_LOAD_OK ? buf : "");
    if (loaded == SETTINGS_LOAD_CORRUPT) g_file_broken = true;
    // Missing/invalid file behaves as {} exactly like Python load_state().
}

bool settings_file_broken() { return g_file_broken; }

static void persist_now() {
    if (!g_persister) return;
    if (g_file_broken) {
        SETTINGS_WARN("refusing to overwrite corrupt settings.json (fix or delete the file)");
        return;
    }
    static char buf[SETTINGS_JSON_MAX];
    if (!settings_dump(*g_store, buf, sizeof(buf))) {
        // Never write partial JSON: the next boot would parse it as garbage
        // and (previously) overwrite it with {}.
        SETTINGS_WARN("settings too large to serialize; not saved");
        return;
    }
    g_persister(buf);
}

double settings_get_f(const char* key, double def) { return settings_get_f(*g_store, key, def); }
int64_t settings_get_i(const char* key, int64_t def) { return settings_get_i(*g_store, key, def); }
bool settings_get_b(const char* key, bool def) { return settings_get_b(*g_store, key, def); }
const char* settings_get_s(const char* key, const char* def, char* out, size_t outlen) {
    return settings_get_s(*g_store, key, def, out, outlen);
}

void settings_set_f_and_save(const char* key, double value) {
    settings_set_f(*g_store, key, value);
    persist_now();
}

void settings_set_i_and_save(const char* key, int64_t value) {
    settings_set_i(*g_store, key, value);
    persist_now();
}

int64_t settings_increment_and_save(const char* key) {
    int64_t v = settings_increment(*g_store, key);
    persist_now();
    return v;
}

bool settings_merge_and_save(const char* patch_json) {
    if (!settings_apply_patch(*g_store, patch_json)) return false;
    persist_now();
    return true;
}
