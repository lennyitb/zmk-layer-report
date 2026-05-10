#include <zephyr/ztest.h>
#include <zmk_layer_report/hid.h>
#include <errno.h>

ZTEST_SUITE(layer_report, NULL, NULL, NULL, NULL, NULL);

ZTEST(layer_report, test_initial_state_is_zero) {
    struct zmk_layer_report *r = zmk_layer_report_get();
    zassert_not_null(r);
    zassert_equal(r->report_id, ZMK_LAYER_REPORT_REPORT_ID);
    zassert_equal(r->body.layer_state, 0);
}

ZTEST(layer_report, test_update_sets_state) {
    int err = zmk_layer_report_update(0x0001);
    zassert_equal(err, 0);

    struct zmk_layer_report *r = zmk_layer_report_get();
    zassert_equal(r->body.layer_state, 0x0001);
}

ZTEST(layer_report, test_update_duplicate_returns_ealready) {
    zmk_layer_report_update(0x0005);
    int err = zmk_layer_report_update(0x0005);
    zassert_equal(err, -EALREADY);
}

ZTEST(layer_report, test_update_different_state_succeeds) {
    zmk_layer_report_update(0x0003);
    int err = zmk_layer_report_update(0x0007);
    zassert_equal(err, 0);

    struct zmk_layer_report *r = zmk_layer_report_get();
    zassert_equal(r->body.layer_state, 0x0007);
}

ZTEST(layer_report, test_update_back_to_zero) {
    zmk_layer_report_update(0x000F);
    int err = zmk_layer_report_update(0x0000);
    zassert_equal(err, 0);

    struct zmk_layer_report *r = zmk_layer_report_get();
    zassert_equal(r->body.layer_state, 0);
}

ZTEST(layer_report, test_update_all_layers) {
    int err = zmk_layer_report_update(0xFFFF);
    zassert_equal(err, 0);

    struct zmk_layer_report *r = zmk_layer_report_get();
    zassert_equal(r->body.layer_state, 0xFFFF);
}

ZTEST(layer_report, test_report_struct_packing) {
    zassert_equal(sizeof(struct zmk_layer_report_body), 2);
    zassert_equal(sizeof(struct zmk_layer_report), 3);
}

ZTEST(layer_report, test_report_id_constant) {
    struct zmk_layer_report *r = zmk_layer_report_get();
    zmk_layer_report_update(0x1234);
    zassert_equal(r->report_id, 32);
}

ZTEST(layer_report, test_descriptor_size) {
    static const uint8_t desc[] = {ZMK_LAYER_REPORT_DESC};
    zassert_true(sizeof(desc) > 0);
    zassert_equal(desc[0], 0x06);
    zassert_equal(desc[sizeof(desc) - 1], 0xC0);
}
