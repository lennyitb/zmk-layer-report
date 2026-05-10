#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keymap.h>
#include <zmk_layer_report/hid.h>

LOG_MODULE_DECLARE(zmk_layer_report, CONFIG_ZMK_LOG_LEVEL);

static int layer_report_listener(const zmk_event_t *eh) {
    const struct zmk_layer_state_changed *ev = as_zmk_layer_state_changed(eh);
    if (ev == NULL) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    zmk_keymap_layers_state_t state = zmk_keymap_layer_state();
    uint16_t layer_state = (uint16_t)(state & 0xFFFF);

    int err = zmk_layer_report_update(layer_state);
    if (err == -EALREADY) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    zmk_layer_report_send();
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(layer_report, layer_report_listener);
ZMK_SUBSCRIPTION(layer_report, zmk_layer_state_changed);
