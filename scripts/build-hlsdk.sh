#!/usr/bin/env bash
# Overlay EFW sources onto hlsdk-portable and build client/server WASM for Xash.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SDK="${HLSDK_PATH:-$ROOT/third_party/hlsdk-portable}"
EMSDK="${EMSDK_PATH:-$ROOT/third_party/emsdk}"
OUT="$ROOT/public/hlsdk"

if [[ ! -d "$SDK/.git" && ! -f "$SDK/CMakeLists.txt" ]]; then
  git clone --depth 1 https://github.com/FWGS/hlsdk-portable.git "$SDK"
fi
if [[ -d "$SDK/.git" ]]; then
  git -C "$SDK" submodule update --init --recursive
fi

python3 "$ROOT/game-logic/overlay.py" "$SDK"

if [[ ! -f "$EMSDK/emsdk" ]]; then
  echo "emsdk not found at $EMSDK" >&2
  exit 1
fi
# shellcheck disable=SC1091
source "$EMSDK/emsdk_env.sh"

BUILD="$SDK/build-wasm"
mkdir -p "$BUILD"
pushd "$BUILD" >/dev/null
emcmake cmake "$SDK" \
  -DCMAKE_BUILD_TYPE=Release \
  -D64BIT=OFF \
  -DBUILD_CLIENT=ON \
  -DBUILD_SERVER=ON \
  -DGOLDSOURCE_SUPPORT=OFF \
  -DCMAKE_STRIP=/bin/true
cmake --build . -j"$(nproc)"
popd >/dev/null

mkdir -p "$OUT"
mapfile -t wasm < <(find "$BUILD" -type f \( -name '*emscripten_wasm32.wasm' -o -name '*emscripten_wasm32.so' -o -name 'client.wasm' -o -name 'hl.wasm' -o -name 'server.wasm' \))
echo "built wasm files:"
printf '  %s\n' "${wasm[@]}"

client=""
server=""
for f in "${wasm[@]}"; do
  base="$(basename "$f")"
  case "$base" in
    client_emscripten_wasm32.wasm|client_emscripten_wasm32.so|client.wasm) client="$f" ;;
    hl_emscripten_wasm32.wasm|hl_emscripten_wasm32.so|hl.wasm|server_emscripten_wasm32.wasm|server_emscripten_wasm32.so|server.wasm) server="$f" ;;
  esac
done
if [[ -z "$client" || -z "$server" ]]; then
  echo "could not locate client/server wasm in $BUILD" >&2
  find "$BUILD" -type f | head -80 >&2
  exit 1
fi
cp "$client" "$OUT/client.wasm"
cp "$server" "$OUT/server.wasm"
echo "installed:"
ls -la "$OUT/client.wasm" "$OUT/server.wasm"
