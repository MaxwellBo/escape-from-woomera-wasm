#!/usr/bin/env bash
# Replace Wine 6's placeholder psapi.dll with a real PE that exports the
# three symbols xash.dll imports. The placeholder has no export table and
# no psapi.dll.so, so the loader reports PSAPI.DLL as missing.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="${PSAPI_DLL:-$ROOT/third_party/psapi.dll}"
WINE_ZIP="${WINE_ZIP:-$ROOT/public/boxedwine/wine6.zip}"

command -v i686-w64-mingw32-gcc >/dev/null || { echo "missing i686-w64-mingw32-gcc" >&2; exit 1; }
mkdir -p "$(dirname "$OUT")"
i686-w64-mingw32-gcc -shared -O2 -s -o "$OUT" "$ROOT/scripts/psapi/psapi.c" -Wl,--kill-at
i686-w64-mingw32-objdump -p "$OUT" | awk '/DLL Name:/{print} /EnumProcessModules|GetModuleBaseNameA|GetModuleInformation/{print}'

python3 - "$OUT" "$WINE_ZIP" << 'PY'
import sys, zipfile
from pathlib import Path
dll, zip_path = Path(sys.argv[1]), Path(sys.argv[2])
data = dll.read_bytes()
if not zip_path.is_file():
    print(f"no {zip_path}; left {dll}")
    raise SystemExit(0)
entry = "home/username/.wine/drive_c/windows/system32/psapi.dll"
tmp = zip_path.with_suffix(".zip.tmp")
with zipfile.ZipFile(zip_path) as src, zipfile.ZipFile(tmp, "w") as dst:
    found = False
    for info in src.infolist():
        payload = data if info.filename == entry else src.read(info)
        if info.filename == entry:
            found = True
        dst.writestr(info.filename, payload, compress_type=info.compress_type)
    if not found:
        dst.writestr(entry, data)
tmp.replace(zip_path)
print(f"patched {entry} in {zip_path} ({len(data)} bytes)")
PY
