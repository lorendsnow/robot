#ifndef _BLUETOOTH_CLIENT_H
#define _BLUETOOTH_CLIENT_H

#include "pico/async_context.h"

const btstack_run_loop_t* bt_client_init(async_context_t* ctx);

#endif  // _BLUETOOTH_CLIENT_H
