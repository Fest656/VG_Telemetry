#include <stdio.h>
#include <pico/stdlib.h>
#include "state.h"
#include "serial.h"
#include "ssd1306.h"

int main() {
    stdio_init_all();
    // Initialize the hardware pins and the SSD1306 screen
    ssd1306_setup();
    SSD1306_init();
    
    // Frame buffer
    uint8_t buf[SSD1306_BUF_LEN];

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

        // Wipe the frame clean
        ssd1306_clear(buf);

        char textBuffer[TEL_BUFFER];
        
        // Our font is 8 pixels tall

        // sprintf just places that formatted string into textBuffer
        sprintf(textBuffer, "Health: %d", state.health);
        // WriteString then just takes textBuffer and puts it in the frame
        WriteString(buf, 0, 0, textBuffer);
        
        sprintf(textBuffer, "Armor : %d", state.armor);
        WriteString(buf, 0, 8, textBuffer);
        
        sprintf(textBuffer, "Mag   : %d", state.magAmmo);
        WriteString(buf, 0, 16, textBuffer);
        
        sprintf(textBuffer, "Res   : %d", state.reserveAmmo);
        WriteString(buf, 0, 24, textBuffer);
        
        // Renders the frame
        ssd1306_render_full(buf);


        sleep_ms(16); // Around 60 Hz
    }
    return 1;
}
