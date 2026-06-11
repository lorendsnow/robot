#ifndef THUMBSTICK_H
#define THUMBSTICK_H

#include "pico/stdlib.h"

/**
 * Pin and input configuration options for the thumbstick module.
 */
struct thumbstick_config {
    uint8_t x_pin;    /// GPIO pin for x-axis potentiometer.
    uint8_t x_input;  /// ADC input to read x-axis potentiometer from.
    uint8_t y_pin;    /// GPIO pin for y-axis potentiometer.
    uint8_t y_input;  /// ADC input to read y-axis potentiometer from.
};

/**
 * Represents a reading of the thumbstick potentiometers' x-y position.
 */
struct thumbstick_state {
    int8_t x;
    int8_t y;
};

/**
 * Initiate the thumbstick module with the given configuration.
 *
 * @param[in] cfg Pointer to a thumbstick configuration struct.
 */
void thumbstick_init(const struct thumbstick_config* cfg);

/**
 * Read the thumbstick's current x-y position.
 *
 * @param[out] state Pointer to thumbstick state to store the x-y position in.
 */
void thumbstick_read(struct thumbstick_state* state);

#endif  // THUMBSTICK_H