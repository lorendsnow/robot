add_subdirectory(bluetooth)
add_subdirectory(motor_control)
add_subdirectory(tcp)
add_subdirectory(thumbstick)

# add_executable(robot
#     main.c
#     tcp_fns.c
# )

# pico_set_program_name(robot "robot")
# pico_set_program_version(robot "0.1")

# pico_enable_stdio_uart(robot 1)

# target_link_libraries(robot
#     PRIVATE
#         pico_stdlib
#         bluetooth
#         motor_control
#         tcp
# )

# if(DEFINED ENV{SSID})
#     target_compile_options(robot PRIVATE "-DSSID=\"$ENV{SSID}\"")
# endif()

# if(DEFINED ENV{WIFI_PASS})
#     target_compile_options(robot PRIVATE "-DWIFI_PASS=\"$ENV{WIFI_PASS}\"")
# endif()

# pico_add_extra_outputs(robot)

add_executable(btclient test_bt_client.c)
pico_set_program_name(btclient "btclient")
pico_set_program_version(btclient "0.1")

pico_enable_stdio_uart(btclient 1)

target_link_libraries(btclient
    PRIVATE
        pico_btstack_cyw43
        pico_cyw43_driver
        pico_lwip
        pico_stdlib
        bluetooth
)

pico_add_extra_outputs(btclient)

add_executable(btserver test_bt_server.c)
pico_set_program_name(btserver "btserver")
pico_set_program_version(btserver "0.1")

pico_enable_stdio_uart(btserver 1)

target_link_libraries(btserver
    PRIVATE
        hardware_adc
        pico_btstack_cyw43
        pico_cyw43_driver
        pico_lwip
        pico_stdlib
        bluetooth
        thumbstick
)

pico_add_extra_outputs(btserver)