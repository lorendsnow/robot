#include <stdio.h>
#include "pico/stdlib.h"
#include "server.h"

#ifndef SSID
#define SSID ""
#endif

#ifndef WIFI_PASS
#define WIFI_PASS ""
#endif

int main() {
    stdio_init_all();

    if (wifi_connect(SSID, WIFI_PASS)) {
        printf("Wi-Fi connection failed\n");
        cyw43_arch_deinit();
    } else {
        Ip4Addr addr;
        get_ip4_addr(addr);

        printf("IP address %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
    }

    queue_t queue;
    queue_init(&queue, sizeof(uint8_t), 256);
    Server s = {.pcb = NULL, .queue = &queue, .err = NULL};

    if (server_init(&s)) {
        printf("error occurred trying to initiate server\n");
    } else {
        printf("successfully initiated server listen\n");
    }

    while (true) {
        printf("Hello world!\n");
        sleep_ms(1000);
    }
}
