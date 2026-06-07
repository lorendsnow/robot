#include "btstack.h"
#include "pico/cyw43_arch.h"
#include "pico/printf.h"
#include "pico/stdlib.h"

#include "bluetooth/ble_client.h"

int main(void) {
    stdio_init_all();
    cyw43_arch_init();
    printf("initing client!\n");
    bt_client_init();
    printf("client inited!\n");

    btstack_run_loop_execute();

    return 0;
}