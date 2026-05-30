#include "pico/stdlib.h"

#include "server.h"
#include "drivetrain.h"
#include "tcp_fns.h"

#ifndef SSID
#define SSID ""
#endif

#ifndef WIFI_PASS
#define WIFI_PASS ""
#endif

#define GREEN_LED_PIN 15  // GPIO 15
#define RED_LED_PIN   14  // GPIO 14

void process_loop(queue_t* q);

int main() {
    stdio_init_all();
    gpio_init_mask((1 << RED_LED_PIN) | (1 << GREEN_LED_PIN));
    gpio_set_dir_out_masked((1 << RED_LED_PIN) | (1 << GREEN_LED_PIN));
    gpio_put(RED_LED_PIN, true);
    gpio_put(GREEN_LED_PIN, false);

    queue_t q;
    queue_init(&q, sizeof(TLVMessage), 256);
    struct Server s = {.queue = &q};

    drivetrain_init();

    sleep_ms(200);

    if (wifi_connect(SSID, WIFI_PASS)) {
        printf("Wi-Fi connection failed\n");
        cyw43_arch_deinit();
    } else {
        gpio_xor_mask((1 << GREEN_LED_PIN) | (1 << RED_LED_PIN));
        Ip4Addr addr;
        get_ip4_addr(addr);

        printf("IP address %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
    }

    if (server_init(&s, accept)) {
        printf("error occurred trying to initiate server\n");
    } else {
        printf("successfully initiated server listen\n");
    }

    process_loop(&q);

    return 0;
}

void process_loop(queue_t* q) {
    printf("in the process loop\n");
    TLVMessage msg;

    while (true) {
        while (!queue_is_empty(q)) {
            printf("parsing commands from queue\n");
            queue_remove_blocking(q, &msg);

            printf("pulled message: ");
            print_msg(&msg, true);

            switch (msg.type) {
                case ACK:
                    printf("pulled ACK message from queue\n");
                    break;
                case DRIVE:
                    if (drive_set_state((uint8_t)msg.payload)) {
                        printf(
                            "error occurred while trying to set drive state to "
                            "%d\n",
                            (uint8_t)msg.payload);
                    }
                    break;
                case DATA:
                    printf("pulled data message from queue with payload %X\n",
                           msg.payload);
                    break;
                default:
                    printf(
                        "pulled message from queue with unkown type value %d\n",
                        msg.type);
                    break;
            }
        }
    }
}