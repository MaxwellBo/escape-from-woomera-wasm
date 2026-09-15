#!/usr/bin/env python3
"""Static inventory of the original Escape from Woomera Win32 DLLs."""
from __future__ import annotations

import re
import sys
from pathlib import Path

import pefile

ROOT = Path(__file__).resolve().parent
BINS = ROOT / "binaries"
OUT = ROOT / "out"
OUT.mkdir(exist_ok=True)

MSVC_MANGLED = re.compile(rb"\?\?[_A-Z][\x20-\x7e]{4,200}")
ASCII = re.compile(rb"[\x20-\x7e]{6,200}")


def undname_rough(s: str) -> str:
    """Best-effort MSVC 6/7 mangled name skim, not a full undname."""
    return s.replace("@@", " ").replace("@", " ")


def dump_pe(path: Path) -> dict:
    pe = pefile.PE(str(path))
    info: dict = {
        "file": path.name,
        "size": path.stat().st_size,
        "machine": hex(pe.FILE_HEADER.Machine),
        "timestamp": pe.FILE_HEADER.TimeDateStamp,
        "exports": [],
        "imports": {},
        "sections": [],
    }
    for s in pe.sections:
        info["sections"].append(
            {
                "name": s.Name.decode("latin1", "replace").strip("\x00"),
                "va": hex(s.VirtualAddress),
                "vsize": s.Misc_VirtualSize,
                "raw": s.SizeOfRawData,
                "chars": hex(s.Characteristics),
            }
        )
    if hasattr(pe, "DIRECTORY_ENTRY_EXPORT") and pe.DIRECTORY_ENTRY_EXPORT:
        for e in pe.DIRECTORY_ENTRY_EXPORT.symbols:
            info["exports"].append(
                {
                    "ord": e.ordinal,
                    "name": e.name.decode("latin1") if e.name else None,
                    "addr": hex(e.address),
                }
            )
    if hasattr(pe, "DIRECTORY_ENTRY_IMPORT"):
        for ent in pe.DIRECTORY_ENTRY_IMPORT:
            dll = ent.dll.decode("latin1", "replace")
            names = []
            for imp in ent.imports:
                names.append(imp.name.decode("latin1") if imp.name else f"ord_{imp.ordinal}")
            info["imports"][dll] = names
    data = path.read_bytes()
    mangled = sorted({m.decode("latin1") for m in MSVC_MANGLED.findall(data)})
    strings = [m.decode("latin1") for m in ASCII.findall(data)]
    info["mangled_count"] = len(mangled)
    info["string_count"] = len(strings)
    (OUT / f"{path.stem}_exports.txt").write_text(
        "\n".join(f"{e['addr']}\t{e['ord']}\t{e['name']}" for e in info["exports"]) + "\n",
        encoding="utf-8",
    )
    (OUT / f"{path.stem}_imports.txt").write_text(
        "\n".join(
            f"[{dll}]\n" + "\n".join(f"  {n}" for n in names)
            for dll, names in info["imports"].items()
        )
        + "\n",
        encoding="utf-8",
    )
    (OUT / f"{path.stem}_mangled.txt").write_text("\n".join(mangled) + "\n", encoding="utf-8")
    interesting = [
        s
        for s in strings
        if re.search(
            r"(?i)efw|conversation|hope|diary|pliers|refugee|squark|monster_|weapon_efw|AddDiary|ShowMenu|CRefugee|CBase",
            s,
        )
    ]
    (OUT / f"{path.stem}_strings_efw.txt").write_text("\n".join(interesting) + "\n", encoding="utf-8")
    (OUT / f"{path.stem}_strings_all.txt").write_text("\n".join(strings) + "\n", encoding="utf-8")
    pe.close()
    return info


def main() -> int:
    reports = []
    for name in ("EscapeFromWoomera.dll", "client.dll"):
        path = BINS / name
        if not path.is_file():
            print("missing", path, file=sys.stderr)
            return 1
        info = dump_pe(path)
        reports.append(
            f"{info['file']} size={info['size']} exports={len(info['exports'])} "
            f"mangled={info['mangled_count']} strings={info['string_count']}"
        )
        print(reports[-1])
        print("  exports:", ", ".join(e["name"] or "?" for e in info["exports"][:30]))
    (OUT / "summary.txt").write_text("\n".join(reports) + "\n", encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
