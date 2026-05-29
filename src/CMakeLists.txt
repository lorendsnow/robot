add_executable(robot
    main.c
    drivetrain.c
    server.c 
    tcp_fns.c
)

pico_set_program_name(robot "robot")
pico_set_program_version(robot "0.1")

pico_enable_stdio_uart(robot 1)

target_include_directories(robot
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(robot
    PRIVATE
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip
        pico_lwip_arch
        pico_lwip_nosys
        pico_stdlib
)

if(DEFINED ENV{SSID})
    target_compile_options(robot PRIVATE "-DSSID=\"$ENV{SSID}\"")
endif()

if(DEFINED ENV{WIFI_PASS})
    target_compile_options(robot PRIVATE "-DWIFI_PASS=\"$ENV{WIFI_PASS}\"")
endif()

pico_add_extra_outputs(robot)