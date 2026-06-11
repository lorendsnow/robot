#ifndef BLUETOOTH_CLIENT_H
#define BLUETOOTH_CLIENT_H

#include "pico/async_context.h"
#include "pico/btstack_run_loop_async_context.h"

/**
 * Represents GPIO pins assigned to green and red indicator lights.
 *
 * The client will indicate red if it is not connected to the BLE server, and
 * indicate green while it is connected.
 */
typedef struct indicator_pins {
    uint8_t green;
    uint8_t red;
} indicator_pins_t;

/**
 * Initiate the BLE client.
 *
 * @param ctx A Pico-SDK async_context used to get the btstack runloop.
 * @param pins An indicator pins struct to set the GPIO pins for the indicator
 * lights
 * @return the btstack runloop instance.
 */
const btstack_run_loop_t* bt_client_init(async_context_t*  ctx,
                                         indicator_pins_t* pins);

#endif  // BLUETOOTH_CLIENT_H
