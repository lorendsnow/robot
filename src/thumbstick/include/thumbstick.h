#ifndef _THUMBSTICK_H
#define _THUMBSTICK_H

#include "pico/stdlib.h"

struct thumbstick_config {
    uint8_t x_pin;
    uint8_t x_input;
    uint8_t y_pin;
    uint8_t y_input;
};

struct thumbstick_state {
    int8_t x;
    int8_t y;
};

void thumbstick_init(struct thumbstick_config* cfg);
void thumbstick_read(struct thumbstick_state* state);

#endif  // _THUMBSTICK_H