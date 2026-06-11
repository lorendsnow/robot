#ifndef BLUETOOTH_SERVER_H
#define BLUETOOTH_SERVER_H

#include "pico/async_context.h"

#include "thumbstick.h"

/**
 * Initiate the BLE server.
 *
 * @param ctx An async context instance used to get the btstack runloop.
 */
void bt_server_init(async_context_t* ctx);

#endif  // BLUETOOTH_SERVER_H