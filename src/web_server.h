// web_server — server.py route parity on the core's built-in WebServer
// (PATCH-capable, bounded non-blocking parser in 3.3.x; no third-party dep).
#pragma once

void web_setup(void);
void web_tick(void);  // server.handleClient()
