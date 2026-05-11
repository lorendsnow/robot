#ifndef _DRIVETRAIN_H
#define _DRIVETRAIN_H

#include "pico/stdlib.h"

void drivetrain_init(void);
void drive_fwd(void);
void drive_reverse(void);
void drive_brake(void);
void drive_coast(void);
void drive_left(void);
void drive_right(void);

#endif  // _DRIVETRAIN_H