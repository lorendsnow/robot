#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include "hardware/i2c.h"

typedef struct ssd1306_cursor {
    uint16_t x;
    uint16_t y;
} ssd1306_cursor_t;

typedef struct ssd1306_render_area {
    uint8_t start_col;
    uint8_t end_col;
    uint8_t start_page;
    uint8_t end_page;
    uint    buflen;
} ssd1306_render_area_t;

void ssd1306_init_display(uint8_t addr, i2c_inst_t* i2c, uint8_t sda_pin,
                          uint8_t scl_pin);
void ssd1306_clear_framebuf(void);
void ssd1306_clear_display(void);
void ssd1306_flash_screen(void);
void ssd1306_write_string(ssd1306_cursor_t cursor, char* str);
void ssd1306_reset_render_area(void);
void ssd1306_set_render_area(ssd1306_render_area_t* ra);
void ssd1306_render(void);

#endif  // SSD1306_H
