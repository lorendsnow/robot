#ifndef _MOTOR_CONTROL_H
#define _MOTOR_CONTROL_H

#include "pico/stdlib.h"

/**
 * Represents a desired drive state, as a bitmask.
 *
 * bit 0 - forward
 * bit 1 - reverse
 * bit 2 - left
 * bit 3 - right
 * bits 4-7 - reserved (speed?)
 */
typedef uint8_t drive_state_t;

void    motor_control_init(void);
void    drive_fwd(void);
void    drive_reverse(void);
void    drive_brake(void);
void    drive_coast(void);
void    drive_left(void);
void    drive_right(void);
uint8_t drive_set_state(drive_state_t state);

#endif  // _MOTOR_CONTROL_H