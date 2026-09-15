#!/usr/bin/env python3
"""Trim CRT from Ghidra EFW dumps and stamp recovered names onto function banners."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent
OUT = ROOT / "out"

# VA (with 0x100 prefix as in Ghidra banners) -> recovered name
RENAME = {
    "1007af20": "LinkUserMessages",
    "100ba040": "efwConversation::Squark",
    "100b86b0": "efwConversation::RegisterDefaults",
    "100b8ff0": "efwConversation::LoadAll",
    "100c1dc0": "efwConversationFile::efwConversationFile",
    "100c2660": "efwConversation::ParseFile",
    "100c3120": "efw_Marker",
    "100c3500": "efwConversation::AddKeyword",
    "100c4700": "weapon_efw_Pliers",
    "100c4770": "weapon_efw_Lever",
    "100c47e0": "weapon_efw_Branch",
    "100c4850": "weapon_efw_MobilePhone",
    "100c48c0": "weapon_efw_IDTag",
    "100c4930": "weapon_efw_RedPhoneCard",
    "100c49a0": "weapon_efw_GreenPhoneCard",
    "100c4a10": "weapon_efw_BluePhoneCard",
    "100c4a80": "weapon_efw_WashingPowder",
    "100c53c0": "monster_patrol_guard",
    "100c5ea0": "monster_refugee",
    "100c5f10": "monster_efw_guard",
    "100c6440": "CRefugee::IdleThink",
    "1000d1d0": "efw_AssignGuardModel",
    "100c6880": "efw_DiaryCount",
    "100c6890": "efw_AddDiary",
    "100c6980": "efw_Player",
    "100c6ad0": "efw_ThinkHope",
    "100c6b60": "efw_SendHudState",
    "100c6c10": "efw_ThinkConversation",
    "100c6e60": "efw_ShowMenu",
    "100c7830": "efw_TalkScan",
    "100c80d0": "efw_DebugPrint",
    "100c81d0": "efw_FailOrNarrate",
    "100c8160": "efw_FStrEq",
    "1001a550": "ClientCommand",
}

BANNER = re.compile(r"^/\* ==== ([0-9a-f]+) (\S+) ==== \*/", re.I)
# MSVC CRT / floating-point helpers start here in the server DLL.
CRT_CUTOFF = 0x100C84B0


def trim_and_annotate(src: Path, dest: Path, cutoff: int | None) -> tuple[int, int]:
    text = src.read_text(encoding="utf-8", errors="replace")
    parts = re.split(r"(?=/\* ==== )", text)
    kept: list[str] = []
    if parts and not parts[0].startswith("/* ===="):
        kept.append(parts[0])
        parts = parts[1:]
    n_in = 0
    n_out = 0
    for part in parts:
        m = BANNER.match(part)
        if not m:
            continue
        n_in += 1
        va = int(m.group(1), 16)
        if cutoff is not None and va >= cutoff:
            continue
        n_out += 1
        key = m.group(1).lower()
        if key in RENAME:
            old = m.group(0)
            new = f"/* ==== {m.group(1)} {RENAME[key]} ==== */"
            part = part.replace(old, new, 1)
            # Also rename the Ghidra FUN_ symbol in the prototype when it matches.
            fun = m.group(2)
            recovered = RENAME[key]
            if fun.startswith("FUN_") and recovered:
                ident = recovered.replace("::", "_").replace(" ", "_")
                part = part.replace(f"{fun}(", f"{ident}(", 1)
        kept.append(part)
    dest.write_text("".join(kept), encoding="utf-8")
    return n_in, n_out


def main() -> int:
    src = OUT / "EscapeFromWoomera_ghidra_efw.c"
    n_in, n_out = trim_and_annotate(src, src, CRT_CUTOFF)
    print(f"server efw: {n_in} -> {n_out} functions (dropped CRT >= {CRT_CUTOFF:#x})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
