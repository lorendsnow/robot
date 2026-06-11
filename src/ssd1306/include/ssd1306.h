#ifndef _SSD1306_H
#define _SSD1306_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"

void ssd1306_init_display(uint8_t addr, i2c_inst_t* i2c, uint8_t sda_pin,
                          uint8_t scl_pin);
void ssd1306_clear_display(void);
void ssd1306_flash_screen(void);

#endif  // _SSD1306