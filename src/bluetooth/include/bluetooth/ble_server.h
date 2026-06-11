#ifndef BLUETOOTH_SERVER_H
#define BLUETOOTH_SERVER_H

#include "pico/async_context.h"

#include "thumbstick.h"

void bt_server_init(async_context_t* ctx);

#endif  // BLUETOOTH_SERVER_H