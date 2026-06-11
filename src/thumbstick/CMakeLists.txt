add_library(thumbstick thumbstick.c)

target_link_libraries(thumbstick
    PUBLIC
        pico_stdlib
        hardware_adc
)

target_include_directories(thumbstick
    PUBLIC
        include
)

target_compile_options(thumbstick
    PRIVATE
        -Wall
        -Werror
        -Wextra
)