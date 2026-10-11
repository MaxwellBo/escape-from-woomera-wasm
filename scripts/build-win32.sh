#!/usr/bin/env bash
# Cross-compile Xash3D FWGS for 32-bit Windows (MinGW) with the software
# renderer. The browser host is Wine via Boxedwine, which has no OpenGL, so
# ref_soft presents through SDL's GDI window surface.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
XASH_COMMIT="${XASH_COMMIT:-51353ff8d65e58300eeb2bd014867673f1f367c6}"
SDL_VERSION="${SDL_VERSION:-2.32.10}"
SRC="${XASH_SRC:-$ROOT/third_party/xash3d-fwgs}"
SDL_ROOT="${SDL_ROOT:-$ROOT/third_party/SDL2-$SDL_VERSION}"
DEST="${XASH_WIN32_DEST:-$ROOT/third_party/xash-win32}"
MINGW_RUNTIME="${MINGW_RUNTIME:-/usr/lib/gcc/i686-w64-mingw32/13-win32}"

if ! command -v i686-w64-mingw32-gcc >/dev/null; then
  echo "i686-w64-mingw32-gcc is required (gcc-mingw-w64-i686)" >&2
  exit 1
fi

mkdir -p "$(dirname "$SRC")"
if [[ ! -d "$SRC/.git" ]]; then
  git init "$SRC"
  git -C "$SRC" remote add origin https://github.com/FWGS/xash3d-fwgs.git
fi
git -C "$SRC" fetch --depth 1 origin "$XASH_COMMIT"
git -C "$SRC" checkout --force FETCH_HEAD
git -C "$SRC" submodule update --init --recursive --depth 1
git -C "$SRC" apply --check "$ROOT/scripts/xash-mingw-i386-rename.patch"
git -C "$SRC" apply "$ROOT/scripts/xash-mingw-i386-rename.patch" || true
git -C "$SRC" apply --check "$ROOT/scripts/xash-console-log.patch"
git -C "$SRC" apply "$ROOT/scripts/xash-console-log.patch" || true
git -C "$SRC" apply --check "$ROOT/scripts/xash-win32-stat-slash.patch"
git -C "$SRC" apply "$ROOT/scripts/xash-win32-stat-slash.patch" || true

if [[ ! -f "$SDL_ROOT/i686-w64-mingw32/include/SDL2/SDL.h" ]]; then
  mkdir -p "$ROOT/third_party"
  curl -fsSL -L -o "$ROOT/third_party/SDL2-devel-$SDL_VERSION-mingw.tar.gz" \
    "https://github.com/libsdl-org/SDL/releases/download/release-$SDL_VERSION/SDL2-devel-$SDL_VERSION-mingw.tar.gz"
  tar -C "$ROOT/third_party" -xzf "$ROOT/third_party/SDL2-devel-$SDL_VERSION-mingw.tar.gz"
fi

WRAP="$ROOT/third_party/mingw-bin"
mkdir -p "$WRAP"
cat > "$WRAP/windres" << 'EOF'
#!/bin/sh
# argv0 must be the prefixed windres, and --use-temp-file avoids the shell
# eating -DALLOCA_H=<malloc.h>.
exec /usr/bin/i686-w64-mingw32-windres --use-temp-file "$@"
EOF
chmod +x "$WRAP/windres"

export PATH="$WRAP:$PATH"
export CC=i686-w64-mingw32-gcc
export CXX=i686-w64-mingw32-g++
export AR=i686-w64-mingw32-ar
export RANLIB=i686-w64-mingw32-ranlib

cd "$SRC"
./waf configure -T release \
  --sdl2="$(dirname "$SDL_ROOT")/SDL2-$SDL_VERSION" \
  --enable-soft --disable-gl --disable-werror
./waf build -j"$(nproc)"
rm -rf "$DEST"
./waf install --destdir="$DEST"
# GUI subsystem leaves stdout detached from the cmd.exe Boxedwine captures.
i686-w64-mingw32-objcopy --subsystem console "$DEST/xash3d.exe"
# client.dll's CRT probes the stack a page at a time. Commit the whole
# reserve up front so that probe does not depend on guard-page growth.
python3 - "$DEST/xash3d.exe" << 'PY'
import struct, sys
from pathlib import Path
path = Path(sys.argv[1])
data = bytearray(path.read_bytes())
e = struct.unpack_from("<I", data, 0x3C)[0]
off = e + 24 + 72
reserve, commit = struct.unpack_from("<II", data, off)
struct.pack_into("<I", data, off + 4, reserve)
path.write_bytes(data)
print(f"stack reserve={reserve:#x} commit={reserve:#x} (was {commit:#x})")
PY
cp -f "$SDL_ROOT/i686-w64-mingw32/bin/SDL2.dll" "$DEST/"
cp -f "$MINGW_RUNTIME/libgcc_s_dw2-1.dll" "$MINGW_RUNTIME/libstdc++-6.dll" "$DEST/"

# waf's vgui.dll is the MinGW ABI. client.dll needs the MSVC one.
XASH_SRC="$SRC" VGUI_DLL="$DEST/vgui.dll" "$ROOT/scripts/build-vgui-msvc.sh"
"$ROOT/scripts/build-psapi-shim.sh"

python3 "$ROOT/scripts/pack-wine.py" --engine "$DEST" --mingw-runtime "$MINGW_RUNTIME"
echo "Windows build installed at $DEST"
