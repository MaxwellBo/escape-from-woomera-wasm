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
- `public/valve.zip` — `valve/` from the official Half-Life: Uplink demo
  (models, sounds, sprites, HUD, `pak0.pak`) plus `delta.lst` from the
  open Half-Life 1 SDK. Unpacked beside the mod.
- `public/engine/` — Xash3D-FWGS WebAssembly runtime
  (`xash.wasm`, renderers, menu, filesystem layer).
- `public/hlsdk/` — stock Half-Life client/server game logic as WASM, plus
  `delta.lst` (the engine’s network field table).
- `src/main.ts` — loader UI: stages the mod, stages vendored Uplink `valve/`
  data, writes everything into WASM memory, then boots with `-game woomera`
  and loads `efw_prototype_level1` (buttons for levels 1–3 + engine console
  included).

## Half-Life data (Uplink demo)

Valve open-sourced the **Half-Life 1 SDK** (game logic), not the GoldSrc
engine binary or the retail `valve/` art. This site uses:

- **Xash3D-FWGS** — open-source GoldSrc-compatible engine, compiled to WASM
- **hlsdk-portable** — that open SDK, compiled to WASM (`public/hlsdk/`)
- **Half-Life: Uplink** demo `valve/` — Valve/Sierra’s freely distributed
  1999 demo (models, sounds, sprites, HUD). Retail WADs such as
  `halflife.wad` are *not* included.
- **`delta.lst`** — from [ValveSoftware/halflife](https://github.com/ValveSoftware/halflife)
  (`network/delta.lst`). Uplink does not ship this file; Xash will not boot
  without it. Redistributed under the Half-Life 1 SDK LICENSE
  (`public/hlsdk/LICENSE`).

`scripts/vendor.sh` downloads the archived Uplink installer, extracts it,
and `scripts/pack-valve.py` writes `public/valve.zip` (Win32 DLLs and the
demo’s own maps omitted; SDK `delta.lst` added). A full Steam install can
still be selected as an override.

1. Wait for steps 1–2 (mod zip + Uplink `valve/`). The engine boots itself.
2. Click the game view to capture the mouse (pointer lock) and keyboard. Esc releases the mouse.
3. Optional: **Override with your install** if you have retail Half-Life.

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
- Engine source: FWGS/Xash3D-FWGS (GPL)
- Base game: Half-Life: Uplink demo `valve/` from
  https://archive.org/download/Half-lifeUplink/hluplink.exe
  (Valve/Sierra, 1999, freely distributed demo)
- `delta.lst` + HLSDK LICENSE: https://github.com/ValveSoftware/halflife
