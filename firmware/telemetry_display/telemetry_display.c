#include <stdint.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include "state.h"
#include "serial.h"
#include "../../config/config.h"
#if BUILD_MODE == E2E
#include "ssd1306.h"
#include "ssd1306_i2c.h"
#include "ssd1306_font.h"
#endif

#define DISPLAY_REFRESH_MS 16

#if BUILD_MODE == E2E
static void drawFrame(GameState *statePtr, uint8_t *frameBuffer, char *textBuffer) {

    ssd1306Clear(frameBuffer);
    
    // Row 0
    WriteString(frameBuffer, 0, 0, "HP");
    snprintf(textBuffer, TEL_BUFFER, "%d", statePtr->health);
    WriteString(frameBuffer, 32, 0, textBuffer);

    WriteString(frameBuffer, 64, 0, "AR");
    snprintf(textBuffer, TEL_BUFFER, "%d", statePtr->armor);
    WriteString(frameBuffer, 96, 0, textBuffer);

    // Row 1
    WriteString(frameBuffer, 0, 8, "MAG");
    snprintf(textBuffer, TEL_BUFFER, "%d", statePtr->magAmmo);
    WriteString(frameBuffer, 32, 8, textBuffer);

    WriteString(frameBuffer, 64, 8, "RES");
    snprintf(textBuffer, TEL_BUFFER, "%d", statePtr->reserveAmmo);
    WriteString(frameBuffer, 96, 8, textBuffer);

    // Row 2
    WriteString(frameBuffer, 0, 16, "K");
    snprintf(textBuffer, TEL_BUFFER, "%d", statePtr->killCount);
    WriteString(frameBuffer, 32, 16, textBuffer);

    WriteString(frameBuffer, 64, 16, "D");
    snprintf(textBuffer, TEL_BUFFER, "%d", statePtr->deathCount);
    WriteString(frameBuffer, 96, 16, textBuffer);
    
    // Renders the frame
    ssd1306RenderFull(frameBuffer);
}
#endif

int main() {
    stdio_init_all();
    sleep_ms(2000);
    // Initialize the hardware pins and the SSD1306 screen
    #if BUILD_MODE == E2E
    ssd1306Setup();
    SSD1306_init();
    
    // Frame buffer
    uint8_t buf[SSD1306_BUF_LEN];
    char textBuffer[TEL_BUFFER];
    #endif

    GameState state;

    while (true) {
        char lineRead[TEL_BUFFER];
        if (!serLineRead(lineRead, TEL_BUFFER)) {
            continue;
        }

        if (!serDataHandler(&state, lineRead)) {
            continue;
        }

        // DataHandler successfully populated the struct, we want to always echo back for testing purposes
        #if BUILD_MODE == PICO_ECHO
        serLineWrite(&state);
        #endif
        #if BUILD_MODE == E2E
        drawFrame(&state, buf, textBuffer);
        #endif

        sleep_ms(DISPLAY_REFRESH_MS);
    }
}
