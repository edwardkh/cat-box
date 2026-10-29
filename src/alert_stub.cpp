#include "alert_stub.h"

#include <stdio.h>
#include <string.h>

#include "settings.h"

bool alert_react_to_cycle(void) {
    int64_t cycles = settings_increment_and_save(SETTING_KEY_CYCLES);
    int64_t limit = settings_get_i(SETTING_KEY_ALERT_BIN_FULL_CYCLES,
                                   settings_get_i(SETTING_KEY_BIN_FULL_CYCLES,
                                                  DEFAULT_BIN_FULL_CYCLES));
    return cycles > limit;  // Python `>` parity
}

char* alert_build_message(char* out, size_t outlen, const char* base_url, int64_t cycles) {
    // alert/message.py caller text in bin_full.py, byte-exact.
    snprintf(out, outlen,
             "Empty the cat box. It has cycled %lld times. Reset with %s/reset",
             (long long)cycles, base_url ? base_url : "");
    return out;
}

void alert_reset(void) { settings_set_i_and_save(SETTING_KEY_CYCLES, 0); }

int64_t alert_current_cycles(void) { return settings_get_i(SETTING_KEY_CYCLES, 0); }
