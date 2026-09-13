# Escape from Woomera — Original Mod in the Browser (WASM)

Boots the **original Escape From Woomera v0.84 Half-Life mod files** (2004, EFW Team)
in the browser using the open-source **Xash3D-FWGS engine cross-compiled to
WebAssembly**, with stock Half-Life game logic likewise compiled to WASM.

Project page that inspired this: https://julianoliver.com/projects/escape-from-woomera/

## How it works

- `public/woomera.zip` — the vendored, freely-distributed mod demo. Unpacked in
  the browser (fflate) and written into the engine's in-memory filesystem as
  `woomera/`. Win32-only `dlls/`/`cl_dlls/` and unused bulk (`SAVE/`,
  `Storyboard/`) are skipped; `liblist.gam` is rewritten to reference the WASM
  game-logic names.
- `public/engine/` — Xash3D-FWGS WebAssembly runtime
  (`xash.wasm`, renderers, menu, filesystem layer).
- `public/hlsdk/` — stock Half-Life client/server game logic as WASM.
- `src/main.ts` — loader UI: stages the mod, intakes your retail `valve/`,
  writes everything into WASM memory, then boots with `-game woomera` and loads
  `efw_prototype_level1` (buttons for levels 1–3 + engine console included).

## You must supply retail Half-Life data (not included)

The engine and the mod ship here, but the base-game `valve/` data is
copyrighted by Valve and is **not** in this repo. In the browser:

1. Wait for step 1 (mod assets) to finish.
2. Install Half-Life once (e.g. via Steam) and click **Select Half-Life
   folder**, choosing the folder that contains `valve/` (needs
   `valve/pak0.pak`). Files never leave your machine — they are copied into
   in-memory WASM storage for the session only.
3. Click **Boot Woomera in WASM**, then click the game view to capture
   mouse/keyboard. Keyboard + mouse required, as with the original.

## Honest limitation: stock game logic, not the mod's custom code

The mod's custom gameplay code (hope meter, conversation system, objectives)
only ever shipped as **Win32 `.dll` binaries**, which no WASM engine can load,
and its source was never released. So the browser runs the mod's **real maps,
models, textures, sounds and WADs** on **stock Half-Life logic**: you can walk
the actual Woomera compound geometry, but scripted systems will be degraded or
missing. This is the closest a browser can get without the mod's source.

## Run it

```bash
npm install
npm run dev
# → http://127.0.0.1:47831/
```

Re-vendor third-party binaries (documents provenance):

```bash
npm run vendor   # scripts/vendor.sh
```

## Provenance

- Mod: `EscapeFromWoomera_v084.zip`, archived official site,
  http://www.ljudmila.org/~selectparks/archive/escapefromwoomera/
  (also on ModDB: https://www.moddb.com/mods/escape-from-woomera)
- Engine WASM: `xash3d-fwgs@1.2.2` (npm)
- Game-logic WASM: `hlsdk-portable@0.1.3` (npm mirror — upstream delisted it;
  source fallback: https://github.com/FWGS/hlsdk-portable)
- Engine source: FWGS/Xash3D-FWGS (GPL). Half-Life data © Valve — not included.
