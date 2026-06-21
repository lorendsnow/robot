#include <ctype.h>
#include <pico/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "ssd1306.h"
#include "font.h"

#define COMMAND_CTRL_BYTE    _u(0x00)
#define COMMAND_CTRL_CO_BYTE _u(0x80)
#define DATA_CTRL_BYTE       _u(0x40)

#define DISPLAY_WIDTH  _u(128)
#define DISPLAY_HEIGHT _u(64)

#define SET_MEMORY_ADDR_MODE _u(0x20)
#define SET_COL_ADDR         _u(0x21)
#define SET_PAGE_ADDR        _u(0x22)
#define SET_CONTRAST         _u(0x81)
#define SET_CHARGE_PUMP      _u(0x8D)
#define SET_ENTIRE_ON        _u(0xA4)
#define SET_ALL_ON           _u(0xA5)
#define DISPLAY_OFF          _u(0xAE)
#define DISPLAY_ON           _u(0xAF)
#define SET_MUX_RATIO        _u(0xA8)
#define SET_COM_PIN_CFG      _u(0xDA)
#define SET_PRECHARGE        _u(0xD9)
#define SET_VCOM_DESEL       _u(0xDB)

#define PAGE_HEIGHT _u(8)
#define NUM_PAGES   (DISPLAY_HEIGHT / PAGE_HEIGHT)
#define BUF_LEN     (NUM_PAGES * DISPLAY_WIDTH)

static struct {
    i2c_inst_t*           i2c;
    ssd1306_render_area_t ra;
    uint8_t               addr;
    uint8_t               frame_buf[BUF_LEN];
} display;

static int write_command(uint8_t cmd) {
    uint8_t buf[2];
    buf[0] = COMMAND_CTRL_CO_BYTE;
    buf[1] = cmd;

    return i2c_write_blocking(display.i2c, display.addr, buf, 2, false);
}

static void write_multi_command(uint8_t* cmds, size_t len) {
    for (int i = 0; i < len; i++) {
        write_command(cmds[i]);
    }
}

static void write_frame_buf(void) {
    uint8_t* temp_buf = malloc(display.ra.buflen + 1);
    temp_buf[0]       = DATA_CTRL_BYTE;
    memcpy(temp_buf + 1, display.frame_buf, display.ra.buflen);

    i2c_write_blocking(display.i2c, display.addr, temp_buf,
                       display.ra.buflen + 1, false);

    free(temp_buf);
}

static void calc_render_area_buflen() {
    display.ra.buflen = (display.ra.end_col - display.ra.start_col + 1) *
                        (display.ra.end_page - display.ra.start_page + 1);
}

static void write_char(ssd1306_cursor_t* cursor, uint8_t ch) {
    if (cursor->x > DISPLAY_WIDTH - 6 || cursor->y > DISPLAY_HEIGHT - 6) {
        return;
    }

    if (ch < ASCII_START || ch > ASCII_END) {
        return;
    }

    uint idx    = ch - ASCII_START;
    uint fb_idx = (cursor->y / 8) * DISPLAY_WIDTH + cursor->x;

    for (uint i = 0; i < 6; i++) {
        display.frame_buf[fb_idx++] = font[idx * 6 + i];
    }
}

void ssd1306_init_display(uint8_t addr, i2c_inst_t* i2c, uint8_t sda_pin,
                          uint8_t scl_pin) {
    display.addr = addr;
    display.i2c  = i2c;

    i2c_init(display.i2c, 100 * 1000);

    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);

    write_command(SET_MEMORY_ADDR_MODE);
    write_command(0x00);
    write_command(SET_MUX_RATIO);
    write_command(DISPLAY_HEIGHT - 1);
    write_command(SET_COM_PIN_CFG);
    write_command(0x12);
    write_command(SET_PRECHARGE);
    write_command(0xF1);
    write_command(SET_VCOM_DESEL);
    write_command(0x30);
    write_command(SET_CONTRAST);
    write_command(0xFF);
    write_command(SET_CHARGE_PUMP);
    write_command(0x14);
    write_command(DISPLAY_ON);
    ssd1306_clear_display();
}

void ssd1306_clear_framebuf(void) { memset(display.frame_buf, 0, BUF_LEN); }

void ssd1306_clear_display(void) {
    ssd1306_reset_render_area();
    ssd1306_clear_framebuf();
    ssd1306_render();
}

void ssd1306_flash_screen(void) {
    write_command(SET_ALL_ON);
    sleep_ms(500);
    write_command(SET_ENTIRE_ON);
}

void ssd1306_write_string(ssd1306_cursor_t cursor, char* str) {
    if (cursor.x > DISPLAY_WIDTH - 6 || cursor.y > DISPLAY_HEIGHT - 6) {
        return;
    }

    while (*str) {
        write_char(&cursor, *str++);
        cursor.x += 6;
    }
}

void ssd1306_reset_render_area(void) {
    display.ra.start_col  = 0;
    display.ra.end_col    = DISPLAY_WIDTH - 1;
    display.ra.start_page = 0;
    display.ra.end_page   = NUM_PAGES - 1;
    calc_render_area_buflen();
}

void ssd1306_set_render_area(ssd1306_render_area_t* ra) {
    display.ra.start_col  = ra->start_col;
    display.ra.end_col    = ra->end_col;
    display.ra.start_page = ra->start_page;
    display.ra.end_page   = ra->end_page;

    calc_render_area_buflen();
}

void ssd1306_render(void) {
    uint8_t data[] = {
        SET_COL_ADDR,  display.ra.start_col,  display.ra.end_col,
        SET_PAGE_ADDR, display.ra.start_page, display.ra.end_page,
    };

    write_multi_command(data, count_of(data));
    write_frame_buf();
    ssd1306_clear_framebuf();
}
