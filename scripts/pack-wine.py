#!/usr/bin/env python3
"""Pack the Windows Xash3D build and original mod into a Boxedwine app zip.

The original v0.84 DLLs are copied as-is. They are loaded by the Windows
engine under Wine; nothing here decompiles or rewrites them.
"""
from __future__ import annotations

import argparse
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

ENGINE_FILES = (
    "xash3d.exe",
    "xash.dll",
    "filesystem_stdio.dll",
    "menu.dll",
    "ref_soft.dll",
    "vgui.dll",
    "SDL2.dll",
    "libgcc_s_dw2-1.dll",
    "libstdc++-6.dll",
)

# Software renderer presents with GDI (SDL_GetWindowSurface). Boxedwine's
# web build has no OpenGL, so ref_gl cannot draw.
RUN_BAT = (
    "@echo off\r\n"
    "cd /d %~dp0\r\n"
    "rem SDL 2.32 probes DirectInput, Windows.Gaming.Input and wmic. Those\r\n"
    "rem Wine 6 builtins are missing and the probe crashes Boxedwine.\r\n"
    "set SDL_DIRECTINPUT_ENABLED=0\r\n"
    "set SDL_XINPUT_ENABLED=0\r\n"
    "set SDL_JOYSTICK_WGI=0\r\n"
    "set SDL_JOYSTICK_RAWINPUT=0\r\n"
    "set SDL_JOYSTICK_HIDAPI=0\r\n"
    "rem Boxedwine -nosound does not reach the engine. SDL audio init\r\n"
    "rem runs after the window is created and stalls the emulator.\r\n"
    "echo EFW-BOOT\r\n"
    "xash3d.exe -game woomera -ref soft -windowed -width 640 -height 480 "
    "-console -nosound -dev 2 +map efw_prototype_level1\r\n"
)

MOD_PREFIX = "EscapeFromWoomera_v084/"


def add_bytes(zf: zipfile.ZipFile, arcname: str, data: bytes) -> None:
    zf.writestr(arcname.replace("\\", "/"), data)


def add_tree_from_zip(zf: zipfile.ZipFile, src: Path, dest_prefix: str, strip: str = "") -> int:
    count = 0
    with zipfile.ZipFile(src) as incoming:
        for info in incoming.infolist():
            if info.is_dir():
                continue
            name = info.filename.replace("\\", "/")
            if strip and name.startswith(strip):
                name = name[len(strip):]
            if not name or name.endswith("/"):
                continue
            arc = f"{dest_prefix}{name}" if dest_prefix else name
            add_bytes(zf, arc, incoming.read(info))
            count += 1
    return count


def pack(engine: Path, mingw_runtime: Path, mod_zip: Path, valve_zip: Path, dest: Path) -> None:
    missing = [name for name in ENGINE_FILES if name not in ("libgcc_s_dw2-1.dll", "libstdc++-6.dll") and not (engine / name).is_file()]
    if missing:
        raise SystemExit(f"engine dir {engine} is missing: {', '.join(missing)}")
    for name in ("libgcc_s_dw2-1.dll", "libstdc++-6.dll"):
        if not (engine / name).is_file() and not (mingw_runtime / name).is_file():
            raise SystemExit(f"missing MinGW runtime {name} in {engine} or {mingw_runtime}")

    dest.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(dest, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=6) as zf:
        add_bytes(zf, "run.bat", RUN_BAT.encode("ascii"))
        for name in ENGINE_FILES:
            path = engine / name
            if not path.is_file():
                path = mingw_runtime / name
            add_bytes(zf, name, path.read_bytes())
        extras = engine / "valve" / "extras.pk3"
        if extras.is_file():
            add_bytes(zf, "valve/extras.pk3", extras.read_bytes())
        n_valve = add_tree_from_zip(zf, valve_zip, "")
        n_mod = add_tree_from_zip(zf, mod_zip, "woomera/", MOD_PREFIX)
    size = dest.stat().st_size
    print(f"wrote {dest} ({size / 1e6:.1f} MB), valve files {n_valve}, mod files {n_mod}")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--engine", type=Path, default=ROOT / "third_party" / "xash-win32")
    parser.add_argument(
        "--mingw-runtime",
        type=Path,
        default=Path("/usr/lib/gcc/i686-w64-mingw32/13-win32"),
    )
    parser.add_argument("--mod", type=Path, default=ROOT / "public" / "woomera.zip")
    parser.add_argument("--valve", type=Path, default=ROOT / "public" / "valve.zip")
    parser.add_argument("--out", type=Path, default=ROOT / "public" / "boxedwine" / "woomera.zip")
    args = parser.parse_args()
    pack(args.engine, args.mingw_runtime, args.mod, args.valve, args.out)


if __name__ == "__main__":
    main()
