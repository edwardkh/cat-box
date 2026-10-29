// settings — global cached settings.json accessors (device + native tests).
// The storage layer (LittleFS on device, in-memory in tests) is installed via
// settings_init(); typed getters follow get_state_of(key, default) parity.
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "config.h"
#include "core/settings_core.h"

// Persist current cache to storage (Python set_state writes on every
// mutation). Device implementation writes atomically (tmp + rename).
typedef bool (*CbPersistFn)(const char* json);
// Load stored JSON into buf. Returns a SettingsLoadCode: 0 absent, 1 parsed
// OK, 2 present-but-corrupt (unparseable or too large for buf — in that case
// buf content is unusable and the file must be preserved, never overwritten).
typedef int (*CbLoadFn)(char* buf, size_t buflen);

void settings_init(SettingsStore* store, CbLoadFn loader, CbPersistFn persister);
// Boot: load file. Missing -> {} (Python parity, boot must not hang);
// corrupt -> in-RAM {} but loudly warned and all writes refused so the
// broken file survives for user recovery.
void settings_begin();
// True while a corrupt settings.json is being preserved (all persistence
// refused) — exposed for diagnostics/tests.
bool settings_file_broken();

double settings_get_f(const char* key, double def);
int64_t settings_get_i(const char* key, int64_t def);
bool settings_get_b(const char* key, bool def);
// Copies into `out`; returns `out`.
const char* settings_get_s(const char* key, const char* def, char* out, size_t outlen);

// set key + persist immediately (set_state_of parity).
void settings_set_f_and_save(const char* key, double value);
void settings_set_i_and_save(const char* key, int64_t value);
int64_t settings_increment_and_save(const char* key);

// PATCH /settings: shallow-merge patch into cache + persist.
// Returns false on invalid JSON (cache unchanged).
bool settings_merge_and_save(const char* patch_json);
