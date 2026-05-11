add_library(server server.c)

target_link_libraries(server
    PRIVATE
        pico_stdlib
    PUBLIC
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip
        pico_lwip_arch
        pico_lwip_nosys
)

target_include_directories(server
    PUBLIC
        include
)