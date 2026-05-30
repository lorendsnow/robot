#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "lwip/tcp.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

typedef struct Server {
    struct tcp_pcb* pcb;
    queue_t*        queue;
    err_t*          err;
} Server;

typedef struct Conn {
    struct tcp_pcb* pcb;
    queue_t*        queue;
} Conn;

typedef enum ServerErr {
    OK      = 0,
    INIT    = 1,
    CONNECT = 2,
    BIND    = 3,
} ServerErr;

ServerErr server_init(Server* s, tcp_accept_fn accept_fn);
ServerErr wifi_connect(char* ssid, char* pw);

#endif  // TCP_SERVER_H
