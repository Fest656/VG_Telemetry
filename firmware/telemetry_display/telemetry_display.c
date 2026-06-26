#include <stdio.h>
#include <pico/stdlib.h>
#include "state.h"
#include "serial.h"
#include "hardware/i2c.h"

// I2C defines
// This project will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9



int main() {
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, (400 * 1000) );
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // Useful: https://github.com/raspberrypi/pico-examples/tree/master/i2c

    // Loop setup for Pico - Host communication
    while (true) {
        GameState state;
        char lineRead[TEL_BUFFER];
        if (serLineRead(lineRead, TEL_BUFFER) == 0) {
            printf("Failed to read line.\n");
            continue;
        }

        if (serDataHandler(&state, lineRead) == 0) {
            printf("Failed to parse data and populate struct.\n");
            continue; // Jumps back to the top of the loop to prevent drawing bad data
        }
        // DataHandler succesfully populated the struct
        serLineWrite(&state);

        sleep_ms(16); // Around 60 Hz
    }
    return 1;
}
