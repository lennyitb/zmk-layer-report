cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr COMPONENTS unittest REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(zmk_layer_report_unit)

set(MODULE_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)

target_include_directories(testbinary PRIVATE ${MODULE_DIR}/include)

target_sources(testbinary PRIVATE
    main.c
    stubs.c
    ${MODULE_DIR}/src/hid.c
)

target_compile_definitions(testbinary PRIVATE
    CONFIG_ZMK_LAYER_REPORT_USAGE_PAGE=0xFF42
    CONFIG_ZMK_LAYER_REPORT_REPORT_ID=32
    CONFIG_ZMK_LOG_LEVEL=0
    __packed=__attribute__\(\(__packed__\)\)
)
