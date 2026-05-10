#include <string.h>
#include <zmk_layer_report/hid.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zmk_layer_report, CONFIG_ZMK_LOG_LEVEL);

static struct zmk_layer_report report = {
    .report_id = ZMK_LAYER_REPORT_REPORT_ID,
    .body = {.layer_state = 0, .modifiers = 0, .mod_flags = 0},
};

struct zmk_layer_report *zmk_layer_report_get(void) { return &report; }

int zmk_layer_report_update(uint16_t layer_state, uint8_t modifiers, uint8_t mod_flags) {
    struct zmk_layer_report_body new_body = {
        .layer_state = layer_state,
        .modifiers = modifiers,
        .mod_flags = mod_flags,
    };
    if (memcmp(&report.body, &new_body, sizeof(new_body)) == 0) {
        return -EALREADY;
    }
    report.body = new_body;
    LOG_DBG("layer state updated: 0x%04x mods: 0x%02x flags: 0x%02x",
            layer_state, modifiers, mod_flags);
    return 0;
}

#if IS_ENABLED(CONFIG_ZMK_LAYER_REPORT_BLE)
int zmk_layer_report_ble_send(struct zmk_layer_report *report);
#endif

#if IS_ENABLED(CONFIG_ZMK_LAYER_REPORT_USB)
int zmk_layer_report_usb_send(struct zmk_layer_report *report);
#endif

int zmk_layer_report_send(void) {
    int err = 0;

#if IS_ENABLED(CONFIG_ZMK_LAYER_REPORT_BLE)
    err = zmk_layer_report_ble_send(&report);
    if (err && err != -ENOTCONN) {
        LOG_WRN("BLE send failed: %d", err);
    }
#endif

#if IS_ENABLED(CONFIG_ZMK_LAYER_REPORT_USB)
    err = zmk_layer_report_usb_send(&report);
    if (err && err != -ENOTCONN) {
        LOG_WRN("USB send failed: %d", err);
    }
#endif

    return err;
}
