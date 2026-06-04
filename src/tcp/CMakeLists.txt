add_library(tcp server.c)

target_link_libraries(tcp
    PUBLIC
        pico_stdlib
        pico_lwip
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_arch
        pico_lwip_nosys
)

target_include_directories(tcp
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)