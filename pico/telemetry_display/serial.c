#include "pico/stdio.h"
#include "pico/error.h"
#include <stdio.h>
#include "serial.h"

/* \brief Return a character from stdin if there is one available within a timeout
 * \ingroup pico_stdio
 *
 * \param timeout_us the timeout in microseconds, or 0 to not wait for a character if none available.
 * \return the character from 0-255 or PICO_ERROR_TIMEOUT if timeout occurs
int stdio_getchar_timeout_us(uint32_t timeout_us);
 */
int serLineRead(char *buffer, int maxLength) {
    int i = 0; // Index
    int c;
    while (i < (maxLength - 1)) { // Need to save room for null termination
        c = getchar_timeout_us(100);
        if (c == PICO_ERROR_TIMEOUT) {
            return 0;
        }
        if (c == '\n') { // End of line, fill buffer
            buffer[i] = '\0';
            return 1;
        }
        // Fill buffer like normal
        buffer[i] = c;
        i++;
    }
    return 0;
}

/*
https://learn.microsoft.com/pt-br/cpp/c-runtime-library/reference/sscanf-sscanf-l-swscanf-swscanf-l?view=msvc-170
https://www.tutorialspoint.com/c_standard_library/c_function_sscanf.htm
*/
int serDataHandler(GameState *statePtr, const char *buffer) {
    int parsedData = sscanf(buffer, "%d;%d;%d;%d", &statePtr->health, &statePtr->armor, &statePtr->magAmmo, &statePtr->reserveAmmo);
    // Check if sscanf actually parsed the correct number of data elements
    if (parsedData != 4) {
        return 0;
    }
    return 1;
}

// We can just use a printf as it prints to the console which will be transmitted via USB to the COM port
void serLineWrite(GameState *statePtr) {
    printf("Health:%d\nArmor:%d\nAmmo in mag:%d\nReserve ammo:%d\n", statePtr->health, statePtr->armor, statePtr->magAmmo, statePtr->reserveAmmo);
}
