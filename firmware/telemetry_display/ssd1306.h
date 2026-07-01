#ifndef SSD1306_H
#define SSD1306_H

/*
 * SSD1306 OLED Display Controller I2C Driver.
 * Adapted from the Raspberry Pi and @daschr's example github repos.
 * Provides the low-level functions and configuration macros required to
 * initialize, manage, and draw to the OLED screen.
 * Goto docs/context for more info
 */

#include <stdint.h>

// Display dimensions and I2C configuration
#define SSD1306_HEIGHT              32
#define SSD1306_WIDTH               128
#define SSD1306_I2C_ADDR            0x3C
#define SSD1306_I2C_CLK             400
#define SSD1306_I2C_PORT            i2c0
#define SSD1306_I2C_SDA             8
#define SSD1306_I2C_SCL             9

#define SSD1306_PAGE_HEIGHT         8
#define SSD1306_NUM_PAGES           (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT)
#define SSD1306_BUF_LEN             (SSD1306_NUM_PAGES * SSD1306_WIDTH)

// Commands (see datasheet)
#define SSD1306_SET_MEM_MODE        0x20
#define SSD1306_SET_COL_ADDR        0x21
#define SSD1306_SET_PAGE_ADDR       0x22
#define SSD1306_SET_HORIZ_SCROLL    0x26
#define SSD1306_SET_SCROLL          0x2E

#define SSD1306_SET_DISP_START_LINE 0x40

#define SSD1306_SET_CONTRAST        0x81
#define SSD1306_SET_CHARGE_PUMP     0x8D

#define SSD1306_SET_SEG_REMAP       0xA0
#define SSD1306_SET_ENTIRE_ON       0xA4
#define SSD1306_SET_ALL_ON          0xA5
#define SSD1306_SET_NORM_DISP       0xA6
#define SSD1306_SET_INV_DISP        0xA7
#define SSD1306_SET_MUX_RATIO       0xA8
#define SSD1306_SET_DISP            0xAE
#define SSD1306_SET_COM_OUT_DIR     0xC0
#define SSD1306_SET_COM_OUT_DIR_FLIP 0xC0

#define SSD1306_SET_DISP_OFFSET     0xD3
#define SSD1306_SET_DISP_CLK_DIV    0xD5
#define SSD1306_SET_PRECHARGE       0xD9
#define SSD1306_SET_COM_PIN_CFG     0xDA
#define SSD1306_SET_VCOM_DESEL      0xDB

#define SSD1306_WRITE_MODE          0xFE
#define SSD1306_READ_MODE           0xFF


/*
Structure representing the coordinates of the display render area.
Defines column and page bounds to perform targeted updates to the screen.
*/
struct render_area {
    uint8_t start_col;
    uint8_t end_col;
    uint8_t start_page;
    uint8_t end_page;

    int buflen;
};

/*
Calculates the flattened buffer length (buflen) needed for the given render area.
Formula: (end_col - start_col + 1) * (end_page - start_page + 1).
*/
void calc_render_area_buflen(struct render_area *area);

/*
Sends a single control command to the display driver.
Prepend control byte (0x80) to indicate Co = 1, D/C = 0 (command to follow).
*/
void SSD1306_send_cmd(uint8_t cmd);

/*
Sequentially transmits a list of command bytes to the display driver.
*/
void SSD1306_send_cmd_list(uint8_t *buf, int num);

/*
Sends the entire framebuffer to the display driver memory.
Prepends the data byte (0x40) to indicate data stream.
*/
void SSD1306_send_buf(uint8_t buf[], int buflen);

/*
Configures the SSD1306 controller with recommended timings and resolution settings.
Initializes memory layout (horizontal addressing mode) and powers on the display.
*/
void SSD1306_init();

/*
Configures the column and page boundaries and transmits the buffer to target render area.
*/
void render(uint8_t *buf, struct render_area *area);

/*
Translates an ASCII character into its corresponding numerical index 
within the 8x8 font array.
*/
int GetFontIndex(uint8_t ch);

/*
Writes a single character to the display
*/
void WriteChar(uint8_t *buf, int16_t x, int16_t y, uint8_t ch);

/*
Uses WriteChar to write an entire string to the display
*/
void WriteString(uint8_t *buf, int16_t x, int16_t y, char *str);

/*
Clears the display by running memset 0 on the frame buffer which is an array withg the size of SSD1306_BUF_LEN
*/
void ssd1306_clear(uint8_t *buffer);

/*
Initializes the RP2040 GPIO pins for I2C communication.
*/
void ssd1306_setup(void);

/*
Calculates the render area for the entire screen and renders the given buffer.
*/
void ssd1306_render_full(uint8_t *buffer);

#endif