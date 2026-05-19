#include <stdio.h>
#include "pico/stdlib.h"
#include "server.h"
#include "tcp_fns.h"

#ifndef SSID
#define SSID ""
#endif

#ifndef WIFI_PASS
#define WIFI_PASS ""
#endif

#define LED_GREEN 0x04
#define LED_RED   0x08

int main() {
    stdio_init_all();

    sleep_ms(1000);

    printf("SSID: %s\n", SSID);
    printf("Password: %s\n", WIFI_PASS);

    gpio_init_mask(LED_GREEN | LED_RED);
    gpio_set_dir_out_masked(LED_GREEN | LED_RED);
    gpio_put(LED_GREEN, 0);
    gpio_put(LED_RED, 1);

    if (wifi_connect(SSID, WIFI_PASS)) {
        printf("Wi-Fi connection failed\n");
        cyw43_arch_deinit();
    } else {
        gpio_put(LED_GREEN, 1);
        gpio_put(LED_RED, 0);
        Ip4Addr addr;
        get_ip4_addr(addr);

        printf("IP address %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
    }

    queue_t queue;
    queue_init(&queue, sizeof(uint8_t), 256);
    Server s = {.pcb = NULL, .err = NULL};

    if (server_init(&s, accept)) {
        printf("error occurred trying to initiate server\n");
    } else {
        printf("successfully initiated server listen\n");
    }

    while (true) {
    }
}
