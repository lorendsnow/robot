#ifndef _MOTOR_CONTROL_H
#define _MOTOR_CONTROL_H

#include "pico/stdlib.h"

#include "thumbstick.h"

void set_motors_from_joystick_coords(struct thumbstick_state* coords);
void motor_control_init(void);
void drive_fwd(void);
void drive_reverse(void);
void drive_brake(void);
void drive_coast(void);
void drive_left(void);
void drive_right(void);

#endif  // _MOTOR_CONTROL_H