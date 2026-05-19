#include "pico/stdlib.h"
#include "drivetrain.h"

int main(void) {
    stdio_init_all();

    drivetrain_init();

    sleep_ms(1000);

    while (1) {
        drive_fwd();
        sleep_ms(5000);

        drive_coast();
        sleep_ms(1000);

        drive_reverse();
        sleep_ms(5000);

        drive_brake();
        sleep_ms(500);

        drive_right();
        sleep_ms(5000);

        drive_left();
        sleep_ms(5000);
    }
}