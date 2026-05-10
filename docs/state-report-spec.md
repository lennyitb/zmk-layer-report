# State Report Protocol Spec

## Overview

A vendor-defined HID input report that communicates keyboard state (active layers and modifier status) to a companion application on the host.

- **Payload:** 4 bytes
- **Usage Page:** `0xFF42` (vendor-defined)
- **Report ID:** `0x20` (32)
- **Transport:** BLE (HOGP) or USB HID
- **Trigger:** Sent on every layer or modifier state change

## Report Layout

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 2 bytes | `layer_state` | Little-endian uint16. Bit N = layer N active. |
| 2 | 1 byte | `modifiers` | Effective modifier bitmask (OR of all sources). |
| 3 | 1 byte | `mod_flags` | Source flags: bit set = modifier is from a sticky/one-shot key. |

### Wire Format (C struct)

```c
struct zmk_state_report_body {
    uint16_t layer_state;  // bytes 0-1: layer bitmask, little-endian
    uint8_t  modifiers;    // byte 2: effective modifier state
    uint8_t  mod_flags;    // byte 3: sticky/one-shot source flags
} __packed;
```

## Layer State (bytes 0-1)

16 bits, little-endian.

- Bit 0 = layer 0 (base layer, typically always 1)
- Bit 15 = layer 15
- Multiple bits may be set simultaneously (layer stack)

Supports up to 16 layers.

## Modifier Bitmask (byte 2)

Standard USB HID modifier bit assignments:

| Bit | Modifier |
|-----|----------|
| 0 | Left Control |
| 1 | Left Shift |
| 2 | Left Alt |
| 3 | Left GUI |
| 4 | Right Control |
| 5 | Right Shift |
| 6 | Right Alt |
| 7 | Right GUI |

This byte reflects the *effective* modifier state -- the OR of all active sources (held keys, sticky/one-shot, mod-tap, etc.). If the bit is set, the modifier is active regardless of how it was activated.

## Modifier Source Flags (byte 3)

Same bit positions as byte 2. A set bit means the corresponding modifier is active due to a sticky (one-shot) key rather than a physical hold.

Interpretation:

| `modifiers` bit | `mod_flags` bit | Meaning |
|-----------------|-----------------|---------|
| 1 | 0 | Modifier is held (physical key down) |
| 1 | 1 | Modifier is latched (sticky/one-shot) |
| 0 | 0 | Modifier inactive |
| 0 | 1 | Invalid (must not occur; treat as inactive) |

## HID Report Descriptor

```
0x06, 0x42, 0xFF,       // Usage Page (Vendor 0xFF42)
0x09, 0x01,             // Usage (Vendor Usage 1)
0xA1, 0x01,             // Collection (Application)
0x85, 0x20,             //   Report ID (32)
0x09, 0x01,             //   Usage (Vendor Usage 1)
0x15, 0x00,             //   Logical Minimum (0)
0x26, 0xFF, 0x00,       //   Logical Maximum (255)
0x75, 0x08,             //   Report Size (8)
0x95, 0x04,             //   Report Count (4)
0x81, 0x02,             //   Input (Data, Var, Abs)
0xC0                    // End Collection
```

## Timing & Delivery

- Report is sent immediately on state change (layer activation/deactivation, modifier press/release/latch).
- Event-driven only -- no periodic polling.
- Duplicate suppression: report is NOT sent if the full 4-byte state is identical to the previous transmission.
- On BLE reconnection, the current state is readable via a GATT read (no notification is re-sent automatically).

## Byte Order

All multi-byte fields are little-endian (native ARM Cortex-M byte order, matches USB/BLE convention for HID).

## Companion App Consumption

1. Enumerate HID devices; find one with Usage Page `0xFF42`.
2. Open and subscribe to input reports with Report ID `0x20`.
3. On each 4-byte report:
   - Parse bytes 0-1 as little-endian uint16 -> active layer set.
   - Parse byte 2 -> active modifiers.
   - Parse byte 3 -> which of those modifiers are sticky/one-shot.
4. Update overlay accordingly.
