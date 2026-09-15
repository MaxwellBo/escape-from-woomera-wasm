#!/bin/bash
# Re-vendor third-party binaries. Provenance:
# - Engine WASM: xash3d-fwgs@1.2.2 on npm (tarball is immutable upstream)
# - Game-logic WASM: rebuilt from FWGS/hlsdk-portable plus game-logic/ via
#   scripts/build-hlsdk.sh (do not overwrite public/hlsdk/{client,server}.wasm
#   here — those are the EFW overlay modules).
# - Mod: EscapeFromWoomera_v084.zip from the archived official site
#   http://www.ljudmila.org/~selectparks/archive/escapefromwoomera/
# - Base-game valve/: official Half-Life: Uplink demo (Valve/Sierra, 1999),
#   freely distributed installer archived at
#   https://archive.org/download/Half-lifeUplink/hluplink.exe
#   extracted with REWise (https://codeberg.org/CYBERDEV/REWise) and packed by
#   scripts/pack-valve.py (Win32 DLLs + Uplink maps omitted).
# - delta.lst: ValveSoftware/halflife network/delta.lst (Half-Life 1 SDK,
#   redistributable per that LICENSE; Uplink does not ship this file).
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

cd "$TMP"
npm pack xash3d-fwgs@1.2.2 >/dev/null
tar xzf xash3d-fwgs-*.tgz
D=package/dist
mkdir -p "$ROOT/public/engine" "$ROOT/public/hlsdk"
cp "$D/xash.wasm" "$D/filesystem_stdio.wasm" "$D/libmenu.wasm" \
   "$D/libref_webgl2.wasm" "$D/libref_soft.wasm" "$ROOT/public/engine/"
cp "$D/valve/extras.pk3" "$ROOT/public/engine/"

cp "$ROOT/scripts/hlsdk-files/delta.lst" "$ROOT/public/hlsdk/delta.lst"
cp "$ROOT/scripts/hlsdk-files/LICENSE" "$ROOT/public/hlsdk/LICENSE"
# client.wasm / server.wasm are produced by scripts/build-hlsdk.sh (EFW overlay).

curl -sSL -o "$ROOT/public/woomera.zip" \
  "http://www.ljudmila.org/~selectparks/archive/escapefromwoomera/EscapeFromWoomera_v084.zip"

# --- Half-Life: Uplink demo → public/valve.zip --------------------------------
curl -sSL -o hluplink.exe \
  "https://archive.org/download/Half-lifeUplink/hluplink.exe"
git clone --depth 1 https://codeberg.org/CYBERDEV/REWise.git rewise
# REWise's Makefile puts -lz before the objects; link explicitly.
make -C rewise src/rewise.o src/CP1252.o src/print.o src/reader.o \
  src/exefile.o src/pkzip.o src/wildcard.o src/wisescript.o src/wiseoverlay.o
gcc -o rewise/rewise -I rewise/src rewise/src/*.o -lz
mkdir -p uplink
rewise/rewise -x uplink hluplink.exe
python3 "$ROOT/scripts/pack-valve.py" uplink/MAINDIR/valve "$ROOT/public/valve.zip"

echo "vendored OK:"
ls -la "$ROOT/public/engine" "$ROOT/public/hlsdk" \
  "$ROOT/public/woomera.zip" "$ROOT/public/valve.zip"
