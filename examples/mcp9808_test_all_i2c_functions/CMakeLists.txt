add_executable(mcp9808_test_all_i2c_functions
    main.c
)

target_link_libraries(mcp9808_test_all_i2c_functions
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap1
    pico_stdlib
    i2c_dma
    common
)

pico_enable_stdio_usb(mcp9808_test_all_i2c_functions 0)
pico_enable_stdio_uart(mcp9808_test_all_i2c_functions 1)

pico_add_extra_outputs(mcp9808_test_all_i2c_functions)

