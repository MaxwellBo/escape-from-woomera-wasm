# Original DLL decompilation

The v0.84 gameplay was compiled as Win32 GoldSrc modules:

- `dlls/EscapeFromWoomera.dll` (server, 1 302 621 bytes)
- `cl_dlls/client.dll` (client HUD, 573 440 bytes)

Those binaries are **not loaded** by the WASM engine. Recovered C lives in
`out/` and `recovered/`. The playable port is `game-logic/`, compiled into
`public/hlsdk/{client,server}.wasm` (hope formula, EFWData, conversation
range, refugee IdleThink, ClientCommand names).

## Recovered output (this tree)

| File | What |
|------|------|
| `out/EscapeFromWoomera_ghidra_efw.c` | Ghidra C for the EFW overlay (~200 functions, CRT stripped, recovered names in banners) |
| `out/client_ghidra_efw.c` | Ghidra C for HUD / diary / storyboard / talk prompts |
| `out/*_functions.txt` | VA, name, body size for every function |
| `out/*_exports.txt` | PE export table |
| `recovered/NAMES.md` | VA → recovered symbol |
| `recovered/ClientCommand_dispatch.c` | Capstone reconstruction of the `efw_*` command chain Ghidra missed |
| `recovered/CRefugee_Spawn.c` | Capstone of CRefugee::Spawn named model table |
| `recovered/FUN_100c4af0_LookUse.c` | Capstone of player look-use (Ghidra gap after WashingPowder) |
| `recovered/FUN_100c6320_StudioHull.c` | Capstone of CRefugee sequence hull |
| `RECOVERED.md` | narrative map of hope / conversations / markers / HUD |

Full dumps `out/*_ghidra.c` (~3.5 MB server, ~1.8 MB client) are gitignored; regenerate with Ghidra.

## Recover binaries

```bash
python3 - <<'PY'
import zipfile, pathlib
z=zipfile.ZipFile('public/woomera.zip')
out=pathlib.Path('decompile/binaries')
out.mkdir(parents=True, exist_ok=True)
for n in ['EscapeFromWoomera_v084/dlls/EscapeFromWoomera.dll','EscapeFromWoomera_v084/cl_dlls/client.dll']:
    (out/pathlib.Path(n).name).write_bytes(z.read(n))
PY
python3 decompile/inventory.py
```

## Ghidra (headless)

Ghidra 12.x + JDK 21. Import the PE as `x86:LE:32:default` / `windows`.

```bash
GHIDRA_MAXMEM=4G /path/to/analyzeHeadless /tmp/ghidra-proj EFW \
  -import decompile/binaries/EscapeFromWoomera.dll \
  -processor x86:LE:32:default -cspec windows \
  -scriptPath decompile/ghidra_scripts \
  -postScript ApplyEngineTable.java \
  -postScript ExportEfwDecomp.java decompile/out
```

Keep the project. `-deleteProject` throws away the engine-slot labels
`ApplyEngineTable.java` writes (143 server slots at `0x10121e08`, 98 client
slots at `0x100a4ff0`, `gpGlobals` at `0x10122044`).

Repeat for `client.dll`. Then:

```bash
python3 decompile/recover_commands.py   # ClientCommand strcmp chain
python3 decompile/annotate_efw.py       # recovered names + drop CRT
python3 decompile/lift_overlay.py       # compile the overlay against the slot table
python3 decompile/lift_overlay.py --test
```

## Mechanical lift

`lift_overlay.py` turns `out/EscapeFromWoomera_ghidra_efw.c` into
`game-logic/efw_lift.c`. Engine calls become `EFW_EngSlots()[n]`
(`engine_slots_server.txt`; slot 2 is `pfnSetModel`). Absolute addresses
go through `EFW_VA` into a relocated copy of the DLL image
(`game-logic/efw_image.c`, generated, not committed). The four Win32
imports the overlay actually makes (`FindFirstFileA` / `FindNextFileA` /
`FindClose`, `GetTickCount`, `OutputDebugStringA`, `GetAsyncKeyState`)
are POSIX stubs in `efw_lift_host.c`.

Every lifted function is prefixed `lift_` so it does not collide with
hlsdk exports such as `monster_refugee`. `scripts/build-hlsdk.sh`
regenerates the lift and links it into the server only.
`EFW_LiftAnchor` keeps it from being stripped. The hand port still owns
hope, commands, and spawn; the anchor does not call `lift_efw_ThinkHope`.

`npm run test:lift` compiles the lift with `gcc -m32` and checks the hope
drain (80 → 79, 10 → 9, clamp at 100) without booting the game.

The same script writes `recovered/client_lift.c` from the client dump.
That file is not compiled or linked; the client HUD stays on the hand port.

## What the exports already prove

The server DLL is stock Half-Life SDK plus a small overlay:

- `CRefugee::IdleThink`
- `monster_refugee`, `monster_efw_guard`, `monster_patrol_guard`
- `efw_Marker`
- `weapon_efw_*`
- conversation engine is **not** exported; strings name `efwConversation::Squark`

The client DLL owns diary sprites, speech-bubble HUD, give/hide icons, storyboards, and issues `efw_Talk` / `efw_Give` / `efw_UseWithMarker` / `efw_ShowMenu`.
