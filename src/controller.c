#include <stdint.h>
#include "btstack.h"
#include "hardware/i2c.h"
#include "pico/async_context.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/printf.h"

#include "bluetooth/ble_server.h"
#include "ssd1306.h"
#include "thumbstick.h"

#define DISPLAY_ADDR     0x3C
#define DISPLAY_SDA_PIN  16
#define DISPLAY_SCL_PIN  17
#define MULTICORE_SIGNAL 1

void core1_routine(void);

int main(void) {
    stdio_init_all();
    cyw43_arch_init();
    async_context_t* ctx = cyw43_arch_async_context();

    multicore_launch_core1(core1_routine);

    printf("setting up thumbstick...\n");

    struct thumbstick_config ts_cfg = {
        .x_pin = 26, .x_input = 0, .y_pin = 27, .y_input = 1};
    thumbstick_init(&ts_cfg);

    printf("setting up bluetooth server...\n");
    bt_server_init(ctx);

    printf("beginning bt run loop...\n");
    btstack_run_loop_execute();
}

void core1_routine(void) {
    ssd1306_init_display(DISPLAY_ADDR, I2C_INSTANCE(0), DISPLAY_SDA_PIN,
                         DISPLAY_SCL_PIN);

    ssd1306_cursor_t cursor = {.x = 0, .y = 0};

    while (1) {
        ssd1306_write_string(cursor, "Waiting on connection...");
        ssd1306_render();

        while (!bt_server_is_connected()) {
            tight_loop_contents();
        }

        ssd1306_write_string(cursor, "Connected to client");
        ssd1306_render();

        while (bt_server_is_connected()) {
            tight_loop_contents();
        }
    }
}
