#ifndef _BLUETOOTH_SERVER_H
#define _BLUETOOTH_SERVER_H

#include "thumbstick.h"
#include "pico/sync.h"

mutex_t state_mtx;

void                     bt_server_init(void);
struct thumbstick_state* get_state(void);

#endif  // _BLUETOOTH_SERVER_H