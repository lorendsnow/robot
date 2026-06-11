#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "ssd1306.h"

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

typedef struct render_area {
    uint8_t start_col;
    uint8_t end_col;
    uint8_t start_page;
    uint8_t end_page;
    int     buflen;
} render_area_t;

static uint8_t     _addr;
static i2c_inst_t* _i2c;

int write_command(uint8_t cmd) {
    uint8_t buf[2];
    buf[0] = COMMAND_CTRL_CO_BYTE;
    buf[1] = cmd;

    return i2c_write_blocking(_i2c, _addr, buf, 2, false);
}

void write_multi_command(uint8_t* cmds, size_t len) {
    for (int i = 0; i < len; i++) {
        write_command(cmds[i]);
    }
}

void write_data_buf(uint8_t* buf, size_t len) {
    uint8_t* temp_buf = malloc(len + 1);
    temp_buf[0]       = DATA_CTRL_BYTE;
    memcpy(temp_buf + 1, buf, len);

    i2c_write_blocking(_i2c, _addr, temp_buf, len + 1, false);

    free(temp_buf);
}

void calc_render_area_buflen(render_area_t* area) {
    area->buflen = (area->end_col - area->start_col + 1) *
                   (area->end_page - area->start_page + 1);
}

void render(uint8_t* buf, render_area_t* area) {
    uint8_t data[] = {
        SET_COL_ADDR,  area->start_col,  area->end_col,
        SET_PAGE_ADDR, area->start_page, area->end_page,
    };

    write_multi_command(data, count_of(data));
    write_data_buf(buf, area->buflen);
}

void ssd1306_init_display(uint8_t addr, i2c_inst_t* i2c, uint8_t sda_pin,
                          uint8_t scl_pin) {
    _addr = addr;
    _i2c  = i2c;

    i2c_init(_i2c, 100 * 1000);

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

void ssd1306_clear_display(void) {
    render_area_t frame = {
        .start_col  = 0,
        .end_col    = DISPLAY_WIDTH - 1,
        .start_page = 0,
        .end_page   = NUM_PAGES - 1,
    };

    calc_render_area_buflen(&frame);

    uint8_t buf[BUF_LEN];
    memset(buf, 0, BUF_LEN);
    render(buf, &frame);
}

void ssd1306_flash_screen(void) {
    write_command(SET_ALL_ON);
    sleep_ms(500);
    write_command(SET_ENTIRE_ON);
}