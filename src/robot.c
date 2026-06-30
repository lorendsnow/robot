#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/printf.h"

#include "bluetooth/ble_client.h"
#include "motor_control.h"
#include "hcsr04.h"

#define GREEN_LED_PIN 15  // GPIO 15
#define RED_LED_PIN   14  // GPIO 14
#define PROX_TRIG_PIN 27
#define PROX_ECHO_PIN 28

void core1_routine(void);

int main(void) {
    indicator_pins_t led_pins = {
        .green = GREEN_LED_PIN,
        .red   = RED_LED_PIN,
    };

    stdio_init_all();

    multicore_launch_core1(core1_routine);

    motor_control_init();

    cyw43_arch_init();
    async_context_t* ctx = cyw43_arch_async_context();
    bt_client_init(ctx, &led_pins);

    btstack_run_loop_execute();

    return 0;
}

void core1_routine(void) {
    hcsr04_init(PROX_TRIG_PIN, PROX_ECHO_PIN);

    while (1) {
        double dist = hcsr04_get_distance(MM);

        printf("Distance: %.2f\n", dist);
    }
}
