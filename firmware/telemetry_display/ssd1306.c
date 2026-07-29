#include <string.h>
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "ssd1306_i2c.h"

/*
https://en.cppreference.com/cpp/string/byte/memset
*/
void ssd1306Clear(uint8_t *buffer) {
    memset(buffer, 0, SSD1306_BUF_LEN);
}

// These are the board specific pins
void ssd1306Setup(void) {
    i2c_init(i2c_default, SSD1306_I2C_CLK * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

}

// Requires the target board to define a default I2C (see pico-sdk board header / i2c_default)
void ssd1306RenderFull(uint8_t *buffer) {
    struct render_area frame_area = {
        .start_col = 0,
        .end_col = SSD1306_WIDTH - 1,
        .start_page = 0,
        .end_page = SSD1306_NUM_PAGES - 1
    };
    calc_render_area_buflen(&frame_area);
    render(buffer, &frame_area);
}