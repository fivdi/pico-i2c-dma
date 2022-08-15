add_executable(mcp9808_x2_max_speed
    main.c
)

target_link_libraries(mcp9808_x2_max_speed
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap1
    pico_stdlib
    i2c_dma
    common
)

pico_enable_stdio_usb(mcp9808_x2_max_speed 0)
pico_enable_stdio_uart(mcp9808_x2_max_speed 1)

pico_add_extra_outputs(mcp9808_x2_max_speed)

