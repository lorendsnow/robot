#include "hardware/clocks.h"
#include "hardware/pwm.h"

#include "motor_control.h"
#include "thumbstick.h"

/* Pin Definitions */
#define FRONT_ENA_PIN    8   /// Physical pin 11 / GPIO pin 8
#define FRONT_INPUT1_PIN 6   /// Physical pin 9 / GPIO pin 6
#define FRONT_INPUT2_PIN 5   /// Physical pin 7 / GPIO pin 5
#define REAR_ENA_PIN     26  /// Physical pin 31 / GPIO pin 26
#define REAR_INPUT1_PIN  22  /// Physical pin 29 / GPIO pin 22
#define REAR_INPUT2_PIN  21  /// Physical pin 27 / GPIO pin 21
#define FRONT_ENB_PIN    10  /// Physical pin 14 / GPIO pin 10
#define FRONT_INPUT3_PIN 4   /// Physical pin 6 / GPIO pin 4
#define FRONT_INPUT4_PIN 3   /// Physical pin 5 / GPIO pin 3
#define REAR_ENB_PIN     16  /// Physical pin 21 / GPIO pin 16
#define REAR_INPUT3_PIN  20  /// Physical pin 26 / GPIO pin 20
#define REAR_INPUT4_PIN  19  /// Physical pin 25 / GPIO pin 19

/* Left-Hand side bitmasks */
#define FRONT_ENA    (1 << FRONT_ENA_PIN)
#define FRONT_INPUT1 (1 << FRONT_INPUT1_PIN)
#define FRONT_INPUT2 (1 << FRONT_INPUT2_PIN)

#define REAR_ENA    (1 << REAR_ENA_PIN)
#define REAR_INPUT1 (1 << REAR_INPUT1_PIN)
#define REAR_INPUT2 (1 << REAR_INPUT2_PIN)

/* Right-hand side bitmasks */
#define FRONT_ENB    (1 << FRONT_ENB_PIN)
#define FRONT_INPUT3 (1 << FRONT_INPUT3_PIN)
#define FRONT_INPUT4 (1 << FRONT_INPUT4_PIN)

#define REAR_ENB    (1 << REAR_ENB_PIN)
#define REAR_INPUT3 (1 << REAR_INPUT3_PIN)
#define REAR_INPUT4 (1 << REAR_INPUT4_PIN)

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

#define PWM_WRAP    100  // 0-100 duty range
#define PWM_FREQ    100  // 100 hz frequency
#define JS_DEADZONE 5    // deadzone to avoid joystick calibration issues

/*
 * For curved turns, scale down the forward PWM to the turn-side motors
 * relative to the non-turn-side motors
 */
#define turn_factor_uncapped(x, y) \
    ((((float)PWM_WRAP - (float)(x)) / (float)PWM_WRAP) * (y))

static const uint32_t ENABLE_PINS[4] = {FRONT_ENA_PIN, FRONT_ENB_PIN,
                                        REAR_ENA_PIN, REAR_ENB_PIN};

static void set_speed(int8_t x, int8_t y) {
    uint16_t scaled_x;
    if (x > 0) {  // turning right
        scaled_x = turn_factor_uncapped(x, y);
        pwm_set_gpio_level(FRONT_ENA_PIN, y);
        pwm_set_gpio_level(REAR_ENB_PIN, y);
        pwm_set_gpio_level(FRONT_ENB_PIN, scaled_x);
        pwm_set_gpio_level(REAR_ENA_PIN, scaled_x);
    } else if (x < 0) {  // turning left
        scaled_x = turn_factor_uncapped(-x, y);
        pwm_set_gpio_level(FRONT_ENA_PIN, scaled_x);
        pwm_set_gpio_level(REAR_ENB_PIN, scaled_x);
        pwm_set_gpio_level(FRONT_ENB_PIN, y);
        pwm_set_gpio_level(REAR_ENA_PIN, y);
    } else {
        for (int i = 0; i < 4; i++) {
            pwm_set_gpio_level(ENABLE_PINS[i], y);
        }
    }
}

static void drive_fwd(void) {
    gpio_set_mask(FRONT_INPUT1 | FRONT_INPUT3 | REAR_INPUT1 | REAR_INPUT3);
    gpio_clr_mask(FRONT_INPUT2 | FRONT_INPUT4 | REAR_INPUT2 | REAR_INPUT4);
}

static void drive_reverse(void) {
    gpio_clr_mask(FRONT_INPUT1 | FRONT_INPUT3 | REAR_INPUT1 | REAR_INPUT3);
    gpio_set_mask(FRONT_INPUT2 | FRONT_INPUT4 | REAR_INPUT2 | REAR_INPUT4);
}

static void drive_brake(void) { gpio_set_mask(ALL_GPIO); }

static void spin(int8_t x) {
    if (x < 0) {
        gpio_set_mask(FRONT_INPUT1 | FRONT_INPUT4 | REAR_INPUT1 | REAR_INPUT4);
        gpio_clr_mask(FRONT_INPUT2 | FRONT_INPUT3 | REAR_INPUT2 | REAR_INPUT3);
        x *= -1;
    } else {
        gpio_set_mask(FRONT_INPUT1 | FRONT_INPUT4 | REAR_INPUT1 | REAR_INPUT4);
        gpio_clr_mask(FRONT_INPUT2 | FRONT_INPUT3 | REAR_INPUT2 | REAR_INPUT3);
    }

    for (int i = 0; i < 4; i++) {
        pwm_set_gpio_level(ENABLE_PINS[i], x);
    }
}

void motor_control_init(void) {
    gpio_init_mask(INPUTS);
    gpio_set_dir_out_masked(INPUTS);

    gpio_set_function_masked(ENABLES, GPIO_FUNC_PWM);

    pwm_config cfg         = pwm_get_default_config();
    uint32_t   clk_divisor = clock_get_hz(clk_sys) / PWM_FREQ;

    pwm_config_set_clkdiv_int(&cfg, clk_divisor);
    pwm_config_set_wrap(&cfg, PWM_WRAP);

    // NOLINTBEGIN(*.FixedAddressDereference)
    for (int i = 0; i < 4; i++) {
        uint slicenum = pwm_gpio_to_slice_num(ENABLE_PINS[i]);
        pwm_init(slicenum, &cfg, false);
        pwm_set_gpio_level(ENABLE_PINS[i], 0);
        pwm_set_enabled(slicenum, true);
    }
    // NOLINTEND(*.FixedAddressDereference)

    drive_brake();
}

void set_motors_from_joystick_coords(struct thumbstick_state* coords) {
    int8_t x =
        (coords->x <= JS_DEADZONE && coords->x >= -JS_DEADZONE) ? 0 : coords->x;
    int8_t y = (coords->y <= JS_DEADZONE && coords->y >= -JS_DEADZONE)
                   ? 0
                   : (int8_t)-coords
                         ->y;  // invert y axis so up on thumbstick is forward

    if (y > 0) {
        drive_fwd();
        set_speed(x, y);
    } else if (y < 0) {
        drive_reverse();
        set_speed(x, -y);  // NOLINT(*-conversions)
    } else {
        if (x == 0) {
            drive_brake();
        } else {
            spin(x);
        }
    }
}