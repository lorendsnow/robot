#include "hardware/adc.h"
#include "pico/stdlib.h"

#include "thumbstick.h"

#define ADC_MAX ((1 << 12) - 1)

static uint8_t X_PIN;
static uint8_t X_INPUT;
static uint8_t Y_PIN;
static uint8_t Y_INPUT;

int8_t scale_reading(uint16_t val) {
    return (int16_t)val * (INT8_MAX / ADC_MAX - (INT8_MAX / 2));
}

void thumbstick_init(struct thumbstick_config* cfg) {
    X_PIN   = cfg->x_pin;
    X_INPUT = cfg->x_input;
    Y_PIN   = cfg->y_pin;
    Y_INPUT = cfg->y_input;

    adc_init();
    adc_gpio_init(X_PIN);
    adc_gpio_init(Y_PIN);
}

void thumbstick_read(struct thumbstick_state* state) {
    adc_select_input(X_INPUT);
    uint16_t x_raw = adc_read();
    adc_select_input(Y_INPUT);
    uint16_t y_raw = adc_read();

    state->x = scale_reading(x_raw);
    state->y = scale_reading(y_raw);
}