add_subdirectory(server)
add_subdirectory(drivetrain)

add_executable(robot main.c)

pico_set_program_name(robot "robot")
pico_set_program_version(robot "0.1")

pico_enable_stdio_usb(robot 1)

target_link_libraries(robot
    PRIVATE
        pico_stdlib
        server
)

target_include_directories(robot
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

if(DEFINED ENV{SSID})
    target_compile_options(robot PRIVATE "-DSSID=\"$ENV{SSID}\"")
endif()

if(DEFINED ENV{WIFI_PASS})
    target_compile_options(robot PRIVATE "-DWIFI_PASS=\"$ENV{WIFI_PASS}\"")
endif()

pico_add_extra_outputs(robot)

add_executable(drivetest drive.c)

pico_enable_stdio_usb(drivetest 1)

target_link_libraries(drivetest
    PRIVATE
        pico_stdlib
        drivetrain
)