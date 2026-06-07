add_library(thumbstick thumbstick.c)

target_link_libraries(thumbstick
    PUBLIC
        pico_stdlib
)

target_include_directories(thumbstick
    PUBLIC
        include
)