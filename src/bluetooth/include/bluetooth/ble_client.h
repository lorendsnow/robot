#ifndef _BLUETOOTH_CLIENT_H
#define _BLUETOOTH_CLIENT_H

#include "pico/async_context.h"

typedef struct indicator_pins {
    uint8_t green;
    uint8_t red;
} indicator_pins_t;

const btstack_run_loop_t* bt_client_init(async_context_t*  ctx,
                                         indicator_pins_t* pins);

#endif  // _BLUETOOTH_CLIENT_H
