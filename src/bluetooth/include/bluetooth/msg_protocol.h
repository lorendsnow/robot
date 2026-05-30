#ifndef _BLUETOOTH_MSG_PROTOCOL_H
#define _BLUETOOTH_MSG_PROTOCOL_H

#include <stdint.h>
#include "pico/stdlib.h"

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

/**
 * Print a TLVMessage in the form `TLVMessage{type: [type], len: [len], payload:
 * [payload]}`.
 *
 * @param msg The message to print.
 * @param newline If true, adds an '\n' to the end of the printed statement.
 */
void print_msg(TLVMessage* msg, bool newline);

#endif  // _BLUETOOTH_MSG_PROTOCOL_H