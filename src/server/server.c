#include "server.h"

err_t receive(void* arg, struct tcp_pcb* tpcb, struct pbuf* p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        free(arg);
        return ERR_OK;
    }

    cyw43_arch_lwip_begin();

    Conn* conn   = (Conn*)arg;
    conn->cursor = 0;
    uint8_t* pl  = (uint8_t*)(p->payload);

    for (uint32_t i = 0; i < p->len; i++) {
        memcpy(conn->buf[conn->cursor], pl + i, sizeof(uint8_t));
        queue_add_blocking(conn->queue, conn->buf + conn->cursor++);
    }

    tcp_recved(tpcb, p->len);

    cyw43_arch_lwip_end();

    return ERR_OK;
}

err_t accept(void* arg, struct tcp_pcb* newpcb, err_t err) {
    cyw43_arch_lwip_begin();
    Conn* conn   = malloc(sizeof(Conn));
    conn->pcb    = newpcb;
    conn->queue  = (queue_t*)arg;
    conn->cursor = 0;

    tcp_arg(conn->pcb, conn);
    tcp_recv(conn->pcb, receive);

    printf("server accepted connection\n");
    cyw43_arch_lwip_end();

    return ERR_OK;
}

ServerErr server_init(Server* s) {
    cyw43_arch_lwip_begin();
    s->pcb = tcp_new();

    err_t err = tcp_bind(s->pcb, IP_ADDR_ANY, 7);
    switch (err) {
        case ERR_USE:
            printf("couldn't bind server to port %d; port already in use\n", 7);
            return BIND;
        case ERR_VAL:
            printf(
                "couldn't bind server to port %d; pcb is not in a valid "
                "state\n",
                7);
            return BIND;
        case ERR_OK:
            printf("successfully bound server to port 7\n");
            break;
        default:
            printf("encountered unknown error trying to bind server: %d\n",
                   err);
            return BIND;
    }

    s->pcb = tcp_listen_with_backlog_and_err(s->pcb, 3, s->err);
    tcp_accept(s->pcb, accept);
    tcp_arg(s->pcb, s->queue);

    cyw43_arch_lwip_end();

    return OK;
}

void get_ip4_addr(Ip4Addr addr) {
    memcpy(addr, &(cyw43_state.netif[0].ip_addr.addr), sizeof(Ip4Addr));
}

ServerErr wifi_connect(char* ssid, char* pw) {
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return INIT;
    }

    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, pw, CYW43_AUTH_WPA2_AES_PSK,
                                           30000)) {
        return CONNECT;
    }

    return OK;
}