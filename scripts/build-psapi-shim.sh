#!/usr/bin/env bash
# Patch the Wine 6 root so the Windows engine can keep running:
# - psapi.dll is a placeholder with no builtin, and xash.dll imports it
# - wmic.exe is a placeholder with no builtin, and opening it aborts Boxedwine
# - ntdll uses xsave/xrstor, which this emulator rejects; fxsave is enough
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="${PSAPI_DLL:-$ROOT/third_party/psapi.dll}"
WMIC="${WMIC_EXE:-$ROOT/third_party/wmic.exe}"
WINE_ZIP="${WINE_ZIP:-$ROOT/public/boxedwine/wine6.zip}"

command -v i686-w64-mingw32-gcc >/dev/null || { echo "missing i686-w64-mingw32-gcc" >&2; exit 1; }
mkdir -p "$(dirname "$OUT")"
i686-w64-mingw32-gcc -shared -O2 -s -o "$OUT" "$ROOT/scripts/psapi/psapi.c" -Wl,--kill-at
i686-w64-mingw32-gcc -O2 -s -mconsole -o "$WMIC" "$ROOT/scripts/wine6/wmic.c"
i686-w64-mingw32-objdump -p "$OUT" | awk '/DLL Name:/{print} /EnumProcessModules|GetModuleBaseNameA|GetModuleInformation/{print}'

python3 - "$OUT" "$WMIC" "$WINE_ZIP" << 'PY'
import sys, zipfile
from pathlib import Path
psapi, wmic, zip_path = map(Path, sys.argv[1:])
if not zip_path.is_file():
    print(f"no {zip_path}")
    raise SystemExit(0)
psapi_bytes = psapi.read_bytes()
wmic_bytes = wmic.read_bytes()
psapi_entry = "home/username/.wine/drive_c/windows/system32/psapi.dll"
wmic_entry = "home/username/.wine/drive_c/windows/system32/wbem/wmic.exe"
ntdll_entry = "opt/wine/lib/wine/ntdll.so"
# xsave/xrstor encodings verified in Wine 6's ntdll .text, rewritten as fxsave/fxrstor.
ntdll_patches = {
    319642: (bytes.fromhex("0fae642470"), bytes.fromhex("0fae042470")),
    317639: (bytes.fromhex("0faea900feffff"), bytes.fromhex("0fae8900feffff")),
}
tmp = zip_path.with_suffix(".zip.tmp")
with zipfile.ZipFile(zip_path) as src, zipfile.ZipFile(tmp, "w") as dst:
    for info in src.infolist():
        payload = src.read(info)
        if info.filename == psapi_entry:
            payload = psapi_bytes
        elif info.filename == wmic_entry:
            payload = wmic_bytes
        elif info.filename == ntdll_entry:
            blob = bytearray(payload)
            for off, (old, new) in ntdll_patches.items():
                got = bytes(blob[off:off + len(old)])
                if got == new:
                    continue
                if got != old:
                    raise SystemExit(f"ntdll.so mismatch at {off}: {got.hex()}")
                blob[off:off + len(new)] = new
            payload = bytes(blob)
        dst.writestr(info, payload)
tmp.replace(zip_path)
print(f"patched psapi, wmic and ntdll in {zip_path}")
PY
