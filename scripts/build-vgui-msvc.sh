#!/usr/bin/env bash
# freevgui built for the MSVC i386 C++ ABI.
#
# The original client.dll imports MSVC-mangled thiscall methods
# (?setPos@Panel@vgui@@UAEXHH@Z and hundreds more). The MinGW vgui.dll
# waf produces uses the Itanium ABI, so Wine reports every one of those
# imports as missing. This script compiles the same sources with
# clang --target=i686-windows-msvc and exports the names client.dll imports.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="${XASH_SRC:-$ROOT/third_party/xash3d-fwgs}/3rdparty/freevgui"
HELPER="$ROOT/scripts/msvc-vgui"
OUT="${VGUI_DLL:-$ROOT/third_party/vgui-msvc.dll}"
WORK="${VGUI_WORK:-/tmp/vgui-msvc-build}"
CLIENT="${CLIENT_DLL:-}"

if [[ ! -f "$SRC/vgui.cpp" ]]; then
  echo "freevgui sources not found at $SRC (set XASH_SRC)" >&2
  exit 1
fi
for tool in clang++ lld-link python3 i686-w64-mingw32-objdump; do
  command -v "$tool" >/dev/null || { echo "missing $tool" >&2; exit 1; }
done

if [[ -z "$CLIENT" ]]; then
  mkdir -p "$WORK"
  CLIENT="$WORK/client.dll"
  python3 - "$ROOT/public/woomera.zip" "$CLIENT" << 'PY'
import sys, zipfile
src, dest = sys.argv[1], sys.argv[2]
z = zipfile.ZipFile(src)
name = next(n for n in z.namelist() if n.replace("\\", "/").lower().endswith("cl_dlls/client.dll"))
open(dest, "wb").write(z.read(name))
PY
fi

rm -rf "$WORK/objs"
mkdir -p "$WORK/objs" "$(dirname "$OUT")"
FLAGS=(
  --target=i686-windows-msvc
  -include "$HELPER/declspec_compat.h"
  -include "$HELPER/stdio_compat.h"
  -D__GNUC__=4 -D__GNUC_MINOR__=2 -D_X86_ -D_MSVCRT_
  -frtti -fno-exceptions -O2
  -Wno-ignored-attributes -Wno-unknown-attributes
  -I "$HELPER/include"
  -I "$SRC"
  -I /usr/i686-w64-mingw32/include
  -I /usr/lib/gcc/i686-w64-mingw32/13-win32/include
)

objs=()
while IFS= read -r -d '' src; do
  rel="${src#"$SRC"/}"
  case "$rel" in
    platform/posix/*|platform/xash3d-fwgs/*) continue ;;
  esac
  obj="$WORK/objs/${rel//\//_}.obj"
  echo "cc $rel"
  clang++ "${FLAGS[@]}" -c "$src" -o "$obj"
  objs+=("$obj")
done < <(find "$SRC" -name '*.cpp' -print0)

clang++ --target=i686-windows-msvc -c "$HELPER/abi.cpp" -O2 -o "$WORK/objs/abi.obj"

python3 - "$CLIENT" "$WORK/vgui.def" << 'PY'
import re, subprocess, sys
client, dest = sys.argv[1], sys.argv[2]
text = subprocess.check_output(["i686-w64-mingw32-objdump", "-p", client], text=True, errors="replace")
names, grab, seen = [], False, set()
for line in text.splitlines():
    if "DLL Name: vgui.dll" in line:
        grab = True
        continue
    if grab and "DLL Name:" in line:
        break
    match = re.search(r"\s(\?[^\s]+)$", line)
    if grab and match and match.group(1) not in seen:
        seen.add(match.group(1))
        names.append(match.group(1))
if not names:
    raise SystemExit(f"no vgui imports in {client}")
open(dest, "w").write("LIBRARY vgui\nEXPORTS\n" + "\n".join(names) + "\n")
print(f"{len(names)} vgui exports")
PY

GCCLIB="$(dirname "$(i686-w64-mingw32-gcc -print-libgcc-file-name)")"
lld-link /dll /nodefaultlib /safeseh:no /entry:DllMain \
  /def:"$WORK/vgui.def" /out:"$OUT" \
  "/alternatename:??_7type_info@@6B@=_ti_vftable" \
  "${objs[@]}" "$WORK/objs/abi.obj" \
  /usr/i686-w64-mingw32/lib/libmsvcrt.a \
  /usr/i686-w64-mingw32/lib/libmingwex.a \
  "$GCCLIB/libgcc.a" \
  /usr/i686-w64-mingw32/lib/libkernel32.a \
  /usr/i686-w64-mingw32/lib/libuser32.a \
  /usr/i686-w64-mingw32/lib/libgdi32.a
if [[ -d "$ROOT/third_party/xash-win32" ]]; then
  cp -f "$OUT" "$ROOT/third_party/xash-win32/vgui.dll"
fi

python3 - "$OUT" "$ROOT/public/boxedwine/woomera.zip" << 'PY'
import sys, zipfile
from pathlib import Path
dll, zip_path = Path(sys.argv[1]), Path(sys.argv[2])
if not zip_path.is_file():
    print(f"no {zip_path}; left the dll at {dll}")
    raise SystemExit(0)
data = dll.read_bytes()
tmp = zip_path.with_suffix(".zip.tmp")
with zipfile.ZipFile(zip_path) as src, zipfile.ZipFile(tmp, "w") as dst:
    found = False
    for info in src.infolist():
        payload = data if info.filename == "vgui.dll" else src.read(info)
        if info.filename == "vgui.dll":
            found = True
        dst.writestr(info.filename, payload, compress_type=zipfile.ZIP_DEFLATED, compresslevel=6)
    if not found:
        dst.writestr("vgui.dll", data, compress_type=zipfile.ZIP_DEFLATED, compresslevel=6)
tmp.replace(zip_path)
print(f"patched vgui.dll in {zip_path} ({len(data)} bytes)")
PY
echo "MSVC-ABI vgui.dll at $OUT"
