#include "drivetrain.h"

/* Left-Hand side input pins */
#define F_ENA    (1 << 7)  /// Physical pin 10 / GPIO pin 7
#define F_INPUT1 (1 << 6)  /// Physical pin 9 / GPIO pin 6
#define F_INPUT2 (1 << 5)  /// Physical pin 7 / GPIO pin 5

#define R_ENA    (1 << 26) /// Physical pin 31 / GPIO pin 26
#define R_INPUT1 (1 << 22) /// Physical pin 29 / GPIO pin 22
#define R_INPUT2 (1 << 21) /// Physical pin 27 / GPIO pin 21

/* Right-hand side input pins */
#define F_ENB    (1 << 2)  /// Physical pin 4 / GPIO pin 2
#define F_INPUT3 (1 << 4)  /// Physical pin 6 / GPIO pin 4
#define F_INPUT4 (1 << 3)  /// Physical pin 5 / GPIO pin 3

#define R_ENB    (1 << 18) /// Physical pin 24 / GPIO pin 18
#define R_INPUT3 (1 << 20) /// Physical pin 26 / GPIO pin 20
#define R_INPUT4 (1 << 19) /// Physical pin 25 / GPIO pin 19

#define ENABLES (F_ENA | F_ENB)
#define INPUTS (F_INPUT1 | F_INPUT2 | F_INPUT3 | F_INPUT4)
#define ALL_GPIO \
    (F_ENA | F_INPUT1 | F_INPUT2 | F_ENB | F_INPUT3 | F_INPUT4)  /// All GPIO pins

void drivetrain_init(void) {
    gpio_init_mask(ALL_GPIO);
    gpio_set_dir_out_masked(ALL_GPIO);
    drive_brake();
}

void drive_fwd(void) {
    gpio_set_mask(F_INPUT1 | F_INPUT3 | ENABLES);
    gpio_clr_mask(F_INPUT2 | F_INPUT4);
}

void drive_reverse(void) {
    gpio_clr_mask(F_INPUT1 | F_INPUT3);
    gpio_set_mask(F_INPUT2 | F_INPUT4 | ENABLES);
}

void drive_brake(void) { gpio_set_mask(ALL_GPIO); }

void drive_coast(void) { gpio_clr_mask(ENABLES); }

void drive_left(void) {
    gpio_set_mask(F_INPUT1 | F_INPUT4);
    gpio_clr_mask(F_INPUT2 | F_INPUT3);
}

void drive_right(void) {
    gpio_set_mask(F_INPUT1 | F_INPUT4);
    gpio_clr_mask(F_INPUT2 | F_INPUT3);
}
