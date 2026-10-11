# Escape from Woomera — original Windows mod under Wine

Boots the **original Escape From Woomera v0.84 Half-Life mod files** (2004, EFW Team)
in the browser. The game logic is the **original Win32 DLLs**, not a decompile.
A 32-bit **Windows** build of the open-source Xash3D FWGS engine loads those DLLs,
and [Boxedwine](https://github.com/danoon2/Boxedwine) runs that Windows build
under **Wine** (x86 emulator compiled to WebAssembly).

Project page that inspired this: https://julianoliver.com/projects/escape-from-woomera/

## How it works

- `public/boxedwine/` — Boxedwine 26R1 single-threaded web build (no
  cross-origin isolation headers) plus the Wine 6.0 root filesystem
  shipped in that build (`wine6.zip`). Wine 11's web filesystem omits
  API-set DLLs, so `wineboot` never finishes and `xash.dll` cannot load.
  Wine 6's `psapi.dll` and `wmic.exe` are placeholders with no builtin.
  `scripts/build-psapi-shim.sh` replaces them and rewrites `ntdll`'s
  `xsave`/`xrstor` as `fxsave`/`fxrstor`, which this emulator can run.
  `run.bat` turns off SDL's DirectInput and Windows.Gaming.Input probes,
  and the page starts Boxedwine with `-nosound`.
- `public/boxedwine/woomera.zip` — the Windows game Boxedwine mounts as
  `C:\files`:
  - `xash3d.exe`, `xash.dll`, `ref_soft.dll`, `menu.dll`, SDL2 and the MinGW
    runtime. `vgui.dll` is freevgui rebuilt for the MSVC i386 C++ ABI
    (`scripts/build-vgui-msvc.sh`), because `client.dll` imports the
    original MSVC-mangled methods and the MinGW build does not export them.
  - `woomera/` from the vendored mod, **including**
    `dlls/EscapeFromWoomera.dll` and `cl_dlls/client.dll`
  - `valve/` from the Half-Life: Uplink demo, plus SDK `delta.lst`
  - `run.bat` launches `xash3d.exe -game woomera -ref soft`
- `src/main.ts` embeds that runtime and starts `run.bat`.

The web build of Boxedwine cannot translate OpenGL or Direct3D to WebGL, so
the engine is built with the software renderer (`--enable-soft`). `ref_soft`
draws into an SDL window surface, which Wine presents with GDI.

## Build the Windows engine

Requires `gcc-mingw-w64-i686` (and `i686-w64-mingw32-windres`).

```bash
npm run build:win32   # scripts/build-win32.sh
```

That clones Xash3D FWGS at the pinned commit, applies
`scripts/xash-mingw-i386-rename.patch` (PE/COFF i386 export names), configures
with `--enable-soft --disable-gl`, and writes `public/boxedwine/woomera.zip`.

Repack an existing `third_party/xash-win32` tree with:

```bash
npm run pack:wine
```

## Run it

```bash
npm install
npm run dev
# → http://127.0.0.1:47831/
```

The first load fetches the Wine filesystem and the Windows game (about 110 MB).
Wine opens a window titled Escape from Woomera. The CPU is interpreted, so
startup is slow. Click the view, then use the mouse and WASD. Sound is left
off; Boxedwine's web audio path is not used.

Re-vendor the mod and Uplink `valve/` archives (the pack script reads them):

```bash
npm run vendor
npm run pack:wine
```

## Provenance

- Mod: `EscapeFromWoomera_v084.zip`, archived official site,
  http://www.ljudmila.org/~selectparks/archive/escapefromwoomera/
  (also on ModDB: https://www.moddb.com/mods/escape-from-woomera)
- Engine: [FWGS/xash3d-fwgs](https://github.com/FWGS/xash3d-fwgs) (GPL),
  commit `51353ff8d65e58300eeb2bd014867673f1f367c6`, MinGW i686
- SDL2 2.32.10 MinGW development package
- Wine host: [Boxedwine 26R1](https://github.com/danoon2/Boxedwine/releases/tag/26R1.0)
  `Boxedwine26R1Web.zip`, single-threaded emulator and its Wine 6.0
  filesystem, copied here as `wine6.zip` (GPL)
- Base game: Half-Life: Uplink demo `valve/` from
  https://archive.org/download/Half-lifeUplink/hluplink.exe
  (Valve/Sierra, 1999, freely distributed demo)
- `delta.lst` + HLSDK LICENSE: https://github.com/ValveSoftware/halflife

`decompile/` and `game-logic/` are leftover notes from an earlier port that
reimplemented the DLLs. The page does not load them.
