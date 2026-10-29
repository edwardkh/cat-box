// alert_stub — alert/bin_full.py parity (email out of scope, deviation D1:
// the "send" is a log-only stub; no TLS/NetworkClientSecure anywhere).
// Uses the global settings API so counters persist exactly like Python's
// set_state_of(); host tests install an in-memory storage layer.
#pragma once

#include <stddef.h>
#include <stdint.h>

// bin_full.react_to_cycle():
//   cycles = increment_state_of("cycles")
//   if cycles > get_state_of("alert_bin_full_cycles", get_bin_full_cycles())
// The `>` comparison (not >=) is Python parity and load-bearing.
// Returns true when the threshold was exceeded; the device layer then builds
// the message and prints it (nothing is ever sent).
bool alert_react_to_cycle(void);

// Python message text: "Empty the cat box. It has cycled N times. Reset with
// <base_url>/reset". Written into `out`; returns out.
char* alert_build_message(char* out, size_t outlen, const char* base_url, int64_t cycles);

// bin_full.reset(): set_state_of("cycles", 0)
void alert_reset(void);

// bin_full.get_current_cycles(): get_state_of("cycles", 0)
int64_t alert_current_cycles(void);
