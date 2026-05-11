#include "drivetrain.h"

/* Left-Hand side input pins */
#define INPUT1 0x40  /// Physical pin 9 / GPIO pin 6
#define INPUT2 0x80  /// Physical pin 10 / GPIO pin 7

/* Right-hand side input pins */
#define INPUT3 0x10000  /// Physical pin 21 / GPIO pin 16
#define INPUT4 0x20000  /// Physical pin 22 / GPIO pin 17

void drivetrain_init(void) {
    gpio_init_mask(INPUT1 | INPUT2 | INPUT3 | INPUT4);
    drive_brake();
}

void drive_fwd(void) {
    gpio_put_masked(INPUT1 | INPUT3, 1);
    gpio_put_masked(INPUT2 | INPUT4, 0);
}

void drive_reverse(void) {
    gpio_put_masked(INPUT1 | INPUT3, 0);
    gpio_put_masked(INPUT2 | INPUT4, 1);
}

void drive_brake(void) {
    gpio_put_masked(INPUT1 | INPUT2 | INPUT3 | INPUT4, 1);
}

void drive_coast(void) {
    gpio_put_masked(INPUT1 | INPUT2 | INPUT3 | INPUT4, 0);
}

void drive_left(void) {
    gpio_put_masked(INPUT1 | INPUT4, 0);
    gpio_put_masked(INPUT2 | INPUT3, 1);
}

void drive_right(void) {
    gpio_put_masked(INPUT1 | INPUT4, 1);
    gpio_put_masked(INPUT2 | INPUT3, 0);
}
