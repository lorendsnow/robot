#ifndef BLUETOOTH_SERVER_H
#define BLUETOOTH_SERVER_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/async_context.h"

/**
 * Initiate the BLE server.
 *
 * @param ctx An async context instance used to get the btstack runloop.
 */
void bt_server_init(async_context_t* ctx);

/**
 * Determine whether the server is connected to a client
 *
 * @return true if connected, false otherwise
 */
bool bt_server_is_connected(void);

/**
 * Get the latest proximity sensor reading received from the robot.
 *
 * Thread-safe; may be called from any core.
 *
 * @return Distance in millimeters.
 */
uint16_t bt_server_get_proximity_mm(void);

#endif  // BLUETOOTH_SERVER_H
