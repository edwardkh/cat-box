# reference/ — spec reference only, NOT built

`python-app/` is the original MicroPython application (moved verbatim from `src/`)
frozen as the behavioral specification for the C++ rewrite in `src/`. It is not
compiled, uploaded, or referenced by the PlatformIO build except that
`scripts/embed_assets.py` reads `python-app/frontend/` to generate
`src/assets/web_assets.h`.

Do not modify anything under `python-app/`.

`settings.example.json` is a seed file for the device's LittleFS
`/settings.json` (all keys optional — every key falls back to its built-in
default; credentials are placeholders).
