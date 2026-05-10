#include <zmk_layer_report/hid.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(zmk_layer_report, CONFIG_ZMK_LOG_LEVEL);

static struct zmk_layer_report report = {
    .report_id = ZMK_LAYER_REPORT_REPORT_ID,
    .body = {.layer_state = 0},
};

struct zmk_layer_report *zmk_layer_report_get(void) { return &report; }

int zmk_layer_report_update(uint16_t layer_state) {
    if (report.body.layer_state == layer_state) {
        return -EALREADY;
    }
    report.body.layer_state = layer_state;
    LOG_DBG("layer state updated: 0x%04x", layer_state);
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
