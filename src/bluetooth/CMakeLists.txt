add_library(bluetooth
    client.c
    msg_protocol.c
    server.c
)

target_link_libraries(bluetooth
    PUBLIC
        pico_stdlib
        pico_btstack_classic
        pico_btstack_cyw43
        pico_cyw43_arch_lwip_threadsafe_background
        pico_cyw43_driver
        pico_lwip
)

target_include_directories(bluetooth
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

pico_enable_stdio_uart(bluetooth 1)