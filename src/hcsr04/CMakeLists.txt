add_library(hcsr04 hcsr04.c)

target_link_libraries(hcsr04
	PUBLIC
		pico_stdlib
)

target_include_directories(hcsr04
	PUBLIC
		include
)
