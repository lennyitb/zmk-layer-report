#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>
#include <zmk/ble.h>
#include <zmk_layer_report/hid.h>

LOG_MODULE_DECLARE(zmk_layer_report, CONFIG_ZMK_LOG_LEVEL);

static const uint8_t layer_report_desc[] = {ZMK_LAYER_REPORT_DESC};

struct hids_info {
    uint16_t version;
    uint8_t code;
    uint8_t flags;
} __packed;

static struct hids_info info = {
    .version = 0x0111,
    .code = 0x00,
    .flags = 0x02,
};

struct hids_report {
    uint8_t id;
    uint8_t type;
} __packed;

enum { HIDS_INPUT = 0x01 };

static struct hids_report input_ref = {
    .id = ZMK_LAYER_REPORT_REPORT_ID,
    .type = HIDS_INPUT,
};

static ssize_t read_hids_info(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                              uint16_t len, uint16_t offset) {
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data,
                             sizeof(struct hids_info));
}

static ssize_t read_hids_report_map(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                    void *buf, uint16_t len, uint16_t offset) {
    return bt_gatt_attr_read(conn, attr, buf, len, offset, layer_report_desc,
                             sizeof(layer_report_desc));
}

static ssize_t read_hids_report_ref(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                    void *buf, uint16_t len, uint16_t offset) {
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data,
                             sizeof(struct hids_report));
}

static ssize_t read_hids_input_report(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                      void *buf, uint16_t len, uint16_t offset) {
    struct zmk_layer_report *report = zmk_layer_report_get();
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &report->body,
                             sizeof(report->body));
}

/* clang-format off */
BT_GATT_SERVICE_DEFINE(
    layer_report_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_HIDS),

    BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_INFO, BT_GATT_CHRC_READ,
                           BT_GATT_PERM_READ, read_hids_info, NULL, &info),

    BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT_MAP, BT_GATT_CHRC_READ,
                           BT_GATT_PERM_READ_ENCRYPT, read_hids_report_map, NULL, NULL),

    BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_REPORT,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_READ_ENCRYPT,
                           read_hids_input_report, NULL, NULL),
    BT_GATT_CCC(NULL, BT_GATT_PERM_READ_ENCRYPT | BT_GATT_PERM_WRITE_ENCRYPT),
    BT_GATT_DESCRIPTOR(BT_UUID_HIDS_REPORT_REF, BT_GATT_PERM_READ_ENCRYPT,
                       read_hids_report_ref, NULL, &input_ref),

    BT_GATT_CHARACTERISTIC(BT_UUID_HIDS_CTRL_POINT,
                           BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                           BT_GATT_PERM_WRITE, NULL, NULL, NULL),
);
/* clang-format on */

int zmk_layer_report_ble_send(struct zmk_layer_report *report) {
    struct bt_conn *conn = zmk_ble_active_profile_conn();
    if (conn == NULL) {
        return -ENOTCONN;
    }

    struct bt_gatt_notify_params notify_params = {
        .attr = &layer_report_svc.attrs[5],
        .data = &report->body,
        .len = sizeof(report->body),
    };

    int err = bt_gatt_notify_cb(conn, &notify_params);
    if (err == -EPERM) {
        bt_conn_set_security(conn, BT_SECURITY_L2);
    } else if (err) {
        LOG_WRN("BLE notify failed: %d", err);
    }

    bt_conn_unref(conn);
    return err;
}
