add_library(ugui INTERFACE)

target_include_directories(ugui INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(ugui INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/ugui.c
)

