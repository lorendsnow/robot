#include <pico/time.h>
#include <stdint.h>
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"

#include "bluetooth/ble_client.h"
#include "motor_control.h"
#include "hcsr04.h"

#define GREEN_LED_PIN 15  // GPIO 15
#define RED_LED_PIN   14  // GPIO 14
#define PROX_TRIG_PIN 27
#define PROX_ECHO_PIN 28

static void core1_routine(void);

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

    /* sensor needs to be initiated AFTER the bluetooth client */
    multicore_launch_core1(core1_routine);

    btstack_run_loop_execute();

    return 0;
}

/*
 * core 1 continually reads the proximity sensor and sends readings to the
 * controller via the bluetooth connection
 */
static void core1_routine(void) {
    hcsr04_init(PROX_TRIG_PIN, PROX_ECHO_PIN);

    while (1) {
        double   dist = hcsr04_get_distance(MM);
        uint16_t dist_mm =
            (dist > 0 && dist < UINT16_MAX) ? (uint16_t)dist : UINT16_MAX;
        bt_client_set_proximity_mm(dist_mm);
        sleep_ms(100);
    }
}
