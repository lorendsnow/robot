#include "hardware/adc.h"
#include "pico/printf.h"
#include "pico/stdlib.h"

#include "thumbstick.h"

#define ADC_MAX  ((1 << 12) - 1)
#define SCALE(x) ((((float)x / (float)ADC_MAX) * 200) - 100)

static uint8_t X_PIN;
static uint8_t X_INPUT;
static uint8_t Y_PIN;
static uint8_t Y_INPUT;

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
    printf("int8 max divided by adc max: %f; int8 max divided by 2: %d\n",
           (float)INT8_MAX / (float)ADC_MAX, INT8_MAX / 2);
    printf("raw x: %d\n", x_raw);
    adc_select_input(Y_INPUT);
    uint16_t y_raw = adc_read();
    printf("raw y: %d\n", y_raw);

    state->x = SCALE(x_raw);
    state->y = SCALE(y_raw);
}