#include "btstack.h"
#include "pico/cyw43_arch.h"
#include "pico/printf.h"
#include "pico/stdlib.h"

#include "bluetooth/ble_client.h"
#include "motor_control.h"

#define GREEN_LED_PIN 15  // GPIO 15
#define RED_LED_PIN   14  // GPIO 14

int main(void) {
    indicator_pins_t led_pins = {
        .green = GREEN_LED_PIN,
        .red   = RED_LED_PIN,
    };

    stdio_init_all();

    motor_control_init();

    cyw43_arch_init();
    async_context_t* ctx = cyw43_arch_async_context();
    bt_client_init(ctx, &led_pins);

    btstack_run_loop_execute();

    return 0;
}
