add_subdirectory(bluetooth)
add_subdirectory(motor_control)
add_subdirectory(tcp)
add_subdirectory(thumbstick)

add_executable(btclient test_bt_client.c)
pico_set_program_name(btclient "btclient")
pico_set_program_version(btclient "0.1")

pico_enable_stdio_uart(btclient 1)

target_link_libraries(btclient
    PRIVATE
        pico_btstack_cyw43
        pico_cyw43_driver
        pico_stdlib
        bluetooth
        motor_control
)

pico_add_extra_outputs(btclient)

add_executable(btserver test_bt_server.c)
pico_set_program_name(btserver "btserver")
pico_set_program_version(btserver "0.1")

pico_enable_stdio_uart(btserver 1)

target_link_libraries(btserver
    PRIVATE
        pico_btstack_cyw43
        pico_cyw43_driver
        pico_stdlib
        bluetooth
        thumbstick
)

pico_add_extra_outputs(btserver)