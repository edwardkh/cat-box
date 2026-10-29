// settings_store — LittleFS persistence layer for the settings cache
// (device only). /settings.json with atomic tmp+rename writes; a missing
// file behaves as {} (Python load_state() parity — boot must not hang).
#pragma once

#include "settings.h"

// Mount LittleFS and install the filesystem loader/persister into the
// settings API against `store`.
void settings_store_setup(SettingsStore* store);
