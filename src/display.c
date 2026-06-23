#include <pico/stdio.h>
#include <pico/time.h>
#include <pico/types.h>
#include <stdint.h>
#include <stdio.h>
#include "hardware/i2c.h"

#include "hcsr04.h"
#include "ssd1306.h"

#define TRIG_PIN 17
#define ECHO_PIN 16

int main(void) {
    char buf[32];
    ssd1306_init_display(0x3C, i2c0, 4, 5);
    ssd1306_cursor_t cursor = {
        .x = 0,
        .y = 0,
    };
    ssd1306_write_string(cursor, "initiating prox sensor...");
    ssd1306_render();

    hcsr04_init(TRIG_PIN, ECHO_PIN);
    sleep_ms(500);

    ssd1306_write_string(cursor, "entering loop...");
    ssd1306_render();

    while (true) {
        double dist = hcsr04_get_distance(MM);
        sprintf(buf, "Distance: %.2f mm", dist);
        ssd1306_write_string(cursor, buf);
        ssd1306_render();
        sleep_ms(500);

        dist = hcsr04_get_distance(CM);
        sprintf(buf, "Distance: %.2f cm", dist);
        ssd1306_write_string(cursor, buf);
        ssd1306_render();
        sleep_ms(500);

        dist = hcsr04_get_distance(IN);
        sprintf(buf, "Distance: %.2f inches", dist);
        ssd1306_write_string(cursor, buf);
        ssd1306_render();
        sleep_ms(500);
    }
}
