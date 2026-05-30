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
    if (p->tot_len % sizeof(TLVMessage) != 0) {
        printf(
            "warning: received bytes don't align on a message length boundary; "
            "got %d extra bytes\n",
            p->tot_len % sizeof(TLVMessage));
    }

    /* copy messages into queue for processing */
    uint8_t msgs = 0;
    for (struct pbuf* q = p; q != NULL; q = q->next) {
        uint16_t copied = 0;
        while (copied <= q->len - sizeof(TLVMessage)) {
            queue_add_blocking(((Conn*)arg)->queue, q->payload + copied);
            copied += sizeof(TLVMessage);
            msgs++;
        }
    }

    printf("added %d messages to queue\n", msgs);

    tcp_recved(tcp_pcb, p->tot_len);
    pbuf_free(p);

    printf("server done receiving\n");

    return ERR_OK;
}

err_t accept(void* arg, struct tcp_pcb* newpcb, err_t err) {
    Conn* conn  = malloc(sizeof(Conn));
    conn->pcb   = newpcb;
    conn->queue = ((Server*)arg)->queue;

    tcp_arg(conn->pcb, conn);
    tcp_recv(conn->pcb, receive);

    printf("server accepted connection\n");

    return ERR_OK;
}