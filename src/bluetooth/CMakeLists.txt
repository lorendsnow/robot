add_library(bluetooth
    client.c
    msg_protocol.c
    server.c
)

target_link_libraries(bluetooth
    PUBLIC
        pico_stdlib
    PRIVATE
        pico_btstack_classic
)

target_include_directories(bluetooth
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)