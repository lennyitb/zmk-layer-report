#pragma once

#include <zephyr/types.h>

#define ZMK_LAYER_REPORT_USAGE_PAGE CONFIG_ZMK_LAYER_REPORT_USAGE_PAGE
#define ZMK_LAYER_REPORT_REPORT_ID CONFIG_ZMK_LAYER_REPORT_REPORT_ID

/*
 * HID Report Descriptor for layer state reporting.
 *
 * Vendor-defined usage page with a single 16-bit input report
 * representing a bitmask of active layers (bit N = layer N active).
 */
#define ZMK_LAYER_REPORT_DESC                                                  \
    0x06, (ZMK_LAYER_REPORT_USAGE_PAGE & 0xFF),                               \
        ((ZMK_LAYER_REPORT_USAGE_PAGE >> 8) & 0xFF), /* Usage Page (Vendor) */ \
        0x09, 0x01,                                   /* Usage (Vendor Usage 1) */ \
        0xA1, 0x01,                                   /* Collection (Application) */ \
        0x85, ZMK_LAYER_REPORT_REPORT_ID,             /*   Report ID */ \
        0x09, 0x01,                                   /*   Usage (Vendor Usage 1) */ \
        0x15, 0x00,                                   /*   Logical Minimum (0) */ \
        0x26, 0xFF, 0x00,                             /*   Logical Maximum (255) */ \
        0x75, 0x08,                                   /*   Report Size (8) */ \
        0x95, 0x02,                                   /*   Report Count (2) */ \
        0x81, 0x02,                                   /*   Input (Data, Var, Abs) */ \
        0xC0                                          /* End Collection */

struct zmk_layer_report_body {
    uint16_t layer_state;
} __packed;

struct zmk_layer_report {
    uint8_t report_id;
    struct zmk_layer_report_body body;
} __packed;

int zmk_layer_report_update(uint16_t layer_state);
int zmk_layer_report_send(void);
struct zmk_layer_report *zmk_layer_report_get(void);
