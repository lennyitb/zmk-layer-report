# ZMK Layer Report

A ZMK module that reports active keyboard layer state to the host via a custom HID report. Designed for use with a companion app that displays an on-screen overlay of key mappings for the active layer(s).

## How it works

When layer keys are pressed or released, the module sends a 2-byte HID report containing a bitmask of active layers (bit N = layer N is active). The report uses a vendor-defined HID usage page (`0xFF42`) so it doesn't interfere with normal keyboard operation.

Supports up to 16 layers.

## Installation

Add the module to your ZMK config's `west.yml`:

```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: your-gh-username
      url-base: https://github.com/your-gh-username
  projects:
    - name: zmk
      remote: zmkfirmware
      import: app/west.yml
    - name: zmk-layer-report
      remote: your-gh-username
      revision: main
  self:
    path: config
```

Enable in your board's `.conf` file:

```
CONFIG_ZMK_LAYER_REPORT=y
```

## Configuration

| Kconfig Option | Default | Description |
|---|---|---|
| `CONFIG_ZMK_LAYER_REPORT` | `n` | Enable the module |
| `CONFIG_ZMK_LAYER_REPORT_BLE` | `y` | Enable BLE transport (requires `ZMK_BLE`) |
| `CONFIG_ZMK_LAYER_REPORT_USB` | `y` | Enable USB transport (requires `USB_DEVICE_STACK`) |
| `CONFIG_ZMK_LAYER_REPORT_USAGE_PAGE` | `0xFF42` | HID Usage Page (vendor-defined) |
| `CONFIG_ZMK_LAYER_REPORT_REPORT_ID` | `32` | HID Report ID |

## HID Report Format

The report is sent as a vendor-defined HID input report:

| Byte | Description |
|---|---|
| 0 | Layer state bits 0-7 (layers 0-7) |
| 1 | Layer state bits 8-15 (layers 8-15) |

Each bit represents a layer: `1` = active, `0` = inactive. Bit 0 of byte 0 is layer 0 (base layer, typically always active).

### HID Report Descriptor

- Usage Page: `0xFF42` (vendor-defined)
- Usage: `0x01`
- Report ID: `0x20` (32)
- Report Size: 8 bits
- Report Count: 2

## Companion App

This module only handles the firmware side. A companion app running on the host is needed to read the HID reports and display the overlay. The companion app should:

1. Find the HID device with usage page `0xFF42`
2. Read input reports (report ID `0x20`)
3. Parse the 2-byte layer bitmask
4. Display the appropriate key mapping overlay
