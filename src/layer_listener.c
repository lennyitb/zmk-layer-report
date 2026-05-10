#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/keymap.h>
#include <zmk/hid.h>
#include <zmk_layer_report/hid.h>

LOG_MODULE_DECLARE(zmk_layer_report, CONFIG_ZMK_LOG_LEVEL);

static void layer_report_work_handler(struct k_work *work) {
    uint16_t layer_state = (uint16_t)(zmk_keymap_layer_state() & 0xFFFF);
    uint8_t modifiers = zmk_hid_get_keyboard_report()->body.modifiers;
    uint8_t mod_flags = 0;

    int err = zmk_layer_report_update(layer_state, modifiers, mod_flags);
    if (err == -EALREADY) {
        return;
    }

    zmk_layer_report_send();
}

static K_WORK_DEFINE(layer_report_work, layer_report_work_handler);

static int layer_report_listener(const zmk_event_t *eh) {
    k_work_submit(&layer_report_work);
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(layer_report, layer_report_listener);
ZMK_SUBSCRIPTION(layer_report, zmk_layer_state_changed);
ZMK_SUBSCRIPTION(layer_report, zmk_keycode_state_changed);
