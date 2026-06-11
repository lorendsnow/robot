bin-dir = build/bin
robot-elf = robot.elf
controller-elf = controller.elf
display-elf = display.elf

minicom :
	minicom -b 115200 -o -D /dev/ttyACM0

clean :
	rm -rf build && cmake -B build -G Ninja

build-all :
	cmake --build build

flash-controller :
	cmake --build build -t controller && picotool load $(bin-dir)/$(controller-elf) -fx

flash-robot :
	cmake --build build -t robot && picotool load $(bin-dir)/$(robot-elf) -fx

flash-display :
	cmake --build build -t display && picotool load $(bin-dir)/$(display-elf) -fx

tidy:
	clang-tidy -p build/compile-commands.json \
	src/*.c \
	src/bluetooth/*.c \
	src/bluetooth/include/*.h \
	src/bluetooth/include/bluetooth/*.h \
	src/motor_control/*.c \
	src/motor_control/include/*.h \
	src/thumbstick/*.c \
	src/thumbstick/include/*.h