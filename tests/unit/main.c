#include <zephyr/ztest.h>
#include <zmk_layer_report/hid.h>
#include <errno.h>

ZTEST_SUITE(layer_report, NULL, NULL, NULL, NULL, NULL);

ZTEST(layer_report, test_get_returns_valid_report) {
    struct zmk_layer_report *r = zmk_layer_report_get();
    zassert_not_null(r);
    zassert_equal(r->report_id, ZMK_LAYER_REPORT_REPORT_ID);
}

ZTEST(layer_report, test_update_sets_state) {
    int err = zmk_layer_report_update(0x0001, 0, 0);
    zassert_equal(err, 0);

    struct zmk_layer_report *r = zmk_layer_report_get();
    zassert_equal(r->body.layer_state, 0x0001);
}

ZTEST(layer_report, test_update_duplicate_returns_ealready) {
    zmk_layer_report_update(0x0005, 0x03, 0);
    int err = zmk_layer_report_update(0x0005, 0x03, 0);
    zassert_equal(err, -EALREADY);
}

ZTEST(layer_report, test_update_different_state_succeeds) {
    zmk_layer_report_update(0x0003, 0, 0);
    int err = zmk_layer_report_update(0x0007, 0, 0);
    zassert_equal(err, 0);

    struct zmk_layer_report *r = zmk_layer_report_get();
    zassert_equal(r->body.layer_state, 0x0007);
}

ZTEST(layer_report, test_update_back_to_zero) {
    zmk_layer_report_update(0x000F, 0x01, 0);
    int err = zmk_layer_report_update(0x0000, 0, 0);
    zassert_equal(err, 0);

    struct zmk_layer_report *r = zmk_layer_report_get();
    zassert_equal(r->body.layer_state, 0);
    zassert_equal(r->body.modifiers, 0);
}

ZTEST(layer_report, test_update_all_layers) {
    int err = zmk_layer_report_update(0xFFFF, 0, 0);
    zassert_equal(err, 0);

    struct zmk_layer_report *r = zmk_layer_report_get();
    zassert_equal(r->body.layer_state, 0xFFFF);
}

ZTEST(layer_report, test_report_struct_packing) {
    zassert_equal(sizeof(struct zmk_layer_report_body), 4);
    zassert_equal(sizeof(struct zmk_layer_report), 5);
}

ZTEST(layer_report, test_report_id_constant) {
    struct zmk_layer_report *r = zmk_layer_report_get();
    zmk_layer_report_update(0x1234, 0xFF, 0x0A);
    zassert_equal(r->report_id, 32);
}

ZTEST(layer_report, test_descriptor_size) {
    static const uint8_t desc[] = {ZMK_LAYER_REPORT_DESC};
    zassert_true(sizeof(desc) > 0);
    zassert_equal(desc[0], 0x06);
    zassert_equal(desc[sizeof(desc) - 1], 0xC0);
}

ZTEST(layer_report, test_update_modifiers_only) {
    zmk_layer_report_update(0x0001, 0x00, 0);
    int err = zmk_layer_report_update(0x0001, 0x04, 0);
    zassert_equal(err, 0);

    struct zmk_layer_report *r = zmk_layer_report_get();
    zassert_equal(r->body.layer_state, 0x0001);
    zassert_equal(r->body.modifiers, 0x04);
}

ZTEST(layer_report, test_update_mod_flags_only) {
    zmk_layer_report_update(0x0001, 0x02, 0x00);
    int err = zmk_layer_report_update(0x0001, 0x02, 0x02);
    zassert_equal(err, 0);

    struct zmk_layer_report *r = zmk_layer_report_get();
    zassert_equal(r->body.mod_flags, 0x02);
}

ZTEST(layer_report, test_update_all_fields) {
    int err = zmk_layer_report_update(0xABCD, 0xFF, 0x55);
    zassert_equal(err, 0);

    struct zmk_layer_report *r = zmk_layer_report_get();
    zassert_equal(r->body.layer_state, 0xABCD);
    zassert_equal(r->body.modifiers, 0xFF);
    zassert_equal(r->body.mod_flags, 0x55);
}

ZTEST(layer_report, test_duplicate_full_body) {
    zmk_layer_report_update(0x0003, 0x11, 0x01);
    int err = zmk_layer_report_update(0x0003, 0x11, 0x01);
    zassert_equal(err, -EALREADY);
}
