add_library(i2c_dma INTERFACE)

target_include_directories(i2c_dma INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(i2c_dma INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/i2c_dma.c
)

target_link_libraries(i2c_dma INTERFACE
    FreeRTOS-Kernel
    pico_stdlib
    hardware_dma
    hardware_i2c
)

