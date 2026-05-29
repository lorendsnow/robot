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

/* Drive state bit-shifts */
#define FWD 1
#define REV (1 << 1)
#define LFT (1 << 2)
#define RGT (1 << 3)

/* Macros to check drive states */
#define BRAKING_STATE(x)  (!(x & (FWD | REV)))
#define FWD_STATE(x)      (x & FWD)
#define REV_STATE(x)      (x & REV)
#define STRAIGHT_STATE(x) (!(x & (LFT | RGT)))
#define LFT_STATE(x)      (x & LFT)
#define RGT_STATE(x)      (x & RGT)
#define BAD_STATE(x) \
    (((x & (FWD | REV)) == (FWD | REV)) || ((x & (LFT | RGT)) == (LFT | RGT)))

static drive_state_t _state = 0;

void print_bits(uint8_t num) {
    printf("0b");
    for (int i = 7; i >= 0; i--) {
        printf("%c", (num & (1 << i)) ? '1' : '0');
    }
}

void print_state(void) {
    printf("current drive state: ");
    print_bits(_state);
    puts("");
}

void drivetrain_init(void) {
    gpio_init_mask(ALL_GPIO);
    gpio_set_dir_out_masked(ALL_GPIO);
    drive_brake();
    print_state();
}

void drive_fwd(void) {
    if (_state & (1 << 1)) {  // brake if we're reversing
        printf("we're going in reverse, gonna brake!\n");
        drive_brake();
        return;
    }

    gpio_set_mask(FRONT_INPUT1 | FRONT_INPUT3 | REAR_INPUT1 | REAR_INPUT3 |
                  ENABLES);
    gpio_clr_mask(FRONT_INPUT2 | FRONT_INPUT4 | REAR_INPUT2 | REAR_INPUT4);

    // clear reverse and direction bits and set fwd bit
    _state &= ~(1 << 1);
    _state &= ~(1 << 2);
    _state &= ~(1 << 3);
    _state |= 1;

    print_state();
}

void drive_reverse(void) {
    if (_state & (1)) {  // brake if we're going forward
        printf("we're going forward, gonna brake!\n");
        drive_brake();
        return;
    }

    gpio_clr_mask(FRONT_INPUT1 | FRONT_INPUT3 | REAR_INPUT1 | REAR_INPUT3);
    gpio_set_mask(FRONT_INPUT2 | FRONT_INPUT4 | REAR_INPUT2 | REAR_INPUT4 |
                  ENABLES);

    // clear fwd and directions bits and set reverse bit
    _state &= ~(1);
    _state &= ~(1 << 2);
    _state &= ~(1 << 3);
    _state |= (1 << 1);

    print_state();
}

void drive_brake(void) {
    gpio_set_mask(ALL_GPIO);
    _state = 0;

    print_state();
}

void drive_coast(void) {
    gpio_clr_mask(ENABLES);
    _state &= 1 << 5;

    print_state();
}

void drive_left(void) {
    gpio_set_mask(FRONT_INPUT1 | FRONT_INPUT4 | REAR_INPUT1 | REAR_INPUT4);
    gpio_clr_mask(FRONT_INPUT2 | FRONT_INPUT3 | REAR_INPUT2 | REAR_INPUT3);

    // set left bit and clear right bit
    _state |= (1 << 2);
    _state &= ~(1 << 3);

    print_state();
}

void drive_right(void) {
    gpio_set_mask(FRONT_INPUT1 | FRONT_INPUT4 | REAR_INPUT1 | REAR_INPUT4);
    gpio_clr_mask(FRONT_INPUT2 | FRONT_INPUT3 | REAR_INPUT2 | REAR_INPUT3);

    // set right bit and clear left bit
    _state |= (1 << 3);
    _state &= ~(1 << 2);

    print_state();
}

uint8_t drive_set_state(drive_state_t state) {
    // can't go fwd & rev or lft & rgt at same time
    if (BAD_STATE(state)) {
        drive_brake();  // stop everything
        return 1;
    }

    if (BRAKING_STATE(state)) {
        drive_brake();
        return 0;
    }

    if (FWD_STATE(state)) {
        drive_fwd();
    } else {
        drive_reverse();
    }

    if (!STRAIGHT_STATE(state)) {
        LFT_STATE(state) ? drive_left() : drive_right();
    }

    return 0;
}
