add_subdirectory(bluetooth)
add_subdirectory(motor_control)
add_subdirectory(tcp)
add_subdirectory(thumbstick)
add_subdirectory(ssd1306)
add_subdirectory(hcsr04)

add_executable(robot robot.c)
pico_set_program_name(robot "robot")
pico_set_program_version(robot "0.1")

pico_enable_stdio_uart(robot 1)

target_link_libraries(robot
    PRIVATE
        pico_btstack_cyw43
        pico_cyw43_driver
        pico_stdlib
        bluetooth
        motor_control
)

target_compile_options(robot
    PRIVATE
        -Wall
        -Werror
        -Wextra
)

pico_add_extra_outputs(robot)

add_executable(controller controller.c)
pico_set_program_name(controller "controller")
pico_set_program_version(controller "0.1")

pico_enable_stdio_uart(controller 1)

target_link_libraries(controller
    PRIVATE
        pico_btstack_cyw43
        pico_cyw43_driver
        pico_stdlib
        bluetooth
        thumbstick
)

target_compile_options(controller
    PRIVATE
        -Wall
        -Werror
        -Wextra
)

pico_add_extra_outputs(controller)

add_executable(display display.c)
pico_set_program_name(display "display")
pico_set_program_version(display "0.1")

pico_enable_stdio_uart(display 1)

target_link_libraries(display
    PRIVATE
        pico_stdlib
        ssd1306
		hcsr04
)

pico_add_extra_outputs(display)
