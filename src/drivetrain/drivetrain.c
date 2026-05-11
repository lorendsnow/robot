#include "drivetrain.h"

/* Left-Hand side input pins */
#define INPUT1 0x40  /// Physical pin 9 / GPIO pin 6
#define INPUT2 0x80  /// Physical pin 10 / GPIO pin 7

/* Right-hand side input pins */
#define INPUT3 0x10000  /// Physical pin 21 / GPIO pin 16
#define INPUT4 0x20000  /// Physical pin 22 / GPIO pin 17

#define ALL_INPUTS (INPUT1 | INPUT2 | INPUT3 | INPUT4)  /// All input pins

void drivetrain_init(void) {
    gpio_init_mask(ALL_INPUTS);
    gpio_set_dir_out_masked(ALL_INPUTS);
    drive_brake();
}

void drive_fwd(void) {
    gpio_set_mask(INPUT1 | INPUT3);
    gpio_clr_mask(INPUT2 | INPUT4);
}

void drive_reverse(void) {
    gpio_clr_mask(INPUT1 | INPUT3);
    gpio_set_mask(INPUT2 | INPUT4);
}

void drive_brake(void) { gpio_set_mask(ALL_INPUTS); }

void drive_coast(void) { gpio_clr_mask(ALL_INPUTS); }

void drive_left(void) {
    gpio_set_mask(INPUT1 | INPUT4);
    gpio_clr_mask(INPUT2 | INPUT3);
}

void drive_right(void) {
    gpio_set_mask(INPUT1 | INPUT4);
    gpio_clr_mask(INPUT2 | INPUT3);
}
