#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "pico/stdlib.h"

#include "thumbstick.h"

/**
 * Initiate GPIOs and PWM for the two L298N motor controllers
 */
void motor_control_init(void);

/**
 * Set direction and speed of motors based on an x-y thumbstick reading.
 *
 * @param coords A thumbstick_state struct holding the x-y coordinates to apply.
 */
void set_motors_from_joystick_coords(struct thumbstick_state* coords);

#endif  // MOTOR_CONTROL_H