#ifndef SSD1306_H
#define SSD1306_H

/*
 * SSD1306 OLED Display Controller I2C Driver.
 * Adapted from the Raspberry Pi, @daschr's and @iliapenev's example github repos.
 * Provides some helper functions to initialize, manage, and draw to the OLED screen.
 * Goto docs/context for more info
 */

#include <stdint.h>

/*
Clears the display by running memset 0 on the frame buffer which is an array with the size of SSD1306_BUF_LEN
*/
void ssd1306Clear(uint8_t *buffer);

/*
Initializes the RP2040 GPIO pins for I2C communication.
*/
void ssd1306Setup(void);

/*
Calculates the render area for the entire screen and renders the given buffer.
Requires the board to define its default i2c
*/
void ssd1306RenderFull(uint8_t *buffer);

#endif