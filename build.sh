#!/usr/bin/env bash
set -euo pipefail

VENV=/workspaces/zmk-venv
ZMK_WORKSPACE=/workspaces/zmk-workspace
MODULE_DIR="$(cd "$(dirname "$0")" && pwd)"
ZEPHYR_SDK=/workspaces/zephyr-sdk-0.17.0

source "$VENV/bin/activate"
export ZEPHYR_SDK_INSTALL_DIR="$ZEPHYR_SDK"
export ZEPHYR_BASE="$ZMK_WORKSPACE/zephyr"

BOARD="${1:-nice_nano_nrf52840_zmk}"
SHIELD="${2:-}"

BUILD_ARGS=(
    -s "$ZMK_WORKSPACE/zmk.git/app"
    -b "$BOARD"
    -d "$MODULE_DIR/build"
    -- -DZMK_EXTRA_MODULES="$MODULE_DIR"
    -DCONFIG_ZMK_LAYER_REPORT=y
)

if [ -n "$SHIELD" ]; then
    BUILD_ARGS+=(-DSHIELD="$SHIELD")
fi

echo "Building for board=$BOARD shield=$SHIELD"
echo "Module: $MODULE_DIR"

west build "${BUILD_ARGS[@]}"

echo ""
echo "Build complete. Firmware at: $MODULE_DIR/build/zephyr/zmk.uf2"
