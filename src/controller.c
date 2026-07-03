#include <stdint.h>
#include <stdio.h>
#include "btstack.h"
#include "hardware/i2c.h"
#include "pico/async_context.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/printf.h"
#include "pico/stdlib.h"

#include "bluetooth/ble_server.h"
#include "ssd1306.h"
#include "thumbstick.h"

#define DISPLAY_ADDR     0x3C
#define DISPLAY_SDA_PIN  16
#define DISPLAY_SCL_PIN  17
#define MULTICORE_SIGNAL 1

static void core1_routine(void);

int main(void) {
    stdio_init_all();
    cyw43_arch_init();
    async_context_t* ctx = cyw43_arch_async_context();

    multicore_launch_core1(core1_routine);

    struct thumbstick_config ts_cfg = {
        .x_pin = 26, .x_input = 0, .y_pin = 27, .y_input = 1};
    thumbstick_init(&ts_cfg);

    bt_server_init(ctx);

    btstack_run_loop_execute();
}

#define DISPLAY_REFRESH_MS 100

/*
 * core 1 receives measurement data from the robot via bluetooth and displays
 * it
 */
static void core1_routine(void) {
    ssd1306_init_display(DISPLAY_ADDR, I2C_INSTANCE(0), DISPLAY_SDA_PIN,
                         DISPLAY_SCL_PIN);

    ssd1306_cursor_t line0 = {.x = 0, .y = 0};
    ssd1306_cursor_t line1 = {.x = 0, .y = 2};
    char             prox_buf[22];

    while (1) {
        ssd1306_write_string(line0, "Waiting on connection...");
        ssd1306_render();

        while (!bt_server_is_connected()) {
            tight_loop_contents();
        }

        while (bt_server_is_connected()) {
            uint16_t dist = bt_server_get_proximity_mm();
            snprintf(prox_buf, sizeof(prox_buf), "Prox: %u mm", dist);

            ssd1306_clear_framebuf();
            ssd1306_write_string(line0, "Connected");
            ssd1306_write_string(line1, prox_buf);
            ssd1306_render();

            sleep_ms(DISPLAY_REFRESH_MS);
        }
    }
}
