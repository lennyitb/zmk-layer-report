#include <string.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>
#include <zephyr/logging/log.h>
#include <zmk_layer_report/hid.h>

LOG_MODULE_DECLARE(zmk_layer_report, CONFIG_ZMK_LOG_LEVEL);

static const uint8_t layer_report_desc[] = {ZMK_LAYER_REPORT_DESC};

static const struct device *hid_dev;

static void in_ready_cb(const struct device *dev) {}

static const struct hid_ops ops = {
    .int_in_ready = in_ready_cb,
};

static int usb_hid_init(void) {
    hid_dev = device_get_binding("HID_LAYER_REPORT");
    if (hid_dev == NULL) {
        LOG_ERR("cannot get HID device binding");
        return -ENODEV;
    }

    usb_hid_register_device(hid_dev, layer_report_desc, sizeof(layer_report_desc), &ops);

    return usb_hid_init_device(hid_dev);
}

SYS_INIT(usb_hid_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

int zmk_layer_report_usb_send(struct zmk_layer_report *report) {
    if (hid_dev == NULL) {
        return -ENODEV;
    }

    uint8_t buf[sizeof(report->body) + 1];
    buf[0] = report->report_id;
    memcpy(&buf[1], &report->body, sizeof(report->body));

    return hid_int_ep_write(hid_dev, buf, sizeof(buf), NULL);
}
