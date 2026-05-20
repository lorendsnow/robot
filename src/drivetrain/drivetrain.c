#include "pico/printf.h"

#include "drivetrain.h"

/* Left-Hand side input pins */
#define FRONT_ENA    (1 << 7)  /// Physical pin 10 / GPIO pin 7
#define FRONT_INPUT1 (1 << 6)  /// Physical pin 9 / GPIO pin 6
#define FRONT_INPUT2 (1 << 5)  /// Physical pin 7 / GPIO pin 5

#define REAR_ENA    (1 << 26)  /// Physical pin 31 / GPIO pin 26
#define REAR_INPUT1 (1 << 22)  /// Physical pin 29 / GPIO pin 22
#define REAR_INPUT2 (1 << 21)  /// Physical pin 27 / GPIO pin 21

/* Right-hand side input pins */
#define FRONT_ENB    (1 << 2)  /// Physical pin 4 / GPIO pin 2
#define FRONT_INPUT3 (1 << 4)  /// Physical pin 6 / GPIO pin 4
#define FRONT_INPUT4 (1 << 3)  /// Physical pin 5 / GPIO pin 3

#define REAR_ENB    (1 << 18)  /// Physical pin 24 / GPIO pin 18
#define REAR_INPUT3 (1 << 20)  /// Physical pin 26 / GPIO pin 20
#define REAR_INPUT4 (1 << 19)  /// Physical pin 25 / GPIO pin 19

/* Aggregated bitmasks */
#define REAR_ENABLES  (REAR_ENA | REAR_ENB)
#define FRONT_ENABLES (FRONT_ENA | FRONT_ENB)
#define ENABLES       (REAR_ENABLES | FRONT_ENABLES)

#define REAR_INPUTS  (REAR_INPUT1 | REAR_INPUT2 | REAR_INPUT3 | REAR_INPUT4)
#define FRONT_INPUTS (FRONT_INPUT1 | FRONT_INPUT2 | FRONT_INPUT3 | FRONT_INPUT4)
#define RIGHT_INPUTS (FRONT_INPUT3 | FRONT_INPUT4 | REAR_INPUT3 | REAR_INPUT4)
#define LEFT_INPUTS  (FRONT_INPUT1 | FRONT_INPUT2 | REAR_INPUT1 | REAR_INPUT2)
#define INPUTS       (FRONT_INPUTS | REAR_INPUTS)

#define ALL_GPIO (ENABLES | INPUTS)  /// All GPIO pins

// state is a bitmask that holds the current drive state.
// bit 0 - forward
// bit 1 - reverse
// bit 2 - left
// bit 3 - right
// bit 4 - coast (wheels unlocked)
static uint8_t state = 0;

void print_bits(uint8_t num) {
    printf("0b");
    for (int i = 7; i >= 0; i--) {
        printf("%c", (num & (1 << i)) ? '1' : '0');
    }
}

void print_state(void) {
    printf("current drive state: ");
    print_bits(state);
    puts("");
}

void drivetrain_init(void) {
    gpio_init_mask(ALL_GPIO);
    gpio_set_dir_out_masked(ALL_GPIO);
    drive_brake();
    print_state();
}

void drive_fwd(void) {
    if (state & (1 << 1)) {  // brake if we're reversing
        printf("we're going in reverse, gonna brake!\n");
        drive_brake();
        return;
    }

    gpio_set_mask(FRONT_INPUT1 | FRONT_INPUT3 | REAR_INPUT1 | REAR_INPUT3 |
                  ENABLES);
    gpio_clr_mask(FRONT_INPUT2 | FRONT_INPUT4 | REAR_INPUT2 | REAR_INPUT4);

    // clear reverse and direction bits and set fwd bit
    state &= ~(1 << 1);
    state &= ~(1 << 2);
    state &= ~(1 << 3);
    state |= 1;

    print_state();
}

void drive_reverse(void) {
    if (state & (1)) {  // brake if we're going forward
        printf("we're going forward, gonna brake!\n");
        drive_brake();
        return;
    }

    gpio_clr_mask(FRONT_INPUT1 | FRONT_INPUT3 | REAR_INPUT1 | REAR_INPUT3);
    gpio_set_mask(FRONT_INPUT2 | FRONT_INPUT4 | REAR_INPUT2 | REAR_INPUT4 |
                  ENABLES);

    // clear fwd and directions bits and set reverse bit
    state &= ~(1);
    state &= ~(1 << 2);
    state &= ~(1 << 3);
    state |= (1 << 1);

    print_state();
}

void drive_brake(void) {
    gpio_set_mask(ALL_GPIO);
    state = 0;

    print_state();
}

void drive_coast(void) {
    gpio_clr_mask(ENABLES);
    state &= 1 << 5;

    print_state();
}

void drive_left(void) {
    gpio_set_mask(FRONT_INPUT1 | FRONT_INPUT4 | REAR_INPUT1 | REAR_INPUT4);
    gpio_clr_mask(FRONT_INPUT2 | FRONT_INPUT3 | REAR_INPUT2 | REAR_INPUT3);

    // set left bit and clear right bit
    state |= (1 << 2);
    state &= ~(1 << 3);

    print_state();
}

void drive_right(void) {
    gpio_set_mask(FRONT_INPUT1 | FRONT_INPUT4 | REAR_INPUT1 | REAR_INPUT4);
    gpio_clr_mask(FRONT_INPUT2 | FRONT_INPUT3 | REAR_INPUT2 | REAR_INPUT3);

    // set right bit and clear left bit
    state |= (1 << 3);
    state &= ~(1 << 2);

    print_state();
}
