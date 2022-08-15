add_executable(access_all_devices_x2_max_speed
    main.c
)

target_link_libraries(access_all_devices_x2_max_speed
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap1
    pico_stdlib
    i2c_dma
    common
)

pico_enable_stdio_usb(access_all_devices_x2_max_speed 0)
pico_enable_stdio_uart(access_all_devices_x2_max_speed 1)

pico_add_extra_outputs(access_all_devices_x2_max_speed)

