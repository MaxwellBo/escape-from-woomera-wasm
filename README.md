# Escape from Woomera — Original Mod in the Browser (WASM)

Boots the **original Escape From Woomera v0.84 Half-Life mod files** (2004, EFW Team)
in the browser using the open-source **Xash3D-FWGS engine cross-compiled to
WebAssembly**, with stock Half-Life game logic likewise compiled to WASM.

Project page that inspired this: https://julianoliver.com/projects/escape-from-woomera/

## How it works

- `public/woomera.zip` — the vendored, freely-distributed mod demo. Unpacked in
  the browser (fflate) and written into the engine's in-memory filesystem as
  `woomera/`. Win32-only `dlls/`/`cl_dlls/` and `SAVE/` are skipped;
  `liblist.gam` is rewritten to reference the WASM game-logic names.
  Conversation scripts, diary sprites, and storyboard screens are staged.
- `public/valve.zip` — `valve/` from the official Half-Life: Uplink demo
  (models, sounds, sprites, HUD, `pak0.pak`) plus `delta.lst` from the
  open Half-Life 1 SDK. Unpacked beside the mod.
- `public/engine/` — Xash3D-FWGS WebAssembly runtime
  (`xash.wasm`, renderers, menu, filesystem layer).
- `public/hlsdk/` — Half-Life client/server game logic as WASM (hlsdk-portable
  plus the in-repo EFW overlay in `game-logic/`), plus `delta.lst`.
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
demo’s own maps omitted; SDK `delta.lst` added).

1. Wait for steps 1–2 (mod zip + Uplink `valve/`). The engine boots itself.
2. Click the game view to capture the mouse (pointer lock) and keyboard. Esc releases the mouse.

## Scripted systems (reimplemented)

The original hope meter, conversations, diary, and markers only shipped as
Win32 DLLs, and their source was never released. This build **reimplements**
those systems on hlsdk-portable so they run in WASM:

- `monster_refugee` NPCs spawn with detainee models and the `Conversations/*.txt` talk tree (Use / click / `efw_spider`)
- Hope HUD drains slowly and rises when diary pages unlock
- Diary (`i`, `[` / `]`) shows the official `EFW_Diary_*.spr` pages
- `efw_Marker` interactions (pliers, kitchen bin, hiding place, ID tag board)
- EFW inventory weapons (`weapon_efw_*`) including the mail package / SIM card

Rebuild the WASM modules after changing `game-logic/`:

```bash
npm run test:efw
npm run build:hlsdk   # needs Emscripten (scripts/build-hlsdk.sh)
```

## Run it

```bash
npm install
npm run dev
# → http://127.0.0.1:47831/
```

Re-vendor third-party binaries (documents provenance):

```bash
npm run vendor      # scripts/vendor.sh
npm run build:hlsdk # overlay + emscripten rebuild of public/hlsdk/*.wasm
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
