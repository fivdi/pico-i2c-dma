add_executable(ssd1306_bouncing_ball
    main.c
)

# Required for ugui library to find ugui_config.h in current directory.
target_include_directories(ssd1306_bouncing_ball PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(ssd1306_bouncing_ball
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap1
    pico_stdlib
    i2c_dma
    common
    ugui
)

pico_enable_stdio_usb(ssd1306_bouncing_ball 0)
pico_enable_stdio_uart(ssd1306_bouncing_ball 1)

pico_add_extra_outputs(ssd1306_bouncing_ball)

