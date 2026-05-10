if(CONFIG_ZMK_LAYER_REPORT)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../include)

target_sources(app PRIVATE
    hid.c
    layer_listener.c
)

target_sources_ifdef(CONFIG_ZMK_LAYER_REPORT_BLE app PRIVATE ble_hid.c)
target_sources_ifdef(CONFIG_ZMK_LAYER_REPORT_USB app PRIVATE usb_hid.c)

endif()
