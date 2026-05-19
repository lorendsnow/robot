#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"

#include "tcp_fns.h"
#include "server.h"

err_t receive(void* arg, struct tcp_pcb* tcp_pcb, struct pbuf* p, err_t err) {
    cyw43_arch_lwip_begin();
    printf("server received data from %d.%d.%d.%d\n",
           (uint8_t)(tcp_pcb->remote_ip.addr),
           (uint8_t)(tcp_pcb->remote_ip.addr >> 8),
           (uint8_t)(tcp_pcb->remote_ip.addr >> 16),
           (uint8_t)(tcp_pcb->remote_ip.addr >> 24));

    printf("number of bytes received: %d\n", p->len);

    if (!p) {
        printf("connection closed by client\n");
        tcp_close(tcp_pcb);
        free(arg);
        return ERR_OK;
    }

    memcpy(((Conn*)arg)->buf, p->payload, p->len);
    printf("received payload: ");
    for (int i = 0; i < p->len; i++) {
        printf("%X ", ((Conn*)arg)->buf[i]);
    }
    puts("");

    tcp_recved(tcp_pcb, p->len);
    cyw43_arch_lwip_end();

    return ERR_OK;
}

err_t accept(void* arg, struct tcp_pcb* newpcb, err_t err) {
    cyw43_arch_lwip_begin();

    Conn* conn = malloc(sizeof(Conn));
    conn->pcb  = newpcb;
    tcp_arg(conn->pcb, conn);
    tcp_recv(conn->pcb, receive);

    printf("server accepted connection\n");
    cyw43_arch_lwip_end();

    return ERR_OK;
}