bin-dir = build/bin
client-elf = btclient.elf
server-elf = btserver.elf

minicom :
	minicom -b 115200 -o -D /dev/ttyACM0

clean :
	rm -rf build && cmake -B build -G Ninja

build-all :
	cmake --build build

flash-server :
	cmake --build build && picotool load $(bin-dir)/$(server-elf) -fx

flash-client :
	cmake --build build && picotool load $(bin-dir)/$(client-elf) -fx