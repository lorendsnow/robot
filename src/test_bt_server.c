#include "btstack.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "bluetooth/ble_server.h"

int main(void) {
    stdio_init_all();
    cyw43_arch_init();
    bt_server_init();

    btstack_run_loop_execute();
}