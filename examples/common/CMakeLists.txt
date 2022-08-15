add_library(common INTERFACE)

target_include_directories(common INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(common INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/freertos_hooks.c
    ${CMAKE_CURRENT_LIST_DIR}/mprintf.c
)

target_link_libraries(common INTERFACE
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap1
    pico_stdlib
)

