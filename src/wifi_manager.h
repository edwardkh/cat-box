// wifi_manager — wifi/connection.py parity (stay_connected policy) with
// deviations D2 (non-blocking boot; Python busy-waited forever offline) and
// D3 (SNTP on first connection; the Python app had no time source at all).
#pragma once

void wifi_setup(void);
void wifi_tick(void);         // keepalive policy, call every 250 ms
bool wifi_is_up(void);
// base_url(): explicit setting ?? "http://<ip>" (empty string when offline,
// mirroring Python's None path that suppressed email).
const char* wifi_base_url(void);
