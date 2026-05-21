#include "pico/stdlib.h"

#include "drivetrain.h"
#include "server.h"
#include "tcp_fns.h"

#ifndef SSID
#define SSID ""
#endif

#ifndef WIFI_PASS
#define WIFI_PASS ""
#endif

#define QUEUE_SIZE 256

void process_loop(queue_t* q, char* buf);

int main() {
    stdio_init_all();

    queue_t q;
    queue_init(&q, sizeof(char), QUEUE_SIZE);
    struct Server s = {.queue = &q};

    drivetrain_init();

    sleep_ms(1000);

    printf("SSID: %s\n", SSID);
    printf("Password: %s\n", WIFI_PASS);

    if (wifi_connect(SSID, WIFI_PASS)) {
        printf("Wi-Fi connection failed\n");
        cyw43_arch_deinit();
    } else {
        Ip4Addr addr;
        get_ip4_addr(addr);

        printf("IP address %d.%d.%d.%d\n", addr[0], addr[1], addr[2], addr[3]);
    }

    if (server_init(&s, accept)) {
        printf("error occurred trying to initiate server\n");
    } else {
        printf("successfully initiated server listen\n");
    }

    char buf[4];
    printf("entering process loop\n");
    process_loop(&q, buf);

    return 0;
}

int parse_cmd(queue_t* q, char* buf) {
    char c;
    queue_remove_blocking(q, &c);
    while (c != ':') {
        if (queue_is_empty(q)) {
            return 0;
        }
        queue_remove_blocking(q, &c);
    }

    for (int i = 0; i < 3; i++) {
        if (queue_is_empty(q)) {
            return -1;
        }
        queue_remove_blocking(q, &c);
        buf[i] = c;
    }

    buf[3] = 0;
    return 0;
}

void process_loop(queue_t* q, char* buf) {
    printf("in the process loop\n");

    while (true) {
        while (!queue_is_empty(q)) {
            printf("parsing commands from queue\n");
            switch (parse_cmd(q, buf)) {
                case -1:
                    printf(
                        "error parsing command - got an incomplete command\n");
                    continue;
                case 1:
                    printf("completed parsing - queue is empty\n");
                    continue;
                default:
                    printf("command parsed to %s\n", buf);
                    break;
            }

            if (!strcmp(buf, "FWD")) {
                drive_fwd();
            } else if (!strcmp(buf, "REV")) {
                drive_reverse();
            } else if (!strcmp(buf, "LFT")) {
                drive_left();
            } else if (!strcmp(buf, "RGT")) {
                drive_right();
            } else if (!strcmp(buf, "STP")) {
                drive_brake();
            } else if (!strcmp(buf, "CST")) {
                drive_coast();
            } else {
                printf("got bad command %s\n", buf);
            }
        }
    }
}