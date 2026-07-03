#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include "hardware/i2c.h"

/**
 * Represents an x-y cursor position on the display.
 */
typedef struct ssd1306_cursor {
    uint16_t x;
    uint16_t y;
} ssd1306_cursor_t;

/**
 * Represents an area of the screen to render when calling ssd1306_render.
 *
 * @note the buflen field is set/overwritten by the library; any user-set
 * buflen will be ignored.
 */
typedef struct ssd1306_render_area {
    uint8_t start_col;
    uint8_t end_col;
    uint8_t start_page;
    uint8_t end_page;
    uint    buflen;
} ssd1306_render_area_t;

/**
 * Initiate the display using I2C.
 *
 * @param addr the bus address of the driver (typically 0x3C)
 * @param i2c a Raspberry Pico i2c instance (I2C0 or I2C1)
 * @param sda_pin GPIO pin used for the SDA line
 * @param scl_pin GPIO pin used for the SCL line
 */
void ssd1306_init_display(uint8_t addr, i2c_inst_t* i2c, uint8_t sda_pin,
                          uint8_t scl_pin);

/**
 * Clear the current frame buffer.
 *
 * @note ssd1306_render() calls this function after rendering.
 */
void ssd1306_clear_framebuf(void);

/**
 * Clear all contents of the display.
 *
 * @note this function resets the display's render area to the entire screen.
 */
void ssd1306_clear_display(void);

/**
 * Cause the screen to flash by turning on all pixels, waiting for the given
 * period, then turning all pixels off.
 *
 * @param period_ms period in milliseconds to keep pixels on.
 */
void ssd1306_flash_screen(size_t period_ms);

/**
 * Write a zero-delimited string to the frame buffer.
 *
 * @param cursor position to starting writing characters at.
 * @param str the zero-delimited string to write to the screen.
 *
 * @note strings that are longer than the display width less cursor.x will be
 * cut off; this function does not wrap lines.
 */
void ssd1306_write_string(ssd1306_cursor_t cursor, char* str);

/**
 * Reset the render area to the entire screen area.
 */
void ssd1306_reset_render_area(void);

/**
 * Set the screen's rendering area.
 *
 * @param ra pointer to a struct holding the area information.
 */
void ssd1306_set_render_area(ssd1306_render_area_t* ra);

/**
 * Render the frame buffer contents to the display.
 *
 * @note this function clears the framebuffer after completion.
 */
void ssd1306_render(void);

#endif  // SSD1306_H
