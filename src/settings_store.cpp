#include "settings_store.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <string.h>

#define SETTINGS_PATH "/settings.json"
#define SETTINGS_TMP "/settings.json.tmp"

// Returns a SettingsLoadCode: absent -> 0; parsed-sized content -> 1; a file
// larger than the caller buffer (which fs would otherwise silently truncate
// into invalid JSON) -> 2 so settings_begin() preserves it instead of
// overwriting it.
static int fs_load(char* buf, size_t buflen) {
    File f = LittleFS.open(SETTINGS_PATH, "r");
    if (!f) return SETTINGS_LOAD_ABSENT;  // missing file -> {} (Python swallowed the error)
    if (f.size() >= buflen) {             // would not fit even with NUL: corrupt-by-our-lights
        f.close();
        return SETTINGS_LOAD_CORRUPT;
    }
    size_t n = f.readBytes(buf, buflen - 1);
    buf[n] = '\0';
    f.close();
    if (n == 0) return SETTINGS_LOAD_ABSENT;  // empty file behaves as {} (Python parity)
    return SETTINGS_LOAD_OK;
}

static bool fs_persist(const char* json) {
    // tmp write + rename = atomic against power loss (on-device checklist #9)
    LittleFS.remove(SETTINGS_TMP);
    File f = LittleFS.open(SETTINGS_TMP, "w");
    if (!f) return false;
    size_t len = strlen(json);
    size_t written = f.write((const uint8_t*)json, len);
    f.close();
    if (written != len) {
        LittleFS.remove(SETTINGS_TMP);
        return false;
    }
    return LittleFS.rename(SETTINGS_TMP, SETTINGS_PATH);
}

void settings_store_setup(SettingsStore* store) {
    // formatOnFail: a worn or foreign-labelled partition must not brick boot.
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS mount failed; settings run from defaults only");
    }
    settings_init(store, fs_load, fs_persist);
}
