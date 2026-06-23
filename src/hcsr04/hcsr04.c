#include <stdint.h>
#include <hardware/gpio.h>
#include <pico/time.h>

#include "hcsr04.h"

#define SOS_FS        1089.90
#define SOS_MS        340.29
#define MM_MULTIPLIER (SOS_MS * 0.001 / 2.0)
#define CM_MULTIPLIER (SOS_MS * 0.0001 / 2.0)
#define IN_MULTIPLIER (SOS_FS * 0.000012 / 2.0)

static struct {
    uint8_t trig;
    uint8_t echo;
} hcsr04_pins;

static inline int64_t pulse_time_high(uint8_t pin) {
    while (!gpio_get(pin)) {
        tight_loop_contents();
    }
    absolute_time_t start = get_absolute_time();
    while (gpio_get(pin)) {
        tight_loop_contents();
    }
    absolute_time_t end = get_absolute_time();
    return absolute_time_diff_us(start, end);
}

void hcsr04_init(uint8_t trig, uint8_t echo) {
    hcsr04_pins.trig = trig;
    hcsr04_pins.echo = echo;

    gpio_init(trig);
    gpio_set_dir(trig, true);
    gpio_put(trig, false);

    gpio_init(echo);
    gpio_set_dir(echo, false);
}

double hcsr04_get_distance(hcsr04_measure_t m) {
    gpio_put(hcsr04_pins.trig, false);
    sleep_us(2);
    gpio_put(hcsr04_pins.trig, true);
    sleep_us(10);
    gpio_put(hcsr04_pins.trig, false);

    double t = (double)pulse_time_high(hcsr04_pins.echo);

    switch (m) {
        case MM:
            t *= MM_MULTIPLIER;
            break;
        case CM:
            t *= CM_MULTIPLIER;
            break;
        case IN:
            t *= IN_MULTIPLIER;
            break;
        default:
            t *= MM_MULTIPLIER;
            break;
    }

    return t;
}
