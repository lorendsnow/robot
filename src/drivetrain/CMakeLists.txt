add_library(drivetrain drivetrain.c)

target_link_libraries(drivetrain
    PUBLIC
        pico_stdlib
)

target_include_directories(drivetrain
    PUBLIC
        include
)