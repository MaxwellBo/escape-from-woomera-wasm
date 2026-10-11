#!/usr/bin/env python3
"""Turn the Ghidra dump of the EFW overlay into C that compiles.

GiveFnptrsToDll copies 0x8f dwords of enginefuncs_t to 0x10121e08 and stores
gpGlobals at 0x10122044. Slot 2 is pfnSetModel. The client Initialize copies
0x62 dwords of cl_enginefunc_t to 0x100a4ff0. This script names those slots,
relocates absolute addresses into a copy of the DLL image, and replaces the
handful of Win32 calls the overlay actually makes.

The server lift is compiled by --test (hope drain, no game boot) and by
scripts/build-hlsdk.sh into the wasm server.
"""
from __future__ import annotations

import re
import subprocess
import sys
import textwrap
import zipfile
from pathlib import Path

import pefile

ROOT = Path(__file__).resolve().parents[1]
DECOMP = Path(__file__).resolve().parent
GAME = ROOT / "game-logic"
ZIP = ROOT / "public" / "woomera.zip"

SERVER_ENG_BASE = 0x10121E08
SERVER_ENG_COUNT = 0x8F
SERVER_GLOBALS = 0x10122044
CLIENT_ENG_BASE = 0x100A4FF0
CLIENT_ENG_COUNT = 0x62

WIN32_RENAMES = {
    "FindFirstFileA": "EFW_FindFirstFileA",
    "FindNextFileA": "EFW_FindNextFileA",
    "FindClose": "EFW_FindClose",
    "GetTickCount": "EFW_GetTickCount",
    "OutputDebugStringA": "EFW_OutputDebugStringA",
    "GetAsyncKeyState": "EFW_GetAsyncKeyState",
    "operator_new": "malloc",
}

KEYWORDS = {
    "if", "for", "while", "switch", "return", "sizeof", "else", "do",
    "case", "break", "continue", "goto", "typedef",
}

BUILTINS = KEYWORDS | {
    "EFW_VA", "EFW_EngSlots", "EFW_InstallImage", "malloc", "memcpy", "memset",
    "fputs", "fprintf", "printf", "stderr", "true", "false", "NULL",
    "SQRT", "NAN", "isnan", "sqrt", "sqrtl", "CONCAT22", "CONCAT31",
    "efw_fbits",
    *WIN32_RENAMES.values(),
}


def load_slots(path: Path, count: int) -> list[str]:
    names = [ln.strip() for ln in path.read_text().splitlines() if ln.strip()]
    if len(names) != count:
        raise SystemExit(f"{path} has {len(names)} names, want {count}")
    return names


def extract_dll(inner: str, dest: Path) -> Path:
    dest.parent.mkdir(parents=True, exist_ok=True)
    if dest.is_file() and dest.stat().st_size > 0:
        return dest
    with zipfile.ZipFile(ZIP) as zf:
        dest.write_bytes(zf.read(inner))
    return dest


def build_image(pe: pefile.PE) -> tuple[int, bytearray]:
    base = pe.OPTIONAL_HEADER.ImageBase
    end = base
    for section in pe.sections:
        span = max(section.Misc_VirtualSize, section.SizeOfRawData)
        end = max(end, base + section.VirtualAddress + span)
    buf = bytearray(end - base)
    for section in pe.sections:
        data = section.get_data()
        off = section.VirtualAddress
        buf[off : off + len(data)] = data
    return base, buf


def float_addresses(text: str) -> set[int]:
    found: set[int] = set()
    for match in re.finditer(
        r"(?:\(float10\)|\(float\))\s*_?DAT_([0-9a-fA-F]{8})", text
    ):
        found.add(int(match.group(1), 16))
    for match in re.finditer(
        r"_?DAT_([0-9a-fA-F]{8})\s*=\s*\(float\)", text
    ):
        found.add(int(match.group(1), 16))
    return found


def transform(text: str, slots: list[str], eng_base: int, image_base: int, image_end: int) -> str:
    floats = float_addresses(text)
    text = re.sub(r"^[ \t]*/\* WARNING:.*?\*/[ \t]*\n", "", text, flags=re.M)
    text = re.sub(r"__thiscall\b|__fastcall\b|__cdecl\b|__stdcall\b", "", text)
    text = text.replace("::", "_")

    def repl_s_field(match: re.Match[str]) -> str:
        addr = int(match.group(1), 16)
        off = int(match.group(2))
        size = int(match.group(3))
        typ = {1: "uint8_t", 2: "uint16_t", 4: "uint32_t"}.get(size, "uint32_t")
        return f"(*({typ} *)(EFW_VA(0x{addr:08x}) + {off}))"

    text = re.sub(
        r"\bs_<[^>]+>_([0-9a-fA-F]{8})\b",
        lambda m: f"((char *)EFW_VA(0x{int(m.group(1), 16):08x}))",
        text,
    )
    text = re.sub(
        r"\bs_[A-Za-z0-9_]*_([0-9a-fA-F]{8})\._(\d+)_(\d+)_",
        repl_s_field,
        text,
    )

    def repl_field(match: re.Match[str]) -> str:
        base = match.group(1)
        off = int(match.group(2))
        size = int(match.group(3))
        typ = {1: "uint8_t", 2: "uint16_t", 4: "uint32_t"}.get(size, "uint32_t")
        return f"(*(({typ} *)((uint8_t *)&({base}) + {off})))"

    text = re.sub(r"\b([A-Za-z_][A-Za-z0-9_]*)\._(\d+)_(\d+)_", repl_field, text)
    text = re.sub(
        r"\b([A-Za-z_][A-Za-z0-9_]*)\s*=\s*\((?:char|byte)\s*\[4\]\)\s*([^;]+);",
        lambda m: (
            "{ uint32_t _efw_tmp = (uint32_t)("
            + m.group(2).strip()
            + "); memcpy("
            + m.group(1)
            + ", &_efw_tmp, 4); }"
        ),
        text,
    )
    text = re.sub(r"\b(C[A-Z][A-Za-z0-9_]*)\s+\*", "uint8_t *", text)
    # void* cannot be dereferenced or incremented in C. Ghidra uses it for
    # both object pointers and byte cursors (WIN32_FIND_DATA filenames).
    text = re.sub(r"\bvoid\s*\*", "uint8_t *", text)
    text = re.sub(r"&LAB_([0-9a-fA-F]{8})", r"EFW_VA(0x\1)", text)
    text = re.sub(
        r"\bPTR___fptrap_([0-9a-fA-F]{8})\b",
        lambda m: f"(*(void **)EFW_VA(0x{int(m.group(1), 16):08x}))",
        text,
    )

    def repl_hex(match: re.Match[str]) -> str:
        # Earlier rewrites already emitted EFW_VA(0x........). Wrapping that
        # argument again turns the relocated pointer into a second offset.
        before = text[max(0, match.start() - 7) : match.start()]
        if before == "EFW_VA(":
            return match.group(0)
        val = int(match.group(1), 16)
        if image_base <= val < image_end:
            return f"((uint32_t)(uintptr_t)EFW_VA(0x{val:08x}))"
        return match.group(0)

    text = re.sub(r"\b0x([0-9a-fA-F]+)\b", repl_hex, text)

    def repl_eng(match: re.Match[str]) -> str:
        addr = int(match.group(1), 16)
        delta = addr - eng_base
        if delta % 4 == 0:
            idx = delta // 4
            if 0 <= idx < len(slots):
                return f"((efw_fn)(uintptr_t)EFW_EngSlots()[{idx}] /* {slots[idx]} */)"
        return match.group(0)

    text = re.sub(r"\(\*(?:_DAT_|DAT_)([0-9a-fA-F]{8})\)", repl_eng, text)

    def repl_star_dat(match: re.Match[str]) -> str:
        addr = int(match.group(1), 16)
        # *DAT_10122044 is gpGlobals->time, a float. Every other starred
        # DAT is a pointer-width load.
        if addr == SERVER_GLOBALS:
            return f"(*(float *)(uintptr_t)(*(uint32_t *)EFW_VA(0x{addr:08x})))"
        return f"(*(uint32_t *)(uintptr_t)(*(uint32_t *)EFW_VA(0x{addr:08x})))"

    # (*DAT_slot) was already rewritten. Every remaining *DAT_ is a load
    # through the pointer stored at that address, including (*DAT_ + off).
    text = re.sub(r"\*(?:_DAT_|DAT_)([0-9a-fA-F]{8})", repl_star_dat, text)

    def repl_str(match: re.Match[str]) -> str:
        addr = int(match.group(1), 16)
        return f"((char *)EFW_VA(0x{addr:08x}))"

    text = re.sub(r"\bs_[A-Za-z0-9_]*_([0-9a-fA-F]{8})\b", repl_str, text)

    def repl_amp(match: re.Match[str]) -> str:
        addr = int(match.group(1), 16)
        return f"EFW_VA(0x{addr:08x})"

    text = re.sub(r"&PTR_(?:FUN|LAB)_([0-9a-fA-F]{8})", repl_amp, text)
    text = re.sub(r"&PTR_s_[A-Za-z0-9_]*_([0-9a-fA-F]{8})", repl_amp, text)
    text = re.sub(r"&(?:PTR_DAT_|_DAT_|DAT_)([0-9a-fA-F]{8})", repl_amp, text)

    def repl_ptr(match: re.Match[str]) -> str:
        addr = int(match.group(1), 16)
        return f"(*(uint8_t **)EFW_VA(0x{addr:08x}))"

    text = re.sub(r"\bPTR_(?:FUN|LAB)_([0-9a-fA-F]{8})\b", repl_ptr, text)
    text = re.sub(r"\bPTR_s_[A-Za-z0-9_]*_([0-9a-fA-F]{8})\b", repl_ptr, text)
    text = re.sub(r"\bPTR_DAT_([0-9a-fA-F]{8})\b", repl_ptr, text)

    # DAT_x[i] indexes the pointer stored at that address (std::string /
    # vtable globals). A uint32 load cannot be subscripted.
    text = re.sub(
        r"\b_?DAT_([0-9a-fA-F]{8})(?=\s*\[)",
        lambda m: (
            "((uint8_t *)(uintptr_t)(*(uint32_t *)EFW_VA(0x"
            f"{int(m.group(1), 16):08x})))"
        ),
        text,
    )

    def repl_dat(match: re.Match[str]) -> str:
        addr = int(match.group(1), 16)
        if addr in floats:
            return f"(*(float *)EFW_VA(0x{addr:08x}))"
        return f"(*(uint32_t *)EFW_VA(0x{addr:08x}))"

    text = re.sub(r"\b_?DAT_([0-9a-fA-F]{8})\b", repl_dat, text)

    for old, new in WIN32_RENAMES.items():
        text = re.sub(rf"\b{old}\b", new, text)
    return text


def function_sigs(text: str) -> list[str]:
    """Signatures of functions whose opening brace is at column 0."""
    lines = text.splitlines()
    sigs: list[str] = []
    for i, line in enumerate(lines):
        if line != "{":
            continue
        j = i - 1
        while j >= 0 and lines[j].strip() == "":
            j -= 1
        parts: list[str] = []
        while j >= 0:
            raw = lines[j].strip()
            if not raw or raw.startswith("/*") or raw.startswith("*") or raw.startswith("#"):
                break
            parts.append(raw)
            if "(" in raw:
                # Ghidra sometimes puts the return type on the line above the name.
                k = j - 1
                while k >= 0 and lines[k].strip() == "":
                    k -= 1
                if k >= 0:
                    prev = lines[k].strip()
                    if (
                        prev
                        and not prev.startswith(("/*", "*", "#"))
                        and all(tok not in prev for tok in (";", "{", "}", "("))
                    ):
                        parts.append(prev)
                break
            j -= 1
        if not parts:
            continue
        sig = " ".join(reversed(parts))
        head = sig.split("(", 1)[0].strip()
        name = head.split()[-1] if head else ""
        if name in KEYWORDS or "(" not in sig:
            continue
        sigs.append(sig)
    return sigs


def sig_name(sig: str) -> str:
    match = re.search(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(", sig)
    return match.group(1) if match else ""


def defined_names(text: str) -> set[str]:
    return {sig_name(sig) for sig in function_sigs(text) if sig_name(sig)}


def _call_span(text: str, open_paren: int) -> tuple[int, int, bool]:
    """Return (close_paren, arity, is_definition) for the call at open_paren."""
    i = open_paren + 1
    depth = 1
    commas = 0
    empty = True
    while i < len(text) and depth:
        ch = text[i]
        if ch == "(":
            # A nested paren is an argument, even when the whole argument
            # is wrapped: name(((uint32_t)EFW_VA(0x...))).
            if depth == 1:
                empty = False
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                break
        elif ch == "," and depth == 1:
            commas += 1
            empty = False
        elif not ch.isspace() and depth == 1:
            empty = False
        i += 1
    arity = 0 if empty and commas == 0 else commas + 1
    nxt = text[i + 1 :].lstrip() if i < len(text) else ""
    is_def = nxt.startswith("{")
    return i, arity, is_def


def call_sites(text: str, name: str) -> list[int]:
    arities = []
    for match in re.finditer(rf"\b{name}\s*\(", text):
        _close, arity, is_def = _call_span(text, match.end() - 1)
        if is_def:
            continue
        arities.append(arity)
    return arities


def address_taken(text: str, name: str) -> bool:
    for match in re.finditer(rf"\b{name}\b", text):
        nxt = text[match.end() :]
        if nxt.lstrip().startswith("("):
            continue
        # The definition's name is followed by '(' too, so it is skipped.
        return True
    return False


def return_type_of(sig: str) -> str:
    name = sig_name(sig)
    if not name or "(" not in sig:
        return ""
    head = sig.split("(", 1)[0]
    idx = head.rfind(name)
    return head[:idx].strip()


def call_value_used(text: str, name: str) -> bool:
    """True when a call is not a discarded statement and not the definition."""
    for match in re.finditer(rf"\b{re.escape(name)}\s*\(", text):
        close, _arity, is_def = _call_span(text, match.end() - 1)
        if is_def or close >= len(text):
            continue
        nxt = text[close + 1 :].lstrip()
        line_start = text.rfind("\n", 0, match.start()) + 1
        prefix = text[line_start : match.start()].strip()
        if prefix == "" and (nxt.startswith(";") or nxt.startswith(",")):
            continue
        return True
    return False


def _function_body_bounds(text: str, name: str) -> tuple[int, int] | None:
    for match in re.finditer(rf"\b{re.escape(name)}\s*\(", text):
        close, _arity, is_def = _call_span(text, match.end() - 1)
        if not is_def:
            continue
        brace = text.find("{", close)
        if brace < 0:
            return None
        depth = 0
        for i in range(brace, len(text)):
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    return brace, i
        return None
    return None


def promote_void_returns(text: str) -> str:
    """Ghidra marks some functions void, then the caller casts the result.

    A cast of a void expression is a constraint violation. Give those
    functions a uint32_t return so the cast compiles. The hope test does
    not execute them; the value is only there to satisfy the type.
    """
    names = []
    for sig in function_sigs(text):
        name = sig_name(sig)
        if return_type_of(sig) == "void" and name and call_value_used(text, name):
            names.append(name)
    for name in names:
        text = re.sub(
            rf"\bvoid(\s+){name}\s*\(",
            rf"uint32_t\1{name}(",
            text,
        )
        bounds = _function_body_bounds(text, name)
        if bounds is None:
            continue
        start, end = bounds
        body = re.sub(r"\breturn\s*;", "return 0;", text[start : end + 1])
        if "return 0;" not in body and "return " not in body:
            body = body[:-1] + "return 0;\n}"
        text = text[:start] + body + text[end + 1 :]
    return text


_INT_SCALARS = {
    "undefined4", "undefined", "undefined1", "undefined2", "undefined3",
    "uint", "int", "dword", "uint32_t", "byte", "ushort", "uint3", "int3",
    "BOOL", "DWORD", "LONG", "ULONG", "char", "CHAR",
}


def _split_top(arglist: str) -> list[str]:
    args: list[str] = []
    depth = 0
    start = 0
    for i, ch in enumerate(arglist):
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        elif ch == "," and depth == 0:
            args.append(arglist[start:i].strip())
            start = i + 1
    tail = arglist[start:].strip()
    if tail and tail != "void":
        args.append(tail)
    return args


def _param_types(sig: str) -> list[str]:
    if "(" not in sig:
        return []
    inside = sig.split("(", 1)[1].rstrip()
    if inside.endswith(")"):
        inside = inside[:-1]
    types: list[str] = []
    for arg in _split_top(inside):
        match = re.match(r"^(.*?)([A-Za-z_][A-Za-z0-9_]*)$", arg.strip())
        types.append(match.group(1).strip() if match else arg)
    return types


def _int_scalar(typ: str) -> bool:
    base = typ.replace("const", "").strip()
    if "*" in base:
        return False
    name = base.split()[-1] if base else ""
    return name in _INT_SCALARS


def rewrite_float_args(text: str) -> str:
    """Ghidra writes FUN(slot, (float)value) for a raw dword parameter.

    The (float) cast is a type annotation. Compiled C would convert the
    value to an integer and store 79 instead of the bits of 79.0f, so
    hope reads back as a denormal zero. Pass the float bits instead.
    """
    types_of = {}
    for sig in function_sigs(text):
        name = sig_name(sig)
        if name:
            types_of[name] = _param_types(sig)
    spans: list[tuple[int, int, str]] = []
    for name, types in types_of.items():
        if not any(_int_scalar(t) for t in types):
            continue
        for match in re.finditer(rf"\b{re.escape(name)}\s*\(", text):
            open_paren = match.end() - 1
            close, _arity, is_def = _call_span(text, open_paren)
            if is_def or close >= len(text):
                continue
            args = _split_top(text[open_paren + 1 : close])
            if len(args) != len(types):
                continue
            changed = False
            new_args: list[str] = []
            for arg, typ in zip(args, types):
                if _int_scalar(typ) and arg.startswith("(float)"):
                    inner = arg[len("(float)") :].strip()
                    new_args.append(f"efw_fbits({inner})")
                    changed = True
                else:
                    new_args.append(arg)
            if changed:
                spans.append((open_paren + 1, close, ", ".join(new_args)))
    for start, end, repl in sorted(spans, reverse=True):
        text = text[:start] + repl + text[end:]
    return text


def prefix_defined(text: str) -> str:
    """Give every lifted function a lift_ prefix.

    The hand port and hlsdk already export C names such as monster_refugee
    and weapon_efw_*. The lift is linked into the same server, so those
    definitions would collide. Stubs stay static and unprefixed.
    """
    names = sorted(defined_names(text), key=len, reverse=True)
    for name in names:
        if name.startswith("lift_") or name == "EFW_LiftKeep":
            continue
        text = re.sub(rf"\b{re.escape(name)}\b", f"lift_{name}", text)
    return text


def emit_stubs(text: str) -> str:
    defined = defined_names(text)
    mentioned = set(re.findall(r"\b((?:FUN_|efw_|C|__)[A-Za-z0-9_]*)\b", text))
    called = set()
    for match in re.finditer(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(", text):
        name = match.group(1)
        if name not in KEYWORDS and name not in BUILTINS:
            called.add(name)
    needed = (mentioned | called) - defined - BUILTINS
    # Drop type-like tokens and labels.
    needed = {n for n in needed if not n.startswith("LAB_") and n not in {
        "undefined", "undefined1", "undefined2", "undefined3", "undefined4",
        "uint3", "int3", "byte", "ushort", "uint", "dword", "float10", "code",
        "efw_fn", "HANDLE", "LPCSTR", "LPSTR", "BOOL", "DWORD",
        "WIN32_FIND_DATAA", "_WIN32_FIND_DATAA",
    }}
    lines = ["", "/* Stubs for callees outside the overlay dump (CRT, Spirit). */"]
    for name in sorted(needed):
        arities = call_sites(text, name)
        taken = address_taken(text, name)
        if not arities and not taken:
            continue
        positive = sorted({a for a in arities if a > 0})
        if len(positive) > 1 and not taken:
            lines.append(f"#define {name}(...) ((uint32_t)0)")
            continue
        n = positive[-1] if positive else 0
        if n == 0:
            lines.append(f"static uint32_t {name}(void) {{ return 0; }}")
        else:
            args = ", ".join(f"uint32_t a{i}" for i in range(n))
            voids = " ".join(f"(void)a{i};" for i in range(n))
            lines.append(f"static uint32_t {name}({args}) {{ {voids} return 0; }}")
    return "\n".join(lines) + "\n"


def emit_keep(names: list[str]) -> str:
    body = ["void EFW_LiftKeep(void)", "{", "\tvolatile uintptr_t x = 0;"]
    for name in names:
        body.append(f"\tx += (uintptr_t){name};")
    body.append("\t(void)x;")
    body.append("}")
    return "\n".join(body) + "\n"


def write_image_c(buf: bytes, dest: Path) -> None:
    lines = [
        '#include "efw_lift_prelude.h"',
        "static const uint8_t efw_image_bytes[] = {",
    ]
    row: list[str] = []
    for i, byte in enumerate(buf):
        row.append(f"0x{byte:02x}")
        if len(row) == 16:
            lines.append("\t" + ", ".join(row) + ",")
            row = []
    if row:
        lines.append("\t" + ", ".join(row) + ",")
    lines.append("};")
    lines.append("__attribute__((constructor)) static void efw_image_ctor(void)")
    lines.append("{")
    lines.append("\tEFW_InstallImage(efw_image_bytes, (uint32_t)sizeof efw_image_bytes);")
    lines.append("}")
    lines.append("")
    dest.write_text("\n".join(lines))


def write_ghidra_script(server: list[str], client: list[str]) -> None:
    def arr(names: list[str]) -> str:
        return ",\n".join(f'\t\t"{n}"' for n in names)

    java = f"""// Name enginefuncs_t / cl_enginefunc_t slots from GiveFnptrsToDll / Initialize.
// Server: 0x8f dwords at 0x10121e08, gpGlobals at 0x10122044.
// Client: 0x62 dwords at 0x100a4ff0. Keep this project; do not -deleteProject.
//@category EFW
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;

public class ApplyEngineTable extends GhidraScript {{
	private static final String[] SERVER = {{
{arr(server)}
	}};
	private static final String[] CLIENT = {{
{arr(client)}
	}};

	@Override
	public void run() throws Exception {{
		String name = currentProgram.getName();
		if (name.contains("EscapeFromWoomera")) {{
			labelTable(0x10121e08L, SERVER);
			createLabel(toAddr(0x10122044L), "gpGlobals", true);
			println("labeled server enginefuncs " + SERVER.length + " + gpGlobals");
		}} else {{
			labelTable(0x100a4ff0L, CLIENT);
			println("labeled client cl_enginefunc " + CLIENT.length);
		}}
	}}

	private void labelTable(long base, String[] names) throws Exception {{
		for (int i = 0; i < names.length; i++) {{
			Address addr = toAddr(base + (long) i * 4L);
			createLabel(addr, "eng_" + names[i], true);
		}}
	}}
}}
"""
    out = DECOMP / "ghidra_scripts" / "ApplyEngineTable.java"
    out.write_text(java)


def spirit_report(pe: pefile.PE, slots: list[str]) -> str:
    sohl = {
        "info_alias", "info_group", "multi_alias", "trigger_changealias",
        "locus_alias", "calc_ratio", "button_target", "scripted_action",
        "scripted_tanksequence", "scripted_trainsequence", "env_warpball",
        "env_beamtrail",
    }
    exports = []
    if hasattr(pe, "DIRECTORY_ENTRY_EXPORT"):
        for exp in pe.DIRECTORY_ENTRY_EXPORT.symbols:
            if exp.name:
                exports.append(exp.name.decode("latin1"))
    plain = [n for n in exports if not n.startswith("?")]
    present = sorted(sohl & set(plain))
    maps = map_classnames()
    used = sorted(sohl & set(maps))
    lines = [
        "Spirit match",
        "============",
        f"enginefuncs slots copied by GiveFnptrsToDll: {len(slots)} "
        f"(last {slots[-1]})",
        f"pfnSetModel is slot {slots.index('pfnSetModel')} at "
        f"{SERVER_ENG_BASE + slots.index('pfnSetModel') * 4:#x}",
        f"pfnRegUserMsg is slot {slots.index('pfnRegUserMsg')}",
        f"gpGlobals stored at {SERVER_GLOBALS:#x}",
        f"exports: {len(exports)} ({len(plain)} unmangled class / C names)",
        "Spirit-only classnames present in the export table:",
        *[f"  {n}" for n in present],
        "Spirit-only classnames spawned by the three prototype maps:",
        *([f"  {n}" for n in used] or ["  (none)"]),
        "Map classnames:",
        *[f"  {n}" for n in sorted(maps)],
        "",
        "The prototype maps spawn stock Half-Life classes plus monster_refugee,",
        "monster_patrol_guard, and efw_Marker. Those three are the overlay.",
        "Spirit-only entities are in the DLL because it was linked as spirit.dll,",
        "and the export table already names them. They are not part of the lift,",
        "and the wasm server stays on hlsdk-portable, which already has the",
        "classes these maps spawn.",
        "",
    ]
    return "\n".join(lines)


def map_classnames() -> set[str]:
    import struct
    names: set[str] = set()
    with zipfile.ZipFile(ZIP) as zf:
        for map_name in (
            "EscapeFromWoomera_v084/maps/efw_prototype_level1.bsp",
            "EscapeFromWoomera_v084/maps/efw_prototype_level2.bsp",
            "EscapeFromWoomera_v084/maps/efw_prototype_level3.bsp",
        ):
            data = zf.read(map_name)
            off, length = struct.unpack_from("<II", data, 4)
            ents = data[off : off + length].split(b"\x00", 1)[0].decode("latin1", "replace")
            for line in ents.splitlines():
                parts = line.strip().split('"')
                if len(parts) >= 4 and parts[1] == "classname":
                    names.add(parts[3])
    return names


def lift_one(
    dll: Path,
    ghidra_c: Path,
    slots: list[str],
    eng_base: int,
    out_c: Path,
    image_c: Path | None,
) -> str:
    pe = pefile.PE(str(dll))
    image_base, image = build_image(pe)
    text = ghidra_c.read_text(encoding="utf-8", errors="replace")
    lifted = transform(text, slots, eng_base, image_base, image_base + len(image))
    lifted = promote_void_returns(lifted)
    lifted = rewrite_float_args(lifted)
    lifted = prefix_defined(lifted)
    sigs = function_sigs(lifted)
    defs = sorted({sig_name(sig) for sig in sigs if sig_name(sig)})
    stubs = emit_stubs(lifted)
    keep = emit_keep([n for n in defs if n not in {"EFW_LiftKeep"}])
    protos = "\n".join(f"{sig};" for sig in sigs)
    extras = ""
    if "iRam00000004" in lifted:
        extras += "static int iRam00000004;\n"
    # Ghidra names for stack slots it failed to declare (efw_DebugPrint).
    declared = set(re.findall(r"\b(stack0x[0-9a-fA-F]+)\s*\[", lifted))
    for name in sorted(set(re.findall(r"\b(stack0x[0-9a-fA-F]+)\b", lifted))):
        if name in declared:
            continue
        extras += f"static uint8_t {name}[64];\n"
    header = textwrap.dedent(
        f"""\
        /* Mechanical lift of {ghidra_c.name}. Generated by decompile/lift_overlay.py.
         * Do not edit. Engine calls are EFW_EngSlots()[n] (see engine_slots_*.txt).
         * Absolute addresses are relocated with EFW_VA into the DLL image.
         */
        #include "efw_lift_prelude.h"

        uint8_t efw_mem[EFW_MEM_SIZE];

        """
    )
    out_c.parent.mkdir(parents=True, exist_ok=True)
    out_c.write_text(header + extras + protos + "\n" + stubs + "\n" + lifted + "\n" + keep)
    if image_c is not None:
        size_h = GAME / "efw_lift_size.h"
        rdata = 0
        for section in pe.sections:
            name = section.Name.decode("latin1").strip("\x00")
            if name == ".rdata":
                rdata = image_base + section.VirtualAddress
        size_h.write_text(
            f"#define EFW_IMAGE_BASE 0x{image_base:08x}u\n"
            f"#define EFW_MEM_SIZE 0x{len(image):x}u\n"
            f"#define EFW_RELOC_LO 0x{rdata:08x}u\n"
        )
        write_image_c(image, image_c)
    pe.close()
    return lifted


def compile_and_run() -> int:
    cmd = [
        "gcc", "-m32", "-std=c99", "-O0", "-w", "-fno-strict-aliasing",
        "-I", str(GAME),
        str(GAME / "efw_lift.c"),
        str(GAME / "efw_lift_host.c"),
        str(GAME / "efw_image.c"),
        str(GAME / "efw_lift_test.c"),
        "-o", "/tmp/efw_lift_test", "-lm",
    ]
    print("compile", " ".join(cmd[:8]), "...")
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        sys.stderr.write(proc.stderr)
        sys.stderr.write(proc.stdout)
        return proc.returncode
    run = subprocess.run(["/tmp/efw_lift_test"], capture_output=True, text=True)
    sys.stdout.write(run.stdout)
    sys.stderr.write(run.stderr)
    return run.returncode


def main() -> int:
    server_slots = load_slots(DECOMP / "engine_slots_server.txt", SERVER_ENG_COUNT)
    client_slots = load_slots(DECOMP / "engine_slots_client.txt", CLIENT_ENG_COUNT)
    server_dll = extract_dll(
        "EscapeFromWoomera_v084/dlls/EscapeFromWoomera.dll",
        DECOMP / "binaries" / "EscapeFromWoomera.dll",
    )
    client_dll = extract_dll(
        "EscapeFromWoomera_v084/cl_dlls/client.dll",
        DECOMP / "binaries" / "client.dll",
    )
    server_c = lift_one(
        server_dll,
        DECOMP / "out" / "EscapeFromWoomera_ghidra_efw.c",
        server_slots,
        SERVER_ENG_BASE,
        GAME / "efw_lift.c",
        GAME / "efw_image.c",
    )
    lift_one(
        client_dll,
        DECOMP / "out" / "client_ghidra_efw.c",
        client_slots,
        CLIENT_ENG_BASE,
        DECOMP / "recovered" / "client_lift.c",
        None,
    )
    write_ghidra_script(server_slots, client_slots)
    pe = pefile.PE(str(server_dll))
    report = spirit_report(pe, server_slots)
    pe.close()
    (DECOMP / "out" / "lift_report.txt").write_text(report)
    print(report)
    named = len(re.findall(r"EFW_EngSlots\(\)\[\d+\]", server_c))
    print(f"server engine call sites named: {named}")
    if "EFW_EngSlots()[2]" not in server_c:
        print("missing pfnSetModel slot", file=sys.stderr)
        return 1
    if "EFW_EngSlots()[75]" not in server_c:
        print("missing pfnRegUserMsg slot", file=sys.stderr)
        return 1
    if "--test" in sys.argv:
        return compile_and_run()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
