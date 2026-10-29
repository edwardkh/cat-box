// settings_store — cached settings.json (persistent_state/state.py parity).
//
// Pure, testable core: shallow merge with unknown-key preservation and
// compact serialization (ujson.dumps parity). File I/O (LittleFS) is a thin
// device-only layer on top; a missing file behaves as {} (Python swallowed
// the open() error — boot must not hang).
#pragma once

#include <ArduinoJson.h>

#include <stdint.h>

struct SettingsStore {
    JsonDocument doc;  // ArduinoJson 7: pooled, heap-backed
};

// Serialize cache compactly into `out`. Returns false if it does not fit:
// `out` must then be treated as garbage and NEVER persisted (a partial JSON
// file would parse as empty and, on the next boot, be overwritten with {} —
// silent settings loss).
bool settings_dump(const SettingsStore& store, char* out, size_t outlen);

// Load outcome for the wrapper: ABSENT keeps Python's "missing file -> {}"
// parity; CORRUPT must be reported loudly and the file preserved (never
// auto-overwritten) so the user can recover it.
enum SettingsLoadCode : uint8_t {
    SETTINGS_LOAD_ABSENT = 0,  // no file (Python swallowed the open error -> {})
    SETTINGS_LOAD_OK = 1,      // parsed as a JSON object
    SETTINGS_LOAD_CORRUPT = 2  // present but unparseable / not an object
};

// Replace cache contents from a JSON string; empty input -> {} (ABSENT);
// invalid input -> in-RAM {} (CORRUPT, like load_state() swallowing errors,
// but now visible to the caller so the file can be preserved on device).
SettingsLoadCode settings_load(SettingsStore& store, const char* json);

// get_state_of(key, default) parity: file value ?? default.
double settings_get_f(const SettingsStore& store, const char* key, double def);
int64_t settings_get_i(const SettingsStore& store, const char* key, int64_t def);
bool settings_get_b(const SettingsStore& store, const char* key, bool def);
// Copies into `out` (static single buffer per store — single caller per tick,
// like the Python transient string). Returns `out`. `def` used when absent.
const char* settings_get_s(const SettingsStore& store, const char* key, const char* def,
                           char* out, size_t outlen);

// Cache mutation (caller persists).
void settings_set_f(SettingsStore& store, const char* key, double value);
void settings_set_i(SettingsStore& store, const char* key, int64_t value);

// increment_state_of(key): file value ?? 0, +1, returns new value.
int64_t settings_increment(SettingsStore& store, const char* key);

// Shallow-merge top-level keys of `patch_json` into the cache (dict.update:
// overwrite in place, append new keys in patch order; unknown keys preserved).
// Invalid/non-object JSON: no-op, returns false.
bool settings_apply_patch(SettingsStore& store, const char* patch_json);
