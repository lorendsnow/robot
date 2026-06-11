#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "ssd1306.h"

int main(void) {
    stdio_init_all();

    ssd1306_init_display(0x3C, i2c0, 4, 5);

    while (true) {
        ssd1306_flash_screen();
        sleep_ms(500);
    }
}