# Development

## Testing

Tests run on `native_posix_64` — no hardware needed. They simulate key presses via ZMK's mock kscan driver and verify layer report output against snapshots.

### Setup (one-time)

Create a west workspace with ZMK:

```bash
mkdir zmk-workspace && cd zmk-workspace
west init -l ../zmk-layer-report/tests
west update
```

### Running tests

```bash
cd zmk-workspace
./zmk-layer-report/tests/run.sh
```

The test runner builds each test case, runs the firmware binary, filters log output through sed patterns, and diffs against expected snapshots.

To auto-accept updated snapshots (when you intentionally change output format):

```bash
# After running tests and reviewing the diffs, copy the filtered output over the snapshot:
cp tests/build/layer_report_normal.filtered.log tests/layer_report/normal/keycode_events.snapshot
```

### Test cases

| Test | Description |
|---|---|
| `normal` | Press and release a single layer key. Verifies bitmask toggles correctly. |
| `multiple_layers` | Hold layer 1, then also hold layer 2, release in reverse. Verifies multi-layer bitmask. |

## HID Report Format

The report is sent as a vendor-defined HID input report (4 bytes):

| Byte | Description |
|---|---|
| 0 | Layer state bits 0-7 (layers 0-7) |
| 1 | Layer state bits 8-15 (layers 8-15) |
| 2 | Effective modifier bitmask (standard USB HID modifier bits) |
| 3 | Modifier source flags (bit set = sticky/one-shot) |

See [state-report-spec.md](state-report-spec.md) for the full protocol spec including byte layout, interpretation rules, and HID descriptor.

### HID Report Descriptor

- Usage Page: `0xFF42` (vendor-defined)
- Usage: `0x01`
- Report ID: `0x20` (32)
- Report Size: 8 bits
- Report Count: 4

## Companion App

This module only handles the firmware side. A companion app running on the host is needed to read the HID reports and display the overlay. The companion app should:

1. Find the HID device with usage page `0xFF42`
2. Read input reports (report ID `0x20`)
3. Parse bytes 0-1 as little-endian uint16 for the active layer set
4. Parse byte 2 for active modifiers, byte 3 for sticky/one-shot flags
5. Display the appropriate overlay
