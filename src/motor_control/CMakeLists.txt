add_library(motor_control motor_control.c)

target_link_libraries(motor_control
    PUBLIC
        pico_stdlib
)

target_include_directories(motor_control
    PUBLIC
        include
)