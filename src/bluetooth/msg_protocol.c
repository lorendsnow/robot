#include "pico/printf.h"

#include "bluetooth/msg_protocol.h"

void print_msg(TLVMessage* msg, bool newline) {
    char* type;
    switch (msg->type) {
        case ACK:
            type = "ACK";
            break;
        case DRIVE:
            type = "DRIVE";
            break;
        case DATA:
            type = "DATA";
            break;
        default:
            type = "unknown";
            break;
    }

    printf("TLVMessage{type: %s, len: %d, payload: %X}", type, msg->len,
           msg->payload);
    if (newline) {
        puts("");
    }
}