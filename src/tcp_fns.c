#include "lwip/tcp.h"
#include "pico/cyw43_arch.h"

#include "tcp_fns.h"
#include "server.h"

err_t receive(void* arg, struct tcp_pcb* tcp_pcb, struct pbuf* p, err_t err) {
    printf("server received data from %d.%d.%d.%d\n",
           (uint8_t)(tcp_pcb->remote_ip.addr),
           (uint8_t)(tcp_pcb->remote_ip.addr >> 8),
           (uint8_t)(tcp_pcb->remote_ip.addr >> 16),
           (uint8_t)(tcp_pcb->remote_ip.addr >> 24));

    if (!p) {
        printf("connection closed by client\n");
        return ERR_CLSD;
    }

    printf("number of bytes received: %d\n", p->tot_len);

    for (struct pbuf* q = p; q != NULL; q = q->next) {
        uint16_t copied = pbuf_copy_partial(q, ((Conn*)arg)->buf, 1024, 0);
        if (!copied) {
            printf("failed to copy bytes\n");
            return ERR_BUF;
        }
    }

    tcp_recved(tcp_pcb, p->tot_len);

    printf("adding bytes to queue: ");
    for (int i = 0; i < p->tot_len; i++) {
        printf("%X ", ((Conn*)arg)->buf[i]);
        queue_add_blocking(((Conn*)arg)->queue, ((Conn*)arg)->buf + i);
    }
    puts("");

    pbuf_free(p);

    return ERR_OK;
}

err_t accept(void* arg, struct tcp_pcb* newpcb, err_t err) {
    Conn* conn  = malloc(sizeof(Conn));
    conn->pcb   = newpcb;
    conn->queue = (queue_t*)arg;

    tcp_arg(conn->pcb, conn);
    tcp_recv(conn->pcb, receive);

    printf("server accepted connection\n");

    return ERR_OK;
}