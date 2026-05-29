#ifndef _SERVER_H
#define _SERVER_H

#include "lwipopts.h"
#include "pico/cyw43_arch.h"
#include "pico/util/queue.h"
#include "pico/stdlib.h"
#include "lwip/tcp.h"

typedef struct Server {
    struct tcp_pcb* pcb;
    queue_t*        queue;
    err_t*          err;
} Server;

typedef struct Conn {
    struct tcp_pcb* pcb;
    queue_t*        queue;
    uint8_t         buf[256];
} Conn;

typedef enum ServerErr {
    OK      = 0,
    INIT    = 1,
    CONNECT = 2,
    BIND    = 3,
} ServerErr;

typedef uint8_t Ip4Addr[4];

/**
 * Represents the type of message being sent in the TLV message format.
 */
typedef enum MessageType {
    ACK   = 0,
    DRIVE = 1,
    DATA  = 2,
} MessageType;

/**
 * Represents a TCP message having a type, payload length, and value (payload).
 */
typedef struct TLVMessage {
    MessageType type;
    uint32_t    len;
    uint32_t    payload;
} TLVMessage;

ServerErr server_init(Server* s, tcp_accept_fn accept_fn);
void      get_ip4_addr(Ip4Addr addr);
ServerErr wifi_connect(char* ssid, char* pw);

#endif  // _SERVER_H