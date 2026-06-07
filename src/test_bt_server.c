#include "btstack.h"
#include "pico/async_context.h"
#include "pico/cyw43_arch.h"
#include "pico/printf.h"
#include "pico/stdlib.h"

#include "bluetooth/ble_server.h"

int main(void) {
    stdio_init_all();
    cyw43_arch_init();
    async_context_t* ctx = cyw43_arch_async_context();

    printf("setting up thumbstick...\n");

    struct thumbstick_config ts_cfg = {
        .x_pin = 26, .x_input = 0, .y_pin = 27, .y_input = 1};
    thumbstick_init(&ts_cfg);

    printf("setting up bluetooth server...\n");
    btstack_run_loop_t* runloop = bt_server_init(ctx);

    printf("beginning bt run loop...\n");
    btstack_run_loop_execute();
}