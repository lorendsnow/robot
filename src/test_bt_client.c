#include "btstack.h"
#include "pico/cyw43_arch.h"
#include "pico/printf.h"
#include "pico/stdlib.h"

#include "bluetooth/ble_client.h"
#include "motor_control.h"

#define GREEN_LED_PIN 15  // GPIO 15
#define RED_LED_PIN   14  // GPIO 14

int main(void) {
    stdio_init_all();
    gpio_init_mask((1 << RED_LED_PIN) | (1 << GREEN_LED_PIN));
    gpio_set_dir_out_masked((1 << RED_LED_PIN) | (1 << GREEN_LED_PIN));
    gpio_put(RED_LED_PIN, true);
    gpio_put(GREEN_LED_PIN, false);

    motor_control_init();

    cyw43_arch_init();
    async_context_t* ctx = cyw43_arch_async_context();
    bt_client_init(ctx);

    btstack_run_loop_execute();

    return 0;
}
