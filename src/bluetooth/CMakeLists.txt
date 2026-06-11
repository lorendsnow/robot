add_library(bluetooth
    ble_client.c
    ble_server.c
)

target_link_libraries(bluetooth
    PUBLIC
        pico_stdlib
        pico_btstack_ble
        pico_btstack_cyw43
        pico_cyw43_arch_lwip_threadsafe_background
        pico_cyw43_driver
        pico_lwip
        thumbstick
        motor_control
)

target_include_directories(bluetooth
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

pico_btstack_make_gatt_header(bluetooth PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include/controller_server.gatt)

target_compile_options(bluetooth
    PRIVATE
        -Wall
        -Werror
        -Wextra
)