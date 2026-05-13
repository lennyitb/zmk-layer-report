# ZMK Layer Report

A ZMK module that reports active keyboard layer and modifier state to the host via a custom HID report. Designed for use with a companion app that can display an on-screen overlay of the current layer's key mappings.

## Installation

From the root of your ZMK config repo, run:

```bash
bash -c "$(curl -fsSL https://raw.githubusercontent.com/lennyitb/zmk-layer-report/main/install.sh)"
```

This updates your `config/west.yml` and board `.conf` file automatically. Then build and flash as usual.

> **Note for split keyboards:** The installer will ask which `.conf` file to use — pick your main half (e.g. `corne_left.conf`). The HID report is sent from the main half only.

<details>
<summary>Manual installation</summary>

### 1. Add the module to `west.yml`

Edit `config/west.yml` to add the `lennyitb` remote and the `zmk-layer-report` project:

```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: lennyitb
      url-base: https://github.com/lennyitb
  projects:
    - name: zmk
      remote: zmkfirmware
      import: app/west.yml
    - name: zmk-layer-report
      remote: lennyitb
      revision: main
  self:
    path: config
```

### 2. Enable in your board config

Add the following to your main half's `.conf` file (e.g. `corne_left.conf`):

```
CONFIG_ZMK_LAYER_REPORT=y
```

### 3. Build and flash

Build your firmware as usual. The module will be pulled in automatically by west.

</details>

## Configuration

All options are optional — the defaults work out of the box.

| Kconfig Option | Default | Description |
|---|---|---|
| `CONFIG_ZMK_LAYER_REPORT` | `n` | Enable the module |
| `CONFIG_ZMK_LAYER_REPORT_BLE` | `y` | Enable BLE transport |
| `CONFIG_ZMK_LAYER_REPORT_USB` | `y` | Enable USB transport |
| `CONFIG_ZMK_LAYER_REPORT_USAGE_PAGE` | `0xFF42` | HID Usage Page (vendor-defined) |
| `CONFIG_ZMK_LAYER_REPORT_REPORT_ID` | `32` | HID Report ID |

## How it works

When layer or modifier state changes, the module sends a 4-byte HID report containing:

- A bitmask of active layers (up to 16)
- The effective modifier state
- Modifier source flags (held vs. sticky/one-shot)

The report uses a vendor-defined HID usage page (`0xFF42`) so it doesn't interfere with normal keyboard operation. A companion app on the host listens for these reports to display the overlay.

See [docs/state-report-spec.md](docs/state-report-spec.md) for the full protocol spec and [docs/development.md](docs/development.md) for testing and development.

## License

[MIT](LICENSE)
