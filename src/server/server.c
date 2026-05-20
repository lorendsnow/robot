#include "server.h"

ServerErr server_init(Server* s, tcp_accept_fn accept_fn) {
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
    tcp_accept(s->pcb, accept_fn);
    tcp_arg(s->pcb, s);

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