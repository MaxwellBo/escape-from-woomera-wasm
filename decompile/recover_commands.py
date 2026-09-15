#!/usr/bin/env python3
"""Recover ClientCommand dispatch and name the EFW overlay functions.

Ghidra missed most of the 0x1001a550 ClientCommand body (say/say_team returned
early; later strcmp chains for efw_Talk / efw_Give / ... were never created as
functions). This walks the PE with Capstone and emits a readable reconstruction.
"""
from __future__ import annotations

import struct
from pathlib import Path

import pefile
from capstone import CS_ARCH_X86, CS_MODE_32, Cs

ROOT = Path(__file__).resolve().parent
DLL = ROOT / "binaries" / "EscapeFromWoomera.dll"
OUT = ROOT / "recovered"
IMG = 0x10000000


def load_pe():
    pe = pefile.PE(str(DLL))
    sections = {}
    for s in pe.sections:
        name = s.Name.decode("latin1").strip("\x00")
        va = IMG + s.VirtualAddress
        sections[name] = (va, s.get_data(), s.Misc_VirtualSize)
    pe.close()
    return sections


def read_cstr(sections, va, maxlen=160):
    for va0, data, vsize in sections.values():
        if va0 <= va < va0 + len(data):
            off = va - va0
            end = data.find(b"\x00", off, off + maxlen)
            if end < 0:
                return None
            raw = data[off:end]
            try:
                return raw.decode("ascii")
            except UnicodeDecodeError:
                return raw.decode("latin1")
    return None


def main() -> int:
    sections = load_pe()
    text_va, text, _ = sections[".text"]
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = False

    # ClientCommand lives in the gap Ghidra left after FUN_1001a550's early return.
    start, end = 0x1001A550, 0x1001BBD0
    blob = text[start - text_va : end - text_va]
    insns = list(md.disasm(blob, start))

    handlers = []
    i = 0
    while i < len(insns):
        ins = insns[i]
        # mov esi, imm32  where imm is a .data/.rdata C string
        if ins.mnemonic == "mov" and ins.op_str.startswith("esi, 0x"):
            try:
                imm = int(ins.op_str.split(",")[1].strip(), 16)
            except ValueError:
                i += 1
                continue
            name = read_cstr(sections, imm)
            if name and (name.startswith("efw_") or name in {"say", "say_team", "give", "drop", "use"}):
                handlers.append((ins.address, name, imm))
        i += 1

    CALL_NAMES = {
        0x100C6980: "efw_Player",
        0x100C6E60: "efw_ShowMenu",
        0x100C7830: "efw_TalkScan",
        0x100C6890: "efw_AddDiary",
        0x100C80D0: "efw_DebugPrint",
        0x100C81D0: "efw_FailOrNarrate",
        0x100C8160: "efw_FStrEq",
        0x100BA040: "efwConversation::Squark",
        0x100C7430: "efw_ClearConversation",
        0x100B94E0: "efw_ResetSpeech",
        0x100AF030: "UTIL_FindEntityByTargetname",
        0x100C81A0: "efw_SetHudByte",
        0x100C81B0: "efw_GetHudByte",
    }

    def calls_between(a: int, b: int) -> list[str]:
        out: list[str] = []
        for ins in insns:
            if not (a <= ins.address < b):
                continue
            if ins.mnemonic != "call":
                continue
            t = ins.op_str
            if t.startswith("0x"):
                try:
                    dest = int(t, 16)
                except ValueError:
                    continue
                label = CALL_NAMES.get(dest, f"FUN_{dest:08x}")
                out.append(f"{ins.address:#010x} -> {label}")
        return out

    bounds = [h[0] for h in handlers] + [end]
    OUT.mkdir(exist_ok=True)
    lines = [
        "/* Recovered ClientCommand dispatch from EscapeFromWoomera.dll",
        " * Ghidra function FUN_1001a550 (0x1001a550) only decompiled the say/say_team",
        " * prefix. The remaining strcmp chain is reconstructed from Capstone.",
        " * Image base 0x10000000. Thiscall/cdecl as on Win32 MSVC 2004.",
        " */",
        "",
        "/* pfnClientCommand(edict_t *pEntity) — GoldSrc DLL_FUNCTIONS slot */",
        "void ClientCommand(edict_t *pEntity)",
        "{",
        "    const char *cmd = CMD_ARGV(0);",
        "    /* 0x1001a550: say / say_team (Ghidra FUN_1001a550) */",
        "",
    ]
    for i, (addr, name, imm) in enumerate(handlers):
        nxt = bounds[i + 1]
        lines.append(f"    /* {addr:#010x} strcmp(cmd, {name!r} @ {imm:#010x}) */")
        lines.append(f"    if (!stricmp(cmd, \"{name}\")) {{")
        for c in calls_between(addr, nxt)[:12]:
            lines.append(f"        /* call {c} */")
        lines.append("    }")
        lines.append("")
    lines.append("}")
    lines.append("")

    (OUT / "ClientCommand_dispatch.c").write_text("\n".join(lines) + "\n", encoding="utf-8")

    # Named overlay map used by RECOVERED.md
    named = [
        ("0x1000d1d0", "efw_AssignGuardModel", "targetname -> models/security.mdl or tradesman.mdl"),
        ("0x1007af20", "LinkUserMessages", "registers ShowMenu, EFWShow, EFWData, EFW_Menu, EFW_Cntxt, EFW_CtPrv"),
        ("0x100ba040", "efwConversation::Squark", "look up NPC by targetname; warn if missing"),
        ("0x100b86b0", "efwConversation::RegisterDefaults", "Gate Guard / Electrician / detainee display names"),
        ("0x100b8ff0", "efwConversation::LoadAll", r"FindFirstFile game dir\\Conversations\\*.txt, then seed ESCAPE/GREET/GOODBYE"),
        ("0x100c1dc0", "efwConversationFile ctor", "0x74-byte conversation parser object"),
        ("0x100c2660", "efwConversation::ParseFile", "open one Conversations/*.txt"),
        ("0x100c3120", "efw_Marker", "LINK_ENTITY_TO_CLASS, alloc 0x15c"),
        ("0x100c3500", "efwConversation::AddKeyword", "insert ESCAPE/GREET/GOODBYE flags"),
        ("0x100c4700", "weapon_efw_Pliers", "w/v/p_Pliers.mdl"),
        ("0x100c53c0", "monster_patrol_guard", "LINK_ENTITY_TO_CLASS"),
        ("0x100c5ea0", "monster_refugee", "CRefugee, alloc 0x3a8, vtable 0x100f93a4"),
        ("0x100c5f10", "monster_efw_guard", "same CRefugee vtable"),
        ("0x100c6440", "CRefugee::IdleThink", "queue / mad_scientist_entity / walk-to-player"),
        ("0x100c6880", "efw_DiaryCount", "return DAT_10134870"),
        ("0x100c6890", "efw_AddDiary", "append diary page; logs 'Diary active item added'"),
        ("0x100c6980", "efw_Player", "return DAT_10134888 (local CBasePlayer*)"),
        ("0x100c6ad0", "efw_ThinkHope", "hope -= dt*(1/12); clamp 0..100; at 0 fire menu 0x4d 'Run out of hope!'"),
        ("0x100c6b60", "efw_SendHudState", "pack hope / diary / conversation into EFWData slots"),
        ("0x100c6c10", "efw_ThinkConversation", "timeout + 'Conversation hidden, partner too far'"),
        ("0x100c6e60", "efw_ShowMenu", "GoldSrc ShowMenu with up to 7 lines; logs CONVERSATION (n messages)"),
        ("0x100c7830", "efw_TalkScan", "sphere search monster_refugee/barney; send EFW_CtPrv"),
        ("0x100c80d0", "efw_DebugPrint", "vsprintf + OutputDebugStringA"),
        ("0x100c81d0", "efw_FailOrNarrate", "hope-fail / 'Where do you think you are going' / EFW_Menu byte"),
        ("0x100c8160", "efw_FStrEq", "case-sensitive entity-name compare"),
        ("0x1001a550", "ClientCommand", "say/say_team plus EFW cmds; strcmp chain in ClientCommand_dispatch.c"),
    ]
    md_lines = ["# Recovered function names", "", "| VA | Recovered name | Role |", "|----|----------------|------|"]
    for va, name, role in named:
        md_lines.append(f"| {va} | `{name}` | {role} |")
    md_lines.append("")
    md_lines.append("## ClientCommand strcmp hits")
    md_lines.append("")
    for addr, name, imm in handlers:
        md_lines.append(f"- `{addr:#010x}` `{name}`")
    md_lines.append("")
    (OUT / "NAMES.md").write_text("\n".join(md_lines), encoding="utf-8")

    print(f"handlers={len(handlers)}")
    for addr, name, imm in handlers:
        print(f"  {addr:#010x}  {name}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
