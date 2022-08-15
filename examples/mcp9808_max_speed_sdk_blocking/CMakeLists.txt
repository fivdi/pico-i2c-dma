add_executable(mcp9808_max_speed_sdk_blocking
    main.c
)

target_link_libraries(mcp9808_max_speed_sdk_blocking
    pico_stdlib
    hardware_i2c
)

pico_enable_stdio_usb(mcp9808_max_speed_sdk_blocking 0)
pico_enable_stdio_uart(mcp9808_max_speed_sdk_blocking 1)

pico_add_extra_outputs(mcp9808_max_speed_sdk_blocking)

