#!/bin/bash
# Re-vendor third-party binaries. Provenance:
# - Engine WASM: xash3d-fwgs@1.2.2 on npm (tarball is immutable upstream)
# - Game-logic WASM: hlsdk-portable@0.1.3 via the npmmirror registry mirror
#   (upstream npm unlisted it 2026-09-08; the FWGS source fallback is
#   https://github.com/FWGS/hlsdk-portable built with Emscripten)
# - Mod: EscapeFromWoomera_v084.zip from the archived official site
#   http://www.ljudmila.org/~selectparks/archive/escapefromwoomera/
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

cd "$TMP"
npm pack xash3d-fwgs@1.2.2 >/dev/null
tar xzf xash3d-fwgs-*.tgz
D=package/dist
cp "$D/xash.wasm" "$D/filesystem_stdio.wasm" "$D/libmenu.wasm" \
   "$D/libref_webgl2.wasm" "$D/libref_soft.wasm" "$ROOT/public/engine/"
cp "$D/valve/extras.pk3" "$ROOT/public/engine/"

curl -sSL -o hlsdk.tgz \
  "https://registry.npmmirror.com/hlsdk-portable/download/hlsdk-portable-0.1.3.tgz"
tar xzf hlsdk.tgz
cp package/dist/valve/dlls/hl_emscripten_wasm32.wasm "$ROOT/public/hlsdk/server.wasm"
cp package/dist/valve/cl_dlls/client_emscripten_wasm32.wasm "$ROOT/public/hlsdk/client.wasm"

curl -sSL -o "$ROOT/public/woomera.zip" \
  "http://www.ljudmila.org/~selectparks/archive/escapefromwoomera/EscapeFromWoomera_v084.zip"

echo "vendored OK:"
ls -la "$ROOT/public/engine" "$ROOT/public/hlsdk" "$ROOT/public/woomera.zip"
