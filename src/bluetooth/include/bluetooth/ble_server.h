#ifndef _BLUETOOTH_SERVER_H
#define _BLUETOOTH_SERVER_H

#include "thumbstick.h"

const btstack_run_loop_t* bt_server_init(async_context_t* ctx);

#endif  // _BLUETOOTH_SERVER_H