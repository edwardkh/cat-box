#include "core/settings_core.h"

#include <string.h>

SettingsLoadCode settings_load(SettingsStore& store, const char* json) {
    store.doc.to<JsonObject>();  // start from {}
    if (!json || !*json) return SETTINGS_LOAD_ABSENT;
    DeserializationError err = deserializeJson(store.doc, json);
    if (err || !store.doc.is<JsonObject>()) {
        store.doc.to<JsonObject>();
        return SETTINGS_LOAD_CORRUPT;
    }
    return SETTINGS_LOAD_OK;
}

bool settings_dump(const SettingsStore& store, char* out, size_t outlen) {
    if (outlen == 0) return false;
    out[0] = '\0';
    size_t n = serializeJson(store.doc, out, outlen);
    return n < outlen;  // false => did not fit; caller must NOT persist `out`
}

static JsonVariantConst settings_lookup(const SettingsStore& store, const char* key) {
    return store.doc[key].as<JsonVariantConst>();
}

double settings_get_f(const SettingsStore& store, const char* key, double def) {
    JsonVariantConst v = settings_lookup(store, key);
    return (v.is<double>() || v.is<int64_t>() || v.is<uint64_t>()) ? v.as<double>() : def;
}

int64_t settings_get_i(const SettingsStore& store, const char* key, int64_t def) {
    JsonVariantConst v = settings_lookup(store, key);
    return (v.is<int64_t>() || v.is<uint64_t>() || v.is<double>()) ? v.as<int64_t>() : def;
}

bool settings_get_b(const SettingsStore& store, const char* key, bool def) {
    JsonVariantConst v = settings_lookup(store, key);
    return v.is<bool>() ? v.as<bool>() : def;
}

const char* settings_get_s(const SettingsStore& store, const char* key, const char* def,
                           char* out, size_t outlen) {
    JsonVariantConst v = settings_lookup(store, key);
    const char* s = v.is<const char*>() ? v.as<const char*>() : def;
    if (!s) {
        out[0] = '\0';
        return out;
    }
    strncpy(out, s, outlen - 1);
    out[outlen - 1] = '\0';
    return out;
}

void settings_set_f(SettingsStore& store, const char* key, double value) {
    store.doc[key] = value;
}

void settings_set_i(SettingsStore& store, const char* key, int64_t value) {
    store.doc[key] = value;
}

int64_t settings_increment(SettingsStore& store, const char* key) {
    int64_t next = settings_get_i(store, key, 0) + 1;
    store.doc[key] = next;
    return next;
}

bool settings_apply_patch(SettingsStore& store, const char* patch_json) {
    JsonDocument patch;
    DeserializationError err = deserializeJson(patch, patch_json ? patch_json : "");
    if (err || !patch.is<JsonObject>()) return false;
    for (JsonPair kv : patch.as<JsonObject>()) {
        store.doc[kv.key()] = kv.value();  // dict.update semantics
    }
    return true;
}
