#!/usr/bin/env python3
"""Build public/valve.zip from an extracted Half-Life: Uplink valve/ folder.

Drops Win32 DLLs and Uplink's own maps (this site boots Woomera, not the demo
levels). Everything else — models, sounds, sprites, HUD gfx, config — stays.
Adds open HLSDK files from scripts/hlsdk-files/ (delta.lst).
"""
from __future__ import annotations

import argparse
import struct
import zipfile
from pathlib import Path

SKIP_PAK_PREFIXES = ("maps/",)
LOOSE_FILES = (
    "cached.wad",
    "decals.wad",
    "gfx.wad",
    "default.cfg",
    "credits.txt",
    "language.cfg",
    "liblist.gam",
    "settings.scr",
    "skill.cfg",
    "titles.txt",
    "valve.rc",
)
# Open Half-Life SDK files (ValveSoftware/halflife). Uplink's pak0 does not
# ship delta.lst; Xash refuses to boot without it.
SDK_DIR = Path(__file__).resolve().parent / "hlsdk-files"
SDK_LOOSE = ("delta.lst",)


def rebuild_pak(src: Path) -> bytes:
    data = src.read_bytes()
    if data[:4] != b"PACK":
        raise SystemExit(f"{src} is not a GoldSrc PACK file")
    off, length = struct.unpack_from("<II", data, 4)
    kept: list[tuple[str, bytes]] = []
    for i in range(0, length, 64):
        rec = data[off + i : off + i + 64]
        name = rec[:56].split(b"\x00", 1)[0].decode("latin1").replace("\\", "/")
        eoff, esize = struct.unpack_from("<II", rec, 56)
        if name.lower().startswith(SKIP_PAK_PREFIXES):
            continue
        kept.append((name, data[eoff : eoff + esize]))
    payload = bytearray()
    dirents = bytearray()
    cursor = 12
    for name, blob in kept:
        raw = name.encode("latin1")
        if len(raw) > 55:
            raise SystemExit(f"PACK name too long: {name}")
        dirents += raw.ljust(56, b"\x00") + struct.pack("<II", cursor, len(blob))
        payload += blob
        cursor += len(blob)
    return b"PACK" + struct.pack("<II", 12 + len(payload), len(dirents)) + payload + dirents


def resolve_loose(valve_dir: Path, name: str) -> Path | None:
    for candidate in (valve_dir / name, valve_dir / name.lower(), valve_dir / name.upper()):
        if candidate.exists():
            return candidate
    matches = list(valve_dir.glob(name)) + list(valve_dir.glob(name.upper()))
    return matches[0] if matches else None


def pack(valve_dir: Path, dest: Path) -> None:
    pak_src = valve_dir / "pak0.PAK"
    if not pak_src.exists():
        pak_src = valve_dir / "pak0.pak"
    if not pak_src.exists():
        raise SystemExit(f"no pak0.pak in {valve_dir}")
    dest.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(dest, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as zf:
        zf.writestr("valve/pak0.pak", rebuild_pak(pak_src))
        for name in LOOSE_FILES:
            path = resolve_loose(valve_dir, name)
            if path is None:
                print(f"warning: missing {name}")
                continue
            zf.write(path, f"valve/{name.lower()}")
        for name in SDK_LOOSE:
            path = SDK_DIR / name
            if not path.exists():
                print(f"warning: missing SDK file {name}")
                continue
            zf.write(path, f"valve/{name}")
    print(f"wrote {dest} ({dest.stat().st_size / 1048576:.1f} MB)")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("valve_dir", type=Path)
    parser.add_argument("dest", type=Path)
    args = parser.parse_args()
    pack(args.valve_dir, args.dest)


if __name__ == "__main__":
    main()
