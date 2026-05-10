#!/usr/bin/env bash
set -euo pipefail

# Test runner for zmk-layer-report
# Requires: west workspace initialized with ZMK (see README)

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
MODULE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Find ZMK app directory — look for it relative to the west workspace
WEST_TOPDIR="$(west topdir 2>/dev/null)" || {
    echo "ERROR: Not in a west workspace. Run from a workspace with ZMK, or see README for setup."
    exit 1
}

ZMK_APP="$WEST_TOPDIR/zmk/app"
if [ ! -d "$ZMK_APP" ]; then
    echo "ERROR: ZMK not found at $ZMK_APP. Run 'west update' first."
    exit 1
fi

BOARD="${BOARD:-native_posix_64}"
PASS=0
FAIL=0
ERRORS=""

run_test() {
    local test_dir="$1"
    local test_name="$(basename "$(dirname "$test_dir")")/$(basename "$test_dir")"

    printf "%-50s " "$test_name"

    local build_dir="$SCRIPT_DIR/build/$(echo "$test_name" | tr '/' '_')"

    # Build
    if ! west build -p -b "$BOARD" "$ZMK_APP" -d "$build_dir" \
        -- -DZMK_CONFIG="$test_dir" \
           -DEXTRA_ZEPHYR_MODULES="$MODULE_DIR" \
        > "$build_dir.build.log" 2>&1; then
        echo "BUILD FAILED"
        FAIL=$((FAIL + 1))
        ERRORS="$ERRORS\n  FAIL (build): $test_name — see $build_dir.build.log"
        return
    fi

    # Run and capture output
    local exe="$build_dir/zephyr/zephyr.exe"
    local raw_log="$build_dir.run.log"
    local filtered_log="$build_dir.filtered.log"

    timeout 30 "$exe" > "$raw_log" 2>&1 || true

    # Filter through sed patterns
    sed -nf "$test_dir/events.patterns" "$raw_log" > "$filtered_log"

    # Compare with snapshot
    if diff -q "$test_dir/keycode_events.snapshot" "$filtered_log" > /dev/null 2>&1; then
        echo "PASS"
        PASS=$((PASS + 1))
    else
        echo "FAIL"
        FAIL=$((FAIL + 1))
        ERRORS="$ERRORS\n  FAIL (output): $test_name"
        diff "$test_dir/keycode_events.snapshot" "$filtered_log" || true
    fi
}

echo "=== zmk-layer-report tests ==="
echo ""

# Find all test cases (directories containing a keymap file)
for keymap in "$SCRIPT_DIR"/layer_report/*/native_posix_64.keymap; do
    test_dir="$(dirname "$keymap")"
    run_test "$test_dir"
done

echo ""
echo "=== Results: $PASS passed, $FAIL failed ==="
if [ -n "$ERRORS" ]; then
    echo -e "$ERRORS"
    exit 1
fi
