#include <pico/time.h>
#include "hardware/i2c.h"

#include "ssd1306.h"

int main(void) {
    ssd1306_init_display(0x3C, i2c0, 4, 5);
    char             msg[]  = "hello there";
    ssd1306_cursor_t cursor = {
        .x = 24,
        .y = 24,
    };

    while (true) {
        ssd1306_flash_screen();
        sleep_ms(500);
        ssd1306_write_string(cursor, msg);
        ssd1306_render();
        sleep_ms(1000);
    }
}
