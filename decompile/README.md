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
  -postScript ExportEfwDecomp.java decompile/out \
  -deleteProject
```

Repeat for `client.dll`. Then:

```bash
python3 decompile/recover_commands.py   # ClientCommand strcmp chain
python3 decompile/annotate_efw.py       # recovered names + drop CRT
```

## What the exports already prove

The server DLL is stock Half-Life SDK plus a small overlay:

- `CRefugee::IdleThink`
- `monster_refugee`, `monster_efw_guard`, `monster_patrol_guard`
- `efw_Marker`
- `weapon_efw_*`
- conversation engine is **not** exported; strings name `efwConversation::Squark`

The client DLL owns diary sprites, speech-bubble HUD, give/hide icons, storyboards, and issues `efw_Talk` / `efw_Give` / `efw_UseWithMarker` / `efw_ShowMenu`.
