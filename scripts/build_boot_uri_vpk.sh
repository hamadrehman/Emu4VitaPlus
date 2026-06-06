#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VITASDK="${VITASDK:-/usr/local/vitasdk}"
OUT_DIR="${OUT_DIR:-"$ROOT/out"}"
BUILD_DIR="${BUILD_DIR:-"$ROOT/build-arch"}"
RELEASE_DIR="${RELEASE_DIR:-"$ROOT/release"}"
RELEASE_VPK="${RELEASE_VPK:-"$RELEASE_DIR/Emu4VitaPlus_v0.68.vpk"}"
STAGE_DIR="${STAGE_DIR:-"$ROOT/build/repack_vpk"}"
OUTPUT_VPK="${OUTPUT_VPK:-"$OUT_DIR/Emu4VitaPlus_v0.68_boot-uri.vpk"}"

export VITASDK
export PATH="$VITASDK/bin:$PATH"
VITA_MAKE_FSELF="$VITASDK/bin/vita-make-fself"

if [[ ! -x "$VITASDK/bin/arm-vita-eabi-gcc" ]]; then
  echo "Missing VitaSDK compiler: $VITASDK/bin/arm-vita-eabi-gcc" >&2
  exit 1
fi
if [[ ! -x "$VITA_MAKE_FSELF" ]]; then
  echo "Missing vita-make-fself: $VITA_MAKE_FSELF" >&2
  exit 1
fi

if [[ ! -f "$RELEASE_VPK" ]]; then
  echo "Missing upstream release VPK: $RELEASE_VPK" >&2
  exit 1
fi

mkdir -p "$OUT_DIR" "$RELEASE_DIR"

cmake -S "$ROOT" -B "$BUILD_DIR" -DBUILD=Arch -DWITH_LOG=ON
cmake --build "$BUILD_DIR" --target Emu4VitaPlus.elf-velf -- -j"$(nproc)"
"$VITA_MAKE_FSELF" -a 0x2800000000000001 \
  "$BUILD_DIR/arch/Emu4VitaPlus.elf.velf" \
  "$BUILD_DIR/arch/eboot_Emu4VitaPlus.bin"

rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR"
unzip -q "$RELEASE_VPK" -d "$STAGE_DIR"
cp "$BUILD_DIR/arch/eboot_Emu4VitaPlus.bin" "$STAGE_DIR/eboot.bin"

rm -f "$OUTPUT_VPK"
(cd "$STAGE_DIR" && zip -qr "$OUTPUT_VPK" . -x '*.DS_Store')

echo "Built:"
echo "  $OUTPUT_VPK"
