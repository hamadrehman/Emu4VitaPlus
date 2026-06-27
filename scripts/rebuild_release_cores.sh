#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_BASE_DIR="${BUILD_BASE_DIR:-$ROOT/build-release-cores}"
VPK_PATH="${VPK_PATH:-$ROOT/out/Emu4VitaPlus_v0.68_boot-uri.vpk}"
VITASDK="${VITASDK:-/usr/local/vitasdk}"
BUILD_ONLY="${BUILD_ONLY:-0}"
STAGE_OUT_DIR="${STAGE_OUT_DIR:-$ROOT/out/cores}"

export VITASDK
export PATH="$VITASDK/bin:$PATH"

if [[ ! -x "$VITASDK/bin/arm-vita-eabi-gcc" ]]; then
  echo "Missing VitaSDK compiler: $VITASDK/bin/arm-vita-eabi-gcc" >&2
  exit 1
fi

if [[ "$BUILD_ONLY" != "1" && ! -f "$VPK_PATH" ]]; then
  echo "Missing VPK to patch: $VPK_PATH" >&2
  exit 1
fi

if [[ "$BUILD_ONLY" == "1" ]]; then
  mkdir -p "$STAGE_OUT_DIR"
fi

require_cmd() {
  local cmd="$1"
  if ! command -v "$cmd" >/dev/null 2>&1; then
    echo "Missing required host tool: $cmd" >&2
    exit 1
  fi
}

declare -A TARGETS=(
  [genesis_plus_gx]="eboot_GenesisPlusGX.bin"
  [picodrive]="eboot_PicoDrive.bin"
  [fba_lite]="eboot_FBALite.bin"
  [snes9x]="eboot_Snes9x.bin"
  [snes9x2005_plus]="eboot_Snes9x2005Plus.bin"
  [chimerasnes]="eboot_ChimeraSNES.bin"
)

declare -A APP_DIRS=(
  [genesis_plus_gx]="genesis_plus_gx"
  [picodrive]="picodrive"
  [fba_lite]="fba_lite"
  [snes9x]="snes9x"
  [snes9x2005_plus]="snes9x2005_plus"
  [chimerasnes]="chimerasnes"
)

declare -A VPK_ENTRIES=(
  [genesis_plus_gx]="eboot_genesis_plus_gx.self"
  [picodrive]="eboot_picodrive.self"
  [fba_lite]="eboot_fba_lite.self"
  [snes9x]="eboot_snes9x.self"
  [snes9x2005_plus]="eboot_snes9x2005_plus.self"
  [chimerasnes]="eboot_chimerasnes.self"
)

declare -A BUILD_TARGETS=(
  [genesis_plus_gx]="VPK_GenesisPlusGX"
  [picodrive]="VPK_PicoDrive"
  [fba_lite]="VPK_FBALite"
  [snes9x]="VPK_Snes9x"
  [snes9x2005_plus]="VPK_Snes9x2005Plus"
  [chimerasnes]="VPK_ChimeraSNES"
)

CORES=("$@")
if [[ ${#CORES[@]} -eq 0 ]]; then
  CORES=(genesis_plus_gx picodrive fba_lite snes9x snes9x2005_plus chimerasnes)
fi

for core in "${CORES[@]}"; do
  if [[ -z "${TARGETS[$core]:-}" ]]; then
    echo "Unknown core: $core" >&2
    echo "Supported: ${!TARGETS[*]}" >&2
    exit 1
  fi

  if [[ "$core" == "picodrive" ]]; then
    require_cmd gcc
    require_cmd g++
    require_cmd file
    require_cmd readelf
    require_cmd make
    sed -i 's/\r$//' "$ROOT/cores/picodrive/tools/mkoffsets.sh"
  fi

  target="${TARGETS[$core]}"
  app_dir="${APP_DIRS[$core]}"
  entry="${VPK_ENTRIES[$core]}"
  build_target="${BUILD_TARGETS[$core]}"
  build_dir="$BUILD_BASE_DIR/$core"
  output_bin="$build_dir/apps/$app_dir/$target"

  rm -rf "$build_dir"
  echo "Configuring $core build tree..."
  cmake -S "$ROOT" -B "$build_dir" \
    -DBUILD="$core" \
    -DWITH_LOG=OFF \
    -DMZ_FETCH_LIBS=OFF \
    -DMZ_FORCE_FETCH_LIBS=OFF \
    -DZLIB_COMPAT=ON \
    -DZLIB_INCLUDE_DIR="$VITASDK/arm-vita-eabi/include" \
    -DZLIB_LIBRARY="$VITASDK/arm-vita-eabi/lib/libz.a" \
    -DFETCHCONTENT_UPDATES_DISCONNECTED=ON

  echo "Building $core -> $build_target"
  cmake --build "$build_dir" --target "$build_target" -- -j"$(nproc)"

  if [[ ! -f "$output_bin" ]]; then
    echo "Expected build output missing: $output_bin" >&2
    exit 1
  fi

  if [[ "$BUILD_ONLY" == "1" ]]; then
    cp "$output_bin" "$STAGE_OUT_DIR/$entry"
    echo "Staged: $STAGE_OUT_DIR/$entry"
  else
    tmpdir="$(mktemp -d)"
    cp "$output_bin" "$tmpdir/$entry"
    zip -qj "$VPK_PATH" "$tmpdir/$entry"
    rm -rf "$tmpdir"
  fi
done

if [[ "$BUILD_ONLY" == "1" ]]; then
  echo "Built cores staged in: $STAGE_OUT_DIR"
else
  echo "Updated:"
  echo "  $VPK_PATH"
fi
